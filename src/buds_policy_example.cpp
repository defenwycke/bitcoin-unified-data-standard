#include "buds_policy_example.h"

#include <algorithm>
#include <set>

namespace buds {

PolicyConfig::PolicyConfig()
{
    // Example defaults, purely illustrative.

    m_policies["da.obfuscated"] = LabelPolicy{
        /*min_feerate_multiplier=*/3.0,
        /*priority_boost=*/-0.5
    };

    m_policies["da.unknown"] = LabelPolicy{
        /*min_feerate_multiplier=*/2.0,
        /*priority_boost=*/-0.2
    };

    m_policies["pay.standard"] = LabelPolicy{
        /*min_feerate_multiplier=*/1.0,
        /*priority_boost=*/0.0
    };

    m_policies["pay.channel_open"] = LabelPolicy{
        /*min_feerate_multiplier=*/1.0,
        /*priority_boost=*/0.2
    };
}

const LabelPolicy& PolicyConfig::ForLabel(const std::string& label) const
{
    auto it = m_policies.find(label);
    if (it != m_policies.end()) {
        return it->second;
    }
    static const LabelPolicy kDefault{};
    return kDefault;
}

PolicyExample::PolicyExample(const PolicyConfig& cfg)
    : m_cfg(cfg)
{
}

double PolicyExample::MinRequiredFeerate(double base_mempool_min,
                                         const TxClassification& classification) const
{
    double multiplier = 1.0;

    for (const auto& tag : classification.tags) {
        for (const auto& label : tag.labels) {
            const auto& lp = m_cfg.ForLabel(label);
            multiplier = std::max(multiplier, lp.min_feerate_multiplier);
        }
    }

    return base_mempool_min * multiplier;
}

double PolicyExample::EffectiveFeerateScore(double tx_feerate,
                                            const TxClassification& classification) const
{
    // Sum priority boosts over unique labels.
    std::set<std::string> seen;
    double boost_sum = 0.0;

    for (const auto& tag : classification.tags) {
        for (const auto& label : tag.labels) {
            if (seen.insert(label).second) {
                const auto& lp = m_cfg.ForLabel(label);
                boost_sum += lp.priority_boost;
            }
        }
    }

    // Clamp boost so it doesn't blow up.
    boost_sum = std::max(-0.9, std::min(boost_sum, 1.0));
    return tx_feerate * (1.0 + boost_sum);
}

} // namespace buds

