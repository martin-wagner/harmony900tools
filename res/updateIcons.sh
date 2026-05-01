#!/usr/bin/env bash
set -euo pipefail

BASE_URL="https://download.kde.org/stable/frameworks"
THEME="BreezeConverted"
SIZE=64

# detect latest
LATEST=$(curl -s "$BASE_URL/" \
  | grep -oE 'href="6\.[0-9]+/' \
  | sed 's|href="||;s|/||' \
  | sort -V \
  | tail -n1)

VERSION="${LATEST}.0"
TARBALL="breeze-icons-${VERSION}.tar.xz"
URL="$BASE_URL/$LATEST/$TARBALL"

echo "Version: $VERSION"
curl -LO "$URL"
tar -xf "$TARBALL"

SRC="breeze-icons-${VERSION}"

OUT="icons/$THEME"
mkdir -p "$OUT"

# create minimal index.theme
cat > "$OUT/index.theme" <<EOF
[Icon Theme]
Name=$THEME
Comment=Converted Breeze Icons
Directories=${SIZE}x${SIZE}/actions,${SIZE}x${SIZE}/apps,${SIZE}x${SIZE}/places,${SIZE}x${SIZE}/status,${SIZE}x${SIZE}/devices,${SIZE}x${SIZE}/mimetypes

EOF

for ctx in actions apps places status devices mimetypes; do
cat >> "$OUT/index.theme" <<EOF
[${SIZE}x${SIZE}/$ctx]
Size=$SIZE
Context=$ctx
Type=Fixed

EOF
done

# function to map path → freedesktop context
map_context() {
    case "$1" in
        */actions/*) echo "actions" ;;
        */apps/*) echo "apps" ;;
        */places/*) echo "places" ;;
        */status/*) echo "status" ;;
        */devices/*) echo "devices" ;;
        */mimetypes/*) echo "mimetypes" ;;
        *) echo "misc" ;;
    esac
}

export -f map_context
export SIZE OUT SRC

convert_one() {
    svg="$1"

    ctx=$(map_context "$svg")
    name=$(basename "${svg%.svg}")

    outdir="$OUT/${SIZE}x${SIZE}/$ctx"
    mkdir -p "$outdir"

    rsvg-convert -w "$SIZE" -h "$SIZE" "$svg" -o "$outdir/$name.png"
}

export -f convert_one

# parallel conversion
find "$SRC" -type f -name "*.svg" \
    | xargs -P "$(nproc)" -I{} bash -c 'convert_one "$@"' _ {}

echo "done -> $OUT"
