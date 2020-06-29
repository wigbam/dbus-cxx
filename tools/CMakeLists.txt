# Check to see if we want to use the bundled libcppgenerate or not
if( TOOLS_BUNDLED_CPPGENERATE )
    include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
    ExternalProject_Add( cppgenerate 
                         SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libcppgenerate
                         DOWNLOAD_COMMAND ""
                         INSTALL_COMMAND "")
    ExternalProject_Get_Property( cppgenerate BINARY_DIR )
    set( cppgenerate_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/libcppgenerate )
    set( cppgenerate_LDFLAGS ${BINARY_DIR}/libcppgenerate.a )
else()
    pkg_check_modules( cppgenerate REQUIRED cppgenerate-static )
endif( TOOLS_BUNDLED_CPPGENERATE )

pkg_check_modules( popt REQUIRED popt )
pkg_check_modules( expat REQUIRED expat )

set( TOOLS_LIBRARY_DIRS ${dbus_LIBRARY_DIRS} ${sigc_LIBRARY_DIRS} ${popt_LIBRARY_DIRS} ${expat_LIBRARY_DIRS} )
set( TOOLS_LIBRARIES dbus-cxx ${dbus_LIBRARIES} ${sigc_LIBRARIES} ${popt_LIBRARIES} ${expat_LIBRARIES} )

if( LIBRT )
	set( TOOLS_LIBRARIES ${TOOLS_LIBRARIES} ${LIBRT} )
endif( LIBRT )

link_directories( ${CMAKE_BINARY_DIR} ${TOOLS_LIBRARY_DIRS} )

include_directories( ${CMAKE_SOURCE_DIR}/dbus-cxx
    ${CMAKE_BINARY_DIR}/dbus-cxx 
    ${dbus_INCLUDE_DIRS} 
    ${sigc_INCLUDE_DIRS}
    ${cppgenerate_INCLUDE_DIRS}
    ${expat_INCLUDE_DIRS}
    ${popt_INCLUDE_DIRS}
)

add_executable( dbus-cxx-xml2cpp 
    generate_adapter.cpp
    generate_proxy.cpp
    parsed_object.cpp
    xml2cpp.cpp
    xml_parser.cpp
    arg.cpp
    method.cpp
    signal.cpp 
    code-generator.cpp 
)
target_link_libraries( dbus-cxx-xml2cpp ${TOOLS_LIBRARIES} ${cppgenerate_LDFLAGS} )
install( TARGETS dbus-cxx-xml2cpp DESTINATION bin )

if( TOOLS_BUNDLED_CPPGENERATE )
    add_dependencies( dbus-cxx-xml2cpp cppgenerate )
endif( TOOLS_BUNDLED_CPPGENERATE )

add_executable( dbus-cxx-introspect
    introspect.cpp )
target_link_libraries( dbus-cxx-introspect ${TOOLS_LIBRARIES} )
install( TARGETS dbus-cxx-introspect DESTINATION bin )

