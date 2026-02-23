cmake_minimum_required(VERSION 3.16)
project(MovieTheaterConsumerExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(MovieTheaterBooking CONFIG REQUIRED)

add_executable(consumer_example
    main.cpp
)

target_link_libraries(consumer_example
    PRIVATE
        MovieTheaterBooking::movie_booking
)
