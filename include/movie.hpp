#pragma once

#include <string>

namespace movie_booking {

/**
 * @struct Movie
 * @brief Lightweight movie value object.
 */
struct Movie {
    /** @brief Default constructor. */
    Movie();
    /**
     * @brief Construct a movie with identifier and title.
     * @param id Movie identifier.
     * @param title Movie title.
     */
    Movie(const std::string &id, const std::string &title);

    /** @brief Movie identifier. */
    std::string id;
    /** @brief Movie display title. */
    std::string title;
};

} // namespace movie_booking
