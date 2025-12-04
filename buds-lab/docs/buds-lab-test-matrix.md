# BUDS LAB – Tag Engine Test Matrix (v2)

This document records the **manual test matrix** used to validate the BUDS LAB
Tag Engine (browser version) for **BUDS v2**, as implemented in:

- `buds-lab/buds-tag-engine.js`
- `registry/registry-v2.json`

The goal is to confirm that **core behaviours** and **tiering (T0–T3)** match
the BUDS v2 design:

- Standard payments → `pay.standard` (T1)
- OP_RETURN lane → `meta.indexer_hint` / `da.op_return_embed` /
  `da.embed_misc` / `commitment.rollup_root`
- Witness lane → `meta.ordinal` / `meta.inscription` /
  `da.unregistered_vendor` / `da.unknown` / `da.obfuscated`
- ARBDA (transaction-level tier) → **worst tier present wins**
  (`T3 > T2 > T1 > T0`)

---

## 1. How to Run These Tests

All tests use the **BUDS LAB** UI in **Builder** mode:

1. Open `buds-lab/index.html` in a browser (or run `npm run dev` if using Vite).
2. Use the **Builder / JSON** toggle in the *Transaction Input* panel.
3. For these tests, use **Builder** mode and:
   - Set `TXID` to any value (e.g. `deadbeef…`) unless explicitly specified.
   - Add **Outputs** using the “+ Add output” button.
   - Add **Witness items** using the “+ Add witness item” button.
4. Ensure **BUDS label** dropdowns are left on **`(auto: infer type)`** for all
   items.
5. Click **Run Tag Engine**.
6. Compare:
   - **Output / Region breakdown**
   - **ARBDA score**
   against the **Expected** values below.

If a test fails in future changes, update both the Tag Engine and this matrix.

---

## 2. Group A – Payment Lane (scriptPubKey → `pay.standard`)

### A1 – Standard P2PKH

**Setup**

- Clear.
- TXID: arbitrary.
- Output #0:
  - Type: `P2PKH` (use template).
- No witness.

**Hex (scriptPubKey)**

```
76a91400112233445566778899aabbccddeeff0011223388ac
```

**Expected**

- Breakdown:
  - `Output #0 → pay.standard (T1)`
- ARBDA:
  - `T1`

---

### A2 – P2WPKH

**Setup**

- Clear.
- Output #0:
  - Type: `Custom`
  - ASM: `0 <20-byte-pubkeyhash>`
  - Hex:

```
001400112233445566778899aabbccddeeff0011
```

- No witness.

**Expected**

- Breakdown:
  - `Output #0 → pay.standard (T1)`
- ARBDA:
  - `T1`

---

### A3 – P2TR

**Setup**

- Clear.
- Output #0:
  - Type: `Custom`
  - ASM: `1 <32-byte-xonly-pubkey>`
  - Hex:

```
512000112233445566778899aabbccddeeff00112233445566778899aabbccdd
```

- No witness.

**Expected**

- Breakdown:
  - `Output #0 → pay.standard (T1)`
- ARBDA:
  - `T1`

---

### A4 – Unknown non-OP_RETURN script

**Setup**

- Clear.
- Output #0:
  - Type: `Custom`
  - ASM: `OP_HASH160 <20-byte-hash> OP_EQUAL`
  - Hex:

```
a91400112233445566778899aabbccddeeff0011223387
```

- No witness.

**Expected**

- Breakdown:
  - `Output #0 → pay.standard (T1)` (fallback economic lane)
- ARBDA:
  - `T1`

---

## 3. Group B – OP_RETURN Lane (data / commitments)

### B1 – Rollup Root Commitment

**Setup**

- Clear.
- Output #0:
  - Type: `OP_RETURN`
  - Hex:

```
6a201111111111111111111111111111111111111111111111111111111111111111
```

(`6a` OP_RETURN, `20` length, 32 bytes of `0x11`.)

**Expected**

- Breakdown:
  - `Output #0 → commitment.rollup_root (T1)`
- ARBDA:
  - `T1`

---

### B2 – Tiny ASCII Hint

**Setup**

- Clear.
- Output #0:
  - Type: `OP_RETURN`
  - Hex:

```
6a026f6b
```

(payload `"ok"`)

**Expected**

- Breakdown:
  - `Output #0 → meta.indexer_hint (T2)`
- ARBDA:
  - `T2`

---

### B3 – Short ASCII Embed

**Setup**

- Clear.
- Output #0:
  - Type: `OP_RETURN`
  - Hex (10-byte `"helloworld"` payload):

```
6a0a68656c6c6f776f726c64
```

**Expected**

- Breakdown:
  - `Output #0 → da.op_return_embed (T2)`
- ARBDA:
  - `T2`

---

### B4 – Medium Non-ASCII Embed (non-rollup)

**Setup**

- Clear.
- Output #0:
  - Type: `OP_RETURN`
  - Hex (40-byte binary payload, non-rollup size):

```
6a281111111111111111111111111111111111111111111111111111111111111111111111
```

**Expected**

- Breakdown:
  - `Output #0 → da.op_return_embed (T2)`
- ARBDA:
  - `T2`

---

### B5 – Large Embed → Misc

**Setup**

- Clear.
- Output #0:
  - Type: `OP_RETURN`
  - Hex: `6a5a` + 90 bytes of ASCII `'A'` (`0x41`):

```
6a5a41414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141
```

**Expected**

- Breakdown:
  - `Output #0 → da.embed_misc (T2)`
- ARBDA:
  - `T2`

---

## 4. Group C – Witness Lane (non-ordinal)

### C1 – Small ASCII Vendor Data

**Setup**

- Clear.
- Output #0: `P2PKH` (template).
- Witness #0 hex:

```
30313233343536373839
```

(ASCII `"0123456789"`.)

**Expected**

- Breakdown:
  - `Witness[0:0] → da.unregistered_vendor (T3)`
  - `Output #0 → pay.standard (T1)`
- ARBDA:
  - `T3`

---

### C2 – Small Random Blob

**Setup**

- Clear.
- Output #0: `P2PKH`.
- Witness #0 hex:

```
0102030405060708090a
```

**Expected**

- Breakdown:
  - `Witness[0:0] → da.unknown (T3)`
- ARBDA:
  - `T3`

---

### C3 – Medium ASCII Vendor Data

**Setup**

- Clear.
- Output #0: `P2PKH`.
- Witness #0 hex (**exact string**):

```
41414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141
```

(120 bytes of ASCII `'A'`.)

**Expected**

- Breakdown:
  - `Witness[0:0] → da.unregistered_vendor (T3)`
- ARBDA:
  - `T3`

---

### C4 – Large Non-ASCII Blob

**Setup**

- Clear.
- Output #0: `P2PKH`.
- Witness #0 hex (**exact string**):

```
000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
```

(~600 bytes of `0x00`.)

**Expected**

- Breakdown:
  - `Witness[0:0] → da.obfuscated (T3)`
- ARBDA:
  - `T3`

---

## 5. Group D – Ordinals / Inscriptions (witness lane)

### D1 – Small Ordinal Marker

**Setup**

- Clear.
- Output #0: `P2PKH`.
- Witness #0 hex:

```
0000006f7264
```

(`6f7264` = ASCII `"ord"`.)

**Expected**

- Breakdown:
  - `Witness[0:0] → meta.ordinal (T2)`
- Tiers present:
  - `T1`, `T2`
- ARBDA:
  - `T2`

---

### D2 – Large Inscription-style Blob

**Setup**

- Clear.
- Output #0: `P2PKH`.
- Witness #0 hex: large blob containing `6f7264` and >256 bytes total
  (for manual testing you can reuse C4’s big `00…00` and append `6f7264`).

**Expected**

- Breakdown:
  - `Witness[0:0] → meta.inscription (T2)`
- Tiers present:
  - `T1`, `T2`
- ARBDA:
  - `T2`

---

## 6. Group E – Mixed-Region ARBDA Behaviour

### E1 – Pure Economic

Same as **A1**.

**Expected**

- Breakdown:
  - `Output #0 → pay.standard (T1)`
- ARBDA:
  - `T1`

---

### E2 – Economic + Metadata (No T3)

**Setup**

- Clear.
- Output #0: `P2PKH`.
- Output #1:
  - Type: `OP_RETURN`
  - Hex:

```
6a026f6b
```

- No witness.

**Expected**

- Breakdown:
  - `Output #0 → pay.standard (T1)`
  - `Output #1 → meta.indexer_hint (T2)`
- Tiers present:
  - `T1`, `T2`
- ARBDA:
  - `T2`

---

### E3 – Economic + Metadata + Obfuscated Blob

**Setup**

- Start from **E2**.
- Add Witness #0 with the large `00…00` blob from **C4**.

**Expected**

- Breakdown includes:
  - `Output #0 → pay.standard (T1)`
  - `Output #1 → meta.indexer_hint (T2)`
  - `Witness[0:0] → da.obfuscated (T3)`
- Tiers present:
  - `T1`, `T2`, `T3`
- ARBDA:
  - `T3`

---

## 7. Notes & Future Extensions

- This matrix focuses on:
  - Clear, canonical behaviours for each label/tier.
  - Demonstrating ARBDA’s “worst tier wins” logic.
- It does **not yet** include:
  - Vault contracts (`contracts.vault`).
  - Channel open/update patterns (`pay.channel_open`, `pay.channel_update`).
  - Detailed `meta.pool_tag` detection in coinbase.

Those can be added as additional test groups if/when the Tag Engine is extended
with richer script parsing or real-node integration.

For BUDS LAB v2, this matrix is the **canonical reference** used to validate
the heuristics in `buds-tag-engine.js`.
