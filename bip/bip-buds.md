BIP: TBD  
Title: Bitcoin Unified Data Standard (BUDS)  
Author: Defenwycke <defenwycke@icloud.com>  
Status: Draft  
Type: Standards Track  
Created: 2025-11-23  
License: BSD-2-Clause
Repository: https://github.com/defenwycke/bitcoin-unified-data-standard

# Abstract

This BIP specifies the Bitcoin Unified Data Standard (BUDS), a minimal,
optional, and non-consensus framework for **describing**, **classifying**, and
**tagging** the data that appears inside Bitcoin transactions.

BUDS defines:

- a machine-readable **registry** of data-type labels,
- common terminology for transaction **surfaces** (scriptSig, witness, scriptPubKey, etc.),
- a **tagging interface** associating byte-ranges with labels,
- a simple conceptual categorisation model (T0–T3),
- an optional **ARBDA** (Arbitrary Data Dominance Assessment) score summarising
  the worst-case tier present in a transaction.

BUDS does **not** change consensus rules or mandate policy behaviour.  
It provides a shared vocabulary that implementations may freely adopt for local
decisions such as mempool policy, fee scoring, block template construction, or
data retention.

---

# Motivation

Bitcoin transactions carry a wide range of data types:

- signatures and scripts,
- addresses and public keys,
- channel state and vault logic,
- Tapscript programs,
- rollup commitments,
- indexer hints and application metadata,
- OP_RETURN messages,
- and arbitrary witness blobs.

Today:

- Nodes treat almost all of this as **opaque bytes**.
- No standard vocabulary exists for identifying or describing data meaning.
- Local policies rely on brittle heuristics.
- Discussions around “spam” lack objective terminology.

BUDS introduces **neutral, descriptive metadata** that helps:

- structure technical discussions,
- enable transparent local policies,
- support advanced tools (indexers, miners, L2 systems, pruning engines),
- without altering Bitcoin’s consensus behaviour.

---

# Specification

A full formal specification is provided in:

- `docs/specification.md`  
- `registry/registry.json`  

A summary is presented below.

## 1. Labels

A **label** is a canonical string describing a data type or semantic meaning.

Examples:

**Consensus / structural**  

```
consensus.sig
consensus.script
consensus.taproot_prog
```

**Payments / systems** 

```
pay.standard
pay.channel_open
contracts.vault
commitment.rollup_root
```

**Metadata**  

```
meta.inscription
meta.ordinal
meta.indexer_hint
da.op_return_embed
```

**Unknown / generic**  

```
da.unknown
da.obfuscated
da.unregistered_vendor
```

Labels describe **intended interpretation**, not a guarantee of truth.

All normative labels for this BIP live in the BUDS registry.

- The canonical v2 registry file is `registry/registry-v2.json`.
- The earlier `registry/registry.json` is retained for legacy and exploratory tooling  
and MAY be referenced by older implementations, but new systems SHOULD prefer v2.


---

## 2. Surfaces and Regions

A **surface** is a logical location for transaction data, such as:

- `scriptSig`
- `scriptpubkey[n]`
- `witness.stack[i]`
- `witness.script`
- `op_return`
- extension lanes

A **region** is a `[start, end)` byte-range within a surface.

Example:

```
surface: "scriptpubkey[1]"
start: 0
end: 4
labels: ["da.op_return_embed"]
```

---

## 3. Tags

A **tag** attaches one or more labels to a region:

```
Tag:
surface: string
start: integer
end: integer
labels: [string]
```

Tags are **non-consensus metadata**.  
Nodes may ignore them or compute them differently.

---

## 4. Registry Structure

Each entry has:

```
{
"label": "pay.channel_open",
"description": "Lightning or L2 channel establishment.",
"surfaces": ["scriptpubkey", "witness.script"],
"suggested_category": "T1"
}
```

Fields:

- `label` — required  
- `description` — human-readable  
- `surfaces` — typical source regions  
- `suggested_category` — **non-normative** tier hint  

The registration process is defined in `docs/registry-process.md`.

---

## 5. Conceptual Categories (T0–T3)

BUDS defines four non-binding tier categories:

```
T0 — consensus-critical
T1 — economic/system-critical
T2 — metadata/application
T3 — unknown/obfuscated
```

No node is required to use these tiers.

---

## 6. ARBDA Tier (Arbitrary Data Dominance Assessment)

The **ARBDA** score is an optional, single-value summary:

- ARBDA = the **highest tier present** in any region of the transaction.

Example:  
- If a transaction contains both a `pay.standard` region (T1)  
  and a large opaque witness blob (T3),  
  → **ARBDA = T3**

ARBDA provides a **worst-case interpretation** for local policy.

---

## 7. Tagging and Classification

Classification uses the **standard BUDS registry** of labels.  
Every BUDS implementation MUST use the canonical registry and label definitions.

The method used to *detect* which label applies to which region, however, is 
left fully implementation-defined. Nodes are free to choose their own
heuristics, pattern-matching rules, or external logic.

Thus:

- The **vocabulary** is standardised.
- The **detection logic** is not.

A typical BUDS-enabled node:

1. Parses a transaction into surfaces and byte-ranges,
2. Applies local heuristics or pattern rules to interpret meaning,
3. Assigns labels from the standard registry,
4. Emits tags.

Different nodes may classify the same transaction differently,
but the labels themselves remain interoperable and consistent.

---

## 8. Optional Policy Integration

Nodes and miners MAY use BUDS metadata for:

- mempool admission,
- fee multipliers,
- block template preferences,
- pruning retention classes,
- statistical analysis.

This BIP defines **no thresholds, fees, penalties, or soft limits**.

All policy use is entirely voluntary and unconstrained.

---

# Rationale

Bitcoin already carries diverse, meaningful data.  
Without structure, nodes treat:

- channel openings,  
- rollup roots,  
- inscriptions,  
- pool metadata,  
- large opaque blobs  

as indistinguishable.

BUDS provides:

- neutral terminology,
- clear metadata structures,
- optional categorisation,
- and a single worst-case indicator (ARBDA),

without changing validation, without consensus coordination, and without imposing
policy.

It enables transparent reasoning and local choice while preserving Bitcoin’s
permissionless nature.

---

# Backwards Compatibility

BUDS is fully backwards compatible:

- No consensus changes.  
- No modified transaction formats.  
- Nodes without BUDS behave normally.  
- Nodes with BUDS require no coordination.

Tags are purely **off-chain metadata**.

---

# Reference Implementation

A full reference implementation, test suite, and browser-based Tag Engine (BUDS LAB) are available in the accompanying repository:

https://github.com/defenwycke/bitcoin-unified-data-standard

These examples illustrate how BUDS metadata may be applied locally without prescribing any particular behaviour.

The repository includes a minimal reference implementation:

```
src/buds_labels.*
src/buds_tagging.*
src/buds_policy_example.*
```

These files demonstrate:

- label resolution  
- region tagging  
- category summarisation  
- ARBDA scoring  
- example fee-policy integration  

`buds-lab/` provides a browser-based Tag Engine for experimentation.

---

# Security and Privacy Considerations

- Misclassification affects **local policy only**.  
- Nodes may disagree on label meaning or categorisation.  
- Implementations should avoid leaking sensitive heuristics through RPCs.  
- User-embedded metadata (e.g., OP_RETURN text) must not be trusted as authoritative.

---

# Acknowledgements

This work draws on long-running community discussions around arbitrary data,
metadata, node incentives, pruning, and local policy.  
BUDS aims to provide a neutral, minimal framework to improve clarity and reduce
brittleness across the ecosystem.
