#include "buds_tagger.h"
#include <iostream>

int main() {
    using namespace buds;

    TagEngine engine(PolicyProfile::Neutral);

    Tx tx;
    tx.txid = "deadbeef";

    // One P2PKH output
    TxOutput o;
    o.spk.hex = "76a91400112233445566778899aabbccddeeff0011223388ac";
    tx.vout.push_back(o);

    // One witness item (ASCII vendor-like)
    Witness w;
    WitnessItem wi;
    wi.hex = "30313233343536373839"; // "0123456789"
    w.stack.push_back(wi);
    tx.witness.push_back(w);

    Classification c = engine.classify(tx);
    Summary s = engine.summarizeTiers(c);
    std::string arbda = engine.computeArbdaTierFromCounts(s.counts);
    PolicyResult p = engine.computePolicy(c, 1.0, 5.0);

    std::cout << "txid: " << c.txid << "\n";
    for (const auto& tag : c.tags) {
        std::cout << "  surface=" << tag.surface
                  << " range=[" << tag.start << "," << tag.end << ") labels=[";
        for (std::size_t i = 0; i < tag.labels.size(); ++i) {
            if (i) std::cout << ",";
            std::cout << tag.labels[i];
        }
        std::cout << "]\n";
    }
    std::cout << "tiers: T0=" << s.counts.T0
              << " T1=" << s.counts.T1
              << " T2=" << s.counts.T2
              << " T3=" << s.counts.T3 << "\n";
    std::cout << "ARBDA tier: " << arbda << "\n";
    std::cout << "policy: mult=" << p.mult
              << " required=" << p.required
              << " score=" << p.score
              << " boostSum=" << p.boostSum << "\n";

    return 0;
}
