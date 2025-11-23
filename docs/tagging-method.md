# Tagging Method – BUDS Classification Pipeline

This document describes how a BUDS implementation **scans** a transaction and
assigns **labels** to data regions.

The goal is:

- **Complete coverage** – every relevant region gets at least one label.
- **Minimal assumptions** – structural rules first, light heuristics second.
- **Local policy-friendly** – labels and tiers are easy to feed into fee and selection logic.

This is **non-consensus** and **non-normative**. Different implementations may
extend this method with additional rules or more advanced heuristics.

---

## 1. Surfaces and regions

A BUDS implementation views a transaction as a set of **surfaces**:

- `scriptpubkey[n]` – the scriptPubKey of output `n`,
- `scriptsig[n]` – the scriptSig of input `n` (not yet used in the demo engine),
- `witness.stack[i:j]` – witness stack item `j` for input `i`,
- `witness.script[i]` – witness script for input `i` (future),
- dedicated surfaces for new lanes (e.g. `segop[k]`) in extensions.

Within each surface, the classifier may define **regions** with byte ranges:

- `start` – byte offset from the beginning of the surface,
- `end` – byte offset (exclusive).

The current reference implementations mark the **entire surface** as a single
region, with:

- `start = 0`
- `end = length_in_bytes`.

---

## 2. Labels

Each region receives one or more **labels** from the registry (see
`registry/registry.json`), for example:

- `pay.standard`
- `da.op_return_embed`
- `da.unknown`
- `da.obfuscated`
- `pay.channel_open`
- `meta.pool_tag`
- `commitment.rollup_root`
- vendor-specific labels, etc.

Labels are **descriptive only**. They do not imply any global policy.

---

## 3. Classification pipeline

The recommended pipeline is:

1. **Structural detection** – recognise obvious patterns by structure.
2. **Protocol / pattern rules** – optional, pluggable rules for known protocols.
3. **Heuristics** – simple generic rules (size, entropy, position, etc.).
4. **Fallback** – if no other rule applies, mark the region as `da.unknown`.

Every region must emerge from the pipeline with **at least one label**.

### 3.1 Structural detection – scriptPubKey

The reference BUDS Tag Engine (browser) applies the following rules to each `scriptpubkey[n]`:

1. **OP_RETURN**
   - If the ASM representation begins with `OP_RETURN`, **or**
   - the hex begins with `0x6a` (`6a…`),
   - then the region is labelled:

     - `da.op_return_embed`

2. **P2PKH**
   - If the hex matches the standard P2PKH pattern:

 ```
     76 a9 14 <20-byte-pubkeyhash> 88 ac
```

     i.e. total length 25 bytes → 50 hex characters, and:

     - prefix: `76a914`
     - suffix: `88ac`

   - then the region is labelled:

     - `pay.standard`.

3. **Fallback (other scriptPubKey)**
   - If no above rule matches, the demo engine currently labels the region as:

     - `pay.standard`.

   - This is intentionally conservative and can be refined:
     - future implementations may detect P2WPKH, P2TR, multisig templates, vaults, channel opens, etc., and use more specific `pay.*` or `contracts.*` labels.

### 3.2 Structural detection – witness stack

For each `witness.stack[i:j]` region, the demo engine applies a single size heuristic:

- Let `L = length_in_bytes` of the witness item.

- If `L > 512` bytes:
  - label: `da.obfuscated`.

- Else:
  - label: `da.unknown`.

The idea:

- large, opaque chunks in witness are likely “data blobs” rather than signatures
  or small parameters,
- anything unknown but small is still tracked but less heavily penalised.

Future engines may refine this with:

- entropy checks (e.g. to distinguish structured ASCII from compressed blobs),
- protocol-specific parsing (e.g. channel state, commitments),
- or joint reasoning across multiple surfaces.

---

## 4. Triage tiers (T0–T3)

For policy and reasoning, each label is mapped to one of four **tiers**:

- **T0 – Consensus / validation-critical**
  - Data required to validate transactions and blocks.
  - E.g. `consensus.scriptsig`, `consensus.witness`, etc.
  - (Currently not emitted by the demo Tag Engine, but reserved in design.)

- **T1 – Economic / Bitcoin-critical**
  - Data representing actual Bitcoin transfers or economic state:
    - `pay.*` – payment outputs and channel opens.
    - `contracts.*` – vaults, contract outputs.
    - `commitment.*` – rollup roots, channel roots, anchored state.
  - Nodes are expected to treat T1 as “high priority”.

- **T2 – Metadata / hints / optional**
  - Application-level or indexer hints, inscriptions, OP_RETURN metadata, etc.
  - Example:
    - `meta.*`
    - `da.op_return_embed`.

- **T3 – Unknown / obfuscated / likely spam**
  - Any label not clearly mapped to T0–T2:
    - `da.unknown`
    - `da.obfuscated`
    - vendor namespaces without special treatment, etc.
  - Nodes may choose to:
    - require higher feerates,
    - cap total T3 weight in block templates,
    - or de-prioritise T3 in mempool eviction.

A classifier should provide a **per-transaction tier summary**, e.g.:

```
tiersPresent = ["T1", "T3"]
counts = { "T0": 0, "T1": 2, "T2": 0, "T3": 1 }
```

This makes it easy for node operators and policy engines to see at a glance:

- “Is this mostly money, or mostly junk?”

---

## 5. Example: minimal Tag Engine (browser demo)

The browser-based Tag Engine used in `buds-lab` applies the above rules:

1. For each `scriptpubkey[n]`:
   - detect OP_RETURN → `da.op_return_embed` (T2),
   - detect P2PKH → `pay.standard` (T1),
   - else → `pay.standard` (T1).

2. For each `witness.stack[i:j]`:
   - `len > 512` bytes → `da.obfuscated` (T3),
   - else → `da.unknown` (T3).

3. For each label, compute the tier (T0–T3).

4. Produce:

   - a list of tags (`surface`, `start`, `end`, `labels[]`),
   - a tier summary (`tiersPresent`, per-tier counts).

This is enough to:

- demonstrate BUDS in the browser,
- drive the example local policy model in BUDS LAB,
- and act as a reference for other implementations.

---

## 6. Local policy integration (non-normative)

BUDS **does not** define policy. It only provides labels and tiers.

However, typical uses in node policy include:

- **Fee requirements**
  - T3 labels (e.g. `da.obfuscated`) require higher feerates.
  - T1/T2 labels may receive neutral or positive treatment.

- **Block template composition**
  - limit T3 weight to a percentage of the block,
  - prefer T1/T2 transactions when choosing from the mempool.

- **Mempool admission / eviction**
  - low-feerate T3 transactions can be deprioritised for relay or evicted earlier.

BUDS LAB implements a small example of this in JavaScript, using:

- per-label minimum multipliers (`minMult`),
- per-label boosts/penalties (`boost`),
- a few policy profiles (`neutral`, `strict`, `permissive`).

These examples are **illustrative only** and are not part of the BUDS standard.

---

## 7. Extensibility

The tagging method is designed to evolve without changing consensus:

- **New labels**
  - can be registered in `registry/registry.json`.

- **New structural rules**
  - can be added by implementations (e.g. vault templates, channel opens).

- **New heuristics**
  - can improve classification of unknown data without breaking existing labels.

As long as implementations:

- respect the registry,
- keep tier mappings coherent,
- and maintain the principle of “complete coverage, minimal assumptions”,

BUDS remains a stable foundation for local node policy and ecosystem-wide discussion of data types.

