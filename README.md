# Bitcoin Unified Data Standard (BUDS)

**BUDS** is a minimal, neutral, non-consensus framework that defines a:

The current reference implementation and browser demo use **BUDS v2** labels
and heuristics, including OP_RETURN sub-types, ordinal/inscription hints, and
a simple ARBDA (worst-tier) score.

The canonical v2 registry is [`registry/registry-v2.json`](registry/registry-v2.json),
and the browser playground for BUDS v2 heuristics is located in
[`buds-lab/`](buds-lab/), with a detailed test matrix in
[`buds-lab/docs/buds-lab-test-matrix.md`](buds-lab/docs/buds-lab-test-matrix.md).

**standard vocabulary for describing data inside Bitcoin transactions.**

BUDS gives node operators, miners, indexers, and L2 builders a shared way to:

- **Label** data (e.g., `pay.standard`, `da.op_return_embed`, `da.obfuscated`)
- **Classify** transaction regions into conceptual tiers (T0–T3)
- **Tag** byte-ranges inside a transaction
- **Optionally score** transactions using ARBDA (worst-tier dominance)
- **Optionally apply local policy** (fee rules, mempool behaviour, block templates)

BUDS changes **no consensus rules**, adds **no new transaction format**, and imposes  
**no mandatory node policy.**  
It simply standardises *meaning* — not behaviour.

---

# Table of Contents

1. [What is BUDS?](#what-is-buds)
2. [Why BUDS?](#why-buds)
3. [Core Concepts](#core-concepts)
   - Labels  
   - Surfaces  
   - Regions  
   - Tags  
   - Tiers (T0–T3)  
   - ARBDA score  
4. [Specification](#specification)
5. [BUDS Registry](#buds-registry)
6. [BUDS Lab (Interactive Tag Engine)](#buds-lab-interactive-tag-engine)
7. [Reference Implementation (C++)](#reference-implementation-c)
8. [Test Suite](#test-suite)
9. [BIP Draft](#bip-draft)
10. [Status](#status)
11. [License](#license)

---

# What is BUDS?

Bitcoin transactions contain many kinds of data:

- payment outputs  
- witness programs  
- signatures  
- channel opens  
- inscriptions  
- rollup roots  
- indexer hints  
- OP_RETURN metadata  
- obfuscated witness blobs  

Until now there has been **no standard way** to describe, reason about, or classify these byte-regions.

**BUDS solves this by defining:**

- a canonical **registry of labels**,  
- a tag format for associating labels with transaction regions,  
- a conceptual tier model (T0–T3),  
- and an optional **ARBDA** score for worst-tier assessment.

All BUDS metadata is **off-chain, advisory, and local**.  
Nodes can use it — or ignore it — exactly as they choose.

---

# Why BUDS?

Today, a Lightning channel open, a rollup root, a 200KB blob of witness data,  
and an inscription are all treated by nodes as… **the same thing: bytes**.

This causes:

- unclear “spam” debates  
- brittle ad-hoc node policies  
- inconsistent indexer behaviour  
- no shared terminology between wallets, L2s, explorers, or miners  

**BUDS introduces structure without consensus rules**, allowing:

- clear analysis  
- transparent fee policy  
- better block template decisions  
- robust pruning strategies  
- tools and explorers to speak the same language  
- developers to classify their own data types  
- future protocols to register new labels  

BUDS is intentionally small, neutral, and flexible.

---

# Core Concepts

### **Labels**
Canonical descriptors of data meaning:
```
pay.standard  
da.op_return_embed  
da.unknown  
da.obfuscated
consensus.script  
contracts.vault  
meta.ordinal
```

### **Surfaces**
Logical locations of data:
- `scriptpubkey[n]`
- `witness.stack[i]`
- `scriptSig`
- `witness.script`

### **Regions**
Byte-ranges within a surface:
```
surface = "scriptpubkey[1]"
range   = [0,4)
```

### **Tags**
Attach one or more labels to a region:
```
{
  "surface": "witness.stack[0]",
  "start": 0,
  "end": 600,
  "labels": ["da.obfuscated"]
}
```

### **Tiers (T0–T3)**
Conceptual classification levels:

- **T0** – consensus-critical  
- **T1** – economic/system  
- **T2** – metadata/application  
- **T3** – unknown/obfuscated  

### **ARBDA (Arbitrary Data Dominance Assessment)**  
A **transaction-level worst-tier score**:

- if any T3 region ⇒ **ARBDA = T3**  
- else if any T2 ⇒ **T2**  
- else if any T1 ⇒ **T1**  
- else ⇒ **T0**  

Used for optional local policy.

---

# Specification

The formal definition of BUDS is located in:

- [`docs/specification.md`](docs/specification.md)  
- [`docs/tagging-method.md`](docs/tagging-method.md)  
- [`docs/classification-table.md`](docs/classification-table.md)  
- [`docs/arbda.md`](docs/arbda.md)  
- [`docs/policy-interface.md`](docs/policy-interface.md)  

These documents cover:

- labels  
- surfaces & regions  
- tag structure  
- tier model  
- ARBDA  
- non-normative policy examples  

---

# BUDS Registry

The canonical machine-readable registry lives at:

[`registry/registry.json`](registry/registry.json)

It defines:

- each label  
- description  
- typical surfaces  
- suggested tier  

Registry extension rules are in:  
[`docs/registry-process.md`](docs/registry-process.md)

---

# BUDS Lab (Interactive Tag Engine)

BUDS includes a browser-based tool for exploring:

- classification  
- region tagging  
- tier detection  
- ARBDA scoring  
- example policy outcomes  
- JSON import/export  
- output/witness manipulation  

**Location:** `buds-lab/index.html`  
Documentation: [`docs/buds-lab.md`](docs/buds-lab.md)

**Features:**

- Build a transaction interactively  
- Remove outputs and witness items  
- Switch between Builder / JSON Mode  
- Run Tag Engine  
- See coloured `(T0–T3)` tier markers  
- See ARBDA score card  
- Export full JSON result  
- Use policy profiles (strict / neutral / permissive)

This is ideal for testing heuristics or teaching the BUDS model.

---

# Reference Implementation (C++)

A small C++ implementation is included for demonstration purposes:

- `buds_labels.*`  
- `buds_tagging.*`  
- `buds_policy_example.*`

Two binaries are provided:

### **buds-demo**
Always runs a built-in example.

### **buds-cli**
Simple CLI interface with:
- `help`
- `example`
- (future: `load <file>` etc.)

Full build instructions:  
[`docs/demo-cli.md`](docs/demo-cli.md)

---

# Test Suite

A minimal test suite validates the Tag Engine’s expected behaviour:

`tests/test_buds_tagger.cpp`  
Documentation: `docs/tests.md`

**Compile and run:**

```
g++ -std=c++17 -Isrc \
    tests/test_buds_tagger.cpp \
    src/buds_labels.cpp \
    src/buds_tagging.cpp \
    -o buds-tests

./buds-tests
```

Tests include:

- P2PKH → `pay.standard`
- OP_RETURN → `da.op_return_embed`
- small witness blob → `da.unknown`
- large witness blob → `da.obfuscated`
- mixed transaction → all above

---

# BIP Draft

The proposed BIP text is in:  
[`bip/bip-buds.md`](bip/bip-buds.md)

It defines:

- registry  
- labels  
- tag format  
- tiers  
- ARBDA  
- classification principles  
- non-consensus nature  
- interoperability guarantees  

This is suitable for peer review and discussion.

---

# Status

The current BUDS draft (`v0.1.x`) is:

- **non-consensus**
- **experiment-ready**
- **BIP-ready for review**
- suitable for early adopters (miners, L2 builders, indexers, explorers)

Nothing in BUDS affects transaction validity.

---

# License

This repository uses a multi-license scheme:

### **Code**  
`src/`, `tests/`, `buds-lab/`  
→ **MIT License**

### **Documentation**  
`README.md`, `docs/`, `registry/`, `whitepaper/`  
→ **CC BY 4.0**

### **BIP Text**  
`bip/bip-buds.md`  
→ **BSD-2-Clause** (required for BIPs)

