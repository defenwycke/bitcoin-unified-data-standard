# BUDS Registry

The BUDS registry is a machine-readable list of **data type labels** that may appear inside Bitcoin transactions.  
Labels describe the *intended meaning* of a region of transaction data.  
They help implementations recognise common patterns and apply local policy if desired.

The registry is stored in `registry/registry.json`.

---

## Purpose

- Provide a **shared vocabulary** for wallets, nodes, indexers, miners, and protocol developers.
- Reduce ambiguity when interpreting transaction data.
- Allow honest protocols to identify their data structures.
- Enable local classification without requiring consensus changes.

The registry does **not** determine priority, tiers, or policy.  
Nodes are free to ignore labels or assign their own treatment.

---

## What a Registry Entry Contains

Each entry in `registry.json` includes:

- **`label`** — canonical name (e.g., `pay.standard`).
- **`description`** — short text explaining the data type.
- **`surfaces`** — which transaction regions typically contain this data  
  (`witness`, `scriptsig`, `scriptpubkey`, `op_return`, etc.).
- **`suggested_category`** — optional hint for conceptual grouping (T0–T3).  
  This is non-binding.

Example entry:

```json
{
  "label": "pay.channel_open",
  "description": "Lightning or L2 channel establishment data.",
  "surfaces": ["scriptpubkey", "witness"],
  "suggested_category": "T1"
}
```

---

## Initial Registry

The initial registry includes common transaction data types such as:

### Consensus surfaces

```
consensus.sig, consensus.script, consensus.taproot_prog
```

### Economic / system-critical data

```
pay.standard, pay.channel_open, contracts.vault, commitment.rollup_root, meta.pool_tag
```

### Metadata / application data

```
da.op_return_embed, meta.inscription, meta.ordinal, meta.indexer_hint, da.embed_misc
```

### Unknown or opaque data

```
da.unknown, da.obfuscated, da.unregistered_vendor
```

See registry/registry.json for the complete list.

## Extending the Registry

New labels may be added via pull request.
See `registry-process.md` for the simple contribution rules.

No protocol is required to register its data types, and unregistered data remains valid.

### Notes

- Registry entries describe intent, not authority.
- Nodes classify data locally, using or ignoring registry labels as they choose.
- BUDS does not enforce any fee rules, limits, or censorship.
- The registry is purely descriptive and exists to support better classification across the network.
