#pragma once

#include "show.hpp"

#include <gtest/gtest.h>


namespace movie_booking_tests {

class ShowTests : public ::testing::Test {
  protected:
    movie_booking::Show show{"m1", "t1"};
};

} // namespace movie_booking_tests
