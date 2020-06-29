include( ../../examples-common.cmake )

add_executable( filedescriptor-client filedescriptor_client.cpp  )
target_link_libraries( filedescriptor-client ${EXAMPLES_LIBRARIES} )

add_executable( filedescriptor-server filedescriptor_server.cpp  )
target_link_libraries( filedescriptor-server ${EXAMPLES_LIBRARIES} )
