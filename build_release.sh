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

# read from .env
# if no .env, variables from CI pipeline are expected
cd "$(dirname "$0")"
if [ -f .env ];
then
  echo "Loading .env file..."
  set -a
  source .env
  set +a
else
  echo "No .env file, running on env variables"
fi

# =============================================================================
# Configuration — check .env_example and create .env
# =============================================================================

# Read version from cmake config
VERSION="$(sed -n 's/^[[:space:]]*VERSION[[:space:]]*//p' CMakeLists.txt | head -n1)"
if [ -z "$VERSION" ]; then
  echo "Could not read VERSION from CMakeLists" >&2
  exit 1
else
    echo "==> Version $VERSION"
fi

# Path to your Qt6 installation (the directory that contains bin/macdeployqt)
QT_DIR="/Users/Shared/Qt/6.11.1/macos"

# Signing and Notarisation
SIGN_IDENTITY="${TEAM_ID}"
NOTARY_PROFILE="${PROFILE}"

# Out-of-source build directory
BUILD_DIR="$(dirname "$0")/build/cmake-release"

# App bundle produced by CMake
APP_NAME="RWA Creator"
APP_BUNDLE="$BUILD_DIR/$APP_NAME.app"

# Where to put the final DMG / zip
DIST_DIR="$(dirname "$0")/dist"
ARCHIVE="$DIST_DIR/$APP_NAME.zip"
DMG_TEMP="$DIST_DIR/dmg_temp"
DMG="$DIST_DIR/$APP_NAME-$VERSION.dmg"

# =============================================================================
# Clean (optional, enable with --clean)
# =============================================================================

if [ "$CLEAN" = true ]; then
  echo "==> Cleaning previous build..."
  rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"

# =============================================================================
# CMake configure
# =============================================================================

echo "==> Configuring..."
cmake -S "$(dirname "$0")" \
      -B "$BUILD_DIR" \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH="$QT_DIR" \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5

# =============================================================================
# Build
# =============================================================================

echo "==> Building..."
cmake --build "$BUILD_DIR" --config Release --parallel "$(sysctl -n hw.logicalcpu)"

# =============================================================================
# Inside-out signing (required for notarization / hardened runtime)
# Sign all nested binaries first, then the bundle itself.
# --deep is NOT used: it doesn't reliably propagate --options runtime
# to nested components, causing Gatekeeper to reject the app.
# =============================================================================

CODESIGN_ARGS=(--sign "$SIGN_IDENTITY" --options runtime --timestamp --force)

echo "==> Signing nested dylibs and .so files..."
find "$APP_BUNDLE" -type f \( -name "*.dylib" -o -name "*.so" \) | while IFS= read -r f; do
    codesign "${CODESIGN_ARGS[@]}" "$f"
done

echo "==> Signing frameworks (deepest path first)..."
find "$APP_BUNDLE" -name "*.framework" -type d | sort -r | while IFS= read -r f; do
    codesign "${CODESIGN_ARGS[@]}" "$f"
done

echo "==> Signing app bundle..."
# Entitlements only apply to the main executable, so they are passed here
# and not to the nested dylib/framework signing above.
codesign "${CODESIGN_ARGS[@]}" \
  --entitlements release.entitlements \
  "$APP_BUNDLE"

# Verify the signature
codesign --verify --deep --strict "$APP_BUNDLE"
echo "    Signature OK"

# =============================================================================
# Package for notarization (zip is simpler than DMG for submission)
# =============================================================================

mkdir -p "$DIST_DIR"
echo "==> Creating archive for notarization..."
ditto -c -k --keepParent "$APP_BUNDLE" "$ARCHIVE"

# =============================================================================
# Notarize
# --wait  blocks until Apple returns a result (typically 1–5 min)
# =============================================================================

echo "==> Submitting for notarization..."
xcrun notarytool submit "$ARCHIVE" \
    --keychain-profile "$NOTARY_PROFILE" \
    --wait

# =============================================================================
# Staple the notarization ticket to the bundle
# =============================================================================

echo "==> Stapling..."
xcrun stapler staple "$APP_BUNDLE"
xcrun stapler validate "$APP_BUNDLE"

# =============================================================================
# Create DMG from the stapled bundle
# =============================================================================

mkdir -p "$DMG_TEMP"
echo "==> Creating DMG..."
rm -f "$DMG"
cp -R "$APP_BUNDLE" "$DMG_TEMP"/
ln -s /Applications "$DMG_TEMP"/Applications

hdiutil create \
    -volname "$APP_NAME" \
    -srcfolder "$DMG_TEMP" \
    -ov \
    -format UDZO \
    -fs HFS+ \
    "$DMG"

rm -rf "$DMG_TEMP"
echo ""
echo "Done. Distributable: $DMG"
