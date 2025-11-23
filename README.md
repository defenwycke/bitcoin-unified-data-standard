# Bitcoin Unified Data Standard (BUDS)

BUDS is a small, neutral standard that defines **common labels** for the different kinds of data that appear inside Bitcoin transactions.  
It gives nodes and miners a shared vocabulary for **classifying** and **reasoning about** transaction data without touching consensus rules.

BUDS enables:

- A **standard registry** of known data types.
- A simple **classification model** (conceptual tiers T0–T3).
- Local node logic to **scan and tag** transaction regions.
- Optional **policy integration** (fee rules, mempool rules, template selection).

BUDS does *not* enforce consensus rules, block limits, censorship, or policy.  
It only defines labels and meanings. All decisions remain local to each node.

---

## Core Idea

Every transaction contains different kinds of data: signatures, scripts, channel opens, inscriptions, hints, blobs, etc.  
BUDS provides a clean framework:

1. **Describe** data types → via a standard registry.  
2. **Classify** them conceptually → T0 (consensus) to T3 (unknown).  
3. **Scan & Tag** transaction regions → local heuristics / structural detection.  
4. **Apply local policy** → nodes choose how to treat each label.

BUDS keeps classification and policy **fully optional** and **implementation-defined**.

---

## Specification

See [`docs/specification.md`](docs/specification.md) for the formal definition of:

- Labels  
- Regions  
- Classification  
- Tagging  
- Policy integration (non-normative)

The machine-readable registry is in [`registry/registry.json`](registry/registry.json).

---

## Status

BUDS v1 is a draft intended for review, experimentation, and integration into node or policy engines (e.g., Bitcoin Ghost, custom miners, research tools).

No consensus changes. No required policies.

---

## Reference Implementation

See the [Demo CLI instructions](docs/demo-cli.md) for build/run details.

---

## License

MIT unless otherwise noted.
