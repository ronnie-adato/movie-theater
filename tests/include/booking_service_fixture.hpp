#pragma once

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "booking_service.hpp"

namespace movie_booking_tests {

class BookingServiceFixture : public ::testing::Test {
  protected:
    static constexpr const char* kMovieId = "m1";
    static constexpr const char* kTheaterId = "t1";

    bool HasSeat(const std::vector<std::string>& seats, const std::string& seatId) const;

    movie_booking::BookingService service;
};

} // namespace movie_booking_tests
