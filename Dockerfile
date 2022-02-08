# syntax=docker/dockerfile:1
FROM ubuntu AS dependencies

# Install dependencies
ENV TZ="America/Chicago" \
    CXX="clang++"
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && \
    apt update --allow-insecure-repositories && \
    apt install -y git cmake clang libicu-dev python sqlite3 libsqlite3-dev python-clang-9 bear swig libpcre3-dev zlib1g-dev

WORKDIR /opt
# Install CppCMS libraries
RUN git clone https://github.com/artyom-beilis/cppcms.git cppcms
RUN mkdir cppcms/build && cd cppcms/build && cmake .. && make install && cd ../..

# Install MuPDF libraries
RUN git clone --recursive git://git.ghostscript.com/mupdf.git mupdf
RUN cd mupdf && ./scripts/mupdfwrap.py -b m01 && mv build/shared-release/libmupdf*.so /usr/local/lib/

# Build files and debug
FROM dependencies AS builder

ENV LD_LIBRARY_PATH="/usr/local/lib/"
RUN apt update && \
    apt install -y valgrind

# Copy files and make executable
WORKDIR /opt/nabu
ADD src src/
ADD include include/
ADD Makefile_release Makefile
RUN mv /opt/mupdf/platform/c++/include/* include/ && \
    mv /opt/mupdf/include/mupdf/* include/mupdf/ && \
    mkdir /var/www /appdata /appdata/logs /appdata/database && \
    make init &&  \
    make
ADD bin/covers/default.png /appdata/covers/default.png

WORKDIR /var/www
ADD bin/resources .
ADD bin/config_release.json config.json

FROM busybox:glibc

# Copy dependencies
COPY --from=dependencies /usr/local/lib/libcppcms.* \
    /usr/local/lib/libbooster.* \
    /usr/local/lib/libmupdf* \
    /usr/lib/x86_64-linux-gnu/libsqlite3.so.0 \
    /usr/lib/x86_64-linux-gnu/libstdc++.so.6 \
    /usr/lib/x86_64-linux-gnu/libm.so.6 \
    /usr/lib/x86_64-linux-gnu/libgcc_s.so.1 \
    /usr/lib/x86_64-linux-gnu/libc.so.6 \
    /usr/lib/x86_64-linux-gnu/libcrypto.so.1.1 \
    /usr/lib/x86_64-linux-gnu/libz.so.1 \
    /usr/lib/x86_64-linux-gnu/libpcre.so.3 \
    /usr/lib/x86_64-linux-gnu/libicuuc.* \
    /usr/lib/x86_64-linux-gnu/libicui18n.* \
    /usr/lib/x86_64-linux-gnu/libpthread.so.0 \
    /usr/lib/x86_64-linux-gnu/libdl.so.2 \
    /usr/lib/x86_64-linux-gnu/libicudata.* \
    /usr/lib/
COPY --from=builder /var/www /var/www
COPY --from=builder /appdata /appdata

VOLUME [ "/appdata", "/media", "/imports" ]

CMD ["/var/www/exec","-c","/var/www/config.json"]