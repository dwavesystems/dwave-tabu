#!/usr/bin/env bash
BUILD_PREFIX="${BUILD_PREFIX:-/usr/local}"

install_from_source() {
    local baseurl="$1"
    local name="$2"
    local ver="$3"

    local fullname="${name}-${ver}"
    local archive="$fullname.tar.gz"
    local url="$baseurl/$archive"
    curl -sLO "$url"
    tar xzf "$archive"
    (
        cd "$fullname"
        ./configure --prefix="$BUILD_PREFIX"
        make -j8
        make install
    )
}

install_pcre() {
    install_from_source "ftp://ftp.pcre.org/pub/pcre/" "pcre" "8.42"
}

install_swig() {
    install_pcre
    install_from_source "https://sourceforge.net/projects/swig/files/swig/swig-3.0.12/" "swig" "3.0.12"
}
