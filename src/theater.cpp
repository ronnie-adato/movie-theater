#include "theater.hpp"

namespace movie_booking {

Theater::Theater() = default;

Theater::Theater(const std::string &id, const std::string &name) : id(id), name(name) {}

} // namespace movie_booking
