# BUDS Classification Table

BUDS defines **labels** for different types of data that may appear inside Bitcoin transactions.  
Each label has an **intended meaning**, but nodes are free to group labels into **local priority classes** (T0–T3) according to their own policy.

This table provides **non-normative guidance** on how labels can be conceptually grouped.  
Implementations MAY override any mapping.

---

## Tier Overview

### **T0 — Consensus-Critical**
Data required for Bitcoin validation or enforcing spending conditions.

Examples: signatures, pubkeys, executed scripts, taproot programs.

Use case: identifies regions that must be preserved and executed by all nodes.

---

### **T1 — Economic / System-Critical**
Data that supports Bitcoin’s broader economic and security ecosystem.

Examples: Lightning channel opens, vault metadata, rollup roots, mining pool tags, covenant descriptors.

Use case: important system-level activity that many nodes may wish to prioritise.

---

### **T2 — Metadata / Application**
Optional data used by higher-level or user-defined applications.

Examples: inscriptions, ordinals, indexer hints, OP_RETURN application metadata.

Use case: not required for consensus or system operation; nodes may treat it neutrally or apply light policy.

---

### **T3 — Unknown / Obfuscated**
Opaque or non-standard data that does not match known structures or protocols.

Examples: large witness blobs, random data pushes, unclassified patterns.

Use case: allows nodes to apply optional protective policies against potential spam.

---

## Suggested Label → Tier Mapping (Non-Normative)

| Label                       | Suggested Tier | Notes |
|-----------------------------|----------------|-------|
| `consensus.sig`            | T0 | Signatures required for validation. |
| `consensus.script`         | T0 | Executed script regions. |
| `consensus.taproot_prog`   | T0 | Valid tapscript programs. |
| `pay.standard`             | T1 | Normal payments / transfers. |
| `pay.channel_open`         | T1 | Lightning / L2 channel establishment. |
| `contracts.vault`          | T1 | Recovery / safety mechanisms. |
| `commitment.rollup_root`   | T1 | Data that anchors L2 state to Bitcoin. |
| `meta.pool_tag`            | T1 | Mining pool identification. |
| `da.op_return_embed`       | T2 | Explicit metadata via OP_RETURN. |
| `meta.inscription`         | T2 | Known inscription-like formats. |
| `meta.ordinal`             | T2 | Ordinal / NFT-related data. |
| `meta.indexer_hint`        | T2 | Optional hints for external indexers. |
| `da.embed_misc`            | T2 | General-purpose embedded metadata. |
| `da.unknown`               | T3 | No matching structure. |
| `da.obfuscated`            | T3 | Large, opaque, or intentionally hidden data. |
| `da.unregistered_vendor`   | T3 | Structured but unregistered vendor formats. |

This mapping is for reference only.  
Nodes **MAY** treat any label differently.

---

## Notes for Implementers

- Labels describe **intent**, not authority.  
- Classification is **local**: each node decides how to interpret a region of a transaction.  
- Tiers are **guidance**, not rules.  
- Policies (feerates, limits, template preference, pruning) remain **fully optional**.

For tagging guidance, see `docs/tagging-method.md`.

