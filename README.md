# Bitcoin Unified Data Standard (BUDS)

BUDS is a minimal, neutral, non-consensus framework that defines a:

- **standard vocabulary** for describing data inside Bitcoin transactions  
- **canonical registry of labels**  
- **tagging structure** for mapping labels to byte-regions  
- **tier model** (T0–T3)  
- optional **ARBDA** (worst-tier) transaction score  
- optional **local policy model** (fee multipliers, mempool behaviour)

The reference implementation and browser lab use **BUDS v2**, which includes:
- OP_RETURN sub-classification  
- ordinal / inscription heuristics  
- vendor / unknown / obfuscated witness detection  
- full tier mapping (T0–T3)  
- ARBDA score  

The canonical **v2 registry** is:

```
registry/registry-v2.json
```

The original `registry/registry.json` is preserved as a **v1 snapshot** for
backwards compatibility.  
All new tooling should target **registry-v2.json**.

The browser playground for BUDS v2 is located in:

```
buds-lab/
```

with a full test matrix in:

```
buds-lab/docs/buds-lab-test-matrix.md
```

BUDS changes **no consensus rules**, adds **no new transaction formats**, and imposes  
**no mandatory node policy**.  
It standardises *meaning* — not behaviour.

---

# Table of Contents

1. [What is BUDS?](#what-is-buds)  
2. [Why BUDS?](#why-buds)  
3. [Core Concepts](#core-concepts)  
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
- channel opens  
- rollup roots  
- inscriptions  
- signatures  
- metadata  
- indexer hints  
- OP_RETURN messages  
- obfuscated witness blobs  

Until now there has been **no standard way** to describe or classify these byte-regions.

**BUDS solves this by defining:**

- a canonical **label registry**  
- a tag format for associating labels with transaction regions  
- a conceptual **tier system** (T0–T3)  
- optional **ARBDA** (worst-tier dominance)  
- optional **local policy model**  

All BUDS metadata is **off-chain and advisory**.  
Nodes may use it — or ignore it — freely.

---

# Why BUDS?

Today, radically different kinds of data — a Lightning channel open, a rollup root,  
a 200KB data blob, and an inscription — are all treated by nodes as **just bytes**.

This results in:

- unclear “spam” debates  
- inconsistent indexer behaviour  
- brittle node policies  
- no shared language for L2s, explorers, or miners  

**BUDS introduces structure without consensus rules**, enabling:

- clearer reasoning  
- transparent fee policy  
- sensible block template selection  
- robust pruning strategies  
- better cross-tool interoperability  
- easier future protocol upgrades  

BUDS is intentionally small, neutral, and extensible.

---

# Core Concepts

### **Labels**
Canonical descriptors of meaning:

```
pay.standard  
da.op_return_embed  
da.unknown  
da.obfuscated  
meta.ordinal  
consensus.sig  
contracts.vault
```

### **Surfaces**
Logical containers of data:

- `scriptpubkey[n]`
- `witness.stack[i]`
- `witness.script`
- `scriptSig`
- `coinbase`

### **Regions**
Byte-ranges within a surface:

```
surface = "scriptpubkey[1]"
range   = [0,4)
```

### **Tags**
Attach one or more labels to a region:

```json
{
  "surface": "witness.stack[0]",
  "start": 0,
  "end": 600,
  "labels": ["da.obfuscated"]
}
```

### **Tiers (T0–T3)**

- **T0** – consensus-critical (scripts, sigs, tapscript)  
- **T1** – economic / system layer (payments, L2 roots, vaults)  
- **T2** – metadata / application data  
- **T3** – unknown / obfuscated / unregistered vendor data  

### **ARBDA (Worst-Tier Score)**

```
if any T3 ⇒ ARBDA = T3
else if any T2 ⇒ T2
else if any T1 ⇒ T1
else ⇒ T0
```

Used for optional local policy.

---

# Specification

Formal BUDS definitions live in:

- `docs/specification.md`  
- `docs/tagging-method.md`  
- `docs/classification-table.md`  
- `docs/arbda.md`  
- `docs/policy-interface.md`  

These cover:

- labels  
- surfaces & regions  
- tag structure  
- tier model  
- ARBDA  
- non-normative policy examples  

---

# BUDS Registry

### **Current versioning model**

- `registry/registry-v2.json` → **canonical BUDS v2 registry**  
- `registry/registry.json` → preserved **v1 snapshot**  

New tooling should import from **registry-v2.json**.

### **Contents**

Each registry entry defines:

- label  
- description  
- typical surfaces  
- suggested tier  

Extension rules are described in:

```
docs/registry-process.md
```

---

# BUDS Lab (Interactive Tag Engine)

The BUDS Lab is a browser-based playground for:

- label classification  
- region tagging  
- tier detection  
- ARBDA scoring  
- policy simulation  
- Builder / JSON mode  
- output & witness editing  
- exportable JSON results  

### **Location**

```
buds-lab/index.html
```

Documentation:

```
docs/buds-lab.md
```

### **Running locally**

Requires Node.js.

```
cd buds-lab
npm install
npm run dev
```

Then open:

```
http://localhost:5173/
```

### **Features**

- Transaction builder (outputs + witness)
- Automatic hex/type inference
- OP_RETURN sub-classification  
- Ordinal / inscription hints  
- Vendor / unknown / obfuscated witness detection  
- ARBDA score card  
- Policy profiles: strict / neutral / permissive  
- Export `lastResult` as JSON  

Ideal for testing heuristics or teaching the BUDS model.

---

# Reference Implementation (C++)

A minimal BUDS v2 Tag Engine implementation is provided:

```
src/buds_tagger.h
src/buds_tagger.cpp
src/buds_demo.cpp
```

### **buds-demo**

Example program that:

- constructs a synthetic transaction  
- runs the v2 tagger  
- prints tags, tiers, and ARBDA  

### **Build (example)**

```
g++ -std=c++17 -Isrc \
    src/buds_tagger.cpp \
    src/buds_demo.cpp \
    -o buds-demo

./buds-demo
```

This implementation is **non-consensus and advisory**, intended as a reference for node or tool developers.

---

# Test Suite

The v2 test suite consists of:

- `tests/test_buds_tagger.cpp`  
- documentation: `docs/tests.md`  
- full JS test matrix: `buds-lab/docs/buds-lab-test-matrix.md`

### **Build & run (C++)**

```
g++ -std=c++17 -Isrc \
    tests/test_buds_tagger.cpp \
    src/buds_tagger.cpp \
    -o buds-tests

./buds-tests
```

Tests cover:

- P2PKH → `pay.standard`  
- OP_RETURN sub-types  
- vendor / unknown / obfuscated witness  
- ordinal / inscription hints  
- mixed tier transactions  
- ARBDA worst-tier scoring  

---

# BIP Draft

The proposed BUDS BIP is located in:

```
bip/bip-buds.md
```

It defines:

- registry model  
- labels  
- tag format  
- tiers  
- ARBDA  
- classification principles  
- interoperability guarantees  
- non-consensus nature  

Suitable for peer review and discussion.

---

# Status

The **BUDS v2 draft** is:

- non-consensus  
- experiment-ready  
- BIP-ready for wider review  
- suitable for early adopters: miners, L2 builders, indexers, explorers  

Nothing in BUDS affects transaction validity.

---

# License

### **Code**  
`src/`, `tests/`, `buds-lab/`  
→ **MIT License**

### **Documentation**  
`README.md`, `docs/`, `registry/`, `whitepaper/`  
→ **CC BY 4.0**

### **BIP Text**  
`bip/bip-buds.md`  
→ **BSD-2-Clause**


