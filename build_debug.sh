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
# A bundle sign binds the Info.plist (so NSBluetoothAlwaysUsageDescription
# is trusted and the com.fhnw.rwa.creator bundle id is used) and seals resources,
# giving CoreBluetooth a stable, valid identity. The debug.entitlements file keeps
# com.apple.security.get-task-allow so lldb can still attach.
#
# Prefer the Developer ID identity (TEAM_ID from .env, same as the release
# build): the application firewall's "automatically allow downloaded signed
# software" then covers the debug build, so incoming UDP/TCP (OSC on :8000,
# project sharing) works without a per-rebuild firewall prompt — an ad-hoc
# signature changes with every build and can never be durably allowed.
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
APP_BUNDLE="$BUILD_DIR/RWA Creator.app"

if [ -f "$SCRIPT_DIR/.env" ]; then
  set -a
  source "$SCRIPT_DIR/.env"
  set +a
fi
SIGN_IDENTITY="${TEAM_ID:-}"
if [ -z "$SIGN_IDENTITY" ]; then
  SIGN_IDENTITY="$(security find-identity -v -p codesigning 2>/dev/null \
    | sed -n 's/.*"\(Developer ID Application[^"]*\)".*/\1/p' | head -n1)"
fi

if [ -n "$SIGN_IDENTITY" ]; then
  echo "==> Signing debug bundle with \"$SIGN_IDENTITY\"..."
  codesign --force --sign "$SIGN_IDENTITY" \
    --entitlements "$SCRIPT_DIR/debug.entitlements" \
    --timestamp=none \
    "$APP_BUNDLE"
else
  echo "==> No signing identity found (.env TEAM_ID or keychain), ad-hoc signing..."
  echo "    Note: the firewall will not auto-allow incoming connections (OSC :8000);"
  echo "    expect an allow/deny prompt or silently dropped packets."
  codesign --force --sign - \
    --entitlements "$SCRIPT_DIR/debug.entitlements" \
    --timestamp=none \
    "$APP_BUNDLE"
fi
codesign -dv "$APP_BUNDLE" 2>&1 | grep -E "Identifier|Signature|Authority|Sealed" || true

echo "Ready to launch in debugger"
exit 0
