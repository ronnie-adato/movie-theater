#include "movie.hpp"

namespace movie_booking {

Movie::Movie() = default;

Movie::Movie(const std::string &id, const std::string &title) : id(id), title(title) {}

} // namespace movie_booking
