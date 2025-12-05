#include "buds_tagger.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace buds {

TagEngine::TagEngine(PolicyProfile profile) : profile_(profile) {
    buildRegistryV2();
}

void TagEngine::setPolicyProfile(PolicyProfile profile) {
    profile_ = profile;
}

// ---------- tiny helpers ----------

std::size_t TagEngine::hexByteLen(const std::string& hex) {
    return hex.size() / 2;
}

bool TagEngine::hexStartsWith(const std::string& hex, const std::string& prefix) {
    if (hex.size() < prefix.size()) return false;
    for (std::size_t i = 0; i < prefix.size(); ++i) {
        char a = std::tolower(static_cast<unsigned char>(hex[i]));
        char b = std::tolower(static_cast<unsigned char>(prefix[i]));
        if (a != b) return false;
    }
    return true;
}

bool TagEngine::hexEndsWith(const std::string& hex, const std::string& suffix) {
    if (hex.size() < suffix.size()) return false;
    std::size_t offset = hex.size() - suffix.size();
    for (std::size_t i = 0; i < suffix.size(); ++i) {
        char a = std::tolower(static_cast<unsigned char>(hex[offset + i]));
        char b = std::tolower(static_cast<unsigned char>(suffix[i]));
        if (a != b) return false;
    }
    return true;
}

bool TagEngine::isMostlyAscii(const std::string& hex) {
    if (hex.empty()) return false;
    std::size_t printable = 0;
    std::size_t total = 0;

    for (std::size_t i = 0; i + 1 < hex.size(); i += 2) {
        unsigned int byte = 0;
        try {
            byte = static_cast<unsigned int>(std::stoul(hex.substr(i, 2), nullptr, 16));
        } catch (...) {
            continue;
        }
        ++total;
        if (byte >= 0x20 && byte <= 0x7e) {
            ++printable;
        }
    }
    if (total == 0) return false;
    return (static_cast<double>(printable) / static_cast<double>(total)) >= 0.8;
}

std::string TagEngine::getOpReturnPayloadHex(const std::string& scriptHex) {
    std::string h = scriptHex;
    std::transform(h.begin(), h.end(), h.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (h.size() < 4) return "";
    if (!hexStartsWith(h, "6a")) return h;
    // crude but fine for demo: skip OP_RETURN (0x6a) + one push-length byte
    return h.substr(4);
}

bool TagEngine::isLikelyOpReturn(const ScriptPubKey& spk) {
    if (!spk.asm_repr.empty()) {
        if (spk.asm_repr.rfind("OP_RETURN", 0) == 0) return true;
    }
    if (hexStartsWith(spk.hex, "6a")) return true;
    return false;
}

bool TagEngine::isLikelyP2PKH(const ScriptPubKey& spk) {
    const std::string& hex = spk.hex;
    if (hex.size() != 50) return false; // 25 bytes
    if (!hexStartsWith(hex, "76a914")) return false;
    if (!hexEndsWith(hex, "88ac")) return false;
    return true;
}

bool TagEngine::isLikelyP2WPKH(const ScriptPubKey& spk) {
    const std::string& hex = spk.hex;
    // 0 <20-byte-pubkeyhash> => 22 bytes => 44 hex chars
    return hex.size() == 44 && hexStartsWith(hex, "0014");
}

bool TagEngine::isLikelyP2TR(const ScriptPubKey& spk) {
    const std::string& hex = spk.hex;
    // 1 <32-byte-xonly-pubkey> => 34 bytes => 68 hex chars
    return hex.size() == 68 && hexStartsWith(hex, "5120");
}

bool TagEngine::isLikelyRollupRootOpReturn(const ScriptPubKey& spk) {
    const std::string& hex = spk.hex;
    if (!hexStartsWith(hex, "6a20")) return false;
    std::size_t byteLen = hexByteLen(hex);
    // 34-byte script is ideal (OP_RETURN + len + 32); allow a small band
    return byteLen >= 32 && byteLen <= 40;
}

bool TagEngine::witnessLooksLikeOrdinal(const std::string& hex) {
    if (hex.empty()) return false;
    std::string h = hex;
    std::transform(h.begin(), h.end(), h.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return h.find("6f7264") != std::string::npos; // "ord"
}

// ---------- registry ----------

void TagEngine::buildRegistryV2() {
    registry_.clear();
    // T0
    registry_["consensus.sig"] = "T0";
    registry_["consensus.script"] = "T0";
    registry_["consensus.taproot_prog"] = "T0";

    // T1
    registry_["pay.standard"] = "T1";
    registry_["pay.channel_open"] = "T1";
    registry_["pay.channel_update"] = "T1";
    registry_["contracts.vault"] = "T1";
    registry_["commitment.rollup_root"] = "T1";
    registry_["meta.pool_tag"] = "T1";

    // T2
    registry_["da.op_return_embed"] = "T2";
    registry_["meta.inscription"] = "T2";
    registry_["meta.ordinal"] = "T2";
    registry_["meta.indexer_hint"] = "T2";
    registry_["da.embed_misc"] = "T2";

    // T3
    registry_["da.unknown"] = "T3";
    registry_["da.obfuscated"] = "T3";
    registry_["da.unregistered_vendor"] = "T3";
}

// ---------- core classification ----------

Classification TagEngine::classify(const Tx& tx) const {
    Classification c;
    c.txid = tx.txid.empty() ? std::string("<no-txid>") : tx.txid;

    // --- scriptPubKey classification ---
    for (std::size_t idx = 0; idx < tx.vout.size(); ++idx) {
        const auto& out = tx.vout[idx];
        const ScriptPubKey& spk = out.spk;
        std::string surface = "scriptpubkey[" + std::to_string(idx) + "]";
        std::size_t scriptLenBytes = hexByteLen(spk.hex);
        std::vector<std::string> labels;

        if (isLikelyOpReturn(spk)) {
            if (isLikelyRollupRootOpReturn(spk)) {
                labels.push_back("commitment.rollup_root");
            } else {
                std::string payloadHex = getOpReturnPayloadHex(spk.hex);
                std::size_t payloadLen = hexByteLen(payloadHex);
                bool asciiLike = isMostlyAscii(payloadHex);

                if (payloadLen <= 8 && asciiLike) {
                    labels.push_back("meta.indexer_hint");
                } else if (payloadLen <= 32 && asciiLike) {
                    labels.push_back("da.op_return_embed");
                } else if (payloadLen <= 80) {
                    labels.push_back("da.op_return_embed");
                } else {
                    labels.push_back("da.embed_misc");
                }
            }
        } else if (isLikelyP2PKH(spk) || isLikelyP2WPKH(spk) || isLikelyP2TR(spk)) {
            labels.push_back("pay.standard");
        } else {
            // Fallback: treat unknown spk as economic lane
            labels.push_back("pay.standard");
        }

        Tag t;
        t.surface = surface;
        t.start = 0;
        t.end = scriptLenBytes;
        t.labels = std::move(labels);
        c.tags.push_back(std::move(t));
    }

    // --- witness classification ---
    constexpr std::size_t largeBlobThreshold = 512;

    for (std::size_t vinIdx = 0; vinIdx < tx.witness.size(); ++vinIdx) {
        const auto& wit = tx.witness[vinIdx];
        for (std::size_t stackIdx = 0; stackIdx < wit.stack.size(); ++stackIdx) {
            const auto& item = wit.stack[stackIdx];
            std::string surface = "witness.stack[" + std::to_string(vinIdx) + ":" +
                                  std::to_string(stackIdx) + "]";
            std::size_t byteLen = hexByteLen(item.hex);
            std::vector<std::string> labels;

            if (witnessLooksLikeOrdinal(item.hex)) {
                if (byteLen > 256) {
                    labels.push_back("meta.inscription");
                } else {
                    labels.push_back("meta.ordinal");
                }
            } else {
                bool asciiLike = isMostlyAscii(item.hex);
                if (byteLen <= 128 && asciiLike) {
                    labels.push_back("da.unregistered_vendor");
                } else if (byteLen > largeBlobThreshold) {
                    labels.push_back("da.obfuscated");
                } else {
                    labels.push_back("da.unknown");
                }
            }

            Tag t;
            t.surface = surface;
            t.start = 0;
            t.end = byteLen;
            t.labels = std::move(labels);
            c.tags.push_back(std::move(t));
        }
    }

    return c;
}

// ---------- tiers / summary ----------

std::string TagEngine::getTierForLabel(const std::string& label) const {
    if (label.empty()) return "T3";

    auto it = registry_.find(label);
    if (it != registry_.end()) {
        return it->second;
    }

    // Fallback prefix rules for unknown labels
    if (label.rfind("consensus.", 0) == 0) return "T0";

    if (label.rfind("pay.", 0) == 0 ||
        label.rfind("commitment.", 0) == 0 ||
        label.rfind("contracts.", 0) == 0) {
        return "T1";
    }

    if (label.rfind("meta.", 0) == 0 ||
        label == "da.op_return_embed" ||
        label == "da.embed_misc") {
        return "T2";
    }

    return "T3";
}

Summary TagEngine::summarizeTiers(const Classification& c) const {
    Summary s;
    TierCounts counts;

    auto addTier = [&](const std::string& tier) {
        if (tier == "T0") ++counts.T0;
        else if (tier == "T1") ++counts.T1;
        else if (tier == "T2") ++counts.T2;
        else ++counts.T3;
    };

    std::vector<std::string> present;

    for (const auto& tag : c.tags) {
        for (const auto& label : tag.labels) {
            std::string tier = getTierForLabel(label);
            addTier(tier);
            if (std::find(present.begin(), present.end(), tier) == present.end()) {
                present.push_back(tier);
            }
        }
    }

    // sort tiers in order T0,T1,T2,T3 if present
    auto tierRank = [](const std::string& tier) {
        if (tier == "T0") return 0;
        if (tier == "T1") return 1;
        if (tier == "T2") return 2;
        return 3;
    };
    std::sort(present.begin(), present.end(),
              [&](const std::string& a, const std::string& b) {
                  return tierRank(a) < tierRank(b);
              });

    s.tiersPresent = std::move(present);
    s.counts = counts;
    return s;
}

std::string TagEngine::computeArbdaTierFromCounts(const TierCounts& counts) const {
    if (counts.T3 > 0) return "T3";
    if (counts.T2 > 0) return "T2";
    if (counts.T1 > 0) return "T1";
    return "T0";
}

// ---------- policy ----------

std::unordered_map<std::string, TagEngine::PolicyEntry>
TagEngine::getPolicyTableForProfile() const {
    std::unordered_map<std::string, PolicyEntry> table;

    switch (profile_) {
    case PolicyProfile::Strict:
        table["da.obfuscated"]       = {4.0,  -0.7};
        table["da.unknown"]          = {3.0,  -0.4};
        table["da.unregistered_vendor"] = {2.5, -0.3};
        table["da.op_return_embed"]  = {2.0,  -0.2};
        table["pay.standard"]        = {1.0,   0.0};
        table["pay.channel_open"]    = {1.0,   0.2};
        break;

    case PolicyProfile::Permissive:
        table["da.obfuscated"]       = {2.0,  -0.3};
        table["da.unknown"]          = {1.5,  -0.1};
        table["da.unregistered_vendor"] = {1.3, -0.05};
        table["da.op_return_embed"]  = {1.2,  -0.05};
        table["pay.standard"]        = {1.0,   0.0};
        table["pay.channel_open"]    = {1.0,   0.1};
        break;

    case PolicyProfile::Neutral:
    default:
        table["da.obfuscated"]       = {3.0,  -0.5};
        table["da.unknown"]          = {2.0,  -0.2};
        table["da.unregistered_vendor"] = {1.7, -0.15};
        table["da.op_return_embed"]  = {1.5,  -0.1};
        table["pay.standard"]        = {1.0,   0.0};
        table["pay.channel_open"]    = {1.0,   0.2};
        break;
    }

    return table;
}

TagEngine::PolicyEntry TagEngine::getPolicyForLabel(const std::string& label) const {
    auto table = getPolicyTableForProfile();
    auto it = table.find(label);
    if (it != table.end()) {
        return it->second;
    }
    return PolicyEntry{1.0, 0.0};
}

PolicyResult TagEngine::computePolicy(const Classification& c,
                                      double baseMinFeerate,
                                      double txFeerate) const {
    double mult = 1.0;
    double boostSum = 0.0;
    std::vector<std::string> seen;

    auto hasSeen = [&](const std::string& label) {
        return std::find(seen.begin(), seen.end(), label) != seen.end();
    };

    for (const auto& tag : c.tags) {
        for (const auto& label : tag.labels) {
            PolicyEntry p = getPolicyForLabel(label);
            if (p.minMult > mult) mult = p.minMult;
            if (!hasSeen(label)) {
                seen.push_back(label);
                boostSum += p.boost;
            }
        }
    }

    if (boostSum < -0.9) boostSum = -0.9;
    if (boostSum > 1.0)  boostSum = 1.0;

    PolicyResult r;
    r.mult = mult;
    r.boostSum = boostSum;
    r.required = baseMinFeerate * mult;
    r.score = txFeerate * (1.0 + boostSum);
    return r;
}

} // namespace buds
