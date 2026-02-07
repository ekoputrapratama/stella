cmake_minimum_required(VERSION 3.15)
project(stella VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)
set(QT_MIN_REQUIRED_VERSION "6.4.1")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(FeatureSummary)

# Find Qt modules
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
  Core
  Gui
  Qml
  QmlModels
  QmlWorkerScript
  Network
  Widgets
  Quick
  QuickControls2
  QuickTemplates2
  DBus
  Xml
  Sql
)
find_package(usb-1.0 REQUIRED)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux") 
  find_package(udev REQUIRED)
  find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS XcbQpa)
elseif(WIN32)
  find_library(USER32 user32)
endif()

include(cmake/StellaSetup.cmake)

add_subdirectory(stella)
add_subdirectory(lib/libpit)
add_subdirectory(plugins/bnr)
add_subdirectory(plugins/unlocker)
add_subdirectory(plugins/flasher)
add_subdirectory(plugins/mirroring)
add_subdirectory(plugins/root)
add_subdirectory(lib/stella-common)


if(WIN32)
  add_subdirectory(lib/qdevicewatcher)

  set(CPACK_GENERATOR NSIS)
  set(CPACK_PACKAGE_NAME "Stella")
  set(CPACK_PACKAGE_VENDOR "Who Am I")
  set(CPACK_VERSION_MAJOR "1")
  set(CPACK_VERSION_MINOR "0")
  set(CPACK_VERSION_PATCH "0")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "Stella")

  include(CPack)
endif()


feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

