FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libncurses-dev \
    doxygen \
    graphviz \
    valgrind \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN cmake -S . -B build \
    && cmake --build build -j"$(nproc)"

CMD ["./build/tividy"]