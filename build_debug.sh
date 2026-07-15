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

# Re-sign the assembled .app bundle.
#
# An ad-hoc bundle sign binds the Info.plist (so NSBluetoothAlwaysUsageDescription
# is trusted and the com.fhnw.rwa.creator bundle id is used) and seals resources,
# giving CoreBluetooth a stable, valid identity. The debug.entitlements file keeps
# com.apple.security.get-task-allow so lldb can still attach.
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
APP_BUNDLE="$BUILD_DIR/RWA Creator.app"
echo "==> Ad-hoc signing debug bundle for CoreBluetooth..."
codesign --force --sign - \
  --entitlements "$SCRIPT_DIR/debug.entitlements" \
  --timestamp=none \
  "$APP_BUNDLE"
codesign -dv "$APP_BUNDLE" 2>&1 | grep -E "Identifier|Signature|Sealed" || true

echo "Ready to launch in debugger"
exit 0
