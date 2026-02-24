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
ENV CONAN_LOG_LEVEL=warning

RUN pip install --no-cache-dir --upgrade pip conan

WORKDIR /app
COPY . .

RUN conan profile detect --force \
    && conan create conan/recipes/ng-log --version=0.8.2 --build=missing -s build_type=Release -v${CONAN_LOG_LEVEL} \
    && conan install conanfile.py --output-folder=build/conan --build=missing -s build_type=Release -v${CONAN_LOG_LEVEL} \
    && cmake --preset conan-release -DBUILD_TESTS=ON -DENABLE_PACKAGE_CONSUMER_TEST=ON \
    && cmake --build --preset conan-release -j \
    && ctest --preset conan-release --output-on-failure

CMD ["./build/conan/build/movie_booking_cli"]
