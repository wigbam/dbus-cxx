find_file( DBUS_RUN_SESSION_PATH NAMES dbus-run-session dbus-run-session.exe )
if( DBUS_RUN_SESSION_PATH STREQUAL "DBUS_RUN_SESSION_PATH-NOTFOUND" )
    message( FATAL_ERROR "Cannot locate 'dbus-run-session' which is required to run tests." )
endif()

file( TO_NATIVE_PATH "${DBUS_RUN_SESSION_PATH}" RUN_WITH_DBUS_EMBEDDED )
file( TO_NATIVE_PATH "${CMAKE_BINARY_DIR}" CMAKE_BINARY_DIR_NATIVE )
file( TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}" CMAKE_CURRENT_BINARY_DIR_NATIVE )

if( WIN32 )
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/test-session.conf ${CMAKE_CURRENT_BINARY_DIR}/test-session.conf COPYONLY )
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/test-session.conf ${CMAKE_CURRENT_BINARY_DIR}/test-session.conf COPYONLY )

    # wait 10ms between starting an embedded DBus session and test execution as DBus needs some time to initialize
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/run-delayed.bat ${CMAKE_CURRENT_BINARY_DIR}/run-delayed.bat COPYONLY )
    list( APPEND RUN_WITH_DBUS_EMBEDDED
        "--config-file=${CMAKE_CURRENT_BINARY_DIR_NATIVE}\\test-session.conf"
        "${CMAKE_CURRENT_BINARY_DIR_NATIVE}\\run-delayed.bat" "1"
    )

    set( DBUX_CXX_BIN_SUBDIR "$<$<CONFIG:Debug>:Debug\\>$<$<CONFIG:Release>:Release\\>" )
    set( DBUX_CXX_TEST_EXT ".exe" )
    set( DBUS_CXX_RUN_CLIENT_SERVER_TEST "run-client-server-test.bat" )

    # On Windows even though dbus-cxx supports UNIX file descriptors libdbus does not
    set( CTEST_CUSTOM_TESTS_IGNORE "messageiterator-filedescriptor" "messageiterator-filedescriptor-2" )
else()
    set( DBUS_CXX_RUN_CLIENT_SERVER_TEST "run-client-server-test.sh" )
endif( WIN32 )

configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/${DBUS_CXX_RUN_CLIENT_SERVER_TEST}" "${CMAKE_CURRENT_BINARY_DIR}/${DBUS_CXX_RUN_CLIENT_SERVER_TEST}" COPYONLY )
configure_file( "${CMAKE_SOURCE_DIR}/cmake-modules/CTestCustom.cmake.in" "${CMAKE_BINARY_DIR}/CTestCustom.cmake" )

file( TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${DBUS_CXX_RUN_CLIENT_SERVER_TEST}" DBUS_CXX_RUN_CLIENT_SERVER_TEST )
set( TEST_LIBRARIES dbus-cxx ${dbus_LIBRARIES} ${sigc_LIBRARIES} )
find_library( LIBRT rt )
if( LIBRT )
	set( TEST_LIBRARIES ${TEST_LIBRARIES} ${LIBRT} )
endif( LIBRT )

function( dbus_cxx_add_test )
    set( oneValueArgs NAME )
    set( multiValueArgs RUNNER COMMAND )
    cmake_parse_arguments( TEST "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
    list( GET TEST_COMMAND 0 TEST_EXECUTABLE )
    list( REMOVE_AT TEST_COMMAND 0 )
    add_test( NAME ${TEST_NAME}
        COMMAND ${TEST_RUNNER} ${CMAKE_CURRENT_BINARY_DIR}/${DBUX_CXX_BIN_SUBDIR}${TEST_EXECUTABLE}${DBUX_CXX_TEST_EXT} ${TEST_COMMAND}
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR_NATIVE}/${DBUX_CXX_BIN_SUBDIR}"
    )
endfunction( dbus_cxx_add_test )

link_directories( ${CMAKE_BINARY_DIR} )

include_directories( ${CMAKE_SOURCE_DIR}/dbus-cxx
    ${CMAKE_BINARY_DIR}/dbus-cxx 
    ${dbus_INCLUDE_DIRS} 
    ${sigc_INCLUDE_DIRS} )

add_executable( test-callmessage callmessagetests.cpp )
target_link_libraries( test-callmessage ${TEST_LIBRARIES} )
target_include_directories( test-callmessage PUBLIC ${CMAKE_SOURCE_DIR} )
target_include_directories( test-callmessage PUBLIC ${CMAKE_CURRENT_BINARY_DIR} )

dbus_cxx_add_test( NAME Callmessage-Bool COMMAND test-callmessage bool )
dbus_cxx_add_test( NAME Callmessage-Byte COMMAND test-callmessage byte )
dbus_cxx_add_test( NAME Callmessage-int16 COMMAND test-callmessage int16 )
dbus_cxx_add_test( NAME Callmessage-uint16 COMMAND test-callmessage uint16 )
dbus_cxx_add_test( NAME Callmessage-int32 COMMAND test-callmessage int32 )
dbus_cxx_add_test( NAME Callmessage-uint32 COMMAND test-callmessage uint32 )
dbus_cxx_add_test( NAME Callmessage-int64 COMMAND test-callmessage int64 )
dbus_cxx_add_test( NAME Callmessage-uint64 COMMAND test-callmessage uint64 )
dbus_cxx_add_test( NAME Callmessage-double COMMAND test-callmessage double )
dbus_cxx_add_test( NAME Callmessage-cstring COMMAND test-callmessage cstring )
dbus_cxx_add_test( NAME Callmessage-string COMMAND test-callmessage string )
dbus_cxx_add_test( NAME Callmessage-array_double COMMAND test-callmessage array_double )
dbus_cxx_add_test( NAME Callmessage-multiple COMMAND test-callmessage multiple )

add_executable( test-messageiterator messageiteratortests.cpp )
target_link_libraries( test-messageiterator ${TEST_LIBRARIES} )
target_include_directories( test-messageiterator PUBLIC ${CMAKE_SOURCE_DIR} )
target_include_directories( test-messageiterator PUBLIC ${CMAKE_CURRENT_BINARY_DIR} )

dbus_cxx_add_test( NAME messageiterator-Bool COMMAND test-messageiterator bool )
dbus_cxx_add_test( NAME messageiterator-Byte COMMAND test-messageiterator byte )
dbus_cxx_add_test( NAME messageiterator-int16 COMMAND test-messageiterator int16 )
dbus_cxx_add_test( NAME messageiterator-uint16 COMMAND test-messageiterator uint16 )
dbus_cxx_add_test( NAME messageiterator-int32 COMMAND test-messageiterator int32 )
dbus_cxx_add_test( NAME messageiterator-uint32 COMMAND test-messageiterator uint32 )
dbus_cxx_add_test( NAME messageiterator-int64 COMMAND test-messageiterator int64 )
dbus_cxx_add_test( NAME messageiterator-uint64 COMMAND test-messageiterator uint64 )
dbus_cxx_add_test( NAME messageiterator-double COMMAND test-messageiterator double )
dbus_cxx_add_test( NAME messageiterator-cstring COMMAND test-messageiterator cstring )
dbus_cxx_add_test( NAME messageiterator-string COMMAND test-messageiterator string )
dbus_cxx_add_test( NAME messageiterator-signature COMMAND test-messageiterator signature )
dbus_cxx_add_test( NAME messageiterator-char COMMAND test-messageiterator char )
dbus_cxx_add_test( NAME messageiterator-int8 COMMAND test-messageiterator int8 )
dbus_cxx_add_test( NAME messageiterator-float COMMAND test-messageiterator float )
dbus_cxx_add_test( NAME messageiterator-long_int COMMAND test-messageiterator long_int )
dbus_cxx_add_test( NAME messageiterator-unsigned_long_int COMMAND test-messageiterator unsigned_long_int )
dbus_cxx_add_test( NAME messageiterator-array_int COMMAND test-messageiterator array_int )
dbus_cxx_add_test( NAME messageiterator-array_string COMMAND test-messageiterator array_string )
dbus_cxx_add_test( NAME messageiterator-array_array_string COMMAND test-messageiterator array_array_string )
dbus_cxx_add_test( NAME messageiterator-filedescriptor COMMAND test-messageiterator filedescriptor )
dbus_cxx_add_test( NAME messageiterator-multiple COMMAND test-messageiterator multiple )

dbus_cxx_add_test( NAME messageiterator-Bool2 COMMAND test-messageiterator bool-2 )
dbus_cxx_add_test( NAME messageiterator-Byte2 COMMAND test-messageiterator byte-2 )
dbus_cxx_add_test( NAME messageiterator-int16-2 COMMAND test-messageiterator int16-2 )
dbus_cxx_add_test( NAME messageiterator-uint16-2 COMMAND test-messageiterator uint16-2 )
dbus_cxx_add_test( NAME messageiterator-int32-2 COMMAND test-messageiterator int32-2 )
dbus_cxx_add_test( NAME messageiterator-uint32-2 COMMAND test-messageiterator uint32-2 )
dbus_cxx_add_test( NAME messageiterator-int64-2 COMMAND test-messageiterator int64-2 )
dbus_cxx_add_test( NAME messageiterator-uint64-2 COMMAND test-messageiterator uint64-2 )
dbus_cxx_add_test( NAME messageiterator-double-2 COMMAND test-messageiterator double-2 )
dbus_cxx_add_test( NAME messageiterator-cstring-2 COMMAND test-messageiterator cstring-2 )
dbus_cxx_add_test( NAME messageiterator-string-2 COMMAND test-messageiterator string-2 )
dbus_cxx_add_test( NAME messageiterator-signature-2 COMMAND test-messageiterator signature-2 )
dbus_cxx_add_test( NAME messageiterator-char-2 COMMAND test-messageiterator char-2 )
dbus_cxx_add_test( NAME messageiterator-int8-2 COMMAND test-messageiterator int8-2 )
dbus_cxx_add_test( NAME messageiterator-float-2 COMMAND test-messageiterator float-2 )
dbus_cxx_add_test( NAME messageiterator-long_int-2 COMMAND test-messageiterator long_int-2 )
dbus_cxx_add_test( NAME messageiterator-unsigned_long_int-2 COMMAND test-messageiterator unsigned_long_int-2 )
dbus_cxx_add_test( NAME messageiterator-array_int-2 COMMAND test-messageiterator array_int-2 )
dbus_cxx_add_test( NAME messageiterator-array_string-2 COMMAND test-messageiterator array_string-2 )
dbus_cxx_add_test( NAME messageiterator-filedescriptor-2 COMMAND test-messageiterator filedescriptor-2 )
dbus_cxx_add_test( NAME messageiterator-multiple-2 COMMAND test-messageiterator multiple-2 )

add_executable( test-path pathclasstests.cpp )
target_link_libraries( test-path ${TEST_LIBRARIES} )
target_include_directories( test-path PUBLIC ${CMAKE_SOURCE_DIR} )
target_include_directories( test-path PUBLIC ${CMAKE_CURRENT_BINARY_DIR} )

dbus_cxx_add_test( NAME path-valid COMMAND test-path test_valid )
dbus_cxx_add_test( NAME path-invalid-chars COMMAND test-path test_invalid_characters )
dbus_cxx_add_test( NAME path-invalid-trail-slash COMMAND test-path test_invalid_trailing_slash )
dbus_cxx_add_test( NAME path-invalid-empty-path COMMAND test-path test_invalid_empty_path )
dbus_cxx_add_test( NAME path-invalid-double-slash COMMAND test-path test_invalid_double_slash )
dbus_cxx_add_test( NAME path-invalid-mising-lead-slash COMMAND test-path test_invalid_missing_leading_slash )
dbus_cxx_add_test( NAME path-decompose-valid COMMAND test-path decompose_valid )
dbus_cxx_add_test( NAME path-decompose-root COMMAND test-path decompose_root )
dbus_cxx_add_test( NAME path-decompose-invalid COMMAND test-path decompose_invalid )
dbus_cxx_add_test( NAME path-decompose-empty COMMAND test-path decompose_empty )
dbus_cxx_add_test( NAME path-append-valid COMMAND test-path append_valid )
dbus_cxx_add_test( NAME path-append-valid-mult COMMAND test-path append_valid_multiple )
dbus_cxx_add_test( NAME path-append-valid-lead-slash COMMAND test-path append_valid_leading_slash )
dbus_cxx_add_test( NAME path-append-valid-trail-slash COMMAND test-path append_valid_trailing_slash )
dbus_cxx_add_test( NAME path-append-invalid-empty COMMAND test-path append_invalid_empty )
dbus_cxx_add_test( NAME path-append-invalid-chars COMMAND test-path append_invalid_empty )
dbus_cxx_add_test( NAME path-append-invalid-double_slash COMMAND test-path append_invalid_double_slash )
dbus_cxx_add_test( NAME path-append-invalid-root COMMAND test-path append_invalid_root )

add_executable( test-timeout timeouttests.cpp )
target_link_libraries( test-timeout ${TEST_LIBRARIES} )
target_include_directories( test-timeout PUBLIC ${CMAKE_SOURCE_DIR} )
target_include_directories( test-timeout PUBLIC ${CMAKE_CURRENT_BINARY_DIR} )

dbus_cxx_add_test( NAME timeout-expires COMMAND test-timeout expires )
dbus_cxx_add_test( NAME timeout-disarm COMMAND test-timeout disarm )
dbus_cxx_add_test( NAME timeout-rearm COMMAND test-timeout rearm )
dbus_cxx_add_test( NAME timeout-outofscope COMMAND test-timeout outofscope )

add_executable( test-connection connectiontests.cpp )
target_link_libraries( test-connection ${TEST_LIBRARIES} )
target_include_directories( test-connection PUBLIC ${CMAKE_SOURCE_DIR} )
target_include_directories( test-connection PUBLIC ${CMAKE_CURRENT_BINARY_DIR} )

dbus_cxx_add_test( NAME connection-proxy-create1 RUNNER ${RUN_WITH_DBUS_EMBEDDED} COMMAND test-connection create_signal_proxy )
dbus_cxx_add_test( NAME connection-proxy-get-iface RUNNER ${RUN_WITH_DBUS_EMBEDDED} COMMAND test-connection get_signal_proxy_by_iface )
dbus_cxx_add_test( NAME connection-proxy-get-iface-name RUNNER ${RUN_WITH_DBUS_EMBEDDED} COMMAND test-connection get_signal_proxy_by_iface_and_name )

#
# Object Tests
#
add_executable( object-tests objecttests.cpp )
target_link_libraries( object-tests ${TEST_LIBRARIES} )
target_include_directories( object-tests PUBLIC ${CMAKE_SOURCE_DIR} )
target_include_directories( object-tests PUBLIC ${CMAKE_CURRENT_BINARY_DIR} )

dbus_cxx_add_test( NAME create-object-proxy RUNNER ${RUN_WITH_DBUS_EMBEDDED} COMMAND object-tests proxy_create )
dbus_cxx_add_test( NAME object-proxy-create-method RUNNER ${RUN_WITH_DBUS_EMBEDDED} COMMAND object-tests proxy_create_method1 )
dbus_cxx_add_test( NAME export-method RUNNER ${RUN_WITH_DBUS_EMBEDDED} COMMAND object-tests export_method )

#
# Data Sending tests - make sure we can actually send data across the bus correctly
#
add_executable( data-tests datatests.cpp )
target_link_libraries( data-tests ${TEST_LIBRARIES} )
target_include_directories( data-tests PUBLIC ${CMAKE_SOURCE_DIR} )
target_include_directories( data-tests PUBLIC ${CMAKE_CURRENT_BINARY_DIR} )

dbus_cxx_add_test( NAME send-integers RUNNER ${RUN_WITH_DBUS_EMBEDDED} ${DBUS_CXX_RUN_CLIENT_SERVER_TEST} COMMAND data-tests send_integers )
dbus_cxx_add_test( NAME call-void-method RUNNER ${RUN_WITH_DBUS_EMBEDDED} ${DBUS_CXX_RUN_CLIENT_SERVER_TEST} COMMAND data-tests void_method )

#
# Signal tests - make sure we can tx and rx singals correctly
add_executable( signal-tests signaltests.cpp )
target_link_libraries( signal-tests ${TEST_LIBRARIES} )
target_include_directories( signal-tests PUBLIC ${CMAKE_SOURCE_DIR} )
target_include_directories( signal-tests PUBLIC ${CMAKE_CURRENT_BINARY_DIR} )

dbus_cxx_add_test( NAME create-signal RUNNER ${RUN_WITH_DBUS_EMBEDDED} COMMAND signal-tests create )
dbus_cxx_add_test( NAME signal-tx-rx RUNNER ${RUN_WITH_DBUS_EMBEDDED} COMMAND signal-tests tx_rx )
