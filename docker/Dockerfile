FROM ghcr.io/linuxserver/baseimage-ubuntu:noble AS builder

# version, it can be a TAG or a branch
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
       desktop-file-utils \
       gcc \
       g++ \
       git \
       qt6-tools-dev \
       qt6-base-dev-tools \
       qmake6 \
       qmake6-bin \
       qt6-base-dev \
       qt6-multimedia-dev \
       qt6-tools-dev-tools \
       libgl-dev \
       qt6-l10n-tools \
       libqt6opengl6-dev \
       libunarr-dev \
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
RUN git clone https://github.com/YACReader/yacreader.git /src/git && \
    cd /src/git && \
    git checkout $YACR_VERSION

# get 7zip source
RUN cd /src/git/compressed_archive && \
    wget "https://github.com/YACReader/yacreader-7z-deps/blob/main/7z2301-src.7z?raw=true" -O /src/git/compressed_archive/7z2301-src.7z && \
    7z x /src/git/compressed_archive/7z2301-src.7z -o/src/git/compressed_archive/lib7zip 

# build yacreaderlibraryserver
RUN cd /src/git/YACReaderLibraryServer && \
    qmake6 PREFIX=/app CONFIG+="7zip server_standalone" YACReaderLibraryServer.pro && \
    qmake6 -v && \
    make  && \
    make install

# install 7z.so with RAR support
RUN echo "Building and installing 7z.so with RAR support..." && \
    cd "/src/git/compressed_archive/lib7zip/CPP/7zip/Bundles/Format7zF" && \
    make -f makefile.gcc && \
    mkdir -p /app/lib/7zip && \
    cp ./_o/7z.so /app/lib/7zip    

# Stage 2: Runtime stage
FROM ghcr.io/linuxserver/baseimage-ubuntu:noble

# env variables
ENV APPNAME="YACReaderLibraryServer"
ENV HOME="/config"
LABEL maintainer="luisangelsm"

# Copy the built application from the builder stage
COPY --from=builder /app /app

# runtime packages
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
       libqt6core5compat6 \
       libpoppler-qt6-3t64 \
       qt6-image-formats-plugins \ 
       libqt6network6t64 \
       libqt6sql6-sqlite && \
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
