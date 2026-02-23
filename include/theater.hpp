#pragma once

#include <string>

namespace movie_booking {

/**
 * @struct Theater
 * @brief Lightweight theater value object.
 */
struct Theater {
    /** @brief Default constructor. */
    Theater();
    /**
     * @brief Construct a theater with identifier and name.
     * @param id Theater identifier.
     * @param name Theater display name.
     */
    Theater(const std::string &id, const std::string &name);

    /** @brief Theater identifier. */
    std::string id;
    /** @brief Theater display name. */
    std::string name;
};

} // namespace movie_booking
