FROM ghcr.io/linuxserver/baseimage-ubuntu:resolute AS base

# ============================================================================
# Stage: sevenzip-builder
# ============================================================================
FROM base AS sevenzip-builder

# get 7zip source
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
       7zip \
       build-essential \
       make \
       wget && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN cd /tmp && \
    wget "https://github.com/YACReader/yacreader-7z-deps/blob/main/7z2301-src.7z?raw=true" -O 7z2301-src.7z && \
    7z x 7z2301-src.7z -o/tmp/lib7zip

# install 7z.so with RAR support
RUN cd "/tmp/lib7zip/CPP/7zip/Bundles/Format7zF" && \
    make -f makefile.gcc && \
    mkdir -p /app/lib/7zip && \
    cp ./_o/7z.so /app/lib/7zip

# ============================================================================
# Stage: yacreader-builder
# ============================================================================
FROM base AS yacreader-builder

# repository URL and version, which can be a tag, branch, or commit SHA
ARG YACR_REPOSITORY="https://github.com/YACReader/yacreader.git"
ARG YACR_VERSION="develop"

# env variables
ARG DEBIAN_FRONTEND="noninteractive"
ENV APPNAME="YACReaderLibraryServer"
ENV HOME="/config"
LABEL maintainer="luisangelsm"

# install build packages
RUN \
    apt-get update && \
    apt-get install -y --no-install-recommends \
       build-essential \
       cmake \
       ninja-build \
       desktop-file-utils \
       gcc \
       g++ \
       git \
       qt6-tools-dev \
       qt6-base-dev-tools \
       qt6-base-dev \
       qt6-multimedia-dev \
       qt6-tools-dev-tools \
       libgl-dev \
       qt6-l10n-tools \
       qt6-declarative-dev \
       libqt6svg6-dev \
       libqt6core5compat6-dev \
       libbz2-dev \
       libglu1-mesa-dev \
       liblzma-dev \
       libqt6gui6 \
       libqt6multimedia6 \
       libqt6network6 \
       libqt6qml6 \
       libqt6quickcontrols2-6 \
       qt6-image-formats-plugins \
       libqt6sql6 \
       libqt6sql6-sqlite \
       make \
       sqlite3 \
       libsqlite3-dev \
       unzip \
       wget \
       7zip \
       7zip-rar \
       libpoppler-qt6-dev \
       zlib1g-dev && \
    ldconfig

# clone YACReader repo
RUN git clone "$YACR_REPOSITORY" /src/git && \
    cd /src/git && \
    git checkout "$YACR_VERSION"

# build yacreaderlibraryserver (7zip source is auto-downloaded by CMake FetchContent)
RUN cd /src/git && \
    cmake -B build -G Ninja \
      -DCMAKE_INSTALL_PREFIX=/app \
      -DDECOMPRESSION_BACKEND=7zip \
      -DPDF_BACKEND=poppler \
      -DBUILD_SERVER_STANDALONE=ON \
      -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --parallel && \
    cmake --install build

# Use pre-built 7z.so from sevenzip-builder (provides RAR support)
COPY --from=sevenzip-builder /app/lib/7zip /app/lib/7zip

# ============================================================================
# Stage: runtime
# ============================================================================
FROM base AS runtime

# env variables
ENV APPNAME="YACReaderLibraryServer"
ENV HOME="/config"
LABEL maintainer="luisangelsm"

# Copy the built application from the builder stage
COPY --from=yacreader-builder /app /app

# runtime packages
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
       libqt6core5compat6 \
       libpoppler-qt6-3t64 \
       qt6-image-formats-plugins \
       libqt6network6 \
       libqt6sql6-sqlite \
       kimageformat6-plugins && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# set ENV
ENV LC_ALL="en_US.UTF-8" \
    PATH="/app/bin:${PATH}"

# copy files
COPY root.tar.gz /
# Extract the contents of root.tar.gz into the / directory
RUN tar -xvpzf /root.tar.gz -C /

# ports and volumes
EXPOSE 8080
VOLUME ["/config", "/comics"]
