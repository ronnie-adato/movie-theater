#pragma once

#include <string>

namespace movie_booking {

struct Movie {
    Movie();
    Movie(const std::string &id, const std::string &title);

    std::string id;
    std::string title;
};

} // namespace movie_booking
