project(bnr VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/include
  ${CMAKE_HOME_DIRECTORY}/stella/include
  ${CMAKE_HOME_DIRECTORY}/lib/stella-common/include
)

set(bnr_RESOURCES
  qml/bnr.qrc
)

set(bnr_SRCS
  src/bnr.cpp
)

set(bnr_HDRS
  include/bnr.h
)

# qt5_add_resources (RCC_SOURCES ${bnr_RESOURCES})
get_property(OUTPUT_PATH GLOBAL PROPERTY TARGET_DIR)
set(LIBRARY_OUTPUT_PATH ${OUTPUT_PATH}/plugins/${PROJECT_NAME})

add_library(bnr SHARED ${bnr_SRCS} ${bnr_HDRS} ${bnr_RESOURCES})
find_package(Qt5 COMPONENTS Core Qml REQUIRED)
target_link_libraries(bnr PRIVATE 
  Qt5::Core
  Qt5::Qml
)

# set(bnrd_SRCS
#   src/bnrd.cpp
#   src/BnrServer.cpp
# )
# set(bnrd_HDRS
#   include/bnrd.h
#   include/BnrServer.h
# )

# add_executable(bnrd ${bnrd_SRCS} ${bnrd_HDRS})

# target_link_libraries(bnrd PRIVATE 
#   Qt5::Core
#   Qt5::DBus
#   Qt5::Network
# )
include_files(bnr src/bnrd.py src/message.py bnr.plugin)
# file(COPY bnr.plugin DESTINATION ${LIBRARY_OUTPUT_PATH})
# file(COPY ${CMAKE_CURRENT_BINARY_DIR}/libbnr.so ${OUTPUT_PATH}/plugins/${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME}
		LIBRARY DESTINATION lib/stella/plugins/${PROJECT_NAME})
