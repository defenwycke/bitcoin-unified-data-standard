# ARBDA – Arbitrary Data Dominance Assessment

ARBDA (Arbitrary Data Dominance Assessment) is an optional **transaction-level risk rating** built *on top of* BUDS classifications.

BUDS labels and tiers describe **regions of a transaction**.  

ARBDA answers a different question:

> **“Given all the data inside this transaction, how risky is the transaction *as a whole* from an arbitrary-data perspective?”**

ARBDA uses a deliberately conservative rule:

> **If any T3 (unknown / obfuscated) data is present anywhere, the entire transaction is ARBDA = T3.**  
>  
> *Guilty until proven innocent.*  
>  
> If a protocol wants better risk treatment, it must present its data in a structured and recognisable form.

ARBDA is *not part of consensus*.  
It is a **local policy scoring layer** that node operators and miners can use when prioritising transactions.

---

## 1. Inputs

ARBDA consumes the output of the BUDS Tag Engine:

- Per-region labels (e.g., `pay.standard`, `da.op_return_embed`, `da.unknown`, `da.obfuscated`)
- Tier summary:
  - which tiers are present: `T0, T1, T2, T3`
  - counts per tier

ARBDA ignores label details and focuses only on the **tier mix**.

---

## 2. ARBDA Tier Rule

Compute `arbda_tx_tier` using:

```
if has_T3:      arbda_tx_tier = T3
else if has_T2: arbda_tx_tier = T2
else if has_T1: arbda_tx_tier = T1
else:           arbda_tx_tier = T0
```

### Meaning:

- A tx with **any T3** (unknown/obfuscated) → **ARBDA = T3**
- A tx with only **T2/T1/T0** → **ARBDA = T2**
- A tx with only payment & consensus data → **ARBDA = T1/T0**

### Motivation:

- If a transaction mixes legitimate structures with opaque blobs,
  the opaque data dominates the risk profile.
- Real protocols that want lower scores should:
  - follow predictable patterns,
  - or register proper labels in the BUDS registry.

---

## 3. Intended Use (non-normative)

Node operators, miners, and policy engines can use `arbda_tx_tier` to:

### **Fee scaling**
Increase required feerates for ARBDA-T3 transactions.

### **Block template composition**
Limit ARBDA-T3 transactions to:
- a percentage of block weight, or
- a maximum per block.

### **Mempool policy**
Preferentially evict or deprioritise ARBDA-T3 in low-feerate eviction.

### **Transparency**
Clearly show users:
- "your transaction is ARBDA-T3 because it contains obfuscated data".

---

## 4. Relationship to BUDS

ARBDA does **not** replace BUDS tiers.

- BUDS → labels **per region**
- ARBDA → **one score per transaction**

This keeps BUDS neutral and descriptive, while ARBDA provides an optional “risk lens” that many node operators will want.

---

## 5. How ARBDA interacts with the registry

As protocols mature:

- They register structured labels (e.g., `contracts.channel_open`, `meta.pool_tag`)
- Implementations recognise them as T1/T2 instead of T3
- ARBDA automatically improves from T3 → T2/T1

This creates a **clear incentive** for protocols to be transparent instead of hiding in giant blobs.

