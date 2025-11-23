#include "buds_labels.h"

namespace buds {

namespace {

std::vector<LabelInfo> BuildDefaultRegistry()
{
    std::vector<LabelInfo> v;

    // Consensus-ish labels
    v.push_back(LabelInfo{
        "consensus.sig",
        "Signatures required for transaction validation.",
        {"witness_stack", "scriptsig"},
        std::string("T0")
    });

    v.push_back(LabelInfo{
        "consensus.script",
        "Executed script regions that enforce spending conditions.",
        {"scriptsig", "witness_script", "scriptpubkey"},
        std::string("T0")
    });

    v.push_back(LabelInfo{
        "consensus.taproot_prog",
        "Taproot or tapscript programs used in validation.",
        {"witness_script"},
        std::string("T0")
    });

    // Economic / system
    v.push_back(LabelInfo{
        "pay.standard",
        "Standard payments and transfers to common scriptpubkey types.",
        {"scriptpubkey"},
        std::string("T1")
    });

    v.push_back(LabelInfo{
        "pay.channel_open",
        "Lightning or L2 channel establishment outputs.",
        {"scriptpubkey", "witness_script"},
        std::string("T1")
    });

    v.push_back(LabelInfo{
        "pay.channel_update",
        "Updates or closes for channel or L2 contract state.",
        {"witness_stack", "witness_script"},
        std::string("T1")
    });

    v.push_back(LabelInfo{
        "contracts.vault",
        "Vault or recovery contract structures.",
        {"scriptpubkey", "witness_script"},
        std::string("T1")
    });

    v.push_back(LabelInfo{
        "commitment.rollup_root",
        "Commitments anchoring L2 or rollup state to Bitcoin.",
        {"scriptpubkey", "witness_stack", "coinbase"},
        std::string("T1")
    });

    v.push_back(LabelInfo{
        "meta.pool_tag",
        "Mining pool identification or metadata in coinbase.",
        {"coinbase"},
        std::string("T1")
    });

    // Metadata / application
    v.push_back(LabelInfo{
        "da.op_return_embed",
        "Explicit metadata embedded using OP_RETURN.",
        {"op_return"},
        std::string("T2")
    });

    v.push_back(LabelInfo{
        "meta.inscription",
        "Known inscription-style payloads or formats.",
        {"witness_stack", "op_return"},
        std::string("T2")
    });

    v.push_back(LabelInfo{
        "meta.ordinal",
        "Ordinal or NFT-related metadata.",
        {"witness_stack", "op_return"},
        std::string("T2")
    });

    v.push_back(LabelInfo{
        "meta.indexer_hint",
        "Optional hints intended for external indexers or apps.",
        {"op_return", "scriptpubkey", "witness_stack"},
        std::string("T2")
    });

    v.push_back(LabelInfo{
        "da.embed_misc",
        "General-purpose embedded metadata not covered by specific labels.",
        {"op_return", "scriptpubkey", "witness_stack"},
        std::string("T2")
    });

    // Unknown / opaque
    v.push_back(LabelInfo{
        "da.unknown",
        "Unclassified data that does not match any known pattern.",
        {"scriptsig", "witness_stack", "witness_script", "scriptpubkey", "op_return", "segop", "coinbase"},
        std::string("T3")
    });

    v.push_back(LabelInfo{
        "da.obfuscated",
        "Large, opaque, or intentionally hidden data blobs.",
        {"scriptsig", "witness_stack", "witness_script", "scriptpubkey", "segop"},
        std::string("T3")
    });

    v.push_back(LabelInfo{
        "da.unregistered_vendor",
        "Structured vendor-specific data for which no public label exists.",
        {"witness_stack", "witness_script", "scriptpubkey", "segop"},
        std::string("T3")
    });

    return v;
}

} // namespace

Registry::Registry()
    : m_labels(BuildDefaultRegistry())
{
}

const LabelInfo* Registry::Get(const std::string& label) const
{
    for (const auto& entry : m_labels) {
        if (entry.label == label) {
            return &entry;
        }
    }
    return nullptr;
}

} // namespace buds

