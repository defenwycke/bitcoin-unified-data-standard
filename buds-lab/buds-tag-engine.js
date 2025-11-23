// buds-tag-engine.js
// BUDS Tag Engine (browser version)
// Mirrors the C++ Tagger + PolicyExample at a high level.

class BudsTagEngine {
  constructor() {
    this.largeBlobThreshold = 512; // bytes
  }

  hexByteLen(hex) {
    return Math.floor((hex || "").length / 2);
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

  // TODO: add P2WPKH, P2TR, multisig detection later if you want

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
        labels.push("pay.standard"); // future: expand patterns
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

  // Example policy model (non-normative)
  getPolicyForLabel(label) {
    const table = {
      "da.obfuscated": { minMult: 3.0, boost: -0.5 },
      "da.unknown": { minMult: 2.0, boost: -0.2 },
      "pay.standard": { minMult: 1.0, boost: 0.0 },
      "pay.channel_open": { minMult: 1.0, boost: 0.2 }
    };
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
