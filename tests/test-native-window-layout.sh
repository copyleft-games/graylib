#!/bin/bash
# test-native-window-layout.sh - graylib's CoreData mirror matches raylib's
# Copyright (C) 2026  Zach Podbielniak
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# src/core/grl-native-window.c mirrors raylib's internal CoreData struct
# (deps/raylib/src/rcore.c) to resize the window on Android.  This
# compares the two, comments and spacing aside, with graylib's names
# (GrlCorePoint, GrlCoreSize, GRL_CORE_MAX_*) mapped back to raylib's,
# and the array sizes the mirror uses with rcore.c's defaults.  A raylib
# bump that moves the struct fails here, with the difference shown.
#
# Usage: test-native-window-layout.sh [GRAYLIB_ROOT]

set -euo pipefail

root="${1:-$(dirname -- "${BASH_SOURCE[0]}")/..}"
rcore="${root}/deps/raylib/src/rcore.c"
mirror="${root}/src/core/grl-native-window.c"
fail=0

[[ -f "${rcore}" ]] || { echo "test-native-window-layout: no ${rcore}" >&2; exit 1; }

# normalize -- the struct block from stdin: no comments, one token run per line
normalize () {
	sed -e 's|//.*$||' -e 's|/\*.*\*/||g' -e 's/[[:space:]]\+/ /g' -e 's/^ //' -e 's/ $//' \
		-e 's/ *\([;{}\[]\) */\1/g' | grep -v '^$'
}

# the struct, Point and Size typedefs through "} CoreData;"
raylib_struct="$(sed -n '/^typedef struct { int x; int y; } Point;/,/^} CoreData;/p' "${rcore}" \
	| sed 's/typedef struct CoreData {/typedef struct {/' | normalize)"
mirror_struct="$(sed -n '/^typedef struct { int x; int y; } GrlCorePoint;/,/^} GrlCoreData;/p' "${mirror}" \
	| sed -e 's/GrlCorePoint/Point/g' -e 's/GrlCoreSize/Size/g' -e 's/GrlCoreData/CoreData/g' \
		-e 's/GRL_CORE_MAX_/MAX_/g' | normalize)"

if [[ -z "${raylib_struct}" || -z "${mirror_struct}" ]]
then
	echo "  FAIL  the CoreData block was not found (raylib: ${#raylib_struct} bytes, mirror: ${#mirror_struct} bytes)" >&2
	fail=1
elif [[ "${raylib_struct}" == "${mirror_struct}" ]]
then
	echo "  ok    the CoreData mirror has raylib's fields, in raylib's order"
else
	echo "  FAIL  the CoreData mirror differs from raylib's rcore.c:" >&2
	diff <(printf '%s\n' "${raylib_struct}") <(printf '%s\n' "${mirror_struct}") >&2 || true
	fail=1
fi

# the array sizes: every MAX_* the struct uses, rcore.c's default against the mirror's
while read -r name
do
	theirs="$(sed -n "s/^ *#define ${name} \{1,\}\([0-9]\{1,\}\).*/\1/p" "${rcore}" | head -n 1)"
	ours="$(sed -n "s/^#define GRL_CORE_${name} \{1,\}(\([0-9]\{1,\}\)).*/\1/p" "${mirror}" | head -n 1)"
	if [[ -n "${theirs}" && "${theirs}" == "${ours}" ]]
	then
		echo "  ok    ${name} is ${ours}"
	else
		echo "  FAIL  ${name}: raylib ${theirs:-?}, mirror ${ours:-?}" >&2
		fail=1
	fi
done < <(grep -o 'MAX_[A-Z_]*' <<< "${raylib_struct}" | sort -u)
# raylib's config.h must not override them (it would change the struct)
if grep -qE '^[[:space:]]*#define[[:space:]]+MAX_(KEYBOARD_KEYS|MOUSE_BUTTONS|GAMEPADS|GAMEPAD_NAME_LENGTH|GAMEPAD_AXES|GAMEPAD_BUTTONS|TOUCH_POINTS|KEY_PRESSED_QUEUE|CHAR_PRESSED_QUEUE)\b' \
	"${root}/deps/raylib/src/config.h"
then
	echo "  FAIL  raylib's config.h sets a CoreData array size" >&2
	fail=1
else
	echo "  ok    raylib's config.h leaves the array sizes at rcore.c's defaults"
fi

exit "${fail}"
