/**
 * @file Role.h
 * @brief A named responsibility that participants hold and work items require.
 */
#ifndef TIVIDY_ENGINE_ROLE_H
#define TIVIDY_ENGINE_ROLE_H

#include <map>
#include <sstream>
#include <string>

namespace tividy {

/**
 * @brief A role (e.g. "reviewer") with optional typed configuration.
 *
 * The templated configure()/setting() pair lets the same generic Role carry
 * industry-specific settings (limits, SLA hours, ...) without subclassing.
 */
class Role {
public:
    explicit Role(const std::string& name = "") : name_(name) {}

    /** @return The role's name. */
    const std::string& name() const { return name_; }

    /** @brief Store a setting of any streamable type under @p key. */
    template <typename T>
    void configure(const std::string& key, const T& value) {
        std::ostringstream os;
        os << value;
        settings_[key] = os.str();
    }

    /** @return true if a setting for @p key exists. */
    bool hasSetting(const std::string& key) const { return settings_.count(key) > 0; }

    /**
     * @brief Read a setting, converting it to T.
     * @param key      Setting name.
     * @param fallback Returned if the key is missing or cannot be converted.
     */
    template <typename T>
    T setting(const std::string& key, const T& fallback) const {
        std::map<std::string, std::string>::const_iterator it = settings_.find(key);
        if (it == settings_.end()) return fallback;
        std::istringstream is(it->second);
        T value;
        if (is >> value) return value;
        return fallback;
    }

private:
    std::string name_;
    std::map<std::string, std::string> settings_;
};

/** @brief String specialisation: returns the whole stored text, spaces included. */
template <>
inline std::string Role::setting<std::string>(const std::string& key, const std::string& fallback) const {
    std::map<std::string, std::string>::const_iterator it = settings_.find(key);
    return it == settings_.end() ? fallback : it->second;
}

} // namespace tividy

#endif
