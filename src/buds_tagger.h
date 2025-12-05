#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace buds {

struct ScriptPubKey {
    std::string asm_repr;  // optional, may be empty
    std::string hex;       // lowercase/uppercase hex, no "0x"
};

struct TxOutput {
    ScriptPubKey spk;
};

struct WitnessItem {
    std::string hex;       // raw hex for this stack item
};

struct Witness {
    std::vector<WitnessItem> stack;
};

struct Tx {
    std::string txid;
    std::vector<TxOutput> vout;
    std::vector<Witness> witness;
};

struct Tag {
    std::string surface;   // e.g. "scriptpubkey[0]" or "witness.stack[0:1]"
    std::size_t start;     // byte offset (always 0 in this simple engine)
    std::size_t end;       // byte length
    std::vector<std::string> labels;
};

struct Classification {
    std::string txid;
    std::vector<Tag> tags;
};

struct TierCounts {
    int T0{0};
    int T1{0};
    int T2{0};
    int T3{0};
};

struct Summary {
    std::vector<std::string> tiersPresent;
    TierCounts counts;
};

enum class PolicyProfile {
    Neutral,
    Strict,
    Permissive
};

struct PolicyResult {
    double required{0.0};   // required feerate (sat/vB)
    double score{0.0};      // effective score (scaled sat/vB)
    double mult{1.0};       // minimum multiplier
    double boostSum{0.0};   // sum of boosts (clamped to [-0.9, 1.0])
};

class TagEngine {
public:
    explicit TagEngine(PolicyProfile profile = PolicyProfile::Neutral);

    void setPolicyProfile(PolicyProfile profile);

    // Core API
    Classification classify(const Tx& tx) const;
    Summary summarizeTiers(const Classification& c) const;
    std::string computeArbdaTierFromCounts(const TierCounts& counts) const;
    PolicyResult computePolicy(const Classification& c,
                               double baseMinFeerate,
                               double txFeerate) const;

    // Tier lookup (T0/T1/T2/T3)
    std::string getTierForLabel(const std::string& label) const;

    // Expose version string for bookkeeping
    std::string budsVersion() const { return "2.0"; }

private:
    PolicyProfile profile_;
    std::unordered_map<std::string, std::string> registry_; // label -> tier "T0".."T3"

    // --- helpers ---
    static std::size_t hexByteLen(const std::string& hex);
    static bool hexStartsWith(const std::string& hex, const std::string& prefix);
    static bool hexEndsWith(const std::string& hex, const std::string& suffix);
    static bool isMostlyAscii(const std::string& hex);
    static std::string getOpReturnPayloadHex(const std::string& scriptHex);
    static bool isLikelyOpReturn(const ScriptPubKey& spk);
    static bool isLikelyP2PKH(const ScriptPubKey& spk);
    static bool isLikelyP2WPKH(const ScriptPubKey& spk);
    static bool isLikelyP2TR(const ScriptPubKey& spk);
    static bool isLikelyRollupRootOpReturn(const ScriptPubKey& spk);
    static bool witnessLooksLikeOrdinal(const std::string& hex);

    // Policy helpers
    struct PolicyEntry {
        double minMult{1.0};
        double boost{0.0};
    };

    std::unordered_map<std::string, PolicyEntry> getPolicyTableForProfile() const;
    PolicyEntry getPolicyForLabel(const std::string& label) const;

    // Internal registry init
    void buildRegistryV2();
};

} // namespace buds
