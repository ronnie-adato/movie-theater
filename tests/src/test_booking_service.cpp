#include "booking_error.hpp"

#include "booking_service_fixture.hpp"

#include <atomic>
#include <thread>
#include <vector>

namespace movie_booking_tests {

bool BookingServiceFixture::HasSeat(const std::vector<std::string> &seats,
                                    const std::string &seatId) const {
    for (const auto &seat : seats) {
        if (seat == seatId) {
            return true;
        }
    }
    return false;
}

TEST_F(BookingServiceFixture, MovieAndTheaterListing) {
    const auto movies = service.listMovies();
    EXPECT_FALSE(movies.empty());

    const auto theaters = service.listTheatersForMovie(kMovieId);
    EXPECT_FALSE(theaters.empty());
}

TEST_F(BookingServiceFixture, BookingSuccessAndAvailability) {
    EXPECT_NO_THROW(service.bookSeats(kMovieId, kTheaterId, {"a1", "a2"}));

    const auto available = service.getAvailableSeats(kMovieId, kTheaterId);
    EXPECT_FALSE(HasSeat(available, "a1"));
    EXPECT_FALSE(HasSeat(available, "a2"));
}

TEST_F(BookingServiceFixture, AtomicBookingOnConflict) {
    EXPECT_NO_THROW(service.bookSeats(kMovieId, kTheaterId, {"a3"}));

    EXPECT_THROW(service.bookSeats(kMovieId, kTheaterId, {"a3", "a4"}),
                 movie_booking::SeatAlreadyBooked);

    const auto available = service.getAvailableSeats(kMovieId, kTheaterId);
    EXPECT_TRUE(HasSeat(available, "a4"));
}

TEST_F(BookingServiceFixture, ConcurrentBookingNoOverbooking) {
    std::atomic<bool> start{false};
    std::atomic<int> successCount{0};
    std::atomic<int> failureCount{0};
    std::atomic<int> seatAlreadyBookedCount{0};
    auto &bookingService = service;

    std::vector<std::thread> threads;
    threads.reserve(32);

    for (int i = 0; i < 32; ++i) {
        threads.emplace_back(
            [&bookingService, &start, &successCount, &failureCount, &seatAlreadyBookedCount]() {
                while (!start.load(std::memory_order_acquire)) {
                }
                try {
                    bookingService.bookSeats(kMovieId, kTheaterId, {"a5"});
                    successCount.fetch_add(1, std::memory_order_acq_rel);
                } catch (const movie_booking::SeatAlreadyBooked &) {
                    failureCount.fetch_add(1, std::memory_order_acq_rel);
                    seatAlreadyBookedCount.fetch_add(1, std::memory_order_acq_rel);
                } catch (...) {
                    failureCount.fetch_add(1, std::memory_order_acq_rel);
                }
            });
    }

    start.store(true, std::memory_order_release);

    for (auto &thread : threads) {
        thread.join();
    }

    EXPECT_EQ(successCount.load(std::memory_order_acquire), 1);
    EXPECT_EQ(failureCount.load(std::memory_order_acquire), 31);
    EXPECT_EQ(seatAlreadyBookedCount.load(std::memory_order_acquire), 31);

    const auto available = service.getAvailableSeats(kMovieId, kTheaterId);
    EXPECT_FALSE(HasSeat(available, "a5"));
}

TEST_F(BookingServiceFixture, InvalidSeatRejected) {
    EXPECT_THROW(service.bookSeats(kMovieId, kTheaterId, {"z99"}), movie_booking::InvalidSeat);
}

} // namespace movie_booking_tests
