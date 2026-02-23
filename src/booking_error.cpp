#include "booking_error.hpp"
#include <stdexcept>

namespace movie_booking {

InvalidSeat::InvalidSeat(const std::string &message, const std::string &seatId)
    : std::invalid_argument(message), seatId(seatId) {}

SeatAlreadyBooked::SeatAlreadyBooked(const std::string &message,
                                     const std::vector<std::string> &rejectedSeats)
    :

      std::runtime_error(message), rejectedSeats(rejectedSeats) {}
} // namespace movie_booking