#ifndef BUDS_LABELS_H
#define BUDS_LABELS_H

#include <string>
#include <vector>
#include <optional>

namespace buds {

struct LabelInfo {
    std::string label;
    std::string description;
    std::vector<std::string> surfaces;
    std::optional<std::string> suggested_category; // e.g. "T0".."T3"
};

/**
 * Simple in-memory view of the BUDS registry.
 * This is a reference snapshot, not a JSON parser.
 */
class Registry {
public:
    Registry();

    const LabelInfo* Get(const std::string& label) const;
    const std::vector<LabelInfo>& All() const { return m_labels; }

private:
    std::vector<LabelInfo> m_labels;
};

} // namespace buds

#endif // BUDS_LABELS_H
