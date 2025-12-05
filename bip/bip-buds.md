BIP: TBD  
Layer: Informational  
Title: Bitcoin Unified Data Standard (BUDS)  
Author: Defenwycke  
Comments-Summary: No comments yet  
Comments-URI: https://github.com/defenwycke/bitcoin-unified-data-standard  
Status: Draft  
Type: Informational  
License: BSD-2-Clause  
Created: 2025-XX-XX  
Post-History: TBD

---

# Abstract

This document defines the **Bitcoin Unified Data Standard (BUDS)** — a neutral,
non-consensus framework for describing data *inside* Bitcoin transactions.

BUDS introduces:

- a canonical **registry of labels**  
- a standard **tag format**  
- a conceptual **tier model (T0–T3)**  
- an optional **ARBDA score** (worst-tier dominance)  
- an optional **policy interface** for node operators  

BUDS does **not** modify Bitcoin consensus, and it does not impose any mandatory
policy. It provides a shared vocabulary for wallets, indexers, miners, explorers,
and L2 protocols to classify and discuss transaction contents consistently.

---

# Motivation

Bitcoin transaction contents vary widely:

- payment scripts  
- signatures and witness data  
- ordinal inscriptions  
- metadata  
- rollup commitments  
- channel operations  
- arbitrary large blobs  

Historically, Bitcoin lacked a unified model for describing these data types.
Each project invented its own terminology, leading to:

- fragile ad-hoc heuristics  
- inconsistent mempool policies  
- unclear “spam” debates  
- incompatible indexer behaviour  
- difficulty coordinating around new data formats  

BUDS fills this gap by defining a **standard descriptive language**.  
Nothing in BUDS affects transaction validity.

---

# Rationale and Scope

BUDS is intentionally:

- **non-consensus**  
- **lightweight**  
- **forward-compatible**  
- **advisory**  

BUDS does *not* define new opcodes, new transaction formats, or new validation
rules. All semantics pertain to:

- classification  
- analysis  
- optional fee/policy decisions  
- interoperability  

The design enables node operators and miners to apply local policies without
needing global agreement or forks.

---

# Specification

## 1. Labels

A **label** is a canonical descriptor of what a region of transaction data
represents.

Examples:

```
pay.standard  
commitment.rollup_root  
meta.indexer_hint  
meta.inscription  
da.op_return_embed  
da.unregistered_vendor  
da.obfuscated  
```

Labels appear in the registry (see below) and may be extended.

---

## 2. Surfaces

A **surface** is a logical location of data:

- `scriptpubkey[n]`
- `witness.stack[i]`
- `scriptSig`
- `witness.script`
- `locktime`, `version` (rarely)

Surfaces do not alter Bitcoin consensus; they are purely descriptive.

---

## 3. Regions

A **region** is a byte-range within a surface:

```
surface = "scriptpubkey[0]"
range   = [0,4)
```

Regions may carry one or more labels.

---

## 4. Tags

A **tag** associates a region with one or more labels:

```
{
  "surface": "witness.stack[0]",
  "start": 0,
  "end": 512,
  "labels": ["da.obfuscated"]
}
```

Tags allow structured analysis of a transaction’s contents.

---

## 5. Tiers (T0–T3)

BUDS defines a conceptual tier model:

- **T0 – Consensus**  
- **T1 – Economic/System**  
- **T2 – Metadata/Application**  
- **T3 – Unknown/Obfuscated**  

Tiers are descriptive, not normative.

---

## 6. ARBDA (Arbitrary Data Dominance Assessment)

ARBDA produces a **single transaction-level tier**, reflecting the **worst**
tier present.

Rules:

- If any region is **T3** → ARBDA = T3  
- Else if any **T2** → T2  
- Else if any **T1** → T1  
- Else → T0  

Nodes and miners may optionally use ARBDA in local mempool or fee policy.

---

## 7. Registry

The canonical registry is stored in:

```
registry/registry-v2.json
```

It includes:

- label  
- description  
- surfaces where it appears  
- suggested tier  
- extension process (vendor namespaces)

Registry changes are backwards-compatible unless otherwise noted.

---

## 8. Reference Implementations

### JavaScript Tag Engine  

`buds-lab/buds-tag-engine.js` implements:

- OP_RETURN heuristics (hints, embeds, rollup roots)  
- witness heuristics (ordinal/inscription markers, vendor metadata, blobs)  
- registry tier mapping  
- ARBDA  
- optional policy scoring  

### C++ Tagger

`src/buds_tagger.cpp` is a simple C++ demonstration of the same conceptual rules.

Neither implementation is consensus-critical.

---

## 9. Examples

### Payment

```
scriptpubkey: OP_DUP OP_HASH160 <pkh> OP_EQUALVERIFY OP_CHECKSIG
→ pay.standard (T1)
```

### OP_RETURN sub-types

```
6a04 6f7264    → "ord" → meta.ordinal (T2)
6a20 <32B>     → rollup root → commitment.rollup_root (T1)
6a 01 41       → ascii → meta.indexer_hint (T2)
```

### Witness blobs

- ASCII small → `da.unregistered_vendor` (T3)  
- random small → `da.unknown` (T3)  
- random large → `da.obfuscated` (T3)  
- inscription-like → `meta.inscription` (T2)

---

# Backwards Compatibility

BUDS is a **pure overlay** on existing Bitcoin transactions.  
It introduces:

- no new consensus rules  
- no changes to block format  
- no new script opcodes  
- no impact on transaction validity  

Nodes and miners may ignore BUDS entirely.

---

# Reference Implementation and Labs

A browser playground is located in:

```
buds-lab/
```

With documentation at:

```
docs/buds-lab.md
buds-lab/docs/buds-lab-test-matrix.md
```

---

# Acknowledgements

Thanks to individuals contributing ideas, feedback, or prior analysis in the
Bitcoin data-classification space.

---

# Copyright

This BIP is licensed under the **BSD-2-Clause license**, as required for BIPs.
