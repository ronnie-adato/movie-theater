# Movie Theater Booking Service (C++)

A thread-safe, in-memory backend service for booking movie theater seats.

## Features

- List movies and theaters
- Query available seats for a given show
- Book multiple seats atomically
- Reject invalid, duplicate, and already-booked seats
- Handle concurrent booking without over-booking

## Tech Stack

- C++23
- CMake (preset-based flow)
- Conan 2.x (`conanfile.py` is the primary recipe)
- GoogleTest unit tests
- ng-log logging backend

## Seat Model

Default seat catalog is 20 seats: `a1..a10` and `b1..b10`.

## Project Layout

- `include/` public headers
- `src/` library + CLI implementation
- `tests/src/` unit tests:
  - `test_booking_service.cpp`
  - `test_show.cpp`
- `conanfile.py` main Conan recipe for this project
- `conan/recipes/ng-log/conanfile.py` local ng-log recipe
- `Dockerfile` containerized build/test/runtime

## Local Build & Unit Tests

### Linux / macOS

```bash
conan profile detect --force
conan create conan/recipes/ng-log --version=0.8.2 --build=missing -s build_type=Release -vwarning
conan install conanfile.py --output-folder=build/conan --build=missing -s build_type=Release -vwarning
cmake --preset conan-release -DBUILD_TESTS=ON
cmake --build --preset conan-release
ctest --preset conan-release --output-on-failure
./build/conan/build/movie_booking_cli
```

### Windows (PowerShell)

```powershell
conan profile detect --force
conan create conan/recipes/ng-log --version=0.8.2 --build=missing -s build_type=Release -vwarning
conan install conanfile.py --output-folder=build/conan --build=missing -s build_type=Release -vwarning
cmake --preset conan-default -DBUILD_TESTS=ON
cmake --build build/conan/build --config Release
ctest --test-dir build/conan/build --build-config Release --output-on-failure
.\build\conan\build\Release\movie_booking_cli.exe
```

## Docker

Build image:

```bash
docker build -t movie-booking .
```

Run CLI:

```bash
docker run --rm -it movie-booking
```

The Docker build runs Conan install, CMake configure/build, and unit tests (`ctest`) during image creation.

## Logging

- App runtime logging uses ng-log (`LOG(INFO)`, `LOG(WARNING)` in service and CLI paths).
- CI and Docker Conan logs are intentionally set to warning level to keep output concise.
- Local builds can increase Conan verbosity when debugging, for example:

```bash
conan install conanfile.py --output-folder=build/conan --build=missing -s build_type=Release -vdebug
```

Quick toggles via VS Code Tasks (`Terminal: Run Task`):

```bash
Conan: Bootstrap (warning)       # first-time setup (profile + ng-log + install)
Conan: Install (warning)         # warning-level logs via CMake target
Conan: Install (debug)           # debug-level logs
Conan: Install (trace)           # trace-level logs
```

`Conan: Bootstrap (warning)` now automatically clears stale `CMakeUserPresets.json`
before regenerating Conan presets/dependencies.

Quick toggles via CMake-generated targets (after configure):

```bash
cmake --build --preset conan-release --target conan-install
cmake --build --preset conan-release --target conan-install-debug
cmake --build --preset conan-release --target conan-install-trace
```

## API Docs (Doxygen)

Generate API documentation:

```bash
cmake --build --preset conan-release --target docs
```

If Doxygen is installed, HTML docs are generated under `build/conan/build/docs/html`.

## Unit Test Coverage Areas

- `BookingService` happy path and validation failures
- Atomic behavior when some requested seats are already booked
- Concurrent booking contention on the same seat
- `Show` invariants (identifiers, availability updates, atomic failures)

## Concurrency Guarantees

`bookSeats(...)` is atomic per show (`movieId + theaterId`):

- If any seat is invalid or already booked, no seat is booked.
- If all seats are valid and free, all requested seats are booked.
- Parallel requests cannot double-book the same seat.

## Reflection

### What aspect of this exercise was most interesting?

Designing the booking flow to preserve strict atomicity under concurrent requests while keeping the API simple for CLI/UI consumers was the most interesting part.

### What was most cumbersome?

Getting the project to compile consistently across platforms was difficult, even with—and at times especially with—Conan and CMake because generator, preset, and linker behavior varied between Linux, macOS, and Windows.
