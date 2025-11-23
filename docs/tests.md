# BUDS Test Suite

This document describes the minimal C++ test suite for the BUDS Tagger (reference implementation).

The goal of the tests is to:

- exercise the **basic classification pipeline**,
- verify that key labels (`pay.standard`, `da.op_return_embed`,
  `da.unknown`, `da.obfuscated`) behave as expected, and
- provide concrete **test vectors** that can be reused in other
  implementations and in the BIP text.

---

## Files

- `tests/test_buds_tagger.cpp`  
  A standalone test runner with several small tests. It uses a very simple
  assertion macro and returns:

  - `0` on success (all tests passed),
  - `1` on failure.

- `src/buds_labels.*`, `src/buds_tagging.*`  
  The reference Tagger and classification types used by the tests.

---

## Building and running the tests

From the repo root:

```
g++ -std=c++17 -Isrc \
    tests/test_buds_tagger.cpp \
    src/buds_labels.cpp \
    src/buds_tagging.cpp \
    -o buds-tests

./buds-tests
```

Expected output (example):

```
[TEST] P2PKH -> pay.standard
[TEST] OP_RETURN -> da.op_return_embed
[TEST] small witness blob -> da.unknown
[TEST] large witness blob -> da.obfuscated
[TEST] mixed tx (P2PKH + OP_RETURN + large witness)
All BUDS tagger tests PASSED.
```

# Test Cases

## 1. `Test_P2PKH_is_pay_standard`

### **Shape**

`SimpleTx` with:

- `txid = "test-p2pkh"`
- one `vout`:
  - `scriptPubKey.asm = "OP_DUP OP_HASH160 <pubkeyhash> OP_EQUALVERIFY OP_CHECKSIG"`
  - `scriptPubKey.hex = "76a91400112233445566778899aabbccddeeff0011223388ac"`

### **Expectation**

One tag:

- `surface = "scriptpubkey[0]"`
- first label = **`pay.standard`**

**Meaning:**  
The Tagger recognises standard P2PKH and treats it as a normal payment lane (**Tier T1**).

---

## 2. `Test_OpReturn_is_da_op_return_embed`

### **Shape**

`SimpleTx` with:

- `txid = "test-opreturn"`
- one `vout`:
  - `scriptPubKey.asm = "OP_RETURN 6f6b"`
  - `scriptPubKey.hex = "6a026f6b"`

### **Expectation**

One tag:

- `surface = "scriptpubkey[0]"`
- first label = **`da.op_return_embed`**

**Meaning:**  
The Tagger recognises OP_RETURN outputs and classifies them as embedded data (**Tier T2**).

---

## 3. `Test_Witness_small_is_da_unknown`

### **Shape**

`SimpleTx` with:

- `txid = "test-wit-small"`
- one witness entry:
  - `stack_items_hex = ["0011223344556677"]` (8 bytes)

### **Expectation**

One tag:

- `surface = "witness.stack[0:0]"`
- first label = **`da.unknown`**

**Meaning:**  
Small witness items (≤ 512 bytes) are treated as unknown data (**Tier T3**),  
but are tracked separately from large blobs.

---

## 4. `Test_Witness_large_is_da_obfuscated`

### **Shape**

`SimpleTx` with:

- `txid = "test-wit-large"`
- one witness entry:
  - `stack_items_hex = "<600 bytes of 0x00>"` (1200 hex chars)

### **Expectation**

One tag:

- `surface = "witness.stack[0:0]"`
- first label = **`da.obfuscated`**

**Meaning:**  
Large witness blobs are classified as **obfuscated data** (Tier T3) and receive heavier penalties.

---

## 5. `Test_Mixed_tx_has_all_expected_tags`

### **Shape**

`SimpleTx` with:

- `txid = "test-mixed"`
- two outputs:
  - P2PKH → **`pay.standard`**
  - OP_RETURN → **`da.op_return_embed`**
- one witness entry:
  - large blob → **`da.obfuscated`**

### **Expectation**

Three tags (one per region).

Across all labels:

- at least one **`pay.standard`**
- at least one **`da.op_return_embed`**
- at least one **`da.obfuscated`**

**Meaning:**  
The Tagger correctly handles mixed transactions and produces multiple labels covering different surfaces.

---

# Relationship to BUDS LAB

These C++ tests mirror the behaviours demonstrated in **BUDS LAB**:

- OP_RETURN → `da.op_return_embed`
- P2PKH → `pay.standard`
- small witness items → `da.unknown`
- large witness blobs → `da.obfuscated`

These examples double as **canonical test vectors** for:

- other language implementations of the BUDS Tag Engine,
- node/policy integrations,
- and the **BUDS BIP** “Test Vectors” section.
