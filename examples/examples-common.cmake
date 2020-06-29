set( EXAMPLES_LIBRARIES dbus-cxx ${dbus_LIBRARIES} ${sigc_LIBRARIES} )

if( LIBRT )
	set( EXAMPLES_LIBRARIES ${EXAMPLES_LIBRARIES} ${LIBRT} )
endif( LIBRT )

link_directories( ${CMAKE_BINARY_DIR} )

include_directories( ${CMAKE_SOURCE_DIR}/dbus-cxx
    ${CMAKE_BINARY_DIR}/dbus-cxx 
    ${dbus_INCLUDE_DIRS} 
    ${sigc_INCLUDE_DIRS} )
