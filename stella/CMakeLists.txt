cmake_minimum_required(VERSION 3.15)
project(stella VERSION 0.1.0)
# need to find a solution to copy python files to output directory
# when the file is changed

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(QT_QML_GENERATE_QMLLS_INI ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set_property(GLOBAL PROPERTY TARGET_DIR ${CMAKE_CURRENT_BINARY_DIR})

find_package(Python 3.14 REQUIRED)
#find_package(peas-1.0 REQUIRED)
# find_package(Boost COMPONENTS
#                 python38
#              REQUIRED)
             
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DSTELLA_DEBUG)  
endif()

set(stella_RESOURCES
  qml/stella.qrc
)

set(stella_HEADERS
  include/main.h
  include/stella.h
  include/StellaPlugin.h
  include/QtPlugin.h
  include/PythonPlugin.h
  include/Settings.h
  include/PluginManager.h
  include/PluginModel.h
)

set(stella_SOURCES
  src/main.cpp
  src/stella.cpp
  src/QtPlugin.cpp
  src/PythonPlugin.cpp
  src/Settings.cpp
  src/PluginManager.cpp
  src/PluginModel.cpp
)

add_executable(stella ${stella_HEADERS} ${stella_SOURCES} ${stella_RESOURCES})

target_include_directories(stella 
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
  ${CMAKE_HOME_DIRECTORY}/lib/stella-common/include
  /usr/include/python3.14
  # ${PYTHON_INCLUDE_DIRS}
  # ${Boost_INCLUDE_DIRS}
)
# find_package(usb-1.0)
find_package(Qt5 COMPONENTS Core Gui Quick Qml QmlModels Widgets Network Xml Sql REQUIRED)
target_link_libraries(stella PUBLIC
  stella-common
  Qt5::Core
  Qt5::Gui
  Qt5::Qml
  Qt5::QmlModels
  Qt5::Network
  Qt5::Widgets
  Qt5::Quick
  Qt5::Xml
  Qt5::Sql
  python3.14
  usb-1.0
  # Boost::python38
  # ${Boost_LIBRARIES} ${PYTHON_LIBRARIES}
)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

	install(FILES ${PROJECT_NAME}.desktop
		DESTINATION share/applications/)

	install(FILES ${PROJECT_NAME}.desktop.appdata.xml
		DESTINATION share/metainfo/)
  
  install(FILES ${PROJECT_NAME}.svg
		DESTINATION share/icons/hicolor/256x256/apps/)

  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/files DESTINATION share/data/${PROJECT_NAME})
  install(TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib)
	
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/files DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/files DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

  # install(CODE "execute_process(COMMAND ${CMAKE_HOME_DIRECTORY}/build.sh windows WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY})")
endif()

# include Qt5 shared library to build directory for creating portable app
if(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_CROSSCOMPILING)
  stella_include_libraries(stella
    Qt5::Core
    Qt5::DBus
    Qt5::Gui
    Qt5::Qml
    Qt5::QmlModels
    Qt5::QmlWorkerScript
    Qt5::Network
    Qt5::Widgets
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::QuickTemplates2 
    Qt5::Xml
    Qt5::Sql
    Qt5::QSQLiteDriverPlugin
    libbz2-1
    libcrypto-1_1
    libfreetype-6
    libgcc_s_dw2-1
    libglib-2.0-0
    libharfbuzz-0
    libharfbuzz-subset-0
    libiconv-2
    libintl-8
    libpcre-1
    libpcre2-16-0
    libpng16-16
    libsqlite3-0
    libssl-1_1
    libstdc++-6
    libwinpthread-1
    libzstd
    zlib1
    libssp-0
    usb-1.0
    libgraphite2
  )

  # Qt5 plugins
  stella_include_plugins(
    imageformats
    platforms
    platformthemes
    sqldrivers
    styles
  )
  # Qt5 Qml plugins
  stella_include_qml_plugins(
    Qt
    QtGraphicalEffects
    QtQml
    QtQuick
    QtQuick.2
  )

endif()
