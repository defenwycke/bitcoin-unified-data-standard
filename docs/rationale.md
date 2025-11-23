# Rationale

Bitcoin has always carried more than pure payment data.  
Signatures, scripts, channel state, rollup roots, inscriptions, messages, pool tags, hints, and vendor-specific blobs all appear in real transactions. The debate is not *whether* arbitrary data exists — it always has — but how nodes should understand and manage it.

Today, every node sees transaction data as an undifferentiated byte stream.  
Some call any non-payment data “spam”. Others rely on metadata for Lightning, vaults, sidechains, indexers, or inscriptions. With no shared definitions, arguments become political and technical discussions stall.

**BUDS solves this by standardising the vocabulary.**

---

## The Problem

1. **Bitcoin transactions contain many types of data**, but there is no shared language to describe them.
2. **Node policy is blind.** Without classification, nodes treat:
   - a Lightning channel open,
   - an inscription,
   - a rollup commitment,
   - a pool tag,
   - and 200 kB of random junk  
   as if they were all the same thing.
3. **Spam debates become circular**, because “spam” is undefined.
4. **Nodes cannot easily apply local preferences** without risking breakage or misidentifying useful protocols.
5. **Arbitrary data is not going away.** Trying to outlaw it at consensus level only fuels conflict and encourages obfuscation.

---

## The BUDS Approach

BUDS introduces **no consensus rules** and **no required policy**.  
It simply provides a clean, minimal model:

1. **Describe data types** → a registry of known labels.  
2. **Classify conceptually** → four broad categories:
   - **T0:** consensus-critical  
   - **T1:** economic/system  
   - **T2:** metadata/application  
   - **T3:** unknown/obfuscated  
3. **Scan and tag transaction regions** using local logic.  
4. **Let nodes apply their own local policy** (feerate rules, block template preference, pruning, etc.).

This turns “spam” from an emotional argument into a **technical classification problem**.

---

## Why This Matters

- **Arbitrary data does have a place on Bitcoin**, but not all data is equal.  
  Lightning channels and rollup roots clearly provide system value.  
  Large opaque blobs may not.

- **Nodes gain visibility**. Instead of guessing, a node can see:
  - “This input carries channel state.”  
  - “This output is indexing metadata.”  
  - “This witness item is large, unknown data.”

- **Miners and node operators gain control.**  
  With labels, they can set precise policy:
  - prioritise economic data,
  - allow metadata up to a chosen limit,
  - require higher fees for unknown payloads,
  - or ignore all of it entirely.

- **Protocols gain clarity.**  
  Honest projects can register labels so they are recognised.  
  Dishonest or abusive patterns get classified as unknown/obfuscated.

- **No one is censored.**  
  BUDS does not ban anything.  
  It standardises *descriptions*, not *decisions*.

---

## Why Not Consensus Rules?

Consensus-based limits on data types lead to:

- permanent ossification around one political view,
- breakage for legitimate L2/L3 protocols,
- incentives for obfuscation and hiding data,
- brittle arguments about what is “allowed” or “spam”.

BUDS avoids all of this.  
It provides structure without coercion.

---

## Summary

BUDS exists because Bitcoin needs a **neutral framework** to understand the data it already carries.  
With shared labels and simple categorisation, node operators can set their own policies, protocols can be recognised cleanly, and spam mitigation becomes a local and transparent choice — not a global fight.

