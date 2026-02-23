#pragma once

#include <string>

namespace movie_booking {

struct Theater {
    Theater();
    Theater(const std::string &id, const std::string &name);

    std::string id;
    std::string name;
};

} // namespace movie_booking
