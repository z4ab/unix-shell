#!/bin/sh
#
# Use this script to run your program LOCALLY.
#
# Note: Changing this script WILL NOT affect how CodeCrafters runs your program.
#
# Learn more: https://codecrafters.io/program-interface

set -e # Exit early if any commands fail

# On Windows (Git Bash / MSYS2) there is no vcpkg toolchain by default and
# CMake's default generator (Visual Studio) is usually not installed. Use the
# MSYS2 UCRT64 GCC toolchain with the "MinGW Makefiles" generator instead.
IS_WINDOWS=0
case "$(uname -s)" in
  MINGW*|MSYS*|CYGWIN*)
    IS_WINDOWS=1
    for dir in /ucrt64/bin /c/msys64/ucrt64/bin /mingw64/bin; do
      if [ -d "$dir" ]; then
        PATH="$dir:$PATH"
        break
      fi
    done
    export PATH
    ;;
esac

# Copied from .codecrafters/compile.sh
#
# - Edit this to change how your program compiles locally
# - Edit .codecrafters/compile.sh to change how your program compiles remotely
(
  cd "$(dirname "$0")" # Ensure compile steps are run within the repository directory

  # Only use the vcpkg toolchain when it actually exists locally.
  TOOLCHAIN=""
  if [ -n "$VCPKG_ROOT" ] && [ -f "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" ]; then
    TOOLCHAIN="-DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
  fi

  if [ "$IS_WINDOWS" = "1" ]; then
    cmake -B build -S . -G "MinGW Makefiles" $TOOLCHAIN
  else
    cmake -B build -S . $TOOLCHAIN
  fi
  cmake --build ./build
)

# Copied from .codecrafters/run.sh
#
# - Edit this to change how your program runs locally
# - Edit .codecrafters/run.sh to change how your program runs remotely
BIN="$(dirname "$0")/build/shell"
# On Windows the executable is built as "shell.exe".
if [ ! -x "$BIN" ] && [ -x "$BIN.exe" ]; then
  BIN="$BIN.exe"
fi
exec "$BIN" "$@"
