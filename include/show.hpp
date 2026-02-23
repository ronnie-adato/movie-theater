#pragma once

#include <mutex>
#include <string>
#include <set>
#include <vector>

namespace movie_booking {

/**
 * @class Show
 * @brief Represents seat inventory and reservations for one movie at one theater.
 * @details
 * Maintains a fixed seat catalog and a reserved-seat set.
 * All reservation-sensitive operations are synchronized with an internal mutex.
 */
class Show {
  public:
    /**
     * @brief Construct a show context.
     * @param movieId Movie identifier.
     * @param theaterId Theater identifier.
     */
    Show(const std::string &movieId, const std::string &theaterId);

    /** @brief Get the movie identifier for this show. */
    const std::string &movieId() const;
    /** @brief Get the theater identifier for this show. */
    const std::string &theaterId() const;

    /**
     * @brief Get currently available seat IDs.
     * @return Seats that are in catalog but not currently reserved.
     */
    std::vector<std::string> availableSeats() const;

    /**
     * @brief Atomically reserve one or more seats for this show.
     * @param seatIds Requested seat IDs.
     * @throws std::invalid_argument If request is empty.
     * @throws InvalidSeat If any seat ID is invalid or duplicated.
     * @throws SeatAlreadyBooked If one or more requested seats are already reserved.
     * @details
     * Reservation is all-or-nothing for the request.
     */
    void reserveSeats(const std::vector<std::string> &seatIds);

  private:
    /** @brief Build default seat catalog for this project. */
    static std::set<std::string> buildSeatCatalog();

    /**
     * @brief Validate seat request shape before lock-protected reservation.
     * @param seatIds Requested seat IDs.
     * @throws std::invalid_argument If request is empty.
     * @throws InvalidSeat If any seat ID is invalid or duplicated.
     */
    void validateEmptyOrDuplicateReservations(const std::vector<std::string> &seatIds) const;

    std::string movieId_;
    std::string theaterId_;
    std::set<std::string> seatCatalog_;
    std::set<std::string> bookedSeats_;
    mutable std::mutex mutex_;
};

} // namespace movie_booking
