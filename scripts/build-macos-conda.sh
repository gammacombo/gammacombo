#!/usr/bin/env bash
# Configure, build and install GammaCombo (core + the modules in
# cmake/combiners.cmake) with the toolchain of an activated conda-forge env on
# macOS, e.g. gc_2026 (ROOT 6.40, clang 20, cmake 4):
#
#   conda activate gc_2026
#   scripts/build-macos-conda.sh                 # fresh configure + build + install in ./build
#   scripts/build-macos-conda.sh --keep-cache    # incremental rebuild (no reconfigure from scratch)
#   scripts/build-macos-conda.sh -j 8 -b build_other
#
# Compiler, cmake, ROOT and Boost all come from $CONDA_PREFIX, so nothing is
# picked up from Xcode's /usr/bin/c++ or Homebrew. Build log: <build-dir>/build.log
set -euo pipefail

SRC="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$SRC/build"
JOBS="$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"
FRESH="--fresh"
while [[ $# -gt 0 ]]; do
  case "$1" in
    -b|--build-dir) BUILD="$2"; shift 2 ;;
    -j|--jobs) JOBS="$2"; shift 2 ;;
    --keep-cache) FRESH=""; shift ;;
    -h|--help) sed -n '2,13p' "$0"; exit 0 ;;
    *) echo "unknown option: $1" >&2; exit 1 ;;
  esac
done

P="${CONDA_PREFIX:-}"
if [[ -z "$P" ]]; then
  echo "error: activate the conda env first (conda activate gc_2026)" >&2; exit 1
fi
CMAKE="$P/bin/cmake"
CXX_BIN="$P/bin/arm64-apple-darwin20.0.0-clang++"
[[ -x "$CXX_BIN" ]] || CXX_BIN="$P/bin/clang++"
for f in "$CMAKE" "$CXX_BIN" "$P/bin/root-config"; do
  [[ -x "$f" ]] || { echo "error: $f not found (is this the right env?)" >&2; exit 1; }
done
mkdir -p "$BUILD"

# Boost: GammaCombo only needs the headers. conda's libboost-headers ships
# them without a BoostConfig.cmake (and libboost-devel can't be added next to
# ROOT 6.40 because of icu pins), so CMake would otherwise pick up Homebrew's
# Boost. If the env has no BoostConfig.cmake, write a header-only one into the
# build directory and point CMake at it.
BOOST_ARGS=()
if ! ls "$P"/lib/cmake/Boost-*/BoostConfig.cmake > /dev/null 2>&1; then
  VH="$P/include/boost/version.hpp"
  [[ -f "$VH" ]] || { echo "error: no Boost headers in $P (conda install libboost-headers)" >&2; exit 1; }
  BV="$(sed -n 's/^#define BOOST_VERSION \([0-9]*\).*/\1/p' "$VH")"
  BVS="$((BV / 100000)).$((BV / 100 % 1000)).$((BV % 100))"
  SHIM="$BUILD/_boost_header_only"
  mkdir -p "$SHIM"
  cat > "$SHIM/BoostConfigVersion.cmake" <<EOF
set(PACKAGE_VERSION "$BVS")
if(PACKAGE_FIND_VERSION VERSION_GREATER PACKAGE_VERSION)
  set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
  if(PACKAGE_FIND_VERSION STREQUAL PACKAGE_VERSION)
    set(PACKAGE_VERSION_EXACT TRUE)
  endif()
endif()
EOF
  cat > "$SHIM/BoostConfig.cmake" <<EOF
# Header-only Boost $BVS from $P/include (written by scripts/build-macos-conda.sh)
set(Boost_VERSION "$BVS")
set(Boost_INCLUDE_DIRS "$P/include")
set(Boost_FOUND TRUE)
if(NOT TARGET Boost::boost)
  add_library(Boost::boost INTERFACE IMPORTED)
  set_target_properties(Boost::boost PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "$P/include")
endif()
if(NOT TARGET Boost::headers)
  add_library(Boost::headers ALIAS Boost::boost)
endif()
EOF
  BOOST_ARGS=(-DBoost_DIR="$SHIM")
  echo "Boost    : $BVS headers from $P/include (header-only config in $SHIM)"
fi

echo "env      : $P"
echo "cmake    : $("$CMAKE" --version | head -1)"
echo "compiler : $("$CXX_BIN" --version | head -1)"
echo "ROOT     : $("$P/bin/root-config" --version)"
echo "build dir: $BUILD (jobs: $JOBS)"

"$CMAKE" $FRESH -S "$SRC" -B "$BUILD" \
  -DCMAKE_CXX_COMPILER="$CXX_BIN" \
  -DCMAKE_OSX_SYSROOT="$(xcrun --show-sdk-path)" \
  -DCMAKE_PREFIX_PATH="$P" \
  -DCMAKE_IGNORE_PREFIX_PATH="/opt/homebrew;/usr/local" \
  -DCMAKE_FIND_FRAMEWORK=LAST \
  ${BOOST_ARGS[@]+"${BOOST_ARGS[@]}"} \
  2>&1 | tee "$BUILD/configure.log"
"$CMAKE" --build "$BUILD" -j "$JOBS" 2>&1 | tee "$BUILD/build.log"
"$CMAKE" --install "$BUILD" 2>&1 | tail -3
echo "done: executables installed into <module>/bin (e.g. gammacombo/bin/gamma_wa)"
