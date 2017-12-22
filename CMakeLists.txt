cmake_minimum_required(VERSION 3.8)

project(DISCOUNT C CXX)

set(LIB_TYPE STATIC)

file(READ "${DISCOUNT_SOURCE_DIR}/VERSION" ${PROJECT_NAME}_VERSION)
string(STRIP "${${PROJECT_NAME}_VERSION}" ${PROJECT_NAME}_VERSION)

set(${PROJECT_NAME}_WITH_TABSTOPS "4" CACHE STRING
    "Set tabstops to N characters (default is 4)")
set(TABSTOP "${${PROJECT_NAME}_WITH_TABSTOPS}")

set(${PROJECT_NAME}_MAKE_INSTALL ON CACHE BOOL
    "Set to OFF to disable install rules (default is ON)")

set(${PROJECT_NAME}_INSTALL_SAMPLES OFF CACHE BOOL
    "Set to ON to install sample programs (default is OFF)")

# Check headers
include(CheckIncludeFile)
check_include_file(libgen.h HAVE_LIBGEN_H)
check_include_file(pwd.h HAVE_PWD_H)
check_include_file(alloca.h HAVE_ALLOCA_H)
check_include_file(malloc.h HAVE_MALLOC_H)
check_include_file(sys/stat.h HAVE_STAT)

# Types detection (from configure.inc: AC_SCALAR_TYPES ())
include(CheckTypeSize)
check_type_size("unsigned long" SIZEOF_ULONG BUILTIN_TYPES_ONLY)
check_type_size("unsigned int" SIZEOF_UINT BUILTIN_TYPES_ONLY)
check_type_size("unsigned short" SIZEOF_USHORT BUILTIN_TYPES_ONLY)

if(SIZEOF_ULONG EQUAL 4)
    set(DWORD "unsigned long")
elseif(SIZEOF_UINT EQUAL 4)
    set(DWORD "unsigned int")
else()
    message(FATAL_ERROR "Could not detect DWORD type")
endif()

if(SIZEOF_UINT EQUAL 2)
    set(WORD "unsigned int")
elseif(SIZEOF_USHORT EQUAL 2)
    set(WORD "unsigned short")
else()
    message(FATAL_ERROR "Could not detect WORD type")
endif()

set(BYTE "unsigned char")

# Check symbols
include(CheckSymbolExists)
foreach(_symbol
        bzero
        strcasecmp _stricmp
        strncasecmp _strnicmp)
    string(TOUPPER ${_symbol} _SYMBOL)
    check_symbol_exists(${_symbol} string.h HAVE_${_SYMBOL})
endforeach()
check_symbol_exists(random stdlib.h HAVE_RANDOM)
check_symbol_exists(srandom stdlib.h HAVE_SRANDOM)
check_symbol_exists(getpwuid pwd.h HAVE_GETPWUID)
check_symbol_exists(basename libgen.h HAVE_BASENAME)
check_symbol_exists(fchdir unistd.h HAVE_FCHDIR)
if(HAVE_STAT)
    check_symbol_exists(S_ISCHR sys/stat.h HAVE_S_ISCHR)
    check_symbol_exists(S_ISFIFO sys/stat.h HAVE_S_ISFIFO)
    check_symbol_exists(S_ISSOCK sys/stat.h HAVE_S_ISSOCK)
endif()

if(NOT HAVE_BZERO)
    set(DEFINE_BZERO "#define bzero(p, n) memset(p, 0, n)")
endif()

if(NOT HAVE_STRCASECMP)
    if(HAVE__STRICMP)
        set(DEFINE_STRCASECMP "#define strcasecmp _stricmp")
    else()
        set(DEFINE_STRCASECMP "#error The symbol strcasecmp is not defined.")
    endif()
endif()

if(NOT HAVE_STRNCASECMP)
    if(HAVE__STRNICMP)
        set(DEFINE_STRNCASECMP "#define strncasecmp _strnicmp")
    else()
        set(DEFINE_STRNCASECMP "#error The symbol strncasecmp is not defined.")
    endif()
endif()

if(NOT HAVE_S_ISCHR OR NOT HAVE_S_ISFIFO OR NOT HAVE_S_ISSOCK)
    set(HAVE_STAT "")
endif()

configure_file(${DISCOUNT_SOURCE_DIR}/cmake/config.h.in
    "config.h"
    @ONLY)

configure_file("${DISCOUNT_SOURCE_DIR}/version.c.in"
    "${DISCOUNT_BINARY_DIR}/version.c"
    @ONLY)
set_property(SOURCE "${DISCOUNT_BINARY_DIR}/version.c" APPEND PROPERTY COMPILE_DEFINITIONS
    BRANCH=""
    VERSION="${${PROJECT_NAME}_VERSION}")

configure_file("${DISCOUNT_SOURCE_DIR}/mkdio.h.in"
    "${DISCOUNT_BINARY_DIR}/mkdio.h"
    @ONLY)

include_directories("${DISCOUNT_SOURCE_DIR}")
include_directories("${DISCOUNT_BINARY_DIR}")

add_executable(mktags
    "${DISCOUNT_SOURCE_DIR}/mktags.c")

add_custom_command(OUTPUT "${DISCOUNT_BINARY_DIR}/blocktags"
    COMMAND mktags > ${DISCOUNT_BINARY_DIR}/blocktags
    WORKING_DIRECTORY "${DISCOUNT_SOURCE_DIR}")

add_library(libmarkdown ${LIB_TYPE}
    "${DISCOUNT_SOURCE_DIR}/mkdio.c"
    "${DISCOUNT_SOURCE_DIR}/markdown.c"
    "${DISCOUNT_SOURCE_DIR}/dumptree.c"
    "${DISCOUNT_SOURCE_DIR}/generate.c"
    "${DISCOUNT_SOURCE_DIR}/resource.c"
    "${DISCOUNT_SOURCE_DIR}/docheader.c"
    "${DISCOUNT_BINARY_DIR}/version.c"
    "${DISCOUNT_SOURCE_DIR}/toc.c"
    "${DISCOUNT_SOURCE_DIR}/css.c"
    "${DISCOUNT_SOURCE_DIR}/xml.c"
    "${DISCOUNT_SOURCE_DIR}/Csio.c"
    "${DISCOUNT_SOURCE_DIR}/xmlpage.c"
    "${DISCOUNT_SOURCE_DIR}/basename.c"
    "${DISCOUNT_SOURCE_DIR}/emmatch.c"
    "${DISCOUNT_SOURCE_DIR}/github_flavoured.c"
    "${DISCOUNT_SOURCE_DIR}/setup.c"
    "${DISCOUNT_BINARY_DIR}/blocktags" 
    "${DISCOUNT_SOURCE_DIR}/tags.c"
    "${DISCOUNT_SOURCE_DIR}/html5.c"
    "${DISCOUNT_SOURCE_DIR}/flags.c"
    "${DISCOUNT_SOURCE_DIR}/pgm_options.c"
    "${DISCOUNT_SOURCE_DIR}/gethopt.c"
)
set_target_properties(libmarkdown PROPERTIES
    OUTPUT_NAME markdown)
set_target_properties(libmarkdown
    PROPERTIES
    DEBUG_OUTPUT_NAME markdown_debug
    RELEASE_OUTPUT_NAME markdown
)

add_executable(markdown
    "${DISCOUNT_SOURCE_DIR}/main.c"
    )
target_link_libraries(markdown PRIVATE libmarkdown)
set_target_properties( markdown
    PROPERTIES
    DEBUG_OUTPUT_NAME markdown_debug
    RELEASE_OUTPUT_NAME markdown
)

add_executable(mkd2html
    "${DISCOUNT_SOURCE_DIR}/mkd2html.c"
    "${DISCOUNT_SOURCE_DIR}/notspecial.c")

target_link_libraries(mkd2html PRIVATE libmarkdown)

add_executable(makepage
    "${DISCOUNT_SOURCE_DIR}/makepage.c"
    )

target_link_libraries(makepage PRIVATE libmarkdown)

add_executable(cpp_example
    "${DISCOUNT_SOURCE_DIR}/examples/cpp/cpp_example.cpp")

target_link_libraries(cpp_example PRIVATE libmarkdown)


if(${PROJECT_NAME}_MAKE_INSTALL)
    string(TOLOWER ${PROJECT_NAME} _PACKAGE_NAME)
    include(GNUInstallDirs)
    if(NOT DEFINED CMAKE_INSTALL_CMAKEDIR)
        set(CMAKE_INSTALL_CMAKEDIR
            "${CMAKE_INSTALL_LIBDIR}/cmake/${_PACKAGE_NAME}"
            CACHE STRING "CMake packages")
    endif()
    install(FILES "${DISCOUNT_BINARY_DIR}/mkdio.h"
        DESTINATION "include/discount")
    target_include_directories(libmarkdown INTERFACE
      $<INSTALL_INTERFACE:include>
    )
    set(_TARGETS libmarkdown markdown)
    if(${PROJECT_NAME}_INSTALL_SAMPLES)
        list(APPEND _TARGETS mkd2html makepage)
    endif()
    install(TARGETS ${_TARGETS} EXPORT ${_PACKAGE_NAME}-targets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
    install(EXPORT ${_PACKAGE_NAME}-targets
        NAMESPACE ${_PACKAGE_NAME}::
        DESTINATION "${CMAKE_INSTALL_CMAKEDIR}")
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
      "${CMAKE_CURRENT_BINARY_DIR}/${_PACKAGE_NAME}-config-version.cmake"
      VERSION ${${PROJECT_NAME}_VERSION}
      COMPATIBILITY AnyNewerVersion
    )
    configure_file("${DISCOUNT_SOURCE_DIR}/cmake/discount-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${_PACKAGE_NAME}-config.cmake"
        @ONLY)
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${_PACKAGE_NAME}-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${_PACKAGE_NAME}-config-version.cmake"
        DESTINATION "${CMAKE_INSTALL_CMAKEDIR}")
    unset(_TARGETS)
    unset(_PACKAGE_NAME)
endif()


