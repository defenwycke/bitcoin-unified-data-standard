#ifndef BUDS_POLICY_EXAMPLE_H
#define BUDS_POLICY_EXAMPLE_H

#include <string>
#include <unordered_map>

#include "buds_tagging.h"

namespace buds {

/**
 * Example per-label policy.
 * All values are non-normative and purely illustrative.
 */
struct LabelPolicy {
    double min_feerate_multiplier = 1.0; // for mempool admission
    double priority_boost = 0.0;         // for effective-feerate scoring
};

class PolicyConfig {
public:
    PolicyConfig();

    const LabelPolicy& ForLabel(const std::string& label) const;

private:
    std::unordered_map<std::string, LabelPolicy> m_policies;
};

class PolicyExample {
public:
    explicit PolicyExample(const PolicyConfig& cfg = PolicyConfig());

    /**
     * Given a base mempool minimum feerate (e.g. sat/vB) and a classification,
     * return the required feerate after applying the maximum label multiplier.
     */
    double MinRequiredFeerate(double base_mempool_min,
                              const TxClassification& classification) const;

    /**
     * Compute an "effective feerate score" by applying priority boosts
     * based on labels present in the transaction.
     */
    double EffectiveFeerateScore(double tx_feerate,
                                 const TxClassification& classification) const;

private:
    PolicyConfig m_cfg;
};

} // namespace buds

#endif // BUDS_POLICY_EXAMPLE_H
