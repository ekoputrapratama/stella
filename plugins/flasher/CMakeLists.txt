project(flasher VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(flashtool REQUIRED)

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/include
  ${CMAKE_HOME_DIRECTORY}/stella/include
  ${CMAKE_HOME_DIRECTORY}/lib/stella-common/include
  ${CMAKE_HOME_DIRECTORY}/lib/libpit
)

set(flasher_RESOURCES
  qml/flasher.qrc
)
set(flasher_SRCS
  src/flasher.cpp
  src/BridgeManager.cpp
  src/Interface.cpp
  src/OdinUtility.cpp
  src/Utility.cpp
)
set(flasher_HDRS
  include/flasher.h
  include/BridgeManager.h
  include/InboundPacket.h
  include/OutboundPacket.h
  include/Packet.h
  include/Packets.h
  include/Interface.h
  include/OdinUtility.h
  include/Utility.h
)

# qt5_add_resources (RCC_SOURCES ${flasher_RESOURCES})
get_property(OUTPUT_PATH GLOBAL PROPERTY TARGET_DIR)
set(LIBRARY_OUTPUT_PATH ${OUTPUT_PATH}/plugins/${PROJECT_NAME})

add_library(${PROJECT_NAME} SHARED ${flasher_SRCS} ${flasher_HDRS} ${flasher_RESOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE 
  ${FLASHTOOL_LIBRARY}
  usb-1.0
  libpit
  Qt5::Core
)

file(COPY flasher.plugin DESTINATION ${LIBRARY_OUTPUT_PATH})
install(TARGETS ${PROJECT_NAME}
		LIBRARY DESTINATION lib/stella/plugins/${PROJECT_NAME})
