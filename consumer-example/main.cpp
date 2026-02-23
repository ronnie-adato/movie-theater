#include <iostream>

#include "booking_service.hpp"

int main() {
    movie_booking::BookingService service;

    const auto movies = service.listMovies();
    std::cout << "Movies available: " << movies.size() << '\n';

    if (!movies.empty()) {
        const auto theaters = service.listTheatersForMovie(movies.front().id);
        std::cout << "Theaters for first movie: " << theaters.size() << '\n';
    }

    return 0;
}
