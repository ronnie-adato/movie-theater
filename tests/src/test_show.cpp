#include "booking_error.hpp"

#include "show_fixture.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <algorithm>
#include <thread>
#include <vector>

namespace movie_booking_tests {

bool HasSeat(const std::vector<std::string> &seats, const std::string &seatId) {
    return std::find(seats.begin(), seats.end(), seatId) != seats.end();
}

TEST_F(ShowTests, ConstructorAndIdentifiers) {
    EXPECT_EQ(show.movieId(), "m1");
    EXPECT_EQ(show.theaterId(), "t1");
}

TEST_F(ShowTests, AvailableSeatsReflectReservations) {
    EXPECT_NO_THROW(show.reserveSeats({"a1", "a3"}));

    const auto available = show.availableSeats();
    EXPECT_EQ(available.size(), 18U);
    EXPECT_FALSE(HasSeat(available, "a1"));
    EXPECT_FALSE(HasSeat(available, "a3"));
    EXPECT_TRUE(HasSeat(available, "a2"));
    EXPECT_TRUE(HasSeat(available, "b10"));
}

TEST_F(ShowTests, DuplicateOrInvalidSeatIsRejected) {
    EXPECT_THROW(show.reserveSeats({"a2", "a2"}), movie_booking::InvalidSeat);
    EXPECT_THROW(show.reserveSeats({"z9"}), movie_booking::InvalidSeat);
}

TEST_F(ShowTests, AlreadyBookedSeatIsRejectedAtomically) {
    EXPECT_NO_THROW(show.reserveSeats({"a4"}));

    EXPECT_THROW(show.reserveSeats({"a4", "a1"}), movie_booking::SeatAlreadyBooked);

    const auto available = show.availableSeats();
    EXPECT_EQ(available.size(), 19U);
    EXPECT_FALSE(HasSeat(available, "a4"));
    EXPECT_TRUE(HasSeat(available, "a1"));
    EXPECT_TRUE(HasSeat(available, "a2"));
    EXPECT_TRUE(HasSeat(available, "b10"));
}

TEST_F(ShowTests, ConcurrentBookingOneSucceedsOthersFail) {
    std::atomic<bool> start{false};
    std::atomic<int> successCount{0};
    std::atomic<int> failureCount{0};
    std::atomic<int> seatAlreadyBookedCount{0};

    std::vector<std::thread> threads;
    threads.reserve(16);

    for (int i = 0; i < 16; ++i) {
        threads.emplace_back(
            [this, &start, &successCount, &failureCount, &seatAlreadyBookedCount]() {
                while (!start.load(std::memory_order_acquire)) {
                }
                try {
                    show.reserveSeats({"a2"});
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
    EXPECT_EQ(failureCount.load(std::memory_order_acquire), 15);
    EXPECT_EQ(seatAlreadyBookedCount.load(std::memory_order_acquire), 15);

    const auto available = show.availableSeats();
    EXPECT_EQ(available.size(), 19U);
    EXPECT_FALSE(HasSeat(available, "a2"));
    EXPECT_TRUE(HasSeat(available, "a1"));
    EXPECT_TRUE(HasSeat(available, "a3"));
    EXPECT_TRUE(HasSeat(available, "b10"));
}

} // namespace movie_booking_tests
