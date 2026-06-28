# rwacreator
Desktop Application for Real World Audio (RWA)

Around 2015 I started with the development of the Real World Audio (RWA) environment. RWA is a middleware for creating interactive binaural soundwalks. It includes a Desktop "Creator" Software, an iOS client and a ESP32-based head tracking device for dynamic binaural synthesis. The RWA Creator is a cross-platform application written in C++/Qt and can be extended with Pure Data patches. Besides location-based audio playback it facilitates mechanisms for implementing more complex game logic without the need for writing code. Once audio files, Pd patches and other assets are placed on the map, the corresponding RWA game can be exported to the iOS client.

![RwaScreenshot](https://user-images.githubusercontent.com/10684202/161531985-9940b234-253b-4754-8ad6-8750f697cc78.png)

## Prequisites

- CMake
- Qt 6
- Xcode (for building and signing on macOS)
- Apple Developer Account (for signing and notarization)

### CMake

```bash
brew install cmake
```

### Qt 6

Install official package (Qt Online Installer), or through aqtinstall (see below), ideally into `/Users/Shared/Qt/6.11.1/`. Reason: Homebrew doesn’t ship the resources for building universal binaries, only arm64. The path is currently hardcoded into the build script.

aqtinstall method:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install --upgrade pip
pip install aqtinstall
# check for version: aqt list-qt mac desktop
aqt install-qt mac desktop 5.15.2 clang_64 --outputdir ~/Qt
echo 'export PATH="$HOME/Qt/5.15.2/clang_64/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

## Getting Started

Clone the repository with submodules:

```bash
# set up git to use HTTPS instead of SSH for GitHub` 
git config --global --unset-all url."https://".insteadOf` 
git config --global --add url."https://".insteadOf git://` 
git config --global --add url."https://".insteadOf http://` 
git config --global --add url."https://github.com/".insteadOf "git@github.com:"

# verify the configuration
git config --global --get-all url."https://".insteadOf

# clone the repository with submodules
git clone --recursive -b h.e.i.-campus-customisation \
  https://github.com/rnd-hsm-klassik/rwa-creator.git
cd rwa-creator
```

## Building

Copy `.env_example` to `.env` and edit accordingly (you can also use adhoc signing if no certificates are available, as seen in the new “legacy” branch).

### Debug Build

Run `build_debug.sh` or manually build using CMake:

```bash
QT_DIR="/Users/Shared/Qt/6.11.1/macos"
BUILD_DIR=build/cmake-debug

# make sure to clean build directory when necessary!
# rm -rf $BUILD_DIR
# mkdir -p $BUILD_DIR

cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH="$QT_DIR" \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "$BUILD_DIR" --config Debug \
  --parallel "$(sysctl -n hw.logicalcpu)"
```

This will also create a `compile_commands.json` file in the build directory, which can be used by IDEs for code navigation and autocompletion (check `.clangd` for configuration).

Launch the app using the debugger:

```bash
lldb "$BUILD_DIR/rwacreator.app/Contents/MacOS/rwacreator"
(lldb) run
```

You may use the debugger facilities of you IDE instead of lldb. Configuration for Zed is provided in `.zed/debug.json`. Make sure to disable the default "C++: on throw" breakpoint.

### Release Build

Use build-script, it will build and sign the app, and create a notarized DMG for distribution:

`./build_release.sh`

Or manually build using CMake (no notarization):

```bash
mkdir -p build/cmake-release
cmake -B build/cmake-release \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="/Users/Shared/Qt/6.11.1/macos" \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build build/cmake-release --config Release --parallel 4
macdeployqt build/cmake-release/rwacreator.app
codesign --force --deep --sign - build/cmake-release/rwacreator.app
```

### Run it

`open build/cmake-release/rwacreator.app`
