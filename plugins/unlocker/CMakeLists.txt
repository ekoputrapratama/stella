project(unlocker VERSION 0.1.0)

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

set(unlocker_RESOURCES
  qml/unlocker.qrc
)
set(unlocker_SRCS
  src/unlocker.cpp
)
set(unlocker_HDRS
  include/unlocker.h
)
# qt5_add_resources (RCC_SOURCES ${unlocker_RESOURCES})
get_property(OUTPUT_PATH GLOBAL PROPERTY TARGET_DIR)
set(LIBRARY_OUTPUT_PATH ${OUTPUT_PATH}/plugins/${PROJECT_NAME})

add_library(${PROJECT_NAME} SHARED ${unlocker_SRCS} ${unlocker_HDRS} ${unlocker_RESOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE 
  Qt5::Core
)

# file(COPY ${CMAKE_CURRENT_BINARY_DIR}/libunlocker.so ${OUTPUT_PATH}/plugins/${PROJECT_NAME})
file(COPY unlocker.plugin DESTINATION ${LIBRARY_OUTPUT_PATH})
install(TARGETS ${PROJECT_NAME}
		LIBRARY DESTINATION lib/stella/plugins/${PROJECT_NAME})
