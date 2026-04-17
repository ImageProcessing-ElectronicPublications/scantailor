#!/usr/bin/env bash
# Crea ScanTailor.app listo para copiar a /Applications (Qt 5 + macdeployqt).
# Uso:
#   ./packaging/osx/bundle-qt5.sh [directorio_build] [destino]
# Ejemplo:
#   ./packaging/osx/bundle-qt5.sh build build
#
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="${1:-$REPO_ROOT/build}"
OUT_DIR="${2:-$BUILD_DIR}"

MACOS_BIN="$BUILD_DIR/scantailor"
if [[ ! -x "$MACOS_BIN" ]]; then
	echo "No encuentro el ejecutable GUI en: $MACOS_BIN" >&2
	echo "Compila antes: cmake --build $BUILD_DIR" >&2
	exit 1
fi

if [[ -x "$(command -v brew)" ]]; then
	QT_PREFIX="$(brew --prefix qt@5 2>/dev/null || true)"
fi
MACDEPLOYQT="${QT_PREFIX:+${QT_PREFIX}/bin/}macdeployqt"
if ! command -v macdeployqt &>/dev/null && [[ ! -x "${QT_PREFIX:-}/bin/macdeployqt" ]]; then
	echo "Instala Qt 5 y macdeployqt, p. ej.: brew install qt@5" >&2
	exit 1
fi
if [[ -x "${QT_PREFIX:-}/bin/macdeployqt" ]]; then
	MACDEPLOYQT="${QT_PREFIX}/bin/macdeployqt"
fi

VERSION="$(grep '#define[[:space:]]\+VERSION[[:space:]]' "$REPO_ROOT/version.h" | sed 's/.*\"\([^\"]*\)\".*/\1/')"
APP="$OUT_DIR/ScanTailor.app"
CONTENTS="$APP/Contents"
EXE_DIR="$CONTENTS/MacOS"
RES_DIR="$CONTENTS/Resources"

rm -rf "$APP"
mkdir -p "$EXE_DIR" "$RES_DIR"

cp "$MACOS_BIN" "$EXE_DIR/ScanTailor"
chmod +x "$EXE_DIR/ScanTailor"

# Traducciones: el arranque intenta cargar desde el directorio del ejecutable.
shopt -s nullglob
for qm in "$BUILD_DIR"/scantailor_*.qm; do
	cp "$qm" "$EXE_DIR/"
done
shopt -u nullglob

PLIST="$CONTENTS/Info.plist"
cat >"$PLIST" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDevelopmentRegion</key>
	<string>en</string>
	<key>CFBundleExecutable</key>
	<string>ScanTailor</string>
	<key>CFBundleIdentifier</key>
	<string>net.sourceforge.Scantailor</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>ScanTailor</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
	<key>CFBundleShortVersionString</key>
	<string>${VERSION}</string>
	<key>CFBundleVersion</key>
	<string>${VERSION}</string>
	<key>LSMinimumSystemVersion</key>
	<string>11.0</string>
	<key>NSHighResolutionCapable</key>
	<true/>
</dict>
</plist>
EOF

echo "Ejecutando $MACDEPLOYQT ..."
"$MACDEPLOYQT" "$APP" -verbose=1

# macdeployqt copia y parchea frameworks Qt; eso invalida firmas previas. Sin volver
# a firmar, dyld puede abortar con CODESIGNING / "Invalid Page" (SIGKILL) en macOS reciente.
echo "Firmando el bundle (firma ad hoc, solo para uso local)..."
codesign --force --deep --sign - "$APP"

echo ""
echo "Listo: $APP"
echo "Puedes abrirlo con: open \"$APP\""
echo "O copiarlo: cp -R \"$APP\" /Applications/"
