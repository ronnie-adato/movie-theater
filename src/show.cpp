
#include "show.hpp"
#include "booking_error.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_set>

namespace {

std::set<std::string> buildDefaultSeatCatalog() {
    std::set<std::string> seats;
    for (char prefix = 'a'; prefix < 'c'; ++prefix) {
        for (int seatNumber = 1; seatNumber <= 10; ++seatNumber) {
            seats.insert(prefix + std::to_string(seatNumber));
        }
    }
    return seats;
}

} // namespace

namespace movie_booking {

Show::Show(const std::string &movieId, const std::string &theaterId)
    : movieId_(movieId), theaterId_(theaterId), seatCatalog_(buildSeatCatalog()) {}

const std::string &Show::movieId() const { return movieId_; }

const std::string &Show::theaterId() const { return theaterId_; }

std::vector<std::string> Show::availableSeats() const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::string> available;
    available.reserve(seatCatalog_.size());

    // Compute set difference
    std::set_difference(seatCatalog_.begin(), seatCatalog_.end(), bookedSeats_.begin(),
                        bookedSeats_.end(), std::back_inserter(available));

    return available;
}

void Show::validateEmptyOrDuplicateReservations(const std::vector<std::string> &seatIds) const {
    if (seatIds.empty()) {
        throw std::invalid_argument("At least one seat must be selected.");
    }

    std::unordered_set<std::string> uniqueSeatIds;
    uniqueSeatIds.reserve(seatIds.size());
    for (const auto &seatId : seatIds) {
        if (!seatCatalog_.contains(seatId)) {
            throw InvalidSeat("One or more seats are invalid.", seatId);
        }

        if (!uniqueSeatIds.insert(seatId).second) {
            throw InvalidSeat("Duplicate seat in request.", seatId);
        }
    }
}

void Show::reserveSeats(const std::vector<std::string> &seatIds) {
    validateEmptyOrDuplicateReservations(seatIds);

    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::string> rejected;
    for (const auto &seatId : seatIds) {
        if (bookedSeats_.contains(seatId)) {
            rejected.push_back(seatId);
        }
    }

    if (!rejected.empty()) {
        throw SeatAlreadyBooked(
            "One or more requested seats are already booked. No seats were booked.", rejected);
    }

    bookedSeats_.insert(seatIds.cbegin(), seatIds.cend());
}

std::set<std::string> Show::buildSeatCatalog() { return buildDefaultSeatCatalog(); }

} // namespace movie_booking
