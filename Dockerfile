# Build stage
FROM ubuntu:22.04
LABEL Description="Build environment"

SHELL ["/bin/bash", "-c"]

RUN apt-get update && apt-get -y --no-install-recommends install \
    build-essential \
    clang \
    cmake \
    g++ \
    net-tools

WORKDIR /ethernet

COPY . .

RUN cmake -S . -B build && cmake --build build && cmake --install build

WORKDIR ./build/bin

# Copy startup scripts
COPY start_server.sh .
COPY start_client.sh .

# Make startup scripts executable
RUN chmod +x start_server.sh
RUN chmod +x start_client.sh