# File: README.md
# Bitcoin Unified Data Standard (BUDS)

**BUDS – Bitcoin Unified Data Standard**

BUDS is a **non-consensus, on-chain data labeling and typing standard** for Bitcoin.

It defines:

- A **simple binary format** for labeling arbitrary data.
- A **shared registry** of type IDs and categories.
- A **minimal set of rules** for parsers, indexers, and wallets.

BUDS does **not** change Bitcoin consensus. Nodes, wallets, and explorers are free to:

- Ignore BUDS completely, or
- Parse and display richer information for users and applications.

---

## Goals

- Provide a **unified way** to classify arbitrary data (metadata, media, protocol messages, etc.).
- Make it easier to **filter, index, and analyse** non-financial data on Bitcoin.
- Stay **implementation-agnostic** and **carrier-agnostic**:
  - Works with `OP_RETURN`, segOP lanes, and other data carriers.

---

## Status

- Spec: **Draft**
- Registry: **Seeded**
- Ref implementation: **Python encoder/decoder**

This repository contains:

- `/docs/spec.md` – formal BUDS specification  
- `/docs/rationale.md` – design motivation  
- `/docs/registry-process.md` – how to request new BUDS types  
- `/registry/` – the canonical BUDS type registry  
- `/examples/` – example uses on Bitcoin  
- `/ref-impl/` – minimal reference implementation and test vectors  

---

## Quick Example (Conceptual)

A BUDS blob might say:

- `type = 0x0001` → "generic-metadata"
- `value = "segOP TLV test payload"`

Attached via:

- `OP_RETURN`, or
- segOP lane payload, or
- other defined carrier.

The **bytes on-chain** are just a small binary structure. Off-chain tools decode it using the BUDS registry.

See `/examples/` and `/ref-impl/buds.py` for details.

---
