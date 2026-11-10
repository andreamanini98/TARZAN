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

RUN set -eux; \
    for libdir in /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu /usr/lib; do \
        if [ -d "$libdir" ]; then \
            find "$libdir" -name "libomp.so.*" -exec ln -sf {} "$libdir/libomp.so" \; 2>/dev/null || true; \
        fi \
    done; \
    echo "/usr/local/lib" > /etc/ld.so.conf.d/usr-local.conf; \
    ldconfig

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
ENV LD_LIBRARY_PATH=/usr/local/lib:/usr/lib/x86_64-linux-gnu:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH

ENTRYPOINT ["/tarzan/artifact/run_artifact.sh"]
