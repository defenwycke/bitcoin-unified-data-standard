// buds-tag-engine.js
// BUDS Tag Engine (browser version) – BUDS v2
// Tighter heuristics for label / type recognition.
//
// Public API used by the lab UI:
//   new BudsTagEngine(profile)
//   engine.classify(tx)
//   engine.summarizeTiers(classification)
//   engine.computeArbdaTierFromCounts(counts)
//   engine.computePolicy(classification, baseMinFeerate, txFeerate)
//   engine.getTierForLabel(label)

const BUDS_REGISTRY_V2 = {
  "consensus.sig": "T0",
  "consensus.script": "T0",
  "consensus.taproot_prog": "T0",

  "pay.standard": "T1",
  "pay.channel_open": "T1",
  "pay.channel_update": "T1",
  "contracts.vault": "T1",
  "commitment.rollup_root": "T1",
  "meta.pool_tag": "T1",

  "da.op_return_embed": "T2",
  "meta.inscription": "T2",
  "meta.ordinal": "T2",
  "meta.indexer_hint": "T2",
  "da.embed_misc": "T2",

  "da.unknown": "T3",
  "da.obfuscated": "T3",
  "da.unregistered_vendor": "T3"
};

class BudsTagEngine {
  constructor(policyProfile = "neutral") {
    this.policyProfile = policyProfile || "neutral";

    // When a blob exceeds this many bytes, treat it as "large".
    this.largeBlobThreshold = 512;

    // BUDS spec version for display / bookkeeping.
    this.budsVersion = "2.0";
  }

  setPolicyProfile(profile) {
    this.policyProfile = profile || "neutral";
  }

  // ---------- tiny helpers ----------

  hexByteLen(hex) {
    return Math.floor(((hex || "").length) / 2);
  }

  hexStartsWith(hex, prefix) {
    return (hex || "").toLowerCase().startsWith((prefix || "").toLowerCase());
  }

  hexEndsWith(hex, suffix) {
    hex = (hex || "").toLowerCase();
    suffix = (suffix || "").toLowerCase();
    if (hex.length < suffix.length) return false;
    return hex.slice(-suffix.length) === suffix;
  }

  // Count how many bytes are printable ASCII.
  isMostlyAscii(hex) {
    if (!hex) return false;
    let printable = 0;
    let total = 0;
    for (let i = 0; i + 2 <= hex.length; i += 2) {
      const byte = parseInt(hex.substr(i, 2), 16);
      if (!Number.isFinite(byte)) continue;
      total++;
      if (byte >= 0x20 && byte <= 0x7e) printable++;
    }
    if (total === 0) return false;
    return (printable / total) >= 0.8;
  }

  // For simple OP_RETURN scripts, skip the opcode + length and return the data.
  getOpReturnPayloadHex(scriptHex) {
    const h = (scriptHex || "").toLowerCase();
    if (!h.startsWith("6a")) return h;
    if (h.length <= 4) return "";
    // crude but OK for demo: skip OP_RETURN (0x6a) + one push-length byte
    return h.slice(4);
  }

  // ---------- script recognition ----------

  isLikelyOpReturn(spk) {
    if (!spk) return false;
    if (spk.asm && spk.asm.startsWith("OP_RETURN")) return true;
    if (this.hexStartsWith(spk.hex, "6a")) return true;
    return false;
  }

  isLikelyP2PKH(spk) {
    if (!spk) return false;
    const hex = spk.hex || "";
    if (hex.length !== 50) return false; // 25 bytes
    if (!this.hexStartsWith(hex, "76a914")) return false;
    if (!this.hexEndsWith(hex, "88ac")) return false;
    return true;
  }

  isLikelyP2WPKH(spk) {
    if (!spk) return false;
    const hex = spk.hex || "";
    // 0 <20-byte-pubkeyhash>  => 22 bytes => 44 hex chars
    return hex.length === 44 && this.hexStartsWith(hex, "0014");
  }

  isLikelyP2TR(spk) {
    if (!spk) return false;
    const hex = spk.hex || "";
    // 1 <32-byte-xonly-pubkey> => 34 bytes => 68 hex chars
    return hex.length === 68 && this.hexStartsWith(hex, "5120");
  }

  // OP_RETURN that looks like a 32-byte rollup root.
  isLikelyRollupRootOpReturn(spk) {
    if (!spk || !spk.hex) return false;
    const hex = (spk.hex || "").toLowerCase();
    if (!hex.startsWith("6a20")) return false;

    const byteLen = this.hexByteLen(hex);
    // 34-byte script is ideal, but allow a small band.
    return byteLen >= 32 && byteLen <= 40;
  }

  // ---------- witness helpers ----------

  // Very rough ordinal/inscription detector: look for "ord" (0x6f7264).
  witnessLooksLikeOrdinal(hex) {
    if (!hex) return false;
    return hex.toLowerCase().includes("6f7264");
  }

  // ---------- main classification ----------

  classify(tx) {
    const tags = [];

    // --- scriptPubKey classification ---
    (tx.vout || []).forEach((out, idx) => {
      const spk = out.scriptPubKey || { asm: "", hex: "" };
      const surface = `scriptpubkey[${idx}]`;
      const scriptLenBytes = this.hexByteLen(spk.hex);
      const labels = [];

      if (this.isLikelyOpReturn(spk)) {
        // OP_RETURN lane with more nets.
        if (this.isLikelyRollupRootOpReturn(spk)) {
          labels.push("commitment.rollup_root");
        } else {
          const payloadHex = this.getOpReturnPayloadHex(spk.hex);
          const payloadLen = this.hexByteLen(payloadHex);
          const isAscii = this.isMostlyAscii(payloadHex);

          // Very small ASCII payloads look like flags / hints.
          if (payloadLen <= 8 && isAscii) {
            labels.push("meta.indexer_hint");
          }
          // Short/medium ASCII text => human-readable metadata.
          else if (payloadLen <= 32 && isAscii) {
            labels.push("da.op_return_embed");
          }
          // Non-ASCII or bigger up to ~80 bytes => structured embed.
          else if (payloadLen <= 80) {
            labels.push("da.op_return_embed");
          }
          // Everything larger => misc bulk metadata.
          else {
            labels.push("da.embed_misc");
          }
        }
      } else if (
        this.isLikelyP2PKH(spk) ||
        this.isLikelyP2WPKH(spk) ||
        this.isLikelyP2TR(spk)
      ) {
        labels.push("pay.standard");
      } else {
        // Unknown scriptpubkey: still treated as economic payment lane for now.
        labels.push("pay.standard");
      }

      tags.push({
        surface,
        start: 0,
        end: scriptLenBytes,
        labels
      });
    });

    // --- witness classification ---
    (tx.witness || []).forEach((wit, vinIdx) => {
      (wit.stack || []).forEach((itemHex, stackIdx) => {
        const byteLen = this.hexByteLen(itemHex);
        const surface = `witness.stack[${vinIdx}:${stackIdx}]`;
        const labels = [];

        if (this.witnessLooksLikeOrdinal(itemHex)) {
          // Ordinal-like: split into small vs big inscriptions.
          if (byteLen > 256) {
            labels.push("meta.inscription");
          } else {
            labels.push("meta.ordinal");
          }
        } else {
          const asciiLike = this.isMostlyAscii(itemHex);

          // Very large blobs are always treated as obfuscated bulk data.
          if (byteLen >= 512) {
            labels.push("da.obfuscated");
          }
          // Small/medium mostly-ASCII witness that isn't ord:
          // looks like vendor protocol state.
          else if (asciiLike && byteLen >= 16) {
            labels.push("da.unregistered_vendor");
          }
          // Medium-large non-ASCII blobs (e.g. 256–511 bytes) are also
          // considered obfuscated, even if they don't cross 512 bytes.
          else if (!asciiLike && byteLen >= 256) {
            labels.push("da.obfuscated");
          }
          // Everything else is unknown bulk data.
          else {
            labels.push("da.unknown");
          }
        }

        tags.push({
          surface,
          start: 0,
          end: byteLen,
          labels
        });
      });
    });

    return { txid: tx.txid || "<no-txid>", tags };
  }

  // ---------- tiers (T0–T3) ----------

  getTierForLabel(label) {
    if (!label) return "T3";

    // 1) registry mapping
    const reg = BUDS_REGISTRY_V2[label];
    if (reg) return reg;

    // 2) fallback prefix rules – for experimental / unregistered labels
    if (label.startsWith("consensus.")) return "T0";

    if (
      label.startsWith("pay.") ||
      label.startsWith("commitment.") ||
      label.startsWith("contracts.")
    ) {
      return "T1";
    }

    if (
      label.startsWith("meta.") ||
      label === "da.op_return_embed" ||
      label === "da.embed_misc"
    ) {
      return "T2";
    }

    return "T3";
  }

  summarizeTiers(classification) {
    const tiersPresent = new Set();
    const counts = { T0: 0, T1: 0, T2: 0, T3: 0 };

    (classification.tags || []).forEach(tag => {
      (tag.labels || []).forEach(label => {
        const tier = this.getTierForLabel(label);
        tiersPresent.add(tier);
        if (counts[tier] !== undefined) counts[tier] += 1;
      });
    });

    return {
      tiersPresent: Array.from(tiersPresent).sort(),
      counts
    };
  }

  // ---------- ARBDA tier ----------

  computeArbdaTierFromCounts(counts) {
    if (!counts) return "T0";
    if ((counts.T3 || 0) > 0) return "T3";
    if ((counts.T2 || 0) > 0) return "T2";
    if ((counts.T1 || 0) > 0) return "T1";
    return "T0";
  }

  // ---------- policy model ----------

  getPolicyTableForProfile() {
    const profile = this.policyProfile || "neutral";

    if (profile === "strict") {
      return {
        "da.obfuscated": { minMult: 4.0, boost: -0.7 },
        "da.unknown": { minMult: 3.0, boost: -0.4 },
        "da.unregistered_vendor": { minMult: 2.5, boost: -0.3 },
        "da.op_return_embed": { minMult: 2.0, boost: -0.2 },
        "pay.standard": { minMult: 1.0, boost: 0.0 },
        "pay.channel_open": { minMult: 1.0, boost: 0.2 }
      };
    }

    if (profile === "permissive") {
      return {
        "da.obfuscated": { minMult: 2.0, boost: -0.3 },
        "da.unknown": { minMult: 1.5, boost: -0.1 },
        "da.unregistered_vendor": { minMult: 1.3, boost: -0.05 },
        "da.op_return_embed": { minMult: 1.2, boost: -0.05 },
        "pay.standard": { minMult: 1.0, boost: 0.0 },
        "pay.channel_open": { minMult: 1.0, boost: 0.1 }
      };
    }

    // neutral
    return {
      "da.obfuscated": { minMult: 3.0, boost: -0.5 },
      "da.unknown": { minMult: 2.0, boost: -0.2 },
      "da.unregistered_vendor": { minMult: 1.7, boost: -0.15 },
      "da.op_return_embed": { minMult: 1.5, boost: -0.1 },
      "pay.standard": { minMult: 1.0, boost: 0.0 },
      "pay.channel_open": { minMult: 1.0, boost: 0.2 }
    };
  }

  getPolicyForLabel(label) {
    const table = this.getPolicyTableForProfile();
    return table[label] || { minMult: 1.0, boost: 0.0 };
  }

  computePolicy(classification, baseMinFeerate, txFeerate) {
    let mult = 1.0;
    const seen = new Set();
    let boostSum = 0.0;

    (classification.tags || []).forEach(tag => {
      (tag.labels || []).forEach(label => {
        const p = this.getPolicyForLabel(label);
        mult = Math.max(mult, p.minMult);
        if (!seen.has(label)) {
          seen.add(label);
          boostSum += p.boost;
        }
      });
    });

    boostSum = Math.max(-0.9, Math.min(boostSum, 1.0));

    const required = baseMinFeerate * mult;
    const score = txFeerate * (1 + boostSum);

    return { required, score, mult, boostSum };
  }
}
