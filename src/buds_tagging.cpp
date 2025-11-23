#include "buds_tagging.h"

#include <algorithm>

namespace buds {

Tagger::Tagger()
    : m_registry()
{
}

TxClassification Tagger::Classify(const SimpleTx& tx) const
{
    TxClassification result;
    result.txid = tx.txid;

    std::vector<Tag> tags;

    // 1) Outputs: OP_RETURN vs standard payment
    for (std::size_t idx = 0; idx < tx.vout.size(); ++idx) {
        const auto& out = tx.vout[idx];
        const auto& spk = out.script_pub_key;

        std::string surface = "scriptpubkey[" + std::to_string(idx) + "]";

        // crude check: does asm start with "OP_RETURN"
        bool is_op_return = false;
        if (!spk.asm_repr.empty()) {
            const std::string prefix = "OP_RETURN";
            if (spk.asm_repr.rfind(prefix, 0) == 0) { // starts with
                is_op_return = true;
            }
        }

        std::size_t byte_len = spk.hex.size() / 2;

        Tag tag;
        tag.surface = surface;
        tag.start = 0;
        tag.end = byte_len;

        if (is_op_return) {
            tag.labels.push_back("da.op_return_embed");
        } else {
            tag.labels.push_back("pay.standard");
        }

        tags.push_back(std::move(tag));
    }

    // 2) Witness: mark large blobs as obfuscated vs unknown
    for (std::size_t vin_idx = 0; vin_idx < tx.witness.size(); ++vin_idx) {
        const auto& wit = tx.witness[vin_idx];
        for (std::size_t stack_idx = 0; stack_idx < wit.stack_items_hex.size(); ++stack_idx) {
            const std::string& item_hex = wit.stack_items_hex[stack_idx];
            std::size_t byte_len = item_hex.size() / 2;

            std::string surface = "witness.stack[" +
                                  std::to_string(vin_idx) + ":" +
                                  std::to_string(stack_idx) + "]";

            Tag tag;
            tag.surface = surface;
            tag.start = 0;
            tag.end = byte_len;

            if (byte_len > 512) {
                tag.labels.push_back("da.obfuscated");
            } else {
                tag.labels.push_back("da.unknown");
            }

            tags.push_back(std::move(tag));
        }
    }

    result.tags = std::move(tags);
    return result;
}

} // namespace buds

