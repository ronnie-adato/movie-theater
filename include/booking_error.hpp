#pragma once

#include <stdexcept>
#include <vector>

/**
 * @namespace movie_booking
 * @brief All movie theater booking related classes and exceptions.
 */
namespace movie_booking {

/**
 * @class InvalidSeat
 * @brief Exception thrown when a seat ID is invalid or duplicated in a booking request.
 * @details Inherits from std::invalid_argument. Contains the invalid seat ID.
 */
class InvalidSeat : public std::invalid_argument {
  public:
    /**
     * @brief Construct an InvalidSeat exception.
     * @param message Description of the error.
     * @param seatId The invalid or duplicate seat ID.
     */
    InvalidSeat(const std::string &message, const std::string &seatId);

    /**
     * @brief The invalid or duplicate seat ID that caused the exception.
     */
    const std::string seatId;
};

/**
 * @class SeatAlreadyBooked
 * @brief Exception thrown when one or more requested seats are already booked.
 * @details Inherits from std::runtime_error. Contains the list of rejected seat IDs.
 */
class SeatAlreadyBooked : public std::runtime_error {
  public:
    /**
     * @brief Construct a SeatAlreadyBooked exception.
     * @param message Description of the error.
     * @param rejectedSeats The list of seat IDs that were already booked.
     */
    SeatAlreadyBooked(const std::string &message, const std::vector<std::string> &rejectedSeats);

    /**
     * @brief The list of seat IDs that were already booked and could not be reserved.
     */
    std::vector<std::string> rejectedSeats;
};

} // namespace movie_booking
