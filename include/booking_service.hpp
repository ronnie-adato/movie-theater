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
 * In-memory movie ticket booking backend.
 *
 * Thread safety:
 * - The service supports concurrent bookings.
 * - Bookings are synchronized per movie+theater show to prevent over-booking.
 */
class BookingService {
  public:
    BookingService();
    ~BookingService();
    BookingService(const BookingService &) = delete;
    BookingService &operator=(const BookingService &) = delete;
    BookingService(BookingService &&) noexcept;
    BookingService &operator=(BookingService &&) noexcept;

    // Returns all playing movies.
    std::vector<Movie> listMovies() const;

    // Returns all theaters showing a given movie.
    // Returns empty vector if movie is unknown.
    std::vector<Theater> listTheatersForMovie(const std::string &movieId) const;

    // Returns available seats for the given movie and theater.
    // Returns empty vector if show is unknown.
    std::vector<std::string> getAvailableSeats(const std::string &movieId,
                                               const std::string &theaterId) const;

    // Attempts to book one or more seats atomically for a given show.
    // If any requested seat is invalid/already booked, no seat is booked.
    void bookSeats(const std::string &movieId, const std::string &theaterId,
                   const std::vector<std::string> &seatIds);
    /*TODO Select a movie*/
    /*TODO Select a theater*/

  private:
    void validateShow(const std::string &movieId, const std::string &theaterId) const;

    std::unordered_map<std::string, Movie> moviesById_;
    std::unordered_map<std::string, Theater> theatersById_;
    std::unordered_map<std::string, std::vector<std::string>> movieToTheaters_;
    std::unordered_map<std::string, std::shared_ptr<Show>> showsByKey_;
};

} // namespace movie_booking
