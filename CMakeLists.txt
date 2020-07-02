#   This file is part of the dbus-cxx library.                            
#                                                                         
#   The dbus-cxx library is free software; you can redistribute it and/or 
#   modify it under the terms of the GNU General Public License           
#   version 3 as published by the Free Software Foundation.               
#                                                                         
#   The dbus-cxx library is distributed in the hope that it will be       
#   useful, but WITHOUT ANY WARRANTY; without even the implied warranty   
#   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU   
#   General Public License for more details.                              
#                                                                         
#   You should have received a copy of the GNU General Public License     
#   along with this software. If not see <http://www.gnu.org/licenses/>.  

project( dbus-cxx )

cmake_minimum_required( VERSION 3.1 )

#
# Check our options
#
option( ENABLE_TESTS "Enable the unit tests" OFF )
option( ENABLE_EXAMPLES "Enable the examples" OFF )
option( ENABLE_TOOLS "Enable dbus-cxx tools" OFF )
option( ENABLE_GLIBMM "Enable GLibMM support" OFF )
option( BUILD_SITE "Build the dbus-cxx website reference" OFF )
option( TOOLS_BUNDLED_CPPGENERATE "Use bundled libcppgenerate" ON )
option( BUILD_STATIC_LIBRARIES "Build static library" OFF )
option( ENABLE_CODE_COVERAGE_REPORT "Enable code coverage report" OFF )

list( APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules" )

include( CheckIncludeFiles )
include( GNUInstallDirs )
include( FindPkgConfig )
include( CheckTypeSize )
include( CheckSymbolExists )
include( CheckCXXSymbolExists )

if( ENABLE_CODE_COVERAGE_REPORT )
    include( CodeCoverage )
endif( ENABLE_CODE_COVERAGE_REPORT )

# version information
set( DBUS_CXX_PACKAGE_MAJOR_VERSION 0 )
set( DBUS_CXX_PACKAGE_MINOR_VERSION 11 )
set( DBUS_CXX_PACKAGE_MICRO_VERSION 0 )
set( PKG_VERSION ${DBUS_CXX_PACKAGE_MAJOR_VERSION}.${DBUS_CXX_PACKAGE_MINOR_VERSION}.${DBUS_CXX_PACKAGE_MICRO_VERSION} )
set( DBUS_CXX_MAJMIN_VERSION ${DBUS_CXX_PACKAGE_MAJOR_VERSION}.${DBUS_CXX_PACKAGE_MINOR_VERSION} )

# Our required dependencies are libdbus and libsigc++
pkg_check_modules( dbus REQUIRED dbus-1>=1.3 )
pkg_check_modules( sigc REQUIRED sigc++-2.0 )

if( WIN32 )
	pkg_check_modules( Boost boost>=1.70 )
	if ( Boost_FOUND )
		include_directories( ${Boost_INCLUDE_DIRS} )
	else()
		find_package( Boost 1.70 REQUIRED )
	endif( Boost_FOUND )
	add_definitions( -DBOOST_ASIO_STANDALONE )
endif( WIN32 )

if( BUILD_STATIC_LIBRARIES )
	link_directories( ${dbus_STATIC_LIBRARY_DIRS} ${sigc_STATIC_LIBRARY_DIRS} )
else()
	link_directories( ${dbus_LIBRARY_DIRS} ${sigc_LIBRARY_DIRS} )
endif( BUILD_STATIC_LIBRARIES )

# Dbus-cxx requires at least C++11
set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

#
# Build environment
#
function( check_symbol NAME FILE VAR )
    check_symbol_exists( ${NAME} "${FILE}" ${VAR} )
    if( ${VAR} )
        add_definitions( -D${VAR} )
    endif( ${VAR} )
endfunction( check_symbol )

function( check_cxx_symbol NAME FILE VAR )
    check_cxx_symbol_exists( ${NAME} "${FILE}" ${VAR} )
    if( ${VAR} )
        add_definitions( -D${VAR} )
    endif( ${VAR} )
endfunction( check_cxx_symbol )

if( WIN32 )
    if( CMAKE_SYSTEM_VERSION )
        set( ver ${CMAKE_SYSTEM_VERSION} )
        string( REGEX MATCH "^([0-9]+).([0-9])" ver ${ver} )
        string( REGEX MATCH "^([0-9]+)" verMajor ${ver} )
        # Check for Windows 10, b/c we'll need to convert to hex 'A'.
        if( "${verMajor}" MATCHES "10" )
            set( verMajor "A" )
            string( REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver} )
        endif()
        # Remove all remaining '.' characters.
        string( REPLACE "." "" ver ${ver} )
        # Prepend each digit with a zero.
        string( REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver} )
        set( WIN32_VER "0x${ver}" )
    endif( CMAKE_SYSTEM_VERSION )

    add_definitions( -D_WIN32_WINNT=${WIN32_VER} )
    check_cxx_symbol( shared_mutex "shared_mutex" HAVE_SHARED_MUTEX )

    set( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON )
	if( MSVC )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus" )
	endif( MSVC )
endif( WIN32 )

check_symbol( localtime_r "time.h" HAVE_LOCALTIME_R )
check_symbol( pipe "unistd.h" HAVE_PIPE )
check_symbol( pipe2 "unistd.h" HAVE_PIPE2 )
check_symbol( sleep "unistd.h" HAVE_SLEEP )

#
# Configure our compile options
#
set( DBUS_CXX_HAVE_DBUS_12 1 )
CHECK_TYPE_SIZE( "long int" DBUS_CXX_SIZEOF_LONG_INT )
configure_file( dbus-cxx-config.h.cmake.in dbus-cxx/dbus-cxx-config.h )

# 
# All sources
#
set( DBUS_CXX_SOURCES
    dbus-cxx/callmessage.cpp
    dbus-cxx/connection.cpp
    dbus-cxx/dispatcher.cpp
    dbus-cxx/error.cpp
    dbus-cxx/errormessage.cpp
    dbus-cxx/interface.cpp
    dbus-cxx/interfaceproxy.cpp
    dbus-cxx/messageappenditerator.cpp
    dbus-cxx/message.cpp
    dbus-cxx/messagefilter.cpp
    dbus-cxx/messagehandler.cpp
    dbus-cxx/messageiterator.cpp
    dbus-cxx/methodbase.cpp
    dbus-cxx/methodproxybase.cpp
    dbus-cxx/object.cpp
    dbus-cxx/objectpathhandler.cpp
    dbus-cxx/objectproxy.cpp
    dbus-cxx/path.cpp
    dbus-cxx/pendingcall.cpp
    dbus-cxx/returnmessage.cpp
    dbus-cxx/signal_base.cpp
    dbus-cxx/signalmessage.cpp
    dbus-cxx/signal_proxy_base.cpp
    dbus-cxx/signalreceiver.cpp
    dbus-cxx/signature.cpp
    dbus-cxx/signatureiterator.cpp
    dbus-cxx/timeout.cpp
    dbus-cxx/utility.cpp
    dbus-cxx/watch.cpp )

# headers that need to go in the include/ directory
set( DBUS_CXX_HEADERS1
    dbus-cxx.h
)

# headers that need to go in the include/dbus-cxx directory
# Auto-generated files are added later
set( DBUS_CXX_HEADERS
    dbus-cxx/accumulators.h
    dbus-cxx/callmessage.h
    dbus-cxx/dbus_cxx_macros.h
    dbus-cxx/dispatcher.h
    dbus-cxx/enums.h
    dbus-cxx/error.h
    dbus-cxx/errormessage.h
    dbus-cxx/filedescriptor.h
    dbus-cxx/forward_decls.h
    dbus-cxx/headerlog.h
    dbus-cxx/messageappenditerator.h
    dbus-cxx/messagefilter.h
    dbus-cxx/message.h
    dbus-cxx/messagehandler.h
    dbus-cxx/messageiterator.h
    dbus-cxx/methodbase.h
    dbus-cxx/objectpathhandler.h
    dbus-cxx/path.h
    dbus-cxx/pendingcall.h
    dbus-cxx/pointer.h
    dbus-cxx/returnmessage.h
    dbus-cxx/signal_base.h
    dbus-cxx/signalmessage.h
    dbus-cxx/signal_proxy_base.h
    dbus-cxx/signalreceiver.h
    dbus-cxx/signature.h
    dbus-cxx/signatureiterator.h
    dbus-cxx/simplelogger_defs.h
    dbus-cxx/simplelogger.h
    dbus-cxx/timeout.h
    dbus-cxx/types.h
    dbus-cxx/utility.h
    dbus-cxx/variant.h
    dbus-cxx/watch.h
    ${CMAKE_CURRENT_BINARY_DIR}/dbus-cxx/dbus-cxx-config.h
)

set( DBUS_CXX_INCLUDE_DIRECTORIES 
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus-cxx
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/dbus-cxx )

if( WIN32 )
	set( DBUS_CXX_INCLUDE_DIRECTORIES ${DBUS_CXX_INCLUDE_DIRECTORIES} ${PROJECT_SOURCE_DIR}/compat )
	set( DBUS_CXX_SOURCES ${DBUS_CXX_SOURCES}
	    compat/poll.c
	    compat/pipe2.c
	    compat/posix_win.c
	)
endif( WIN32 )

include_directories( ${DBUS_CXX_INCLUDE_DIRECTORIES}
    ${dbus_INCLUDE_DIRS} 
    ${sigc_INCLUDE_DIRS} )


#
# Target for the library
#
add_library( dbus-cxx SHARED ${DBUS_CXX_SOURCES} )
set_target_properties( dbus-cxx PROPERTIES VERSION 1.0.0 SOVERSION 1 )
set( DBUS_CXX_EXTRA_LIBS )

find_library( LIBRT rt )
if( LIBRT )
	set( DBUS_CXX_EXTRA_LIBS ${DBUS_CXX_EXTRA_LIBS} ${LIBRT} )
endif( LIBRT )

set( THREADS_PREFER_PTHREAD_FLAG ON )
find_package( Threads )
if( Threads_FOUND )
	set( DBUS_CXX_EXTRA_LIBS ${DBUS_CXX_EXTRA_LIBS} Threads::Threads )
endif( Threads_FOUND )

target_link_libraries( dbus-cxx ${dbus_LIBRARIES} ${sigc_LIBRARIES} ${DBUS_CXX_EXTRA_LIBS} )

if( BUILD_STATIC_LIBRARIES )
	add_library( dbus-cxx-static STATIC ${DBUS_CXX_SOURCES} )
	target_compile_definitions( dbus-cxx-static PUBLIC DBUS_STATIC_BUILD DBUS_CXX_STATIC_BUILD )
	if( NOT WIN32 )
        set_target_properties( dbus-cxx-static PROPERTIES OUTPUT_NAME "dbus-cxx" )
    endif( NOT WIN32 )
	target_link_libraries( dbus-cxx-static
	    ${dbus_STATIC_LIBRARIES}
	    ${sigc_STATIC_LIBRARIES}
	    ${DBUS_CXX_EXTRA_LIBS}
	)
endif( BUILD_STATIC_LIBRARIES )

#
# If GLIBMM is used, add in the appropriate packages
#
if( ENABLE_GLIBMM )
    pkg_check_modules( glibmm-2.4 REQUIRED glibmm-2.4 )

    function( build_glibmm_library TARGET_NAME TARGET_TYPE DEP_TARGET_NAME PROP_PREFIX )
        link_directories( ${glibmm-2.4_${PROP_PREFIX}LIBRARY_DIRS} )
        add_library( ${TARGET_NAME} ${TARGET_TYPE} dbus-cxx-glibmm/dispatcher.cpp )
        add_dependencies( ${TARGET_NAME} ${DEP_TARGET_NAME} )
        target_link_libraries( ${TARGET_NAME} ${DEP_TARGET_NAME} ${glibmm-2.4_${PROP_PREFIX}LIBRARIES} )
        target_include_directories( ${TARGET_NAME} PRIVATE
            ${glibmm-2.4_${PROP_PREFIX}INCLUDE_DIRS}
            ${glibmm-2.4_${PROP_PREFIX}INCLUDEDIR}
        )
        set( install_targets ${install_targets} ${TARGET_NAME} PARENT_SCOPE )
    endfunction( build_glibmm_library )

    build_glibmm_library( dubs-cxx-glibmm SHARED dbus-cxx "" )
    set_target_properties( dubs-cxx-glibmm PROPERTIES VERSION 1.0.0 SOVERSION 1 )
	if( BUILD_STATIC_LIBRARIES )
        build_glibmm_library( dbus-cxx-glibmm-static STATIC dbus-cxx-static "STATIC_" )
	    if( NOT WIN32 )
            set_target_properties( dbus-cxx-glibmm-static PROPERTIES OUTPUT_NAME "dbus-cxx-glibmm" )
        endif( NOT WIN32 )
    endif( BUILD_STATIC_LIBRARIES )

    install( FILES dbus-cxx-glibmm.h DESTINATION include/dbus-cxx-${DBUS_CXX_MAJMIN_VERSION} )
    install( FILES dbus-cxx-glibmm/dispatcher.h DESTINATION include/dbus-cxx-${DBUS_CXX_MAJMIN_VERSION}/dbus-cxx-glibmm/ )
endif( ENABLE_GLIBMM )

#
# gnerate files with m4
#
# https://samthursfield.wordpress.com/2015/11/21/cmake-dependencies-between-targets-and-files-and-custom-commands/
# https://gist.github.com/socantre/7ee63133a0a3a08f3990
foreach( filename 
    dbus_signal.h.m4
    interface.h.m4
    interfaceproxy.h.m4
    method.h.m4
    method_impl.h.m4
    methodproxybase.h.m4
    methodproxy.h.m4
    objectproxy.h.m4
    signal_proxy.h.m4
    connection.h.m4
    object.h.m4 )

    string( REPLACE ".m4" "" sanitized_file ${filename} )

    set( output_file ${CMAKE_CURRENT_BINARY_DIR}/dbus-cxx/${sanitized_file} )
    set( input_file ${CMAKE_CURRENT_SOURCE_DIR}/dbus-cxx ${CMAKE_CURRENT_SOURCE_DIR}/dbus-cxx/${filename} )
    set( DBUS_CXX_HEADERS ${DBUS_CXX_HEADERS} ${output_file} )

    # Add the custom command which calls m4 to generate the headers
    add_custom_command(
        OUTPUT
            ${output_file}
        COMMAND
            m4 -I ${input_file} > ${output_file}
        VERBATIM
    )

    # The custom target tells cmake that we need to generate this
    add_custom_target( generate-${sanitized_file} 
        DEPENDS 
            ${output_file}
    )

    # we need to add a dependency to the library to ensure that the files get regenerated
    add_dependencies( dbus-cxx generate-${sanitized_file} )
	if( BUILD_STATIC_LIBRARIES )
		add_dependencies( dbus-cxx-static generate-${sanitized_file} )
	endif( BUILD_STATIC_LIBRARIES )
endforeach( filename )

#
# Library install information
#
set( install_targets ${install_targets} dbus-cxx )
if( BUILD_STATIC_LIBRARIES )
	set( install_targets ${install_targets} dbus-cxx-static )
endif( BUILD_STATIC_LIBRARIES )

install( TARGETS ${install_targets}
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" )
install( FILES ${DBUS_CXX_HEADERS} DESTINATION include/dbus-cxx-${DBUS_CXX_MAJMIN_VERSION}/dbus-cxx )
install( FILES ${DBUS_CXX_HEADERS1} DESTINATION include/dbus-cxx-${DBUS_CXX_MAJMIN_VERSION} )

#
# pkg-config script creation and install
#
if( WIN32 )
    SET( PKG_CONFIG_INSTALL_PREFIX "\${INSTALL_PREFIX}" )
else()
    SET( PKG_CONFIG_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" )
endif( WIN32 )

SET(PKG_CONFIG_LIBDIR
    "\${prefix}/lib"
)
SET(PKG_CONFIG_INCLUDEDIR
    "\${prefix}/include/dbus-cxx-${DBUS_CXX_MAJMIN_VERSION}"
)
SET(PKG_CONFIG_LIBS
    "-L\${libdir} -ldbus-cxx"
)
SET(PKG_CONFIG_LIBS_MM
    "-L\${libdir} -ldbus-cxx-glibmm"
)
SET(PKG_CONFIG_CFLAGS
    "-I\${includedir}"
)
SET(PKG_CONFIG_REQUIRES
    "dbus-1 >= 1.3, sigc++-2.0"
)
if ( BUILD_STATIC_LIBRARIES )
    if( CMAKE_THREAD_LIBS_INIT )
        set( PKG_CONFIG_LIBS_PRIVATE ${CMAKE_THREAD_LIBS_INIT} )
    endif( CMAKE_THREAD_LIBS_INIT )
    if( WIN32 )
        set( PKG_CONFIG_LIBS_PRIVATE ${PKG_CONFIG_LIBS_PRIVATE} "-ldbus-cxx-static" )
        set( PKG_CONFIG_LIBS_MM_PRIVATE "-ldbus-cxx-glibmm-static" )
    endif( WIN32 )
endif( BUILD_STATIC_LIBRARIES )

CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/dbus-cxx-1.0.pc.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/dbus-cxx-1.0.pc"
)
INSTALL( FILES "${CMAKE_BINARY_DIR}/dbus-cxx-1.0.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

if( ENABLE_GLIBMM )
    CONFIGURE_FILE(
        "${CMAKE_CURRENT_SOURCE_DIR}/dbus-cxx-glibmm-1.0.pc.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/dbus-cxx-glibmm-1.0.pc"
    )
    INSTALL( FILES "${CMAKE_BINARY_DIR}/dbus-cxx-glibmm-1.0.pc"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif( ENABLE_GLIBMM )

#
# Include the directory for the examples
#
if( ENABLE_EXAMPLES )
    add_subdirectory( examples )
endif( ENABLE_EXAMPLES )

#
# Include the directory for the tools
#
if( ENABLE_TOOLS )
    add_subdirectory( tools )
endif( ENABLE_TOOLS )

#
# If we want to build the site, we must have doxygen
#
if( BUILD_SITE )
    find_package( Doxygen 
                  REQUIRED dot )
    file( COPY ${CMAKE_CURRENT_SOURCE_DIR}/doc/
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/doc/ )
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile
                    ${CMAKE_CURRENT_BINARY_DIR}/doc @ONLY )

    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
        DEPENDS dbus-cxx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
    add_custom_target( tar_site ALL
        COMMAND tar -czf dbus-cxx-website-${PKG_VERSION}.tar.gz -C doc/reference/html .
        DEPENDS doc_doxygen
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM )
endif( BUILD_SITE )

#
# Check if tests are enabled
#
if( ENABLE_TESTS )
    enable_testing()
    add_subdirectory( unit-tests )
endif( ENABLE_TESTS )

#
# See if code coverage is enabled
#
if( ENABLE_CODE_COVERAGE_REPORT )
    APPEND_COVERAGE_COMPILER_FLAGS()
    set(COVERAGE_LCOV_EXCLUDES 'unit-tests/*' '/usr/include/*' '/usr/local/include/*' )
    SETUP_TARGET_FOR_COVERAGE_LCOV(
        NAME dbus-cxx-coverage
        EXECUTABLE "ctest" )
endif( ENABLE_CODE_COVERAGE_REPORT )

#
# Cpack for creating the dist files
#
set( CPACK_SOURCE_PACKAGE_FILE_NAME "dbus-cxx-${PKG_VERSION}" )
set( CPACK_PACKAGE_VENDOR dbus-cxx )
set( CPACK_PACKAGE_VERSION_MAJOR ${DBUS_CXX_PACKAGE_MAJOR_VERSION} )
set( CPACK_PACKAGE_VERSION_MINOR ${DBUS_CXX_PACKAGE_MINOR_VERSION} )
set( CPACK_PACKAGE_VERSION_PATCH ${DBUS_CXX_PACKAGE_MICRO_VERSION} )
set( CPACK_SOURCE_GENERATOR "TGZ" )
set( CPACK_SOURCE_IGNORE_FILES ".git/;build/" )
set( CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/COPYING" )

if( ${CPACK_BINARY_NSIS} )
    string( TOLOWER ${CMAKE_GENERATOR_PLATFORM} CMAKE_GENERATOR_PLATFORM_LOWER )
    set( CPACK_NSIS_PACKAGE_NAME "dbus-cxx-${CMAKE_GENERATOR_PLATFORM_LOWER}-${PKG_VERSION}" )
    set( CPACK_NSIS_DISPLAY_NAME "${CPACK_NSIS_PACKAGE_NAME}" )
    set( CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_NSIS_PACKAGE_NAME}" )
    set( CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_NSIS_PACKAGE_NAME}" )
    set( CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/.folder.ico" )
    set( CPACK_NSIS_MUI_UNIICON "${CPACK_NSIS_MUI_ICON}" )
    set( CPACK_NSIS_HELP_LINK "https://dbus-cxx.github.io/" )
    set( CPACK_NSIS_URL_INFO_ABOUT "https://dbus-cxx.github.io/" )
    set( CPACK_NSIS_MODIFY_PATH ON )

    # Patch NSIS.template.in to include helper functions
    set( DBUS_CXX_NSIS_TEMPLATE_FILE "${CMAKE_CURRENT_BINARY_DIR}/NSIS.template.in" )
    configure_file( "${CMAKE_SOURCE_DIR}/cmake-modules/NSISExtra.nsh" "${DBUS_CXX_NSIS_TEMPLATE_FILE}" COPYONLY )
    file( GLOB_RECURSE DBUS_CXX_NSIS_TEMPLATE FOLLOW_SYMLINKS "${CMAKE_ROOT}/Modules/*NSIS.template.in" )
    file( READ ${DBUS_CXX_NSIS_TEMPLATE} NSIS_TEMPLATE_CONTENTS )
    file( APPEND ${DBUS_CXX_NSIS_TEMPLATE_FILE} "${NSIS_TEMPLATE_CONTENTS}" )
    list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}" )

    # Populate correct pkg-config PREFIX paths after install
    file( GLOB DBUS_CXX_PKG_CONFIG_FILES "${CMAKE_CURRENT_BINARY_DIR}/*.pc" )
    foreach( DBUS_CXX_PKG_CONFIG_FILE IN LISTS DBUS_CXX_PKG_CONFIG_FILES )
        get_filename_component( DBUS_CXX_PKG_CONFIG_FILENAME ${DBUS_CXX_PKG_CONFIG_FILE} NAME )
        list( APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS
            "\\\${StrRep} \\\$0 \\\"\\\$INSTDIR\\\" \\\"\\\\\\\" \\\"/\\\"" )
        list( APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS
            "!insertmacro _ReplaceInFile \\\"\\\$INSTDIR\\\\${CMAKE_INSTALL_LIBDIR}\\\\pkgconfig\\\\${DBUS_CXX_PKG_CONFIG_FILENAME}\\\" \\\"\\\$\\\${INSTALL_PREFIX}\\\" \\\"\\\$0\\\"" )
    endforeach()
    string( REPLACE ";" "\n" CPACK_NSIS_EXTRA_INSTALL_COMMANDS "${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}" )
endif( ${CPACK_BINARY_NSIS} )

include(CPack)

#
# Print configuration information for the user
#
message(STATUS "")
message(STATUS "")
message(STATUS "dbus-cxx configuration summary:")
message(STATUS "")

message(STATUS "  Build type ...................... : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix .................. : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Library location ................ : ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
message(STATUS "  C++ compiler .................... : ${CMAKE_CXX_COMPILER}")
message(STATUS "  Build examples .................. : ${ENABLE_EXAMPLES}")
message(STATUS "  Build tests ..................... : ${ENABLE_TESTS}")
message(STATUS "  Build tools ..................... : ${ENABLE_TOOLS}")
message(STATUS "  Build static libraries........... : ${BUILD_STATIC_LIBRARIES}")
message(STATUS "  Use bundled cppgenerate ......... : ${TOOLS_BUNDLED_CPPGENERATE}")
message(STATUS "  Enable GLibmm support ........... : ${ENABLE_GLIBMM}")
message(STATUS "  Build website ................... : ${BUILD_SITE}")
message(STATUS "  Enable code coverage report ..... : ${ENABLE_CODE_COVERAGE_REPORT}")
