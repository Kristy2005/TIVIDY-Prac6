FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Install build tools, C++ dependencies and Nginx
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libncurses-dev \
    doxygen \
    graphviz \
    valgrind \
    nginx \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy the project
COPY . .

# Build the C++ engine and application
RUN cmake -S . -B build \
    && cmake --build build -j"$(nproc)"

# Copy GUI files into Nginx's web directory
COPY GUI/ /var/www/html/

EXPOSE 80

# Start the GUI web server
CMD ["nginx", "-g", "daemon off;"]