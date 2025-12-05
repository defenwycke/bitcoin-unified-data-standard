# BUDS Lab — Interactive Tag Engine Playground

The **BUDS Lab** is a browser-based interactive environment for exploring
**BUDS v2 classification**, **tier detection (T0–T3)**, **region tagging**, and
the **ARBDA (worst-tier) score**.

It allows users to interactively build transactions, adjust outputs and witness
items, and run the Tag Engine to understand how BUDS interprets data inside
Bitcoin transactions.

---

## 1. Overview

The BUDS Lab includes:

- **Builder mode** (UI controls)
- **JSON mode** (paste a SimpleTx JSON object)
- **Per-region labels** (e.g. `pay.standard`, `meta.indexer_hint`)
- **Tier markers** (`T0`, `T1`, `T2`, `T3`)
- **ARBDA score card**
- **Policy profile** (strict / neutral / permissive)
- **JSON import / export**
- **Witness & OP_RETURN data editing**
- **Sub-type detection**  
  - rollup roots  
  - OP_RETURN subtypes  
  - ordinal/inscription hints  
  - vendor data  
  - unclassified / obfuscated blobs  

The Lab is powered by:

- `buds-tag-engine.js` (browser heuristics)
- Vite development server
- the canonical registry (registry-v2.json)

---

## 2. Running the Lab

### 2.1 Install dependencies

```bash
cd buds-lab
npm install
```

### 2.2 Start the development server

```bash
npm run dev
```

This launches the Lab at:

```
http://localhost:5173/
```

---

## 3. Using the Interface

### Modes

- **Builder Mode**:  
  Create a transaction visually (add/remove outputs, witness items, edit fields).

- **JSON Mode**:  
  Directly paste a SimpleTx JSON object to test classification programmatically.

### Running the Tag Engine

Click **Run Tag Engine** to display:

- region-level labels  
- tier markers (T0–T3)  
- ARBDA score  
- policy effects (based on policy profile)  
- raw tags breakdown  

### Example Features

- Switch policy profile (strict / neutral / permissive)
- Export full classification JSON
- Add/remove outputs and witness items
- See worst-tier ARBDA instantly
- Identify OP_RETURN sub-types (T1/T2/T3)
- Identify ordinal/inscription witness signatures
- Detect unregistered vendor metadata
- Detect obfuscated or bulk witness blobs

---

## 4. Test Matrix (A–G)

The full test matrix is stored at:

```
buds-lab/docs/buds-lab-test-matrix.md
```

### Summary

- **Group A – Baseline Payments (T1)**
  - P2PKH / P2WPKH / P2TR
  - → `pay.standard` → ARBDA = T1

- **Group B – OP_RETURN Sub-types**
  - ASCII hints → `meta.indexer_hint` (T2)
  - medium embeds → `da.op_return_embed` (T2)
  - rollup roots → `commitment.rollup_root` (T1)

- **Group C – Witness Blobs**
  - C1: ASCII vendor blobs → `da.unregistered_vendor` (T3)
  - C2: small random → `da.unknown` (T3)
  - C3: large random >512B → `da.obfuscated` (T3)
  - C4: ord/inscription signatures → `meta.ordinal` / `meta.inscription` (T2)

- **Group D – Mixed TX**
  - Payment + OP_RETURN + witness → ARBDA = worst detected tier

- **Group E – Multiple Outputs**
  - Mixed OP_RETURN sub-types

- **Group F – Policy Effects**
  - strict vs. neutral vs. permissive

- **Group G – Edge Cases**
  - empty witness  
  - malformed OP_RETURN  
  - unknown scriptPubKey  

Each item includes:
- setup
- exact hex inputs
- expected labels
- expected tiers
- expected ARBDA

---

## 5. Registry

The Lab uses:

```
registry/registry-v2.json
```

This file defines:

- label names  
- descriptions  
- canonical tier mapping  
- typical surfaces  
- extension rules  

---

## 6. Developer Notes

`buds-tag-engine.js` implements:

- OP_RETURN heuristics (rollup root, ascii-hint, general embed)
- witness heuristics (ordinal/inscription detection, vendor, blob)
- tier mapping (registry + fallback prefix rules)
- ARBDA worst-tier logic
- optional policy scoring

The C++ reference implementation is located in:

```
src/buds_tagger.* 
```

Both JS and C++ follow the same conceptual rules.

---

## 7. License

- **Code** (buds-lab/, src/, tests/) — MIT  
- **Documentation** — CC BY 4.0  
- **BIP text** — BSD-2-Clause  

---

BUDS Lab is the recommended environment for experimenting with BUDS v2 and
previewing how future policy decisions or registry extensions might behave.
