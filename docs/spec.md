# File: docs/spec.md
# Bitcoin Unified Data Standard (BUDS) – Specification (Draft)

**Version:** 0.1.0  
**Status:** Draft  
**Scope:** Non-consensus standard for typed arbitrary data on Bitcoin.

---

## 1. Overview

The **Bitcoin Unified Data Standard (BUDS)** defines:

1. A **binary container format** for typed data (“BUDS blobs”).
2. A **global registry** of BUDS type IDs.
3. Minimal **rules for producers and parsers**.

BUDS is:

- **Carrier-agnostic**: it can be embedded in any data-carrying field (e.g. `OP_RETURN`, segOP lanes, witness data, side-channel commitments).
- **Consensus-neutral**: it does not change transaction validity or block rules.
- **Optional**: nodes, wallets, and explorers may ignore BUDS.

---

## 2. Terminology

- **BUDS Blob**: A byte sequence conforming to this specification.
- **Type ID**: A 16-bit unsigned integer identifying the semantic meaning of a value.
- **Value**: Arbitrary bytes whose meaning is defined in the registry.
- **Entry**: A (type, value) pair with length prefix.
- **Producer**: Software that creates BUDS blobs.
- **Parser**: Software that decodes BUDS blobs.

---

## 3. Encoding Format

### 3.1 High-level structure

A BUDS blob is a sequence of entries with a leading version byte:

```
+------------+-------------------+
| 1 byte     | N entries...      |
| version    | (TLV sequence)    |
+------------+-------------------+
```

Each entry uses a simple TLV structure:

```
+------------+------------+--------------------+
| 2 bytes    | 2 bytes    | L bytes            |
| type_id    | length L   | value (L bytes)    |
+------------+------------+--------------------+
```

All multi-byte integers are big-endian.

### 3.2 Fields

- version (`uint8`):
  - MUST be set to 0x01 for this draft.
  - Parsers encountering higher versions MAY reject or treat as opaque.

- type_id (`uint16`, big-endian):
  - MUST be defined in `/registry/registry.json` or fall under a defined extension rule.

- Types with the high bit set (`0x8000`–`0xFFFF`) are reserved for private / experimental use.

- length (`uint16`, big-endian):
  - Number of bytes in `value`.
  - MAY be zero.

- value (`bytes[length]`):
  - Semantics defined in the registry entry for `type_id`.

### 3.3 Constraints

- Maximum total BUDS blob size SHOULD respect the carrier’s policy (e.g. `OP_RETURN` size, segOP lane limits).
- Producers SHOULD avoid emitting multiple entries with the same `type_id` unless the registry explicitly allows lists/repetition.

## 4. Carriers

BUDS does not mandate a particular carrier. The following are recommended patterns.

### 4.1 OP_RETURN

A transaction output MAY carry a BUDS blob in scriptPubKey using OP_RETURN:

```
`OP_RETURN <buds_blob>`
```

- Producers SHOULD put the entire BUDS blob in a single push.
- Parsers MAY treat any `OP_RETURN` output that:
  - has at least 1 + 4 bytes, and
  - starts with a known BUDS version byte as a candidate BUDS blob.

### 4.2 segOP Lanes (Optional)

Where a separate, prunable data lane (e.g. segOP) is available, BUDS blobs MAY be used as:
- The top-level payload, or
- Nested structures within a broader TLV scheme.

In such environments:
- The BUDS blob SHOULD remain self-contained (leading version byte and TLVs).
- Additional envelope structure (e.g. outer TLV key for “BUDS”) MAY be defined by the platform.

### 4.3 Other Carriers

Future carriers (witness, side-chains, commitments) MAY embed BUDS blobs unchanged.

## 5. Type Registry
### 5.1 Global registry

The canonical list of `type_id` assignments is maintained in:
- `/registry/registry.json` (machine-readable)
- `/registry/registry.md` (human-readable)

Each type entry MUST include at least:
- `id` – hex string (e.g. `"0x0001"`)
- `name` – short identifier
- `category` – broad classification (e.g. `"metadata"`, `"media"`, `"protocol"`)
- `description` – brief human-readable explanation
- `status` – `"standard"`, `"draft"`, or `"reserved"`

### 5.2 Reserved ranges

The 16-bit `type_id` space is partitioned as follows:
- `0x0000` – reserved, MUST NOT be used.
- `0x0001`–`0x1FFF` – global standard types (assigned via registry process).
- `0x2000`–`0x7FFF` – extension / ecosystem ranges (allocated in blocks).
- `0x8000`–`0xFFFF` – private / experimental use, no central registration.

Parsers:
- MUST NOT assume any semantics for private types.
- SHOULD handle unknown `type_ids` gracefully (see §7).

## 6. Producer Guidelines

Producers (wallets, protocols, tools):

- MUST emit well-formed blobs:
  - Correct version byte.
  - Non-overlapping entries.
  - Valid lengths (no truncation).
- MUST only use standard `type_ids` when the semantics match the registry definition.
- SHOULD avoid information leakage by:
  - Minimising unnecessary metadata.
  - Using private ranges for experimental protocols until stable.

If a protocol needs complex nested structures, it SHOULD encode that inside the `value` field (e.g. JSON, CBOR, protobuf) while keeping the outer BUDS layer simple.

## 7. Parser Guidelines

Parsers (nodes, explorers, analysis tools) SHOULD:

- Validate basic structure:
  - At least 1 byte for version.
  - Remaining length divisible into valid TLVs.
- On errors (truncation, impossible length, etc.):
  - MAY treat the blob as opaque.
  - MUST NOT assume partially parsed data is trustworthy.
- For unknown `type_ids`:
  - MUST NOT fail decoding the entire blob.
  - SHOULD expose them as `"unknown"` or `"private"` with hex values.

When multiple entries share the same `type_id`:
- Parsers MAY:
  - Expose them as a list, or
  - Use the last occurrence, depending on the registry’s guidance.

## 8. Security and Privacy

- BUDS does not add new consensus surface, but it does:
  - Make some data more easily classified, which can be useful or harmful.
- Implementers SHOULD consider:
  - Whether to index or expose certain types (e.g. sensitive metadata).

Providing user controls to disable decoding or display of particular types.

See `/docs/rationale.md` for further discussion.

## 9. Versioning

This document defines version 0x01 of the BUDS format.

Future versions:
- MUST use a different version byte.
- SHOULD be defined in updated specs.
- MAY retain backward compatibility at the TLV level but are not required to.

Parsers:
- MAY support multiple versions.
- SHOULD treat unknown versions as opaque blobs.

## 10. Registry Process

The process for requesting new `type_ids` is specified in:

`/docs/registry-process.md`

In summary:
- Changes are proposed via pull requests.
- Each new type MUST be documented with a clear, narrow purpose.
- Type definitions SHOULD be stable before assignment.
