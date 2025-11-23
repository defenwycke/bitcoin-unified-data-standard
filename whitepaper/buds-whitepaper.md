# Bitcoin Unified Data Standard (BUDS)
### A Minimal Framework for Classifying Bitcoin Transaction Data
### Draft – 2025-11-23

## 1. Introduction

Bitcoin transactions contain many types of data: signatures, scripts,
channel state, vault logic, mining metadata, inscriptions, indexer hints,
and arbitrary payloads. These appear in different regions (“surfaces”) of a
transaction and carry different levels of importance.

Today, nodes treat almost all of this as undifferentiated bytes. As a result:

- “spam” is undefined,
- useful protocols rely on brittle heuristics,
- node policy cannot distinguish economic-critical data from noise,
- debates about arbitrary data become political instead of technical.

BUDS provides a **neutral, minimal, non-consensus** framework for describing,
classifying, and tagging transaction data.

Its purpose is simple:

**Give Bitcoin a shared vocabulary and let node operators decide policy for themselves.**

---

## 2. Goals

BUDS aims to be extremely small:

- define **standard labels** for data types,
- group them into **conceptual categories** (T0–T3),
- describe how nodes can **tag regions** of transactions,
- allow **optional** node/miner policy based on tags,
- make **no** consensus changes,
- avoid prescribing any fee or block rules.

BUDS is descriptive, not prescriptive.

---

## 3. The Four-Step Model

### 1. Describe  
Standardised labels defined in a public registry.

### 2. Classify  
Conceptual grouping into:

```
T0 – Consensus-critical  
T1 – Economic / system-critical  
T2 – Metadata / application  
T3 – Unknown / obfuscated  
```

These are optional hints only.

### 3. Scan & Tag  
Nodes inspect transaction surfaces and attach labels using:

- structural recognition,
- protocol patterns,
- heuristics.

### 4. Local Policy  
Nodes may use tags for:

- mempool rules,
- fee scoring,
- block selection,
- pruning.

All optional.  
No behaviour is required.

---

## 4. Why Bitcoin Needs BUDS

### Arbitrary Data Already Exists

Bitcoin has always carried arbitrary data:

- redeem scripts,  
- OP_RETURN metadata,  
- witness pushes,  
- inscriptions,  
- channel states,  
- pool tags,  
- rollup roots.

Ignoring this doesn’t prevent data usage — it only hides it.

### The Real Issue: No Shared Language

Without a standard vocabulary:

- node policy becomes brittle and inconsistent,
- classification becomes guesswork,
- “spam” becomes an emotional term.

BUDS introduces a neutral, shared language.

### Spam Mitigation via Visibility, Not Enforcement

BUDS does **not** attempt to ban data.  
Instead, it makes data **legible**, enabling:

- visibility into data purpose,
- separation of economic-critical vs optional,
- optional local spam mitigation.

Nodes keep complete sovereignty.

---

## 5. Registry

Canonical labels exist in:

```
registry/registry.json
```

Examples:

```
consensus.sig  
consensus.script  
pay.standard  
pay.channel_open  
commitment.rollup_root  
meta.inscription  
meta.indexer_hint  
da.op_return_embed  
da.unknown  
da.obfuscated  
```

Registration is optional.

---

## 6. Tagging

Nodes break transactions into surfaces:

- scriptsig  
- witness.stack[i]  
- witness.script  
- scriptpubkey[n]  
- op_return  
- coinbase  

Then assign labels to byte ranges.

Example:

```
{
  "surface": "witness.stack[1]",
  "start": 0,
  "end": 4096,
  "labels": ["da.obfuscated"]
}
```

Tagging is local.  
Nodes may disagree — and that’s acceptable.

---

## 7. Policy (Optional)

BUDS **does not**:

- set fee multipliers,
- create caps,
- require censorship,
- modify consensus.

Nodes MAY use labels for:

- fee scoring,
- mempool behaviour,
- block template preference,
- pruning.

Or ignore BUDS entirely.

BUDS enables policy; it does not define it.

---

## 8. Benefits

### For Node Operators
- Greater visibility  
- Optional spam mitigation  
- Full freedom

### For Miners
- Cleaner block template logic  
- More predictable fee markets

### For Protocols
- Reduced misclassification  
- Predictable treatment of structured data

### For the Ecosystem
- Shared language  
- Reduced political noise  
- Zero consensus risk

---

## 9. Scope & Non-Goals

BUDS **does not**:

- restrict what users put in transactions,
- assume users tell the truth,
- enforce node policy,
- modify Bitcoin’s rules,
- define “spam”.

Its job:

**Define labels → Enable classification → Leave all policy to nodes.**

---

## 10. Conclusion

BUDS is a minimal, neutral framework for describing Bitcoin transaction data.
It reflects real Bitcoin usage without restricting it, empowers node operators,
and improves clarity — all without touching consensus.

BUDS is a vocabulary, not a rulebook.
