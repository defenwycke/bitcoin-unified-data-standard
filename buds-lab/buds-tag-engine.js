// buds-tag-engine.js
// BUDS Tag Engine (browser version)
// Mirrors the C++ Tagger + PolicyExample at a high level.

class BudsTagEngine {
  constructor(policyProfile = "neutral") {
    this.largeBlobThreshold = 512; // bytes
    this.policyProfile = policyProfile;
  }

  setPolicyProfile(profile) {
    this.policyProfile = profile || "neutral";
  }

  // --- small helpers ---

  hexByteLen(hex) {
    return Math.floor(((hex || "").length) / 2);
  }

  hexStartsWith(hex, prefix) {
    return (hex || "").toLowerCase().startsWith(prefix.toLowerCase());
  }

  hexEndsWith(hex, suffix) {
    hex = (hex || "").toLowerCase();
    suffix = suffix.toLowerCase();
    if (hex.length < suffix.length) return false;
    return hex.slice(-suffix.length) === suffix;
  }

  // --- script recognition ---

  isLikelyOpReturn(spk) {
    if (spk.asm && spk.asm.startsWith("OP_RETURN")) return true;
    if (this.hexStartsWith(spk.hex, "6a")) return true;
    return false;
  }

  isLikelyP2PKH(spk) {
    const hex = spk.hex || "";
    if (hex.length !== 50) return false; // 25 bytes
    if (!this.hexStartsWith(hex, "76a914")) return false;
    if (!this.hexEndsWith(hex, "88ac")) return false;
    return true;
  }

  // --- main classification ---

  classify(tx) {
    const tags = [];

    // scriptPubKey classification
    (tx.vout || []).forEach((out, idx) => {
      const spk = out.scriptPubKey || { asm: "", hex: "" };
      const surface = `scriptpubkey[${idx}]`;
      const byteLen = this.hexByteLen(spk.hex);
      const labels = [];

      if (this.isLikelyOpReturn(spk)) {
        labels.push("da.op_return_embed");
      } else if (this.isLikelyP2PKH(spk)) {
        labels.push("pay.standard");
      } else {
        // Future: detect P2WPKH, P2TR, multisig, etc.
        labels.push("pay.standard");
      }

      tags.push({
        surface,
        start: 0,
        end: byteLen,
        labels
      });
    });

    // witness classification
    (tx.witness || []).forEach((wit, vinIdx) => {
      (wit.stack || []).forEach((itemHex, stackIdx) => {
        const byteLen = this.hexByteLen(itemHex);
        const surface = `witness.stack[${vinIdx}:${stackIdx}]`;
        const labels = [];
        if (byteLen > this.largeBlobThreshold) {
          labels.push("da.obfuscated");
        } else {
          labels.push("da.unknown");
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

  // --- triage tiers T0–T3 ---

  /**
   * Map a label to conceptual tiers:
   *  T0 – consensus / validation-critical
   *  T1 – economic / Bitcoin-critical (payments, commitments, contracts)
   *  T2 – metadata / hints / optional but often useful
   *  T3 – unknown / obfuscated / likely spam
   */
  getTierForLabel(label) {
    if (!label) return "T3";

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
      label === "da.op_return_embed"
    ) {
      return "T2";
    }

    // anything unknown / obfuscated / vendor / da.* default to T3
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

  // --- ARBDA – transaction-level tier (guilty until proven innocent) ---

  /**
   * Compute ARBDA tx tier from per-tier counts:
   *
   * if has T3 -> T3
   * else if has T2 -> T2
   * else if has T1 -> T1
   * else -> T0
   */
  computeArbdaTierFromCounts(counts) {
    if (!counts) return "T0";
    if ((counts.T3 || 0) > 0) return "T3";
    if ((counts.T2 || 0) > 0) return "T2";
    if ((counts.T1 || 0) > 0) return "T1";
    return "T0";
  }

  // --- policy model ---

  getPolicyTableForProfile() {
    // You can tune these to simulate different node policies.
    // neutral  – demo defaults (mild penalties)
    // strict   – heavier penalties for T3 data
    // permissive – almost no penalties
    const profile = this.policyProfile || "neutral";

    if (profile === "strict") {
      return {
        "da.obfuscated": { minMult: 4.0, boost: -0.7 },
        "da.unknown": { minMult: 3.0, boost: -0.4 },
        "da.op_return_embed": { minMult: 2.0, boost: -0.2 },
        "pay.standard": { minMult: 1.0, boost: 0.0 },
        "pay.channel_open": { minMult: 1.0, boost: 0.2 }
      };
    }

    if (profile === "permissive") {
      return {
        "da.obfuscated": { minMult: 2.0, boost: -0.3 },
        "da.unknown": { minMult: 1.5, boost: -0.1 },
        "da.op_return_embed": { minMult: 1.2, boost: -0.05 },
        "pay.standard": { minMult: 1.0, boost: 0.0 },
        "pay.channel_open": { minMult: 1.0, boost: 0.1 }
      };
    }

    // neutral (default)
    return {
      "da.obfuscated": { minMult: 3.0, boost: -0.5 },
      "da.unknown": { minMult: 2.0, boost: -0.2 },
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
