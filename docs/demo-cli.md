## Demo CLI

This repository includes a small example program `main_demo_cli.cpp` that shows
BUDS classification and policy integration end-to-end.

### Build

(code start)
# From repo root
g++ -std=c++17 -Isrc \
    main_demo_cli.cpp \
    src/buds_labels.cpp \
    src/buds_tagging.cpp \
    src/buds_policy_example.cpp \
    -o buds-demo
(code end)

### Run

(code start)
./buds-demo
(code end)

The demo:

- builds an example transaction with:
  - a standard payment output,
  - an OP_RETURN metadata output,
  - a large witness blob,
- runs the BUDS tagger to classify regions,
- applies an example policy to show:
  - the required feerate (after label-based multipliers),
  - an effective feerate score for template selection.
