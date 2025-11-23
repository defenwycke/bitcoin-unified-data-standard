# Policy Interface (Non-Normative)

BUDS does not define node policy.  
It only provides **labels** that nodes may use when applying their own local rules.

This document describes how implementations *may* integrate BUDS tags into
mempool, feerate, block template, and pruning decisions.  
All examples are optional.

---

## 1. Inputs to Policy

After scanning a transaction, an implementation has:

- A set of **regions** extracted from the transaction.
- A set of **labels** assigned to each region.
- Optional **local tier assignments** (T0–T3).

Policy modules can treat these as hints or ignore them entirely.

---

## 2. Common Policy Hooks

Nodes may use BUDS labels at four decision points:

### **2.1 Mempool Admission**
Nodes may define local rules such as:

- Require a minimum feerate for certain labels.
- Reject or deprioritise transactions dominated by unknown/obfuscated data.
- Apply custom logic for specific protocols (e.g., Lightning, vaults).

Example (non-normative):

> “If any region is tagged `da.obfuscated`, require a higher feerate.”

---

### **2.2 Feerate Calculation**
Nodes may adjust effective feerate during transaction sorting.

Examples (non-normative):

- Increase priority for `pay.channel_open`.
- Apply a multiplier for `da.unknown`.
- Leave all labels neutral.

No behaviour is required.

---

### **2.3 Block Template Construction**
Miners may choose to:

- Prefer certain labels when blocks are full.
- Cap the weight contribution of specific label groups.
- Ignore labels entirely.

Example (non-normative):

> “Include as much T1 data as fits; include T2 if fees are high enough.”

Again, this is optional and entirely local.

---

### **2.4 Pruning and Retention**
Nodes may use labels to decide:

- Which data to retain during pruning.
- Which regions to discard first.
- Whether to keep metadata used by an application.

Example (non-normative):

> “Retain consensus and T1 regions; prune T2/T3 when space is tight.”

---

## 3. Local Tier Mapping

Nodes may define their own mapping of labels → tiers (T0–T3).  
Suggested mappings are provided in `classification-table.md`, but implementations
are free to override them.

No default tiering is enforced.

---

## 4. Example Policy Config (Optional)

This is only to illustrate how tags may be referenced:

```ini
[label "da.obfuscated"]
min_feerate_multiplier = 3.0
block_weight_soft_cap   = 0.05

[label "pay.channel_open"]
min_feerate_multiplier = 1.0
priority_boost          = 0.1
```

Nodes may use completely different structures, files, or heuristics.

---

## 5. Guarantees

BUDS guarantees:
- No consensus changes.
- No enforced behaviour.
- No required interpretation of tags.
- Full freedom for node operators.

BUDS simply provides a shared vocabulary so policy engines can make informed, local decisions.
