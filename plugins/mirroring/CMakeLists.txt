project(mirroring VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(WIN32)
  find_library(USER32 user32)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_package(X11 REQUIRED)
endif()

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/include
  ${CMAKE_HOME_DIRECTORY}/stella/include
  ${CMAKE_HOME_DIRECTORY}/lib/stella-common/include
  /usr/include/qt
  /usr/include/qt/QtCore
  /usr/include/qt/QtQuick
  /usr/include/qt/QtNetwork
  /usr/include/qt/QtGui
)

set(mirroring_RESOURCES
  resources/mirroring.qrc
)

set(mirroring_SRCS
  src/mirroring.cpp
  src/DeviceView.cpp
  src/DeviceMessage.cpp
  src/MinicapClient.cpp
  src/DeviceServer.cpp
  src/DeviceController.cpp
  src/InputManager.cpp
  src/ControlMessage.cpp
  src/EventConverter.cpp
  src/StringUtil.cpp
  src/LocalSocket.cpp
  src/net.cpp
)

set(mirroring_HDRS
  include/mirroring.h
  include/DeviceView.h
  include/DeviceMessage.h
  include/MinicapClient.h
  include/DeviceServer.h
  include/DeviceController.h
  include/InputManager.h
  include/ControlMessage.h
  include/EventConverter.h
  include/StringUtil.h
  include/LocalSocket.h
  include/net.h
)

# qt5_add_resources (RCC_SOURCES ${flasher_RESOURCES})
get_property(OUTPUT_PATH GLOBAL PROPERTY TARGET_DIR)
set(LIBRARY_OUTPUT_PATH ${OUTPUT_PATH}/plugins/${PROJECT_NAME})

add_library(${PROJECT_NAME} SHARED ${mirroring_SRCS} ${mirroring_HDRS} ${mirroring_RESOURCES})

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_package(Qt5 COMPONENTS Core Quick QuickControls2 Network REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt5::Core
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::Network
    X11
  )
elseif(WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt5::Core
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::Network
    USER32
  )
endif()

file(COPY mirroring.plugin DESTINATION ${LIBRARY_OUTPUT_PATH})

install(TARGETS ${PROJECT_NAME}
		LIBRARY DESTINATION lib/stella/plugins/${PROJECT_NAME})
