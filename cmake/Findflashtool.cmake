
set(FLASHTOOL_ROOT_DIR
    "${FLASHTOOL_ROOT_DIR}"
	CACHE
	PATH
    "Directory to search for flashtool")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(FLASHTOOL_SUFFIX "linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(FLASHTOOL_SUFFIX "win32")
endif()

find_library(FLASHTOOL_LIBRARY
	NAMES
    flashtool
	PATHS
	  ${CMAKE_HOME_DIRECTORY}/lib/flashtool
	  # ${PC_LIBUDEV_LIBDIR}
	HINTS
	  "${FLASHTOOL_ROOT_DIR}"
	PATH_SUFFIXES
	  ${FLASHTOOL_SUFFIX}
)

get_filename_component(_libdir "${FLASHTOOL_LIBRARY}" PATH)

find_path(FLASHTOOL_INCLUDE_DIR
	NAMES
	  flashtool.h
	HINTS
	  "${_libdir}"
	  "${_libdir}/.."
	  "${FLASHTOOL_ROOT_DIR}"
	PATH_SUFFIXES
	  include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(flashtool
	DEFAULT_MSG
	FLASHTOOL_LIBRARY
	FLASHTOOL_INCLUDE_DIR
)

if(FLASHTOOL_FOUND)
	list(APPEND FLASHTOOL_LIBRARIES ${FLASHTOOL_LIBRARY})
	list(APPEND FLASHTOOL_INCLUDE_DIRS ${FLASHTOOL_INCLUDE_DIR})
	mark_as_advanced(FLASHTOOL_ROOT_DIR)
endif()

mark_as_advanced(FLASHTOOL_INCLUDE_DIR
	FLASHTOOL_LIBRARY)

