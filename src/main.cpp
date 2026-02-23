#include "booking_error.hpp"
#include "booking_service.hpp"

#include <ng-log/logging.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

void ShutdownLogging() { nglog::ShutdownLogging(); }

std::string trim(const std::string &value) {
    const auto first = value.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return "";
    }

    const auto last = value.find_last_not_of(" \t\n\r");
    return value.substr(first, last - first + 1);
}

std::vector<std::string> parseSeatsCsv(const std::string &input) {
    std::vector<std::string> seats;
    std::istringstream stream(input);
    std::string token;

    while (std::getline(stream, token, ',')) {
        token = trim(token);
        if (!token.empty()) {
            seats.push_back(token);
        }
    }

    return seats;
}

int readIndex() {
    int index = -1;
    std::cin >> index;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return index;
}

void printSeats(const std::vector<std::string> &seats) {
    if (seats.empty()) {
        std::cout << "No seats available.\n";
        return;
    }

    for (std::size_t i = 0; i < seats.size(); ++i) {
        std::cout << seats[i];
        if ((i + 1) % 10 == 0 || i + 1 == seats.size()) {
            std::cout << '\n';
        } else {
            std::cout << ", ";
        }
    }
}

} // namespace

int main(int argc, char **argv) {
    nglog::InitializeLogging(argc > 0 ? argv[0] : "movie_booking_cli");
    LOG(INFO) << "CLI started.";

    movie_booking::BookingService service;

    std::cout << "Movie Theater Booking CLI\n";
    std::cout << "-------------------------\n";

    while (true) {
        const auto movies = service.listMovies();
        std::cout << "\nPlaying movies:\n";
        for (std::size_t i = 0; i < movies.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << movies[i].title << " (" << movies[i].id
                      << ")\n";
        }
        std::cout << "  0. Exit\n";
        std::cout << "Select a movie: ";

        const int movieChoice = readIndex();
        if (movieChoice == 0) {
            std::cout << "Goodbye.\n";
            LOG(INFO) << "CLI terminated by user.";
            ShutdownLogging();
            return 0;
        }

        if (movieChoice < 1 || static_cast<std::size_t>(movieChoice) > movies.size()) {
            std::cout << "Invalid movie selection.\n";
            continue;
        }

        const auto &selectedMovie = movies[static_cast<std::size_t>(movieChoice - 1)];
        const auto theaters = service.listTheatersForMovie(selectedMovie.id);
        if (theaters.empty()) {
            std::cout << "No theaters found for selected movie.\n";
            continue;
        }

        std::cout << "\nTheaters showing " << selectedMovie.title << ":\n";
        for (std::size_t i = 0; i < theaters.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << theaters[i].name << " (" << theaters[i].id
                      << ")\n";
        }
        std::cout << "Select a theater: ";

        const int theaterChoice = readIndex();
        if (theaterChoice < 1 || static_cast<std::size_t>(theaterChoice) > theaters.size()) {
            std::cout << "Invalid theater selection.\n";
            continue;
        }

        const auto &selectedTheater = theaters[static_cast<std::size_t>(theaterChoice - 1)];
        const auto availableSeats = service.getAvailableSeats(selectedMovie.id, selectedTheater.id);

        std::cout << "\nAvailable seats for " << selectedMovie.title << " at "
                  << selectedTheater.name << ":\n";
        printSeats(availableSeats);

        std::cout << "Enter seats to book (comma-separated, e.g. a1,a2) or empty to cancel: ";
        std::string seatInput;
        std::getline(std::cin, seatInput);

        seatInput = trim(seatInput);
        if (seatInput.empty()) {
            std::cout << "Booking canceled.\n";
            continue;
        }

        auto requestedSeats = parseSeatsCsv(seatInput);
        std::transform(requestedSeats.begin(), requestedSeats.end(), requestedSeats.begin(),
                       [](std::string seat) {
                           std::transform(
                               seat.begin(), seat.end(), seat.begin(),
                               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                           return seat;
                       });

        try {
            service.bookSeats(selectedMovie.id, selectedTheater.id, requestedSeats);
            std::cout << "Booking was succefull." << std::endl;
        } catch (movie_booking::SeatAlreadyBooked &exception) {
            std::cout << exception.what() << std::endl;
            std::cout << "Rejected seats: ";
            for (std::size_t i = 0; i < exception.rejectedSeats.size(); ++i) {
                std::cout << exception.rejectedSeats[i]
                          << (i + 1 < exception.rejectedSeats.size() ? ", " : "\n");
            }
        } catch (std::exception &exception) {
            std::cout << exception.what() << std::endl;
        }
    }
}
