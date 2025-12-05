# BUDS Registry

The BUDS registry is a machine-readable list of **data type labels** that may
appear inside Bitcoin transactions. Labels describe the *intended meaning* of a
region of transaction data. They help implementations recognise common patterns
and apply local policy if desired.

The canonical v2 registry is stored in:

- `registry/registry-v2.json`

For legacy tooling, the original v1 registry is still provided:

- `registry/registry.json` (deprecated, kept for backwards compatibility)

New implementations SHOULD prefer the v2 registry, which:

- tightens label definitions
- adds OP_RETURN sub-types and vendor / ordinal hints
- aligns directly with the v2 TagEngine heuristics (JS + C++)

---

## Purpose

- Provide a **shared vocabulary** for wallets, nodes, indexers, miners, and protocol developers.
- Reduce ambiguity when interpreting transaction data.
- Allow honest protocols to identify their data structures.
- Enable local classification without requiring consensus changes.
- Give node operators a stable set of labels to plug into policy and monitoring tools.

Registration is optional. Unregistered data types continue to work normally and
may still be classified locally as `da.unknown`, `da.obfuscated`, or
`da.unregistered_vendor`.

---

## File Format

The registry file is a JSON object with:

- `version` – registry schema / content version
- `labels` – an array of label entries

Each label entry contains:

- `label` – canonical label name (e.g. `pay.standard`)
- `description` – brief human-readable explanation
- `surfaces` – typical locations where this label appears
- `suggested_category` – suggested conceptual tier (T0–T3)

Implementations are free to:

- add local labels in their own configuration, or
- override suggested tiers for local policy purposes.

---

## Initial Registry (v2 Overview)

Examples of v2 labels include:

### Consensus surfaces

    consensus.sig
    consensus.script
    consensus.taproot_prog

### Economic / system

    pay.standard
    pay.channel_open
    pay.channel_update
    contracts.vault
    commitment.rollup_root
    meta.pool_tag

### Metadata / application

    da.op_return_embed
    da.embed_misc
    meta.inscription
    meta.ordinal
    meta.indexer_hint

### Unknown / obfuscated

    da.unknown
    da.obfuscated
    da.unregistered_vendor

See `registry/registry-v2.json` for the authoritative machine-readable version.

---

## Extending the Registry

Protocols and projects that wish to register new labels should follow the
process described in:

- `docs/registry-process.md`

Registration is recommended when:

- a protocol wants nodes to understand its data structure
- a pattern appears frequently across the network
- a label would reduce misclassification or reliance on heuristics

Registration is *not* required for experimentation, private use, or purely local
logic.
