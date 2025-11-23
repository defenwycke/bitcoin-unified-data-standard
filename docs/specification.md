# BUDS Specification

The Bitcoin Unified Data Standard (BUDS) defines a minimal, optional framework for
**describing**, **classifying**, and **tagging** data that appears inside Bitcoin transactions.

BUDS does not modify Bitcoin consensus or enforce any node policy.  
It provides a shared vocabulary so implementations can reason about transaction data consistently.

---

## 1. Definitions

### **1.1 Label**
A **label** is a canonical name describing a type of data that may appear inside a transaction.

Examples:  
`pay.standard`, `meta.inscription`, `commitment.rollup_root`, `da.obfuscated`.

The full set of known labels is defined in `registry/registry.json`.

Labels describe **intent**, not guaranteed truth.

---

### **1.2 Region**
A **region** is a byte range inside a transaction located within a specific surface, such as:

- `scriptsig`
- `witness.stack[n]`
- `witness.script`
- `scriptpubkey`
- `op_return`
- `segop`
- `coinbase`
- `tx-level` fields (version, locktime, sequence)

A region may contain one or more labels.

---

### **1.3 Tag**
A **tag** is the association of:

- a region  
- one or more labels  

Tags are produced by the classification process described in this specification.

---

### **1.4 Suggested Categories (T0–T3)**  
BUDS defines four **conceptual categories**, used only as guidance:

- **T0 Consensus-critical**  
- **T1 Economic/System-critical**  
- **T2 Metadata/Application**  
- **T3 Unknown/Obfuscated**

These categories are **non-normative**.  
Implementations may ignore them or reassign labels arbitrarily.

---

## 2. Registry

The BUDS registry defines the set of known labels.

- Stored in: `registry/registry.json`
- Each entry includes:
  - `label`
  - `description`
  - `surfaces`
  - `suggested_category` (optional hint)

Nodes may rely on registry information, ignore it, or override it.

The registration process is described in `registry-process.md`.

---

## 3. Classification

Classification is the process of identifying regions of a transaction and associating labels.

BUDS does **not** prescribe how implementations perform classification.  
Nodes are free to use:

- protocol-specific patterns,
- structural recognition,
- heuristics,
- or private logic.

A minimal reference approach is documented in `tagging-method.md`.

Classification SHOULD produce:

- a list of regions
- zero or more labels per region

Implementations may assign multiple labels to the same region.

---

## 4. Tagging Interface

Tagging is the output of classification.  
A tag is defined as:

```
{
"surface": "witness.stack[1]",
"start": 0,
"end": 123,
"labels": ["da.obfuscated"]
}
```

Implementations MAY:

- provide tags to policy modules,
- expose tags through RPCs or logs,
- or ignore tagging entirely.

Tags have no effect on consensus validation.

---

## 5. Policy Integration (Optional)

Nodes and miners MAY use labels or categories as inputs to local policy, including:

- mempool admission,
- feerate adjustments,
- block template construction,
- pruning/retention rules.

BUDS does **not** define any policy mechanics, thresholds, or required behaviour.

Example usage (non-normative) is in `policy-interface.md`.

---

## 6. Non-Goals

BUDS does **not**:

- define consensus rules,
- enforce fee policies,
- mandate censorship or prioritisation,
- restrict transaction formats,
- require the use of labels,
- guarantee the accuracy of classification.

BUDS is descriptive, not prescriptive.

---

## 7. Security and Privacy Notes

- Misclassified data does not affect consensus validity.
- Nodes may disagree on tags or classifications.
- Classification heuristics must not assume correctness of user-provided data.
- Implementations should avoid leaking sensitive information when exposing tags.

---

## 8. Reference Implementations

Non-normative example code is provided in:

```
src/buds_labels.*
src/buds_tagging.*
src/buds_policy_example.*
```

These are illustrative only.

---

## 9. Compatibility

- Fully compatible with all current and future Bitcoin transactions.
- No changes required to transaction format, consensus rules, or script.
- Nodes may adopt or ignore BUDS independently.

---

## 10. Summary

BUDS provides:

- a standard data type registry,
- a simple classification model,
- a neutral tagging interface,
- optional hooks for local policy.

Everything else remains under full control of the node operator.

