# Tagging Method (Non-Normative)

This document describes a simple, optional approach for scanning a Bitcoin
transaction and assigning BUDS labels to its regions.  
Implementations may use this method, extend it, or replace it entirely.

BUDS does not mandate any tagging logic.

---

## 1. Goal

Identify **regions** inside a transaction and assign **labels** that describe the
data found in those regions.

A region is defined by:

- the surface (e.g., `witness.stack[0]`, `scriptsig`, `scriptpubkey[n]`),
- a byte range,
- one or more labels.

Example:

```
label: da.obfuscated
surface: witness.stack[1]
range: 0–2048
```


---

## 2. Surfaces to Scan

Implementations typically walk these surfaces:

- **Input surfaces**
  - `scriptsig`
  - `witness.stack[i]`
  - `witness.script`

- **Output surfaces**
  - `scriptpubkey[n]`
  - `op_return`
  - `segop` (if present)

- **Transaction-level surfaces**
  - version, locktime, sequence (usually not labelled beyond consensus)

Each surface may contain validation data, structured metadata, or arbitrary data.

---

## 3. Example Tagging Flow

A simple reference flow is:

### **Step 1 — Identify structural patterns**

Check whether the region matches a known pattern such as:

- standard payments,
- Lightning channel open templates,
- tapscript programs,
- OP_RETURN formats,
- known inscription / ordinal patterns,
- mining pool tags,
- rollup commitments.

When matched, assign the appropriate registry label(s).

Example:

```
region: scriptpubkey[0]
pattern: 2-of-2 channel template
label: pay.channel_open
```


---

### **Step 2 — Identify consensus-relevant regions**

Regions that parse as:

- signatures,
- public keys,
- executed script fragments,
- valid taproot scripts/programs,

may be tagged using consensus-oriented labels such as:

- `consensus.sig`
- `consensus.script`
- `consensus.taproot_prog`

This tagging is descriptive only.

---

### **Step 3 — Identify metadata / application data**

Regions that contain:

- small intentional payloads,
- OP_RETURN data,
- structured messages,
- indexer hints,

may be labelled:

- `da.op_return_embed`
- `meta.inscription`
- `meta.indexer_hint`
- `da.embed_misc`

based on context or recognisable structure.

---

### **Step 4 — Detect unknown or obfuscated data**

If a region:

- is unusually large,
- appears opaque or high-entropy,
- does not match any known structure,
- is located in a place not typical for validation data (e.g., large witness items),
- appears to contain application payloads but no identifiable pattern,

implementations may assign a generic label such as:

- `da.unknown`
- `da.obfuscated`

This is optional and purely heuristic.

---

## 4. Multiple Labels per Region

A region may receive more than one label.  
Example:

```
labels: ["da.op_return_embed", "meta.inscription_like"]
surface: scriptpubkey[1]
```


Applications and node policy modules can choose which labels matter.

---

## 5. Non-Goals

This method does **not**:

- state what counts as “spam,”  
- assign tiers automatically,  
- impose policies,  
- require protocols to self-identify,  
- guarantee correct interpretation.

All classification is **local**, and disagreements between nodes are expected.

---

## 6. Output Format (Example)

An implementation may return tags in a format similar to:

```json
[
  {
    "surface": "witness.stack[0]",
    "start": 0,
    "end": 71,
    "labels": ["consensus.sig"]
  },
  {
    "surface": "scriptpubkey[1]",
    "start": 0,
    "end": 80,
    "labels": ["da.op_return_embed"]
  }
]
```

Output format is not standardised.
Nodes may expose tags through logs, RPCs, or internal APIs.

---

## 7. Summary

- Tagging is a local classification step.
- BUDS only provides the labels and definitions.
- Implementations may use structural detection, heuristics, or proprietary logic.
- Tags are descriptive and optional, not authoritative.

For how tags may integrate into node policy, see `policy-interface.md`.
