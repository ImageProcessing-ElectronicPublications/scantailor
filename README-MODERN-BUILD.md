# Scan Tailor — modern build (Qt5, macOS, current toolchains)

This document records the **modifications made to build Scan Tailor on current systems** (replacing the unmaintained Qt 4 / old CMake setup). Use it as a reference for future maintenance.

## Summary

- **Qt 4 → Qt 5.15** via CMake (`find_package(Qt5 ...)`, `qt5_wrap_ui`, `qt5_add_resources`, `qt5_add_translation`, global `CMAKE_AUTOMOC`).
- **macOS**: no X11 requirement; **Linux** still uses X11 XRender where the original project expected it.
- **Windows**: Qt DLL staging was replaced with **`windeployqt`** post-build; re-validate installers/NSIS if you ship Windows builds.
- **Apple `.app` bundle**: script under `packaging/osx/` plus **`codesign`** after `macdeployqt` (required on recent macOS to avoid `CODESIGNING` / “Invalid Page” crashes).

---

## Requirements

| Component | Notes |
|-----------|--------|
| **CMake** | 3.16 or newer |
| **C++** | C++11 (`CMAKE_CXX_STANDARD 11`) |
| **Qt 5** | Widgets, Xml, Network, LinguistTools — e.g. Homebrew `qt@5` |
| **Boost** | Test components (`unit_test_framework`, `prg_exec_monitor`) |
| **JPEG, PNG, TIFF, zlib** | Dev libraries (e.g. Homebrew kegs) |

### macOS (Homebrew example)

```bash
brew install cmake qt@5 boost jpeg-turbo libpng libtiff zlib
```

Configure with Qt on the prefix path:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$(brew --prefix qt@5)"
cmake --build build -j8
ctest --test-dir build
```

Run the GUI from `build/scantailor` or the CLI from `build/scantailor-cli`.

---

## CMake changes (high level)

### Root `CMakeLists.txt`

- `CMAKE_MINIMUM_REQUIRED(VERSION 3.16)`; `cmake_policy(SET CMP0167 OLD)` for legacy `FindBoost` usage.
- `find_package(Qt5 REQUIRED COMPONENTS Widgets Xml Network LinguistTools)`.
- `set(CMAKE_AUTOMOC ON)`; removed `QT4_*` macros.
- `qt5_wrap_ui`, `qt5_add_resources`, `qt5_add_translation`.
- **X11 / XRender**: only when `CMAKE_SYSTEM_NAME STREQUAL "Linux"`.
- **Pthreads**: still resolved for Unix (including macOS) via existing `FindPthreads.cmake`.
- **Linking**: executables and static libraries use **`Qt5::Widgets`**, **`Qt5::Xml`**, **`Qt5::Gui`**, **`Qt5::Core`** (and **`Qt5::Network`** for the crash reporter target on Windows).
- **`QJPEG_LIBRARIES`**: set to empty on non-Windows; JPEG is handled via system/lib linkage.
- **Windows**: `windeployqt` as a `POST_BUILD` step on `scantailor` instead of copying Qt 4 DLLs by name.
- **`SkinnedButton`**: moved from `gui_only_sources` into **`common_sources` / `stcore`** because **`StageListView`** (in `stcore`) uses it and **`scantailor-cli`** links `stcore`.

### Subdirectory `CMakeLists.txt`

- Replaced `QT4_WRAP_UI` → `qt5_wrap_ui`, removed `QT4_AUTOMOC`.
- Added `target_link_libraries(... Qt5::...)` on static libs (`foundation`, `interaction`, `zones`, `dewarping`, `imageproc`, `math`, all filter libs, etc.).

### Translations

- `QT_LUPDATE_EXECUTABLE` is resolved from **`Qt5::lupdate`** (or `PATH`) for the `update_translations` custom target in `cmake/UpdateTranslations.cmake`.
- `.qm` files are generated with **`qt5_add_translation`**.

---

## C++ and compatibility fixes

| Area | Change |
|------|--------|
| **MOC includes** | `#include "Class.h.moc"` → `#include "Class.moc"` for CMake AUTOMOC; ensure **`#include "Class.h"`** before the `.moc` include where the cpp file had no prior include. |
| **QString** | `fromAscii` / `toAscii` → `fromLatin1` / `toLatin1`. |
| **HTML escaping** | `Qt::escape(...)` → `QString(...).toHtmlEscaped()`. |
| **QImage indexed** | `setNumColors` / `numColors` → `setColorCount` / `colorCount`. |
| **QHeaderView (Qt 5)** | `setResizeMode` → `setSectionResizeMode`; `setMovable(false)` → `setSectionsMovable(false)`. |
| **Headers** | Added includes such as **`QPainterPath`**, **`QButtonGroup`** where Qt5 no longer pulled them in transitively. |
| **Boost** | `compat/boost_multi_index_foreach_fix.h`: `is_noncopyable` specialization only for **`BOOST_VERSION < 107200`** to avoid redefinition with newer Boost (e.g. 1.88). |

---

## macOS application bundle

Script: **`packaging/osx/bundle-qt5.sh`**

- Builds `ScanTailor.app` from the compiled `build/scantailor`, copies `scantailor_*.qm` into `Contents/MacOS/` (loader looks next to the executable first).
- Runs **`macdeployqt`**, then **`codesign --force --deep --sign -`** on the `.app` so libraries modified by `macdeployqt` are not rejected by the system (fixes `CODESIGNING` / “Invalid Page”).

```bash
./packaging/osx/bundle-qt5.sh build build
open build/ScanTailor.app
```

Ad hoc signing is for **local use**. Distribution requires a **Developer ID** and (typically) **notarization**.

---

## Tests

Registered in CMake:

- `ImageProcTests` (`imageproc/tests`)
- `ScanTaylorTests` (`tests`)

---

## Known follow-ups

- **Windows NSIS** (`cmake/generate_nsi_file.cmake.in`) may still reference **Qt 4** JPEG plugin variables; update if you regenerate Windows installers.
- **Crash reporter** remains MSVC-oriented; unchanged for macOS/Linux.
- Original upstream **README.md** still describes the old project status and wiki links; this file only documents the **modern build fork/maintainer changes**.

---

## File touch list (for grep / archaeology)

Key areas: root **`CMakeLists.txt`**, **`foundation/`**, **`interaction/`**, **`zones/`**, **`dewarping/`**, **`imageproc/`**, **`math/`**, **`filters/*/`**, **`crash_reporter/`**, **`tests/`**, **`imageproc/tests/`**, many **`*.cpp`** for Qt5 API and MOC includes, **`compat/boost_multi_index_foreach_fix.h`**, **`packaging/osx/bundle-qt5.sh`**.
