# BUDS Specification (Version 2.0 — Draft)

The Bitcoin Unified Data Standard (BUDS) defines a minimal, optional framework for
**describing**, **classifying**, and **tagging** data that appears inside Bitcoin transactions.

BUDS does **not** modify Bitcoin consensus or enforce any node policy.  
It provides a shared vocabulary for wallets, nodes, indexers, and off-chain systems.

The canonical v2 registry:

- `registry/registry-v2.json`

Earlier v1 registry files remain valid for legacy tooling, but new systems SHOULD
prefer v2.

---

## 1. Definitions

### **1.1 Label**

A **label** is a canonical name describing a type of data that can appear in a transaction.

Examples:  
- `pay.standard`  
- `meta.inscription`  
- `commitment.rollup_root`  
- `da.obfuscated`

Labels describe **intent**, not guaranteed truth.

---

### **1.2 Region**

A **region** is a slice of bytes within a transaction.  
Regions live on a specific **surface**, such as:

- `scriptsig`
- `witness.stack[n]`
- `witness.script`
- `scriptpubkey`
- `op_return`
- `coinbase`
- `tx-level` fields like version/locktime/sequence

---

### **1.3 Tag**

A **tag** associates:

- a region  
- with zero or more labels.

Tags are the output of the classification process.

---

### **1.4 Suggested Tiers (T0–T3)**

Tiers are **conceptual**, used for visualisation and optional policy:

- **T0 — Consensus / Validation**  
  (signatures, validation script, taproot programs)

- **T1 — Economic / System**  
  (channels, rollup anchors, vault structures, pool tags)

- **T2 — Metadata / Application**  
  (inscriptions, ordinal metadata, OP_RETURN embeds)

- **T3 — Unknown / Bulk**  
  (opaque pushes, large witness blobs, unclassified vendor data)

Each label in the registry includes a `suggested_tier`.  
These are **not mandatory** and may be overridden.

---

## 2. Registry

The registry is the canonical list of known labels.

- v2 registry location:  
  `registry/registry-v2.json`

Each entry includes:

- `label`
- `description`
- `surfaces`
- `suggested_tier`
- optional metadata fields

Nodes may ignore or override registry hints.

The registration process is defined in:

- `docs/registry-process.md`

---

## 3. Classification

Classification is the process of identifying:

- the **regions** of interest in a transaction
- the **labels** that apply to each region

BUDS does not mandate a specific algorithm.  
Implementations may use:

- pattern matching  
- structure decoding  
- heuristics  
- vendor-specific logic  

A minimal approach is documented in:

- `docs/tagging-method.md`

Classification SHOULD produce:

- a list of regions
- zero or more labels per region

Implementations MAY assign multiple labels to a single region.

---

## 4. Tagging Interface

A tag is defined as:

```
{
  "surface": "witness.stack[1]",
  "start": 0,
  "end": 123,
  "labels": ["da.obfuscated"]
}
```

Implementations MAY:

- provide tags to policy modules
- expose tags through RPC APIs
- log them for analytics
- or ignore tagging entirely

Tags have **zero effect on consensus validation**.

---

## 5. Policy Integration (Optional)

Node and miner policies MAY use tags or tiers for local decisions, such as:

- mempool admission  
- dynamic feerate requirements  
- block template selection  
- pruning/retention strategies  

BUDS provides **no** policy rules.  
All policy is implementation defined.

Examples of optional policy usage are described in:

- `docs/policy-interface.md`
- `docs/arbda.md`

ARBDA provides a transaction-level “worst-case tier” summary.  
A common rule (non-normative):

> If any region is T3 → the transaction is ARBDA T3.

---

## 6. Surfaces & Encodings

BUDS is **encoding-agnostic**.  
It applies to any data structure that appears on a surface.

Surfaces include:

- `scriptsig`
- `scriptpubkey`
- `witness.stack[n]`
- `witness.script`
- `op_return`
- `coinbase`
- transaction-level metadata

Data may be encoded as:

- raw bytes  
- JSON  
- TLV  
- CBOR  
- or any arbitrary vendor-defined structure  

BUDS does **not** require or prefer any encoding.

---

## 7. Security & Privacy Considerations

- Misclassification does not affect consensus.
- Nodes may disagree on classification.
- Implementations should avoid leaking sensitive metadata.
- Do not assume correctness of user-provided data.
- Optional policies must not imply censorship without operator intent.

---

## 8. Compatibility

- Fully compatible with all Bitcoin transactions.  
- Requires no consensus changes.  
- v1 registry files remain usable for legacy systems.  
- v2 refines naming, tiers, and registry structure.  
- Nodes may adopt BUDS partially or fully.  

---

## 9. Summary

BUDS provides:

- a standardised data‐label registry  
- a simple tagging model  
- optional tier hints  
- optional policy hooks  

Everything else remains fully under operator control.

