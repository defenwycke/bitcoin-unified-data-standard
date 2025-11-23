BIP: TBD  
Title: Bitcoin Unified Data Standard (BUDS)  
Author: Defenwycke <defenwycke@icloud.com>  
Status: Draft  
Type: Standards Track  
Created: 2025-11-23  
License: BSD-2-Clause

## Abstract

This BIP specifies the Bitcoin Unified Data Standard (BUDS), a minimal and
optional framework for **describing**, **classifying**, and **tagging** the data
that appears inside Bitcoin transactions.

BUDS defines:

- a registry of data type labels,
- a common terminology for transaction regions (“surfaces”),
- a tagging interface for associating labels with byte ranges.

BUDS does **not** change consensus rules or prescribe node policy.  
It only provides a shared vocabulary that implementations may use for local
decisions (mempool, fee scoring, block templates, pruning).

---

## Motivation

Bitcoin transactions contain more than payments: signatures, scripts, channel
state, vaults, rollup roots, inscriptions, pool tags, and arbitrary application
data.

Today:

- Nodes see all of this as opaque bytes.  
- No standard exists to describe or classify data types.  
- Local policies become ad-hoc and brittle.  
- “Spam” debates lack technical clarity.

BUDS addresses this by defining:

- a **registry** of labels for common data types,
- a simple **classification model** (conceptual tiers),
- a **tagging interface** to attach labels to regions of a transaction.

This gives node operators and miners a clear, explicit way to understand
transaction data while keeping all enforcement fully local.

---

## Specification

This section summarises the BUDS standard.  
See `docs/specification.md` and `registry/registry.json` for full details.

### Labels

A **label** is a canonical string describing a type of data found inside a
transaction.

Examples:

(consensus)

```
consensus.sig  
consensus.script  
consensus.taproot_prog  
```

(pay and system)

```
pay.standard  
pay.channel_open  
contracts.vault  
commitment.rollup_root  
meta.pool_tag  
```

(metadata)

```
meta.inscription  
meta.ordinal  
meta.indexer_hint  
da.op_return_embed  
```

(unknown / generic)

```
da.unknown  
da.obfuscated  
da.unregistered_vendor  
```

The authoritative list is stored in `registry/registry.json`.

Labels describe **intended meaning**, not guaranteed truth.

---

### Regions and Surfaces

A **surface** is a logical location for transaction data:

- scriptsig  
- witness.stack[i]  
- witness.script  
- scriptpubkey[n]  
- op_return  
- coinbase  
- implementation-specific lanes (e.g., segop)

A **region** is a byte range within a surface.

---

### Tags

A **tag** associates a region with one or more labels.

```
Tag:
  surface: string
  start: integer
  end: integer
  labels: [string]
```

Tags are advisory metadata only.  
They have no effect on consensus validation and nodes may ignore them.

---

### Registry

Registry entries define:

```
{
  "label": "pay.channel_open",
  "description": "Lightning or L2 channel establishment.",
  "surfaces": ["scriptpubkey", "witness_script"],
  "suggested_category": "T1"
}
```

Fields:

- `label` (required)  
- `description`  
- `surfaces`  
- `suggested_category` (optional hint)

The registry process is defined in `docs/registry-process.md`.

---

### Conceptual Categories (T0–T3)

For guidance only, BUDS defines four conceptual tiers:

```
T0: consensus-critical  
T1: economic/system-critical  
T2: metadata/application  
T3: unknown/obfuscated  
```

These categories are **non-normative**:

- nodes may ignore them,
- nodes may override them,
- no behaviour is implied.

---

### Classification and Tagging

Classification is the local process where a node:

1. Parses a transaction into regions.  
2. Uses patterns, heuristics, or private logic to determine meaning.  
3. Assigns labels to regions.  
4. Produces tags.

BUDS **does not** define any required method.  
A sample approach is documented in `docs/tagging-method.md`.

Different nodes may classify the same transaction differently.

---

### Policy Integration (Optional)

Nodes and miners MAY use tags for:

- mempool admission  
- fee scoring  
- block template construction  
- soft limits  
- pruning and retention

This BIP defines **no** policy requirements or thresholds.  
See `docs/policy-interface.md` for optional examples.

---

## Rationale

Bitcoin already carries diverse data types. Without classification, nodes treat:

- a Lightning channel open,  
- an inscription,  
- a rollup root,  
- a pool tag,  
- and 200 KB of opaque witness data  

as equivalent “bytes”.

BUDS gives the ecosystem:

- a shared vocabulary,  
- a neutral classification model,  
- and a simple tagging interface.

Policy remains entirely local.

This preserves freedom for node operators, improves transparency, and provides a
technical foundation for spam mitigation without introducing new consensus rules.

---

## Backwards Compatibility

BUDS is fully backwards compatible:

- No consensus changes.  
- No modified transaction formats.  
- Non-BUDS nodes are unaffected.  
- BUDS-enabled nodes require no coordination.

Tags and labels are off-chain metadata.

---

## Reference Implementation

This repository includes a minimal example (non-normative):

```
src/buds_labels.*  
src/buds_tagging.*  
src/buds_policy_example.*  
```

These files illustrate classification, tagging, and policy integration.

---

## Security and Privacy Considerations

- Misclassification may affect **local** policy only.  
- Nodes may disagree on labels or categories.  
- Implementations should avoid exposing sensitive heuristics through RPCs.  
- User-supplied data (e.g., OP_RETURN text) must not be trusted as authoritative.

---

## Acknowledgements

Inspired by long-standing community discussions on arbitrary data, metadata, pruning, and miner policy.  
Aims to provide a neutral, minimal tool for structuring those conversations.
