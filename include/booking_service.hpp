#pragma once

#include "movie.hpp"
#include "show.hpp"
#include "theater.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace movie_booking {

/**
 * @class BookingService
 * @brief In-memory movie ticket booking backend.
 * @details
 * Stores movie/theater/show state in memory and delegates per-show reservation
 * logic to `Show`.
 *
 * Thread safety:
 * - Supports concurrent requests.
 * - Reservations are synchronized per show (movie + theater) to prevent
 *   over-booking.
 */
class BookingService {
  public:
    /** @brief Construct a service with seeded in-memory catalog data. */
    BookingService();
    /** @brief Destructor. */
    ~BookingService();

    /** @brief Non-copyable. */
    BookingService(const BookingService &) = delete;
    /** @brief Non-copyable. */
    BookingService &operator=(const BookingService &) = delete;

    /** @brief Movable. */
    BookingService(BookingService &&) noexcept;
    /** @brief Movable. */
    BookingService &operator=(BookingService &&) noexcept;

    /**
     * @brief List all movies currently in the service catalog.
     * @return Vector of movies.
     */
    std::vector<Movie> listMovies() const;

    /**
     * @brief List theaters that show a given movie.
     * @param movieId Movie identifier.
     * @return Vector of theaters for `movieId`, or empty if unknown.
     */
    std::vector<Theater> listTheatersForMovie(const std::string &movieId) const;

    /**
     * @brief Get currently available seats for a specific show.
     * @param movieId Movie identifier.
     * @param theaterId Theater identifier.
     * @return Available seat IDs, or empty if show is unknown.
     */
    std::vector<std::string> getAvailableSeats(const std::string &movieId,
                                               const std::string &theaterId) const;

    /**
     * @brief Atomically reserve one or more seats for a show.
     * @param movieId Movie identifier.
     * @param theaterId Theater identifier.
     * @param seatIds Requested seat IDs.
     * @throws std::invalid_argument If movie/theater/show identifiers are invalid.
     * @throws InvalidSeat If any seat ID is invalid or duplicated in the request.
     * @throws SeatAlreadyBooked If any requested seat is already reserved.
     * @details
     * Reservation is all-or-nothing: if any requested seat cannot be reserved,
     * no seat from the request is committed.
     */
    void bookSeats(const std::string &movieId, const std::string &theaterId,
                   const std::vector<std::string> &seatIds);

  private:
    /**
     * @brief Validate that movie/theater/show identifiers resolve to a known show.
     * @param movieId Movie identifier.
     * @param theaterId Theater identifier.
     * @throws std::invalid_argument If the show is invalid or unknown.
     */
    void validateShow(const std::string &movieId, const std::string &theaterId) const;

    std::unordered_map<std::string, Movie> moviesById_;
    std::unordered_map<std::string, Theater> theatersById_;
    std::unordered_map<std::string, std::vector<std::string>> movieToTheaters_;
    std::unordered_map<std::string, std::shared_ptr<Show>> showsByKey_;
};

} // namespace movie_booking
