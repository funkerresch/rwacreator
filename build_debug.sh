#!/usr/bin/env bash
set -euo pipefail

# Parse arguments
CLEAN=false
for arg in "$@"; do
  case "$arg" in
    --clean) CLEAN=true ;;
    *)
      echo "Unknown argument: $arg" >&2
      exit 1
      ;;
  esac
done

QT_DIR="/Users/Shared/Qt/6.11.1/macos"
BUILD_DIR="$(dirname "$0")/build/cmake-debug"

if [ "$CLEAN" = true ]; then
  echo "==> Cleaning previous build..."
  rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"

echo "==> Configuring..."
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH="$QT_DIR" \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

echo "==> Building..."
cmake --build "$BUILD_DIR" --config Debug \
  --parallel "$(sysctl -n hw.logicalcpu)"

echo "Ready to launch in debugger"
exit 0
