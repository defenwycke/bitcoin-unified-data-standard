# BUDS Demo CLI

The BUDS repository includes a small C++ demonstration binary:

- `buds-demo` – runs a built-in example transaction through the BUDS v2 TagEngine.

The demo uses the reference implementation:

- `src/buds_tagger.*`

It is **non-normative** and exists only to show how BUDS tagging, tiers, ARBDA,
and simple policy scoring can be wired together.

---

## 1. Building the demo

From the repository root on a machine with a C++17 compiler:

    g++ -std=c++17 -Isrc \
        src/buds_demo.cpp \
        src/buds_tagger.cpp \
        -o buds-demo

On Windows (PowerShell / Command Prompt) you can write this as:

    g++ -std=c++17 -Isrc src\buds_demo.cpp src\buds_tagger.cpp -o buds-demo.exe

Adjust the compiler command if you are using `clang++` or a different toolchain.

---

## 2. What the demo does

The demo:

1. Constructs a simple transaction in memory with:
   - a standard payment output (`pay.standard`)
   - an OP_RETURN region
   - a witness blob
2. Runs the BUDS TagEngine to:
   - classify each region
   - compute tier counts (T0–T3)
   - compute the ARBDA worst-tier score
3. Applies an example policy profile to compute:
   - a **required feerate** (after multipliers)
   - an **effective score** for template selection / ordering
4. Prints results to stdout.

Exact thresholds and labels are illustrative only. They are not a recommendation
for mainnet policy.

---

## 3. Relationship to the Browser Lab

The C++ demo is conceptually aligned with the browser-based lab in `buds-lab/`:

- both use the BUDS v2 labels and tiers
- both apply similar heuristics for OP_RETURN / witness classification
- both compute ARBDA as a worst-tier transaction score

The lab offers an interactive, visual way to explore behaviour.  
The C++ demo shows how a node or off-chain tool might integrate tagging in a
native environment.

---

## 4. Status

The demo is **optional**:

- not part of Bitcoin consensus
- not required for BUDS adoption
- safe to remove or replace in downstream projects

It is included for experimentation, documentation, and regression testing of the
C++ TagEngine.
