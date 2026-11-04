FROM archlinux:latest

RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm \
    base-devel \
    cmake \
    gcc \
    gcc-libs \
    glibc \
    linux-api-headers \
    tbb \
    openmp \
    boost \
    boost-libs \
    abseil-cpp \
    python \
    python-pip \
    git && \
    pacman -Scc --noconfirm

RUN pip install --break-system-packages --no-cache-dir numpy pandas matplotlib seaborn jinja2

WORKDIR /tarzan
COPY . /tarzan
RUN chmod +x /tarzan/entrypoint.sh /tarzan/artifact/run_artifact.sh

# BUILD_JOBS=0 usa tutti i core disponibili (nproc). 
# Può essere sovrascritto es. --build-arg BUILD_JOBS=15 per macchine con 16GB di RAM.
ARG BUILD_JOBS=0

RUN mkdir -p build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_TESTING=ON \
          -DENABLE_APPIMAGE=ON \
          -DENABLE_OPTIMIZATION=ON \
          .. && \
    if [ "\$BUILD_JOBS" -le 0 ]; then make -j$(nproc); else make -j$BUILD_JOBS; fi

ENV ROOT_DIR=/tarzan
ENV ENTRYPOINT=/tarzan/entrypoint.sh
ENV GAMES_DIR=/tarzan/executables/games_executables

ENTRYPOINT ["/tarzan/artifact/run_artifact.sh"]
