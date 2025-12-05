#include <iostream>
#include <string>
#include <vector>

#include "buds_tagger.h"

using namespace buds;

#define ASSERT_TRUE(expr)                                                   \
    do {                                                                    \
        if (!(expr)) {                                                      \
            std::cerr << "ASSERT FAILED: " #expr                            \
                      << " at " << __FILE__ << ":" << __LINE__ << "\n";     \
            return false;                                                   \
        }                                                                   \
    } while (0)

static bool hasLabelOnSurface(const Classification& cls,
                              const std::string& surface,
                              const std::string& label) {
    for (const auto& tag : cls.tags) {
        if (tag.surface != surface) continue;
        for (const auto& l : tag.labels) {
            if (l == label) return true;
        }
    }
    return false;
}

// --- Tests ---

static bool test_p2pkh_pay_standard() {
    std::cout << "[TEST] P2PKH -> pay.standard (T1)\n";

    Tx tx;
    tx.txid = "test-p2pkh";

    TxOutput out;
    out.spk.asm_repr = "OP_DUP OP_HASH160 <pkh> OP_EQUALVERIFY OP_CHECKSIG";
    out.spk.hex = "76a91400112233445566778899aabbccddeeff0011223388ac";
    tx.vout.push_back(out);

    TagEngine engine;
    Classification cls = engine.classify(tx);
    Summary summary = engine.summarizeTiers(cls);

    ASSERT_TRUE(cls.txid == "test-p2pkh");
    ASSERT_TRUE(hasLabelOnSurface(cls, "scriptpubkey[0]", "pay.standard"));
    ASSERT_TRUE(!summary.tiersPresent.empty());
    ASSERT_TRUE(summary.counts.T1 >= 1);

    std::string arbda = engine.computeArbdaTierFromCounts(summary.counts);
    ASSERT_TRUE(arbda == "T1");

    return true;
}

static bool test_opreturn_hint_and_rollup() {
    std::cout << "[TEST] OP_RETURN hint / rollup detection\n";

    // 1) Small ASCII hint (<=8 bytes) -> meta.indexer_hint or da.op_return_embed
    {
        Tx tx;
        tx.txid = "test-hint";

        TxOutput out;
        out.spk.asm_repr = "OP_RETURN 6f6b";   // "ok"
        out.spk.hex = "6a026f6b";
        tx.vout.push_back(out);

        TagEngine engine;
        Classification cls = engine.classify(tx);
        ASSERT_TRUE(
            hasLabelOnSurface(cls, "scriptpubkey[0]", "meta.indexer_hint") ||
            hasLabelOnSurface(cls, "scriptpubkey[0]", "da.op_return_embed")
        );
    }

    // 2) 32-byte OP_RETURN payload ("rollup root") -> commitment.rollup_root
    {
        Tx tx;
        tx.txid = "test-rollup";

        TxOutput out;
        out.spk.asm_repr = "OP_RETURN <32-byte-root>";
        // 6a20 + 32 bytes of "11"
        std::string hex = "6a20";
        hex += std::string(64, '1');
        out.spk.hex = hex;
        tx.vout.push_back(out);

        TagEngine engine;
        Classification cls = engine.classify(tx);
        ASSERT_TRUE(hasLabelOnSurface(cls, "scriptpubkey[0]", "commitment.rollup_root"));
    }

    return true;
}

static bool test_witness_vendor_unknown_obfuscated() {
    std::cout << "[TEST] witness: vendor / unknown / obfuscated\n";

    TagEngine engine;

    // 1) Small ASCII witness -> da.unregistered_vendor
    {
        Tx tx;
        tx.txid = "test-vendor";

        Witness w;
        WitnessItem item;
        // 32 bytes of ASCII 'A' (0x41)
        std::string hex;
        for (int i = 0; i < 32; ++i) hex += "41";
        item.hex = hex;
        w.stack.push_back(item);
        tx.witness.push_back(w);

        Classification cls = engine.classify(tx);
        ASSERT_TRUE(hasLabelOnSurface(cls, "witness.stack[0:0]", "da.unregistered_vendor"));
    }

    // 2) Medium blob -> da.unknown
    {
        Tx tx;
        tx.txid = "test-unknown";

        Witness w;
        WitnessItem item;
        // 64 bytes of 0x00
        std::string hex;
        for (int i = 0; i < 64; ++i) hex += "00";
        item.hex = hex;
        w.stack.push_back(item);
        tx.witness.push_back(w);

        Classification cls = engine.classify(tx);
        ASSERT_TRUE(hasLabelOnSurface(cls, "witness.stack[0:0]", "da.unknown"));
    }

    // 3) Large blob (>512B) -> da.obfuscated, ARBDA = T3
    {
        Tx tx;
        tx.txid = "test-obfuscated";

        Witness w;
        WitnessItem item;
        // 600 bytes of 0x00 -> 1200 hex chars
        std::string hex;
        for (int i = 0; i < 600; ++i) hex += "00";
        item.hex = hex;
        w.stack.push_back(item);
        tx.witness.push_back(w);

        Classification cls = engine.classify(tx);
        Summary summary = engine.summarizeTiers(cls);

        ASSERT_TRUE(hasLabelOnSurface(cls, "witness.stack[0:0]", "da.obfuscated"));
        ASSERT_TRUE(summary.counts.T3 >= 1);

        std::string arbda = engine.computeArbdaTierFromCounts(summary.counts);
        ASSERT_TRUE(arbda == "T3");
    }

    return true;
}

static bool test_ordinal_inscription() {
    std::cout << "[TEST] ordinal / inscription witness detection\n";

    TagEngine engine;

    // Small ord-like witness -> meta.ordinal (or meta.inscription depending on size threshold)
    {
        Tx tx;
        tx.txid = "test-ord-small";

        Witness w;
        WitnessItem item;
        // hex for ASCII "ord" = 6f7264
        std::string hex = "6f7264";
        item.hex = hex;
        w.stack.push_back(item);
        tx.witness.push_back(w);

        Classification cls = engine.classify(tx);
        ASSERT_TRUE(
            hasLabelOnSurface(cls, "witness.stack[0:0]", "meta.ordinal") ||
            hasLabelOnSurface(cls, "witness.stack[0:0]", "meta.inscription")
        );
    }

    // Larger ord-like witness -> meta.inscription
    {
        Tx tx;
        tx.txid = "test-ord-large";

        Witness w;
        WitnessItem item;
        std::string hex;
        // Build a larger payload containing "6f7264" many times
        for (int i = 0; i < 200; ++i) hex += "6f7264";
        item.hex = hex;
        w.stack.push_back(item);
        tx.witness.push_back(w);

        Classification cls = engine.classify(tx);
        ASSERT_TRUE(hasLabelOnSurface(cls, "witness.stack[0:0]", "meta.inscription"));
    }

    return true;
}

int main() {
    if (!test_p2pkh_pay_standard()) return 1;
    if (!test_opreturn_hint_and_rollup()) return 1;
    if (!test_witness_vendor_unknown_obfuscated()) return 1;
    if (!test_ordinal_inscription()) return 1;

    std::cout << "All BUDS TagEngine tests passed.\n";
    return 0;
}
