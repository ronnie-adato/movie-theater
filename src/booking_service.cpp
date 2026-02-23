#include "booking_service.hpp"

#include <ng-log/logging.h>

#include <algorithm>
#include <stdexcept>

namespace movie_booking {

namespace {

std::string makeShowKey(const std::string &movieId, const std::string &theaterId) {
    return movieId + "|" + theaterId;
}

} // namespace

BookingService::BookingService() {
    moviesById_ = {
        {"m1", {"m1", "Inception"}},
        {"m2", {"m2", "Interstellar"}},
        {"m3", {"m3", "The Matrix"}},
    };

    theatersById_ = {
        {"t1", {"t1", "Downtown Cinema"}},
        {"t2", {"t2", "Grand Multiplex"}},
        {"t3", {"t3", "City Lights Theater"}},
    };

    movieToTheaters_ = {
        {"m1", {"t1", "t2"}},
        {"m2", {"t2", "t3"}},
        {"m3", {"t1", "t3"}},
    };

    for (const auto &[movieId, movie] : moviesById_) {
        const auto mapIt = movieToTheaters_.find(movieId);
        if (mapIt == movieToTheaters_.end()) {
            continue;
        }

        for (const auto &theaterId : mapIt->second) {
            showsByKey_.emplace(makeShowKey(movie.id, theaterId),
                                std::make_shared<Show>(movie.id, theaterId));
        }
    }

    LOG(INFO) << "BookingService initialized with in-memory movie and theater data.";
}

BookingService::~BookingService() = default;
BookingService::BookingService(BookingService &&) noexcept = default;
BookingService &BookingService::operator=(BookingService &&) noexcept = default;

void BookingService::validateShow(const std::string &movieId, const std::string &theaterId) const {
    if (!moviesById_.contains(movieId)) {
        throw std::invalid_argument("Movie not found.");
    }

    if (theatersById_.find(theaterId) == theatersById_.end()) {
        throw std::invalid_argument("Theater not found.");
    }

    const auto key = makeShowKey(movieId, theaterId);
    if (!showsByKey_.contains(key)) {
        throw std::invalid_argument("Selected theater is not showing this movie.");
    }
}

std::vector<Movie> BookingService::listMovies() const {
    std::vector<Movie> movies;
    movies.reserve(moviesById_.size());
    std::transform(moviesById_.cbegin(), moviesById_.cend(), std::back_inserter(movies),
                   [](const auto &pair) { return pair.second; });
    return movies;
}

std::vector<Theater> BookingService::listTheatersForMovie(const std::string &movieId) const {
    std::vector<Theater> theaters;

    const auto mapIt = movieToTheaters_.find(movieId);
    if (mapIt == movieToTheaters_.end()) {
        return theaters;
    }

    theaters.reserve(mapIt->second.size());
    for (const auto &theaterId : mapIt->second) {
        const auto theaterIt = theatersById_.find(theaterId);
        if (theaterIt != theatersById_.end()) {
            theaters.push_back(theaterIt->second);
        }
    }

    return theaters;
}

std::vector<std::string> BookingService::getAvailableSeats(const std::string &movieId,
                                                           const std::string &theaterId) const {
    validateShow(movieId, theaterId);

    const auto showIt = showsByKey_.find(makeShowKey(movieId, theaterId));
    if (showIt == showsByKey_.end()) {
        return {};
    }

    return showIt->second->availableSeats();
}

void BookingService::bookSeats(const std::string &movieId, const std::string &theaterId,
                               const std::vector<std::string> &seatIds) {
    LOG(INFO) << "Booking request received for movie=" << movieId << ", theater=" << theaterId
              << ", seat_count=" << std::to_string(seatIds.size());

    try {
        validateShow(movieId, theaterId);
    } catch (const std::invalid_argument &exception) {
        LOG(WARNING) << "Booking validation failed: " << exception.what();
        throw;
    }

    const auto showIt = showsByKey_.find(makeShowKey(movieId, theaterId));
    if (showIt == showsByKey_.end()) {
        LOG(WARNING) << "Booking failed: show not found for movie=" << movieId
                     << ", theater=" << theaterId;
        throw std::invalid_argument("Selected theater is not showing this movie.");
    }

    try {
        showIt->second->reserveSeats(seatIds);
    } catch (const std::invalid_argument &exception) {
        LOG(WARNING) << "Booking failed: " << exception.what();
        throw;
    }

    LOG(INFO) << "Booking successful for movie=" << movieId << ", theater=" << theaterId;
}

} // namespace movie_booking
