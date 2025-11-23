# Registry Process

The BUDS registry lists known data type labels and their intended meanings.  
It exists so node implementations can recognise common patterns and apply local policy consistently.

Registration is optional.  
Unregistered data types continue to work normally and may still be classified locally.

This document describes the simple process for adding new labels.

---

## 1. How to Propose a New Label

To propose a new data type label:

1. Open a pull request modifying `registry/registry.json`.
2. Add a new entry with:
   - **`label`** – the canonical name (e.g., `meta.protocol_v1`).
   - **`description`** – short explanation of intended use.
   - **`surfaces`** – where the data typically appears (e.g., `witness`, `scriptPubKey`, `op_return`).
   - **`suggested_category`** – optional hint (T0–T3).  
     This is non-binding and may be ignored by implementations.
3. Provide a short example or reference if helpful.

Labels should be concise, descriptive, and protocol-agnostic where possible.

---

## 2. What Registration Does *Not* Do

Registering a label:

- does **not** grant priority,
- does **not** enforce any tier (T0–T3),
- does **not** guarantee favourable treatment,
- does **not** change consensus rules,
- does **not** require other nodes to recognise or trust the label.

Registration only adds the label to the shared vocabulary.

---

## 3. When to Register a Label

Registration is recommended when:

- A protocol wants nodes to understand its data structure.
- A pattern appears frequently across the network.
- A label would reduce misclassification or reliance on heuristics.

Registration is *not* required for experimentation, private use, or local logic.

---

## 4. Review Criteria

PRs adding new labels are assessed on:

- **Clarity** — is the description understandable?
- **Uniqueness** — does it duplicate an existing label?
- **Neutrality** — is the label general enough for future use?
- **Scope** — is this a data type, not a policy rule?

There is no judgement about whether a protocol is “useful” or “valid”.  
The registry is descriptive, not authoritative.

---

## 5. Removal or Modification

Labels may be updated or deprecated if:

- The description becomes inaccurate,
- A protocol evolves,
- A better general-purpose label exists.

Deprecation does not impact consensus or policy.  
Nodes may continue to recognize older labels if desired.

---

## 6. Unregistered Data

Anything not in the registry:

- remains valid Bitcoin data,
- may still be recognised structurally by implementations,
- may be classified under generic labels such as `da.unknown` or `da.obfuscated`.

Registration is encouraged but never required.


