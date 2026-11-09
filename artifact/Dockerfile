FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive
ARG BUILD_JOBS=0
ARG BOOST_VERSION=1.92.0

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        ninja-build \
        pkg-config \
        ca-certificates \
        curl \
        git \
        libtbb-dev \
        libomp-dev \
        libabsl-dev \
        python3 \
        python3-pip \
        python3-numpy \
        python3-pandas \
        python3-matplotlib \
        python3-seaborn \
        python3-jinja2 \
    && rm -rf /var/lib/apt/lists/*

RUN set -eux; \
    JOBS=$([ "${BUILD_JOBS}" -le 0 ] && nproc || echo "${BUILD_JOBS}"); \
    BOOST_US=$(echo "${BOOST_VERSION}" | tr . _); \
    cd /tmp; \
    curl -fsSL -o boost.tar.bz2 \
      "https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_US}.tar.bz2"; \
    tar -xjf boost.tar.bz2; \
    cd "boost_${BOOST_US}"; \
    ./bootstrap.sh --prefix=/usr/local; \
    ./b2 -j"${JOBS}" variant=release link=shared threading=multi install; \
    ldconfig; \
    cd /; rm -rf /tmp/boost*

WORKDIR /tarzan
COPY . /tarzan
RUN chmod +x /tarzan/entrypoint.sh /tarzan/artifact/run_artifact.sh

RUN set -eux; \
    JOBS=$([ "${BUILD_JOBS}" -le 0 ] && nproc || echo "${BUILD_JOBS}"); \
    mkdir -p build && cd build; \
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_TESTING=ON \
          -DENABLE_APPIMAGE=ON \
          -DENABLE_OPTIMIZATION=ON \
          -DCMAKE_PREFIX_PATH=/usr/local \
          ..; \
    make -j"${JOBS}"

ENV ROOT_DIR=/tarzan
ENV ENTRYPOINT=/tarzan/entrypoint.sh
ENV GAMES_DIR=/tarzan/executables/games_executables

ENTRYPOINT ["/tarzan/artifact/run_artifact.sh"]
