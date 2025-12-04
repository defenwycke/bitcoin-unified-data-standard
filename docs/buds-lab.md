# BUDS LAB – Tag Engine Playground

BUDS LAB is a small, browser-based playground for the **Bitcoin Unified Data Standard (BUDS)**.

It lets you:

- Build or paste a simple Bitcoin transaction structure.
- Run the **BUDS Tag Engine** on it.
- Inspect:
  - per-region labels (e.g. `pay.standard`, `da.op_return_embed`, `da.obfuscated`),
  - conceptual **tiers** (`T0–T3`),
  - and an **example local node policy** (fee multipliers and scores).

BUDS LAB is **non-consensus** and **non-normative**.  
It is for experimentation, education, and demonstrating how BUDS could be used inside node policy, not a production policy engine.

---

## 1. Layout

BUDS LAB is a single page with two main panels:

### Left: Transaction Input

Two modes:

- **Builder mode**
  - Set a `TXID`.
  - Add **Outputs (scriptPubKey)**:
    - Choose a template:
      - `P2PKH`
      - `P2WPKH` (placeholder template)
      - `P2TR` (placeholder template)
      - `OP_RETURN`
      - or `Custom`
    - Each output has:
      - `ASM` – human-readable script, and
      - `Hex` – raw `scriptPubKey` hex.
  - Add **Witness (stack items)**:
    - Hex-encoded stack elements.
    - Large items (> 512 bytes) are treated as potential data blobs.

- **JSON mode**
  - Paste a `SimpleTx`-style JSON object:

```
    {
      "txid": "…",
      "vout": [
        {
          "scriptPubKey": {
            "asm": "…",
            "hex": "…"
          }
        }
      ],
      "witness": [
        {
          "stack": ["…"]
        }
      ]
    }
```

  - This mirrors the in-memory structure used by the reference Tag Engine.

The top of the panel has:

- `Builder` / `JSON` toggle
- `Load example`
- `Clear`

### Right: BUDS Tag Engine Output

Contains:

1. **Node Policy & Fees (example)**
   - `Base mempool min (sat/vB)` – baseline local minimum feerate.
   - `Tx feerate (sat/vB)` – feerate for the transaction under test.
   - `Policy profile` – one of:
     - `neutral` – mild penalties for unknown/obfuscated data.
     - `strict` – heavy penalties for unknown/obfuscated data.
     - `permissive` – light penalties, more tolerant.
   - These are **local policy knobs**, not consensus rules.

2. **Classification summary**
   - All unique labels present (e.g. `pay.standard`, `da.op_return_embed`, `da.obfuscated`, `da.unknown`).
   - **Triage tiers present** (see below): some subset of `T0, T1, T2, T3`.
   - Short reminder:
     - `T0` – consensus / validation-critical.
     - `T1` – economic / Bitcoin-critical data.
     - `T2` – metadata / hints / optional.
     - `T3` – unknown / obfuscated / likely spam.

3. **Example policy result**
   - Using the selected profile and fee knobs, the engine reports:
     - `base mempool min` – input baseline.
     - `tx feerate` – input.
     - `min multiplier` – maximum multiplier implied by all labels.
     - `required feerate` – `base_min * min_multiplier`.
     - `effective score` – `tx_feerate * (1 + boostSum)`, where `boostSum`
       is an aggregate of label-specific boosts/penalties.
   - This is a **demo of how a node *could* use BUDS labels**, not a recommendation.

4. **Tags panel**
   - A line per tagged region, for example:

```
     surface=scriptpubkey[0]  range=[0,25)  labels=[pay.standard]
     surface=scriptpubkey[1]  range=[0,4)   labels=[da.op_return_embed]
     surface=witness.stack[0:0]  range=[0,600)  labels=[da.obfuscated]
```

5. **ARBDA transaction tier**

BUDS LAB also computes an **ARBDA (Arbitrary Data) tier** for the whole transaction:

- If any `T3` data is present → `ARBDA = T3`  
- Else if any `T2` data is present → `ARBDA = T2`  
- Else if any `T1` data is present → `ARBDA = T1`  
- Else → `ARBDA = T0`

This reflects a conservative “guilty until proven innocent” rule for arbitrary/obfuscated data. 
Even if a transaction has many T0/T1 regions, a single T3 blob is enough to make the whole tx `ARBDA = T3`.

The ARBDA tier is shown in the LAB summary and included in the JSON export (`"arbdaTier"` field).

---

## 2. BUDS Tag Engine (browser version)

The browser Tag Engine (`buds-lab/buds-tag-engine.js`) is a **reference implementation** of the BUDS classification pipeline.

### 2.1 Surfaces and regions

For now, the engine looks at:

- `scriptpubkey[n]` – each transaction output.
- `witness.stack[i:j]` – each witness stack item.

Each **tag** includes:

- `surface` – which region,
- `start`, `end` – byte range (currently always `[0, length)`),
- `labels[]` – one or more BUDS labels.

### 2.2 ScriptPubKey classification

For each `scriptpubkey[n]`, the demo Tag Engine applies a small set of
non-consensus heuristics. These are meant to roughly mirror what many wallets
and nodes would already infer from standard script templates.

1. **Detect OP_RETURN lane**

   If the script is recognised as `OP_RETURN` (ASM starts with `OP_RETURN` or
   hex starts with `6a…`), the engine:

   - extracts the pushed payload (skipping `OP_RETURN` + 1 push-length byte),
   - inspects payload length and whether it looks like mostly-ASCII text.

   It then chooses:

   - `commitment.rollup_root`  
     - if the script looks like `6a20……` carrying a ~32-byte blob  
     - intended to approximate “L2 / rollup root commitment” use cases.

   - `meta.indexer_hint`  
     - if payload is **≤ 8 bytes** and mostly ASCII  
     - think short flags, version markers, or “tag” strings.

   - `da.op_return_embed`  
     - if payload is ASCII and **up to ~32 bytes**, or a short/medium blob  
     - general human-readable or small structured metadata.

   - `da.embed_misc`  
     - if payload is larger than ~80 bytes  
     - catch-all for bulk metadata that is still explicitly embedded via OP_RETURN.

2. **Detect standard payment templates**

   If the script is **not** OP_RETURN, the engine tries common payment forms:

   - P2PKH  
     - hex length `25 bytes` (`50` hex chars)  
     - prefix `76a914`, suffix `88ac`.

   - P2WPKH  
     - hex length `22 bytes` (`44` hex chars)  
     - prefix `0014`.

   - P2TR  
     - hex length `34 bytes` (`68` hex chars)  
     - prefix `5120`.

   Any of these patterns → label:

   - `pay.standard` (T1).

3. **Fallback**

   If none of the above match, the demo engine currently still labels the
   scriptPubKey as:

   - `pay.standard`.

   This is intentionally conservative: unknown non-OP_RETURN outputs are
   treated as “normal payment lane” unless a future version adds more specific
   `pay.*` / `contracts.*` recognisers.

### 2.3 Witness classification

For each witness stack item (`witness.stack[i:j]`), the demo engine:

1. Computes the byte length from hex.
2. Checks for simple ordinal / inscription-like patterns.
3. Falls back to ASCII / size heuristics.

The rules are:

1. **Ordinal / inscription lane**

   - If the hex contains `6f7264` (`"ord"` in ASCII), we treat it as
     ordinal-style data:

     - If `length > 256` bytes → `meta.inscription` (T2)
     - Else → `meta.ordinal` (T2)

2. **Unregistered vendor protocols**

   - If the payload is **≤ 128 bytes** and is mostly printable ASCII, and did
     not match the ordinal rule above, it is treated as:

     - `da.unregistered_vendor` (T3)

   This is a soft hint: “looks like a structured vendor protocol that hasn’t
   claimed a public BUDS label”.

3. **Large opaque blobs**

   - If `length > 512` bytes → `da.obfuscated` (T3)

   This is the “worst-case” catch-all for big, non-transparent blobs in the
   witness lane.

4. **Everything else**

   - Otherwise → `da.unknown` (T3)

Together, these rules give a simple but more expressive breakdown of witness
data without relying on any specific protocol’s internals.

### 2.4 Tier mapping (T0–T3)

Each label is mapped to one of four conceptual **tiers**:

- **T0 – consensus / validation-critical**
  - e.g. `consensus.*` (not yet emitted by the browser engine, but reserved).

- **T1 – economic / Bitcoin-critical**
  - e.g. `pay.*`, `commitment.*`, `contracts.*`.

- **T2 – metadata / hints / optional**
  - e.g. `meta.*`, `da.op_return_embed`.

- **T3 – unknown / obfuscated / likely spam**
  - any other label, including:
    - `da.unknown`
    - `da.obfuscated`
    - vendor-specific labels without special treatment.

The summary shows:

- which tiers are present (`tiersPresent`), and
- how many tags mapped into each (`counts`).

This gives node operators a **coarse view** of “what kind of data this transaction carries”.

---

## 3. Local policy model (profiles & knobs)

The Tag Engine includes a simple, configurable **policy model**:

- Every label maps to:
  - `minMult` – minimum feerate multiplier.
  - `boost` – a bonus or penalty applied once per unique label.

Profiles:

- **Neutral (default)**  
  - moderate penalties for `da.unknown` and `da.obfuscated`,
  - small penalty for `da.op_return_embed`,
  - neutral or slightly positive for payment labels.

- **Strict**  
  - high `minMult` and strong negative `boost` for `da.unknown` / `da.obfuscated`,
  - OP_RETURN also penalised more.

- **Permissive**  
  - low multipliers and gentle penalties for data labels.

Given:

- `base_min_feerate` (from the UI),
- `tx_feerate` (from the UI),

the engine computes:

- `required = base_min_feerate * max_label_minMult`
- `score   = tx_feerate * (1 + boostSum)`

This does **not** represent any real network policy. It is only:

- a worked example of how BUDS labels could feed into:
  - mempool admission,
  - eviction,
  - or block template scoring.

---

## 4. Exporting results

The **Export JSON** button copies a structured result to your clipboard:

- `tx` – the SimpleTx object,
- `classification` – txid and all tags,
- `tiers` – tiers present and per-tier counts,
- `policy` – required feerate, effective score, multiplier, boost sum,
- `profile` – which policy profile was used.

Example export:

```
{
  "tx": { ... },
  "classification": {
    "txid": "...",
    "tags": [
      {
        "surface": "scriptpubkey[0]",
        "start": 0,
        "end": 4,
        "labels": ["da.op_return_embed"]
      }
    ]
  },
  "tiers": {
    "tiersPresent": ["T2"],
    "counts": { "T0": 0, "T1": 0, "T2": 1, "T3": 0 }
  },
  "policy": {
    "required": 1.2,
    "score": 4.75,
    "mult": 1.2,
    "boostSum": -0.05
  },
  "profile": "permissive"
}
```

This is useful for:

- filing issues,
- comparing different Tag Engine versions,
- or feeding the output into external tools.

---

## 5. Running BUDS LAB

BUDS LAB is a static HTML + JavaScript app.

### Local

```
git clone https://github.com/defenwycke/bitcoin-unified-data-standard.git
cd bitcoin-unified-data-standard/buds-lab
python -m http.server 8080
```

Then open:

- `http://localhost:8080` in your browser.

### GitHub Codespaces

From a Codespace terminal:

```
cd /workspaces/bitcoin-unified-data-standard/buds-lab
python3 -m http.server 8080
```

Then open port `8080` from the **Ports** panel.

---

## 6. Limitations

- The Tag Engine currently implements a **small but still simplified** subset
  of BUDS:
  - OP_RETURN heuristics (rollup-like roots, indexer hints, general embeds).
  - Basic payment templates (P2PKH / P2WPKH / P2TR).
  - Simple ordinal / inscription / vendor / unknown / obfuscated witness
    classification.

Future work may include:

- more script templates (P2WPKH, P2TR, multisig, channel opens),
- richer metadata labels,
- a closer match to the C++ reference implementation, and
- optional integration with real node RPCs.

