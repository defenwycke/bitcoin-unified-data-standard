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

### 2.2 Script classification (scriptPubKey)

Rules (simplified):

- If the script is detected as `OP_RETURN`:
  - label: `da.op_return_embed`  
    (treated as data embedded via OP_RETURN)

- Else if it matches a simple P2PKH template:
  - hex pattern: `76a914 <20 bytes> 88ac`
  - label: `pay.standard`

- Else (fallback):
  - label: `pay.standard`  
    (for now, everything non-OP_RETURN in scriptPubKey is treated as “normal payment lane”)

Future versions can extend this with P2WPKH, P2TR, multisig, channel opens, etc., while keeping the same interface.

### 2.3 Witness classification

For each witness stack item:

- Compute its length in bytes.
- If `length > 512` bytes:
  - label: `da.obfuscated`
- Else:
  - label: `da.unknown`

The idea:

- large, opaque blobs in witness are likely “data abuse”,
- smaller items are “unknown” but cheaper to tolerate.

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

- The Tag Engine currently implements only a **minimal** subset of BUDS:
  - OP_RETURN detection,
  - simple P2PKH detection,
  - size-based witness classification.
- It is not aware of:
  - real transaction structure (inputs, value, sighash flags, etc.),
  - complex contract scripts,
  - specific L2 protocols.
- The policy model is intentionally simple and opinionated.

Future work may include:

- more script templates (P2WPKH, P2TR, multisig, channel opens),
- richer metadata labels,
- a closer match to the C++ reference implementation, and
- optional integration with real node RPCs.

