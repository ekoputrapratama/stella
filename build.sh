#!/usr/bin/env bash

# store repo root as variable
PROJECT_ROOT=$(pwd)

# make sure Qt plugin finds QML sources so it can deploy the imported files
export QML_SOURCES_PATHS="$PROJECT_ROOT"/stella/qml
export ARCH=x86_64

# ./linuxdeploy-plugin-qt-x86_64.AppImage --appdir AppDir --extra-plugin=sqldrivers/libqsqlite.so,\
# styles,xcbglintegrations,platforms,platformthemes
TARGET=$([ -z "$1" ] && echo "linux" || echo $1);

if [ $TARGET == "windows" ]; then
  echo "building for windows";
  mkdir -p dist
  cp -r build-win32/stella/* dist
  rm -r dist/CMakeFiles
  rm -r dist/stella_autogen
  rm dist/cmake_install.cmake
else
  # patchelf --set-rpath '$ORIGIN/../lib' build/appdir/usr/bin/stella
  # patchelf --set-rpath '$ORIGIN/../../..' build/appdir/usr/lib/stella/plugins/bnr/libbnr.so
  
  ./linuxdeploy-$ARCH.AppImage --appdir build/appdir --plugin=qt --output appimage
fi

