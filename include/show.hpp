#pragma once

#include <mutex>
#include <string>
#include <set>
#include <vector>

namespace movie_booking {

class Show {
  public:
    Show(const std::string &movieId, const std::string &theaterId);

    const std::string &movieId() const;
    const std::string &theaterId() const;

    std::vector<std::string> availableSeats() const;
    void reserveSeats(const std::vector<std::string> &seatIds);

  private:
    static std::set<std::string> buildSeatCatalog();
    void validateEmptyOrDuplicateReservations(const std::vector<std::string> &seatIds) const;

    std::string movieId_;
    std::string theaterId_;
    std::set<std::string> seatCatalog_;
    std::set<std::string> bookedSeats_;
    mutable std::mutex mutex_;
};

} // namespace movie_booking
