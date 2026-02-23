FROM ubuntu:24.04

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        python3 \
        python3-venv \
        ca-certificates \
        git \
    && rm -rf /var/lib/apt/lists/*

RUN python3 -m venv /opt/venv
ENV PATH="/opt/venv/bin:${PATH}"

RUN pip install --no-cache-dir --upgrade pip conan

WORKDIR /app
COPY . .

RUN conan profile detect --force \
    && conan export conan/recipes/ng-log --version=0.8.2 \
    && conan install conanfile.txt --output-folder=build/conan --build=missing -s build_type=Release \
    && cmake --preset conan-release -DBUILD_TESTS=ON -DENABLE_PACKAGE_CONSUMER_TEST=ON \
    && cmake --build --preset conan-release -j \
    && ctest --test-dir build/conan/build/Release --output-on-failure

CMD ["./build/conan/build/Release/movie_booking_cli"]
