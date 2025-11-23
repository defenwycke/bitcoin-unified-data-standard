# BUDS LAB – Tag Engine Playground

BUDS LAB is a small, browser-based playground for the Bitcoin Unified Data Standard (BUDS).

It lets you:

- Build a simple transaction structure on the left (outputs + witness blobs).
- Run the BUDS **Tag Engine** on it.
- Inspect the resulting labels and an example local policy calculation.

This is a **non-consensus**, **non-normative** reference tool intended for experimentation and education.

---

## Layout

- **Left panel – Transaction Builder**
  - Set a TXID.
  - Add outputs:
    - Choose a template:
      - `P2PKH`
      - `P2WPKH`
      - `P2TR`
      - `OP_RETURN`
      - or `Custom`
    - Edit ASM and hex directly if desired.
  - Add witness stack items:
    - Hex-encoded blobs.

- **Right panel – BUDS Tag Engine Output**
  - Shows all tags:
    - `surface` (e.g. `scriptpubkey[0]`, `witness.stack[0:0]`)
    - `range` in bytes
    - `labels` (e.g. `pay.standard`, `da.op_return_embed`, `da.obfuscated`)
  - Shows a summary:
    - All unique labels present.
    - Example local policy calculation:
      - base mempool minimum feerate
      - label-based minimum multiplier
      - required feerate
      - effective feerate score

---

## Files

- `buds-lab/index.html`
  - The UI and layout.
  - Loads `buds-tag-engine.js`.

- `buds-lab/buds-tag-engine.js`
  - The BUDS **Tag Engine** implemented in JavaScript.
  - Mirrors the C++ reference classifier at a high level:
    - Detects `OP_RETURN` scripts.
    - Detects simple P2PKH scripts.
    - Treats other scripts as `pay.standard` (for now).
    - Tags large witness blobs as `da.obfuscated`, smaller ones as `da.unknown`.
  - Includes a small, non-normative policy example.

---

## Running BUDS LAB

You do **not** need a backend server. It is a static HTML + JS app.

### Option 1 – Local file

1. Clone the repo:
 
```
   git clone https://github.com/defenwycke/bitcoin-unified-data-standard.git
   cd bitcoin-unified-data-standard
```

2. Open `buds-lab/index.html` in your browser:
   - On most systems you can double-click the file,
   - or use "Open With → Browser".

### Option 2 – Simple static server

From the repo root:

```
cd buds-lab
python -m http.server 8080
```

Then open `http://localhost:8080` in your browser.

### Option 3 – GitHub Codespaces / Web IDE

If you are using GitHub Codespaces or another web-based IDE, use its built-in HTML preview or live server to open `buds-lab/index.html` in a browser tab.

---

## Updating the Tag Engine

To evolve classification heuristics:

- Edit `buds-lab/buds-tag-engine.js`.
- For example, you can:
  - add detection for P2WPKH, P2TR, or multisig patterns,
  - tune the `largeBlobThreshold`,
  - add new label mappings as the BUDS registry grows.

Keep the C++ and JS implementations logically aligned so that:

- BUDS LAB gives a realistic preview of how a node would tag the same transaction.

---
