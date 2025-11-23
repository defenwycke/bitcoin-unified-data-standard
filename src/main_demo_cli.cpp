#include <iostream>
#include <iomanip>
#include <string>

#include "buds_labels.h"
#include "buds_tagging.h"
#include "buds_policy_example.h"

using namespace buds;

static SimpleTx BuildExampleTx()
{
    SimpleTx tx;
    tx.txid = "deadbeefcafebabe0011223344556677";

    // --- Output 0: standard payment ---
    TxOutput out0;
    out0.script_pub_key.asm_repr = "OP_DUP OP_HASH160 <pubkeyhash> OP_EQUALVERIFY OP_CHECKSIG";
    out0.script_pub_key.hex = "76a91400112233445566778899aabbccddeeff0011223388ac"; // dummy
    tx.vout.push_back(out0);

    // --- Output 1: OP_RETURN metadata ---
    TxOutput out1;
    out1.script_pub_key.asm_repr = "OP_RETURN 6f6b"; // "ok"
    out1.script_pub_key.hex = "6a026f6b";           // OP_RETURN 'ok'
    tx.vout.push_back(out1);

    // --- Witness: one large blob to trigger da.obfuscated ---
    TxInputWitness wit0;
    std::string big_hex;
    big_hex.reserve(600 * 2);
    for (int i = 0; i < 600; ++i) {
        big_hex += "00";
    }
    wit0.stack_items_hex.push_back(big_hex);
    tx.witness.push_back(wit0);

    return tx;
}

int main()
{
    try {
        // Build example tx
        SimpleTx tx = BuildExampleTx();

        // Classify
        Tagger tagger;
        TxClassification classification = tagger.Classify(tx);

        // Example policy
        PolicyConfig cfg;
        PolicyExample policy(cfg);

        double base_mempool_min = 1.0; // sat/vB (example)
        double tx_feerate = 5.0;       // sat/vB (example)

        double required = policy.MinRequiredFeerate(base_mempool_min, classification);
        double score    = policy.EffectiveFeerateScore(tx_feerate, classification);

        std::cout << "BUDS demo for txid: " << classification.txid << "\n\n";

        std::cout << "Tags:\n";
        for (const auto& tag : classification.tags) {
            std::cout << "  surface=" << tag.surface
                      << "  range=[" << tag.start << "," << tag.end << ")"
                      << "  labels=[";
            for (std::size_t i = 0; i < tag.labels.size(); ++i) {
                std::cout << tag.labels[i];
                if (i + 1 < tag.labels.size()) std::cout << ",";
            }
            std::cout << "]\n";
        }

        std::cout << "\nPolicy example:\n";
        std::cout << "  base mempool min feerate : " << base_mempool_min << " sat/vB\n";
        std::cout << "  tx feerate                : " << tx_feerate << " sat/vB\n";
        std::cout << "  required feerate (BUDS)   : " << required << " sat/vB\n";
        std::cout << "  effective score (BUDS)    : " << score << " (scaled sat/vB)\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error in BUDS demo: " << e.what() << "\n";
        return 1;
    }
}

