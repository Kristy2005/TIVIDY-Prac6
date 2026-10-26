#ifndef ROLE_H
#define ROLE_H

#include <string>

/**
 * @brief Named responsibility that can be assigned to a participant.
 *
 * The template parameter is retained to preserve the project's existing
 * Role<T> and WorkflowBuilder interfaces. Role currently stores only its
 * human-readable name, so T does not affect assignment behaviour.
 *
 * @tparam T Existing type parameter retained for source compatibility.
 */
template <typename T>
class Role {
public:
    /** @brief Construct a role with an optional name. */
    explicit Role(const std::string& roleName = std::string()) : name(roleName) {}

    /**
     * @brief Keep the UML-defined configuration hook.
     *
     * No additional role configuration is currently required; the role's name
     * is the information used by role-based assignment.
     */
    void configureRole() {
    }

    /** @brief Return the role name. */
    const std::string& getName() const {
        return name;
    }

    /** @brief Change the role name. */
    void setName(const std::string& roleName) {
        name = roleName;
    }

private:
    std::string name;
};

#endif // ROLE_H
