# Movie Theater Booking Service (C++)

[![CI](https://github.com/<your-username>/<repo-name>/actions/workflows/ci.yml/badge.svg)](https://github.com/<your-username>/<repo-name>/actions/workflows/ci.yml)

A thread-safe, in-memory backend service for booking movie theater seats.

## Features

- View all playing movies
- Select a movie
- List theaters showing that movie
- Select a theater
- View available seats (`a1` .. `a20`)
- Book one or more seats atomically
- Handles concurrent booking requests without over-booking

## Technical Notes

- Language: C++17
- Build system: CMake
- Storage: in-memory (no DBMS)
- API: documented headers in `include/`
- Unit tests: GoogleTest-based suite under `tests/` (auto-fetched by CMake when `BUILD_TESTS=ON`)
- Concurrency: per-show locking ensures thread-safe seat booking
- Logging: ng-log (required)

## Project Structure

- `include/movie.hpp` + `src/movie.cpp` - movie model class
- `include/theater.hpp` + `src/theater.cpp` - theater model class
- `include/show.hpp` + `src/show.cpp` - movie-in-theater reservation class (seat inventory, locking, booking)
- `include/booking_result.hpp` + `src/booking_result.cpp` - booking result class
- `include/booking_service.hpp` - backend service API for consumers
- `src/booking_service.cpp` - service orchestration (delegates reservation state/logic to `Show`)
- `src/main.cpp` - simple CLI consumer for manual usage
- `tests/src/test_booking_service.cpp` - service unit/concurrency tests
- `tests/models/` - per-model/unit tests (`Movie`, `Theater`, `BookingResult`, `BookingError`, `Show`)
- `Dockerfile` - optional container build/run support

## Service Architecture

### Core components

- `BookingService`

  - Owns catalog state (`movies_`, `theatersById_`, `movieToTheaters_`).
  - Owns show registry (`showsByKey_`) keyed by `movieId|theaterId`.
  - Validates movie/theater/show existence and delegates reservation behavior to `Show`.

- `Show` (movie in a specific theater)

  - Owns reservation state (`seatCatalog_`, `bookedSeats_`).
  - Provides thread-safe operations via internal mutex:
    - `availableSeats()`
    - `reserveSeats(...)`
  - Enforces atomic booking semantics for each show.

- `BookingResult`

  - Encapsulates operation outcome (`success`, `error`, accepted/rejected seats, message).

### Booking flow

1. Client calls `BookingService::bookSeats(movieId, theaterId, seatIds)`.
1. Service validates that movie, theater, and show exist.
1. Service resolves `Show` by key and delegates to `Show::reserveSeats(...)`.
1. `Show` validates requested seats and performs reservation under lock.
1. Result is returned as `BookingResult`.

### Concurrency model

- Lock granularity is per-show (`Show` instance).
- Concurrent requests for different shows proceed independently.
- Concurrent requests for the same show are serialized by that show's mutex.
- Exactly one request can book a specific free seat; competing requests for the same seat fail cleanly.

## Build and Run (Cross-Platform)

`ng-log` is required, and this project is configured to use Conan-generated CMake presets.

### Conan prerequisites

- CMake >= 3.16
- A C++17 compiler
  - Linux: GCC or Clang
  - macOS: AppleClang (Xcode Command Line Tools)
  - Windows: MSVC (Visual Studio 2022+) or MinGW
- Conan 2.x

 1. If any error occurs (invalid movie, theater, seat, or already booked), an exception is thrown. Otherwise, the booking succeeds atomically.

Linux / macOS:

```bash
conan profile detect --force
conan export conan/recipes/ng-log --version=0.8.2
conan install conanfile.txt --output-folder=build/conan --build=missing -s build_type=Release
```

Windows (PowerShell):

```powershell
conan profile detect --force
conan export conan/recipes/ng-log --version=0.8.2
conan install conanfile.txt --output-folder=build/conan --build=missing -s build_type=Release
```

______________________________________________________________________

### Linux / macOS (Preset build)

```bash
cmake --preset conan-release -DBUILD_TESTS=ON -DENABLE_PACKAGE_CONSUMER_TEST=ON
cmake --build --preset conan-release
ctest --test-dir build/conan/build/Release --output-on-failure
./build/conan/build/Release/movie_booking_cli
```

### Windows (PowerShell, Preset build)

```powershell
cmake --preset conan-release -DBUILD_TESTS=ON -DENABLE_PACKAGE_CONSUMER_TEST=ON
cmake --build --preset conan-release
ctest --test-dir build/conan/build/Release -C Release --output-on-failure
.\build\conan\build\Release\movie_booking_cli.exe
```

### Format source code (clang-format)

```bash
cmake --build --preset conan-release --target format
```

### Format Markdown files

```bash
cmake --build --preset conan-release --target format-md
```

`format-md` automatically uses `mdformat` (preferred) or `prettier` if available.

Install one of the Markdown formatters:

Linux / macOS:

```bash
python3 -m pip install mdformat
# or
npm install -g prettier
```

Windows (PowerShell):

```powershell
py -m pip install mdformat
# or
npm install -g prettier
```

### Format everything

```bash
cmake --build --preset conan-release --target format-all
```

## Install and Package Artifacts

### Install locally from build output

Linux / macOS:

```bash
cmake --install build/conan/build/Release --prefix ./dist
```

Windows (PowerShell):

```powershell
cmake --install build/conan/build/Release --config Release --prefix .\dist
```

### Create distributable archives (CPack)

Linux / macOS:

```bash
cd build/conan/build/Release
cpack
```

Windows (PowerShell):

```powershell
cd build/conan/build/Release
cpack -C Release
```

## Consume as a CMake Package

After installing to a prefix (for example `./dist`), downstream projects can use:

```cmake
find_package(MovieTheaterBooking CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE MovieTheaterBooking::movie_booking)
```

Example configure command (Linux/macOS):

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/absolute/path/to/dist
```

Example configure command (Windows PowerShell):

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH=C:\absolute\path\to\dist
```

### Runnable consumer example

An example consumer project is available in `consumer-example/`.

Linux / macOS:

```bash
cmake --preset conan-release -DBUILD_TESTS=ON
cmake --build --preset conan-release
cmake --install build/conan/build/Release --prefix ./dist
cmake -S consumer-example -B consumer-example/build -DCMAKE_PREFIX_PATH=$PWD/dist
cmake --build consumer-example/build
./consumer-example/build/consumer_example
```

Windows (PowerShell):

```powershell
cmake --preset conan-release -DBUILD_TESTS=ON
cmake --build --preset conan-release
cmake --install build/conan/build/Release --config Release --prefix .\dist
cmake -S consumer-example -B consumer-example\build -DCMAKE_PREFIX_PATH=$PWD\dist
cmake --build consumer-example\build --config Release
.\consumer-example\build\Release\consumer_example.exe
```

### Optional automated consumer integration test (CTest)

Enable this to make `ctest` automatically verify package consumption end-to-end:

Linux / macOS:

```bash
cmake --preset conan-release -DBUILD_TESTS=ON -DENABLE_PACKAGE_CONSUMER_TEST=ON
cmake --build --preset conan-release
ctest --test-dir build/conan/build/Release --output-on-failure
```

Windows (PowerShell):

```powershell
cmake --preset conan-release -DBUILD_TESTS=ON -DENABLE_PACKAGE_CONSUMER_TEST=ON
cmake --build --preset conan-release
ctest --test-dir build/conan/build/Release -C Release --output-on-failure
```

## Run tests from VS Code Testing panel

1. Install recommended extensions (`ms-vscode.cmake-tools`, `ms-vscode.cpptools`).
1. Open this workspace in VS Code.
1. Run command: `CMake: Configure`.
1. Open the **Testing** panel.
1. Click **Refresh Tests** (if needed), then run `movie_booking_tests` or all tests.

Workspace settings already default to the `conan-release` preset and enable CMake test explorer integration.

## API Usage Example

```cpp
#include "booking_service.hpp"

movie_booking::BookingService service;
auto movies = service.listMovies();
auto theaters = service.listTheatersForMovie("m1");
auto seats = service.getAvailableSeats("m1", "t1");
auto result = service.bookSeats("m1", "t1", {"a1", "a2"});
```
## Concurrency Behavior
`bookSeats(...)` is atomic for a given show (movie + theater):
- If any requested seat is invalid or already booked, the request fails and books nothing.
- If all requested seats are valid and free, all are booked together.
- Concurrent requests are synchronized so the same seat cannot be double-booked.
Internally, reservation state and synchronization are encapsulated in `Show`, which represents a specific movie in a specific theater.
## Running with Docker (Bonus)
Build image:
```bash
docker build -t movie-booking .
```

Run CLI:
 try {
     service.bookSeats("m1", "t1", {"a1", "a2"});
     // Booking succeeded
 } catch (const movie_booking::InvalidSeat &ex) {
     // Handle invalid seat (ex.what() gives details)
 } catch (const movie_booking::SeatAlreadyBooked &ex) {
     // Handle already booked seat (ex.rejectedSeats has details)
 } catch (const std::invalid_argument &ex) {
     // Handle other validation errors (movie/theater/show not found)
 }
```bash
docker run --rm -it movie-booking
```

## Conan Package Management

- If any requested seat is invalid or already booked, an exception is thrown and no seats are booked.
- If all requested seats are valid and free, all are booked together.
- Concurrent requests are synchronized so the same seat cannot be double-booked.
- Use `conanfile.py` for extensibility (custom build/package hooks).

Conan requirements:

- `ng-log/0.8.2` (required)

`ng-log/0.8.2` is provided in this repository as a local Conan recipe under
`conan/recipes/ng-log/`.

### Prerequisites

- Conan 2.x installed

### Linux / macOS (Conan 2)

```bash
conan profile detect --force
conan export conan/recipes/ng-log --version=0.8.2
conan install conanfile.txt --output-folder=build/conan --build=missing -s build_type=Release
cmake --preset conan-release -DBUILD_TESTS=ON
cmake --build --preset conan-release
ctest --test-dir build/conan/build/Release --output-on-failure
./build/conan/build/Release/movie_booking_cli
```

To verify `ng-log` is active, check configure output includes:

```text
-- ng-log found via Conan: logging backend enabled
```

### Linux / macOS (Conan 2, Python recipe)

```bash
conan profile detect --force
conan create . --build=missing -s build_type=Release
```

### Windows (PowerShell, Conan 2 + MSVC)

```powershell
conan profile detect --force
conan export conan/recipes/ng-log --version=0.8.2
conan install conanfile.txt --output-folder=build/conan --build=missing -s build_type=Release
cmake --preset conan-release -DBUILD_TESTS=ON
cmake --build --preset conan-release
ctest --test-dir build/conan/build/Release -C Release --output-on-failure
.\build\conan\build\Release\movie_booking_cli.exe
```

To verify `ng-log` is active, check configure output includes:

```text
-- ng-log found via Conan: logging backend enabled
```

### Windows (PowerShell, Conan 2 Python recipe)

```powershell
conan profile detect --force
conan create . --build=missing -s build_type=Release
```

## Reflection

### What aspect was most interesting?

Designing the booking operation to be both atomic and thread-safe while keeping the code simple and readable in an in-memory system.

### What was most cumbersome?

Balancing strict seat-validation and conflict handling with a minimal API that remains easy for CLI/UI consumers to integrate.