#!/usr/bin/env bash
#
# Builds the editor target. This is the only supported compile.
#
# Unreal Engine cannot be installed on a hosted GitHub runner, so there is no
# hosted compile gate. Run this locally after every C++ change. The optional
# self-hosted `compile` job in .github/workflows/ci.yml runs this same script,
# so local and CI cannot drift.
#
# Overridable by environment:
#   LASTTRAIN_ENGINE_ROOT   default /Users/Shared/Epic Games/UE_5.8
#   LASTTRAIN_TARGET        default LastTrainEditor
#   LASTTRAIN_PLATFORM      default Mac
#   LASTTRAIN_CONFIGURATION default Development
#   LASTTRAIN_SKIP_XCODE_CHECK  set to 1 to skip the external drive check

set -euo pipefail

ENGINE_ROOT="${LASTTRAIN_ENGINE_ROOT:-/Users/Shared/Epic Games/UE_5.8}"
TARGET="${LASTTRAIN_TARGET:-LastTrainEditor}"
PLATFORM="${LASTTRAIN_PLATFORM:-Mac}"
CONFIGURATION="${LASTTRAIN_CONFIGURATION:-Development}"

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PROJECT="$REPO_ROOT/LastTrain.uproject"

fail() {
	echo "compile.sh: $1" >&2
	exit 1
}

if [ "$(uname -s)" != "Darwin" ]; then
	fail "the supported build is macOS only. This machine is $(uname -s), so there is nothing to compile here."
fi

[ -f "$PROJECT" ] || fail "no LastTrain.uproject at $PROJECT"

BUILD_SCRIPT="$ENGINE_ROOT/Engine/Build/BatchFiles/Mac/Build.sh"
if [ ! -x "$BUILD_SCRIPT" ]; then
	fail "no engine build script at $BUILD_SCRIPT. Set LASTTRAIN_ENGINE_ROOT to the UE 5.8 install."
fi

if [ "${LASTTRAIN_SKIP_XCODE_CHECK:-0}" != "1" ]; then
	XCODE_PATH="$(xcode-select -p 2>/dev/null || true)"
	[ -n "$XCODE_PATH" ] || fail "xcode-select -p returned nothing. Xcode is not selected."

	# Xcode lives on an external drive on the development machine. Without it
	# mounted the Metal toolchain is missing and shaders cannot compile.
	case "$XCODE_PATH" in
	/Volumes/*)
		VOLUME="/$(echo "$XCODE_PATH" | cut -d/ -f2,3)"
		[ -d "$VOLUME" ] || fail "$VOLUME is not mounted, so Xcode at $XCODE_PATH is unreachable."
		;;
	esac
fi

echo "compile.sh: building $TARGET $PLATFORM $CONFIGURATION"
echo "compile.sh: engine   $ENGINE_ROOT"
echo "compile.sh: project  $PROJECT"

exec "$BUILD_SCRIPT" "$TARGET" "$PLATFORM" "$CONFIGURATION" -Project="$PROJECT"
