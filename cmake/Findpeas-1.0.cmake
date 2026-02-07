
set(PEAS_ROOT_DIR
    "${PEAS_ROOT_DIR}"
	CACHE
	PATH
    "Directory to search for peas")

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
	pkg_check_modules(PC_LIBPEAS peas-1.0)
endif()


find_library(PEAS_LIBRARY
	NAMES
    peas-1.0
	  peas
	PATHS
	  ${PC_LIBPEAS_LIBRARY_DIRS}
	  ${PC_LIBPEAS_LIBDIR}
	HINTS
	  "${PEAS_ROOT_DIR}"
	PATH_SUFFIXES
	  lib
    lib64
)
  
get_filename_component(_libdir "${PEAS_LIBRARY}" PATH)
get_filename_component(_librealpath "${PEAS_LIBRARY}" REALPATH)

find_path(PEAS_INCLUDE_DIR
	NAMES
	  peas.h
	PATHS
	  ${PC_LIBPEAS_INCLUDE_DIRS}
	  ${PC_LIBPEAS_INCLUDEDIR}
	HINTS
	  "${_libdir}"
	  "${_libdir}/.."
	  "${PEAS_ROOT_DIR}"
	PATH_SUFFIXES
    include/libpeas-1.0/libpeas
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(peas-1.0
	DEFAULT_MSG
	PEAS_LIBRARY
  PEAS_INCLUDE_DIR
)

if(PEAS_FOUND)
	list(APPEND PEAS_LIBRARIES ${PEAS_LIBRARY})
	list(APPEND PEAS_INCLUDE_DIRS ${PEAS_INCLUDE_DIR})
	mark_as_advanced(PEAS_ROOT_DIR)
endif()

mark_as_advanced(PEAS_INCLUDE_DIR
	PEAS_LIBRARY)

