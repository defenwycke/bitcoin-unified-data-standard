#include <iostream>
#include <string>
#include <vector>

#include "buds_labels.h"
#include "buds_tagging.h"

using namespace buds;

#define ASSERT_TRUE(expr)                                                   \
    do {                                                                    \
        if (!(expr)) {                                                      \
            std::cerr << "ASSERT FAILED: " #expr                            \
                      << " at " << __FILE__ << ":" << __LINE__ << "\n";     \
            return false;                                                   \
        }                                                                   \
    } while (0)

static bool Test_P2PKH_is_pay_standard()
{
    std::cout << "[TEST] P2PKH -> pay.standard\n";

    SimpleTx tx;
    tx.txid = "test-p2pkh";

    TxOutput out;
    out.script_pub_key.asm_repr = "OP_DUP OP_HASH160 <pubkeyhash> OP_EQUALVERIFY OP_CHECKSIG";
    out.script_pub_key.hex = "76a91400112233445566778899aabbccddeeff0011223388ac";
    tx.vout.push_back(out);

    Tagger tagger;
    TxClassification cls = tagger.Classify(tx);

    ASSERT_TRUE(cls.txid == "test-p2pkh");
    ASSERT_TRUE(cls.tags.size() == 1);
    ASSERT_TRUE(cls.tags[0].surface == "scriptpubkey[0]");
    ASSERT_TRUE(!cls.tags[0].labels.empty());
    ASSERT_TRUE(cls.tags[0].labels[0] == "pay.standard");

    return true;
}

static bool Test_OpReturn_is_da_op_return_embed()
{
    std::cout << "[TEST] OP_RETURN -> da.op_return_embed\n";

    SimpleTx tx;
    tx.txid = "test-opreturn";

    TxOutput out;
    out.script_pub_key.asm_repr = "OP_RETURN 6f6b";
    out.script_pub_key.hex = "6a026f6b";
    tx.vout.push_back(out);

    Tagger tagger;
    TxClassification cls = tagger.Classify(tx);

    ASSERT_TRUE(cls.txid == "test-opreturn");
    ASSERT_TRUE(cls.tags.size() == 1);
    ASSERT_TRUE(cls.tags[0].surface == "scriptpubkey[0]");
    ASSERT_TRUE(!cls.tags[0].labels.empty());
    ASSERT_TRUE(cls.tags[0].labels[0] == "da.op_return_embed");

    return true;
}

static bool Test_Witness_small_is_da_unknown()
{
    std::cout << "[TEST] small witness blob -> da.unknown\n";

    SimpleTx tx;
    tx.txid = "test-wit-small";

    TxInputWitness wit;
    wit.stack_items_hex.push_back("0011223344556677"); // 8 bytes
    tx.witness.push_back(wit);

    Tagger tagger;
    TxClassification cls = tagger.Classify(tx);

    ASSERT_TRUE(cls.txid == "test-wit-small");
    ASSERT_TRUE(cls.tags.size() == 1);
    ASSERT_TRUE(cls.tags[0].surface == "witness.stack[0:0]");
    ASSERT_TRUE(!cls.tags[0].labels.empty());
    ASSERT_TRUE(cls.tags[0].labels[0] == "da.unknown");

    return true;
}

static bool Test_Witness_large_is_da_obfuscated()
{
    std::cout << "[TEST] large witness blob -> da.obfuscated\n";

    SimpleTx tx;
    tx.txid = "test-wit-large";

    // 600 bytes of 0x00 -> 1200 hex chars
    std::string big_hex(600 * 2, '0');

    TxInputWitness wit;
    wit.stack_items_hex.push_back(big_hex);
    tx.witness.push_back(wit);

    Tagger tagger;
    TxClassification cls = tagger.Classify(tx);

    ASSERT_TRUE(cls.txid == "test-wit-large");
    ASSERT_TRUE(cls.tags.size() == 1);
    ASSERT_TRUE(cls.tags[0].surface == "witness.stack[0:0]");
    ASSERT_TRUE(!cls.tags[0].labels.empty());
    ASSERT_TRUE(cls.tags[0].labels[0] == "da.obfuscated");

    return true;
}

static bool Test_Mixed_tx_has_all_expected_tags()
{
    std::cout << "[TEST] mixed tx (P2PKH + OP_RETURN + large witness)\n";

    SimpleTx tx;
    tx.txid = "test-mixed";

    // Output 0: P2PKH
    {
        TxOutput out;
        out.script_pub_key.asm_repr = "OP_DUP OP_HASH160 <pubkeyhash> OP_EQUALVERIFY OP_CHECKSIG";
        out.script_pub_key.hex = "76a91400112233445566778899aabbccddeeff0011223388ac";
        tx.vout.push_back(out);
    }

    // Output 1: OP_RETURN
    {
        TxOutput out;
        out.script_pub_key.asm_repr = "OP_RETURN 6f6b";
        out.script_pub_key.hex = "6a026f6b";
        tx.vout.push_back(out);
    }

    // Witness: large blob
    {
        std::string big_hex(600 * 2, '0');
        TxInputWitness wit;
        wit.stack_items_hex.push_back(big_hex);
        tx.witness.push_back(wit);
    }

    Tagger tagger;
    TxClassification cls = tagger.Classify(tx);

    ASSERT_TRUE(cls.txid == "test-mixed");
    ASSERT_TRUE(cls.tags.size() == 3);

    bool saw_pay = false;
    bool saw_opret = false;
    bool saw_obf = false;

    for (const auto& tag : cls.tags) {
        for (const auto& label : tag.labels) {
            if (label == "pay.standard")      saw_pay = true;
            if (label == "da.op_return_embed") saw_opret = true;
            if (label == "da.obfuscated")     saw_obf = true;
        }
    }

    ASSERT_TRUE(saw_pay);
    ASSERT_TRUE(saw_opret);
    ASSERT_TRUE(saw_obf);

    return true;
}

int main()
{
    int failed = 0;

    if (!Test_P2PKH_is_pay_standard())        failed++;
    if (!Test_OpReturn_is_da_op_return_embed()) failed++;
    if (!Test_Witness_small_is_da_unknown())  failed++;
    if (!Test_Witness_large_is_da_obfuscated()) failed++;
    if (!Test_Mixed_tx_has_all_expected_tags()) failed++;

    if (failed == 0) {
        std::cout << "All BUDS tagger tests PASSED.\n";
        return 0;
    } else {
        std::cout << failed << " test(s) FAILED.\n";
        return 1;
    }
}
