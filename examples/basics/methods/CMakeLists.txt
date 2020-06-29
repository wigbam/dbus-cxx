include( ../../examples-common.cmake )

pkg_check_modules( popt REQUIRED popt )
include_directories( ${popt_INCLUDE_DIRS} )
link_directories( ${popt_LIBRARY_DIRS} )

add_executable( caller-c caller_c.cpp  )
target_link_libraries( caller-c ${EXAMPLES_LIBRARIES} ${popt_LIBRARIES} )

add_executable( callee-c callee_c.cpp  )
target_link_libraries( callee-c ${EXAMPLES_LIBRARIES} ${popt_LIBRARIES} )

add_executable( caller-raw caller_raw.cpp  )
target_link_libraries( caller-raw ${EXAMPLES_LIBRARIES} ${popt_LIBRARIES} )

add_executable( callee-raw callee_raw.cpp  )
target_link_libraries( callee-raw ${EXAMPLES_LIBRARIES} ${popt_LIBRARIES} )

add_executable( caller caller.cpp  )
target_link_libraries( caller ${EXAMPLES_LIBRARIES} ${popt_LIBRARIES} )

add_executable( callee callee.cpp  )
target_link_libraries( callee ${EXAMPLES_LIBRARIES} ${popt_LIBRARIES} )

add_executable( callee-2 callee_2.cpp  )
target_link_libraries( callee-2 ${EXAMPLES_LIBRARIES} ${popt_LIBRARIES} )

add_executable( callee-object callee_object.cpp  )
target_link_libraries( callee-object ${EXAMPLES_LIBRARIES} ${popt_LIBRARIES} )

add_executable( caller-object caller_object.cpp  )
target_link_libraries( caller-object ${EXAMPLES_LIBRARIES} ${popt_LIBRARIES} )

add_executable( callee-object-inherited callee_object_inherited.cpp  )
target_link_libraries( callee-object-inherited ${EXAMPLES_LIBRARIES} ${popt_LIBRARIES} )

add_executable( callee-dispatched callee_dispatched.cpp  )
target_link_libraries( callee-dispatched ${EXAMPLES_LIBRARIES} ${popt_LIBRARIES} )

add_executable( caller-dispatched caller_dispatched.cpp  )
target_link_libraries( caller-dispatched ${EXAMPLES_LIBRARIES} ${popt_LIBRARIES} )
