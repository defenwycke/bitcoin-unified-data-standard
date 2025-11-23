# BUDS Demo CLI

The BUDS repository includes two small command-line programs:

- `buds-demo` – simple demo binary, always runs the built-in example transaction.
- `buds-cli` – small CLI wrapper with `help` / `example` commands.

Both use the same reference implementation:

- `buds_labels.*`
- `buds_tagging.*`
- `buds_policy_example.*`

They are **non-normative** and exist only to show how BUDS tagging and policy can work end-to-end.

---

## Build

From the repository root:

### Build the demo binary

```
g++ -std=c++17 -Isrc \
    src/buds_demo.cpp \
    src/buds_labels.cpp \
    src/buds_tagging.cpp \
    src/buds_policy_example.cpp \
    -o buds-demo
```

### Build the CLI binary

```
g++ -std=c++17 -Isrc \
    src/buds_cli.cpp \
    src/buds_labels.cpp \
    src/buds_tagging.cpp \
    src/buds_policy_example.cpp \
    -o buds-cli
```

If your shell doesn’t like the line breaks, put each command on a single line.

---

## Run

### Demo (no arguments)

```
./buds-demo
```

Runs the built-in example transaction and prints:

- all BUDS tags (surface, byte range, labels),
- required feerate given the example policy,
- effective feerate score.

### CLI

Default (same as `example`):

```
./buds-cli
```

Help:

```
./buds-cli help
```

Explicit example command:

```
./buds-cli example
```

---

## What the Example Does

The built-in example transaction contains:

- one standard payment output → labelled `pay.standard`
- one OP_RETURN output → labelled `da.op_return_embed`
- one large witness blob → labelled `da.obfuscated`

The binaries:

1. Build this transaction in memory.
2. Run the BUDS tagger to classify its regions.
3. Print all tags.
4. Apply the example policy to compute:
   - the **required feerate** (after label-based multipliers), and
   - an **effective feerate score** (for template selection or ordering).

This is only a reference demonstration.  
Policies, labels, and thresholds are for illustration, not a recommendation.
