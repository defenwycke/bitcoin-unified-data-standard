#ifndef BUDS_TAGGING_H
#define BUDS_TAGGING_H

#include <string>
#include <vector>
#include <cstddef>

#include "buds_labels.h"

namespace buds {

// Minimal "script pubkey" view for the reference impl.
struct ScriptPubKey {
    std::string asm_repr; // e.g. "OP_RETURN 6f6b"
    std::string hex;      // raw hex bytes
};

struct TxOutput {
    ScriptPubKey script_pub_key;
};

// Minimal witness representation: vector of hex strings per input.
struct TxInputWitness {
    std::vector<std::string> stack_items_hex;
};

struct SimpleTx {
    std::string txid;
    std::vector<TxOutput> vout;
    std::vector<TxInputWitness> witness;
};

struct Tag {
    std::string surface;           // e.g. "scriptpubkey[0]" or "witness.stack[0:1]"
    std::size_t start = 0;
    std::size_t end = 0;
    std::vector<std::string> labels;
};

struct TxClassification {
    std::string txid;
    std::vector<Tag> tags;
};

/**
 * Example, NON-NORMATIVE classifier.
 * - Tags OP_RETURN outputs as da.op_return_embed
 * - Tags other outputs as pay.standard
 * - Tags witness items: large blobs as da.obfuscated, small as da.unknown
 */
class Tagger {
public:
    Tagger();

    TxClassification Classify(const SimpleTx& tx) const;

private:
    Registry m_registry;
};

} // namespace buds

#endif // BUDS_TAGGING_H
