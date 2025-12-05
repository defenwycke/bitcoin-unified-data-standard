# BUDS Test Documentation

This document describes how the Bitcoin Unified Data Standard (BUDS) v2
implementation is tested across:

- the JS Tag Engine in `buds-lab/`
- the C++ Tag Engine in `src/`

No part of this document uses triple-backtick code blocks to ensure it never
breaks GitHub or the ChatGPT rendering window.

---

## 1. Test Structure Overview

There are two layers of tests:

1. JS / Browser (BUDS Lab)
2. C++ / Native (buds_tagger.cpp)

The JS engine is the behavioural reference.  
The C++ engine mirrors those rules.

---

## 2. JS Tests — BUDS Lab

### 2.1 Files

- Tag Engine: `buds-lab/buds-tag-engine.js`
- Lab UI: `buds-lab/index.html`
- Test Matrix: `buds-lab/docs/buds-lab-test-matrix.md`

### 2.2 Running BUDS Lab

    cd buds-lab
    npm install
    npm run dev

Then open:

    http://localhost:5173/

### 2.3 What You Can Test in the Lab

- Construct transactions (builder mode)
- Paste a JSON `SimpleTx` object (JSON mode)
- Click “Run Tag Engine” to:
  - show labels for each region
  - show tier markers (T0–T3)
  - show ARBDA score
  - show policy effects for strict / neutral / permissive

### 2.4 Test Groups (A–G)

#### Group A — Baseline Payments
- P2PKH / P2WPKH / P2TR outputs
- Expected label: pay.standard (T1)
- Expected ARBDA: T1

#### Group B — OP_RETURN Sub-types
- small ASCII payloads → meta.indexer_hint (T2)
- small/medium ASCII → da.op_return_embed (T2)
- rollup-style “6a20…” roots → commitment.rollup_root (T1)
- large payloads → da.embed_misc (T2)

#### Group C — Witness: Vendor / Unknown / Obfuscated
- small ASCII witness ≤128 bytes → da.unregistered_vendor (T3)
- mixed/non-ASCII medium blobs → da.unknown (T3)
- >512 byte large blobs → da.obfuscated (T3)

#### Group D — Ordinal / Inscription
- contains ASCII “ord”
- small → meta.ordinal (T2)
- large → meta.inscription (T2)

#### Group E — Mixed Transactions
Validates:
- correct label per region
- expected tier mapping
- ARBDA dominance (worst tier wins)

#### Groups F / G — Extended
Reserved for adversarial and corner cases.

---

## 3. C++ Tests — Tag Engine

### 3.1 Files

- Tagger: `src/buds_tagger.cpp`, `src/buds_tagger.h`
- Example: `src/buds_demo.cpp`
- Tests: `tests/test_buds_tagger.cpp`

### 3.2 Build the C++ Tests

    g++ -std=c++17 -Isrc \
        tests/test_buds_tagger.cpp \
        src/buds_tagger.cpp \
        -o buds-tests

Run:

    ./buds-tests

### 3.3 What the Tests Validate

#### Payment Recognition
- P2PKH / P2WPKH / P2TR →
  - label: pay.standard
  - tier: T1
  - ARBDA: T1

#### OP_RETURN Classification
- ascii payload ≤8 bytes → meta.indexer_hint (T2)
- ≤32 bytes ASCII → da.op_return_embed (T2)
- ≤80 bytes → da.op_return_embed (T2)
- rollup 32-byte payload → commitment.rollup_root (T1)
- large payload → da.embed_misc (T2)

#### Witness Classification
- small ASCII → da.unregistered_vendor (T3)
- medium non-ASCII → da.unknown (T3)
- >512 bytes → da.obfuscated (T3)

#### Ordinal Detection
- contains “ord”
  - small → meta.ordinal (T2)
  - large → meta.inscription (T2)

#### ARBDA
- if any T3 → ARBDA = T3
- else if any T2 → ARBDA = T2
- else if any T1 → ARBDA = T1
- else → T0

---

## 4. Manual Testing

Use BUDS Lab to experiment with:

- real transactions
- custom witness stacks
- OP_RETURN patterns
- adversarial blobs
- new protocol hints

Shows instantly:
- region labels
- tier mapping
- ARBDA
- fee policy interpretations

---

## 5. Adding New Tests

1. Extend registry (`registry/registry-v2.json`)
2. Update JS heuristics
3. Update C++ heuristics
4. Add JS test cases in test matrix
5. Add C++ test in `tests/test_buds_tagger.cpp`
6. Validate JS and C++ match

---

## 6. Non-Goals

Tests do NOT check:
- consensus validity
- full node behaviour
- mempool logic
- script execution
- segwit verification

BUDS is purely descriptive.

---
