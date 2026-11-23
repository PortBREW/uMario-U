#!/usr/bin/env bash
#
# Builds a ready-to-copy SD card layout in dist/.
#
#   Copy the CONTENTS of dist/ to the root of your SD card.
#
# Default (Aroma):
#   dist/wiiu/apps/uMarioU/uMarioU.wuhb   <- listed on the Wii U Menu
#
# The .wuhb is self-contained: executable + all game data + icon + splashes.
#
# Optional:
#   ./tools/make_dist.sh --with-rpx
#     Also ships uMarioU.rpx plus the unpacked content/ tree, for Tiramisu /
#     Homebrew Launcher users. Roughly doubles the size.
#
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

WITH_RPX=0
for arg in "$@"; do
	case "$arg" in
		--with-rpx) WITH_RPX=1 ;;
		*) echo "unknown option: $arg" >&2; exit 2 ;;
	esac
done

APP_DIR="dist/wiiu/apps/uMarioU"
RPX="build/uMarioU.rpx"
WUHB="build/uMarioU.wuhb"

if [[ ! -f "$WUHB" ]]; then
	echo "error: $WUHB not found. Build first:" >&2
	echo "  see README.md -> Build from source" >&2
	exit 1
fi

rm -rf dist
mkdir -p "$APP_DIR"

# --- .wuhb (primary, self-contained) ----------------------------------------
cp "$WUHB" "$APP_DIR/uMarioU.wuhb"

# --- optional .rpx + loose data ---------------------------------------------
if [[ "$WITH_RPX" == "1" ]]; then
	if [[ ! -f "$RPX" ]]; then
		echo "error: $RPX not found but --with-rpx was given." >&2
		exit 1
	fi

	cp "$RPX" "$APP_DIR/uMarioU.rpx"

	# The .rpx resolves its data relative to its own folder on the SD card.
	# Layout must match the probe paths in src/platform/PlatformPaths.cpp.
	cp -r content/. "$APP_DIR/"
fi

# --- Homebrew Launcher / Aroma metadata -------------------------------------
cat > "$APP_DIR/meta.xml" <<'XML'
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<app version="1">
  <name>uMario U</name>
  <coder>PortBREW</coder>
  <version>1.2.7</version>
  <release_date>20261123000000</release_date>
  <short_description>Native Wii U port of uMario</short_description>
  <long_description>Native Wii U port of uMario, the C++/SDL2 widescreen Super Mario Bros. remake.
Original engine and assets by Lukasz Jakowski.
Wii U port by PortBREW.
  </long_description>
</app>
XML

# --- 256x96 launcher icon (derived from the supplied artwork) ---------------
python3 - <<'PY'
from PIL import Image

src = Image.open("imagery/splash.png").convert("RGBA")
w, h = 256, 96

sw, sh = src.size
scale = max(w / sw, h / sh)
nw, nh = int(sw * scale + 0.5), int(sh * scale + 0.5)
resized = src.resize((nw, nh), Image.LANCZOS)

left = (nx := (nw - w) // 2)
top = (ny := (nh - h) // 2)
icon = resized.crop((nx, ny, nx + w, ny + h))
icon.save("dist/wiiu/apps/uMarioU/icon.png")
print("icon.png", icon.size)
PY

# --- zip --------------------------------------------------------------------
if command -v zip >/dev/null 2>&1; then
	( cd dist && zip -qr uMarioU-sd.zip wiiu )
	echo "built dist/uMarioU-sd.zip"
fi

echo
echo "dist/ layout:"
find dist -not -path "*/uNext/files/*" -not -name "*.wav" -not -name "*.bmp" | sort | sed 's|^|  |'
echo
echo "Copy the CONTENTS of dist/ to the root of your SD card, so you end up with:"
echo "  sd:/wiiu/apps/uMarioU/uMarioU.wuhb"
