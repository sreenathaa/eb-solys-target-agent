cmake_minimum_required(VERSION 2.8)

PROJECT ( ResourceMonitorPlugin )

#Platform compatibility matrix
#	    QNX 	     WINDOWS	      Linux
#ARM	     x		|	x	|	x	|
#i686	     x		|	x	|	x	|
#x86_64	     x		|	x	|	x	|
#SH2   	     x		|	x	|	x	|
#SH4	     x		|	x	|	x	|
#AMD64	     x		|	x	|	x	|
set(resource_monitor_platformSupportMatrix  "0 0 1|0 0 1|0 0 1|0 0 0|0 0 0|0 0 0" PARENT_SCOPE)
set(resource_monitor_Capabilities "\t\t<${plugin}>\n\t\t\t<Path attr=\"./plugins/${file_name}\"/>
		\n\t\t\t<!-- Resource Monitor Sampling Rate: the recommended range is [50ms,5000ms] default is 1000ms -->
		\n\t\t\t<!-- Setting a sampling rate outside this interval may lead to unreliable measurements-->
		\n\t\t\t<!-- Uncomment the line below and adapt it according to your needs-->
		\n\t\t\t<samplingRate attr=\"1000\"/>
		\n\t\t\t<!-- Observe threads for the process identified by-->
		\n\t\t\t<!-- The filename of the executable as displayed in /proc/[pid]/stat-->
		\n\t\t\t<!-- all instances of the process will be monitored-->
		\n\t\t\t<!-- Uncomment the line below and adapt it according to your needs-->
		\n\t\t\t<!-- <procName attr=\"top\"/> -->
		\n\t\t\t<!-- Observe threads for the process identified by-->
		\n\t\t\t<!-- The process id-->
		\n\t\t\t<!-- Uncomment the line below and adapt it according to your needs-->
		\n\t\t\t<!-- <procID   attr=\"1234\"/> -->
		\n\t\t</${plugin}>" PARENT_SCOPE)


		



SET(PROCFS_READER "src/utils/procFsReader.cpp")
IF(QNXNTO)
	MESSAGE(STATUS "Compile QNXNTO Version of Resource Monitor")
	UNSET(PROCFS_READER)
	SET(PROCFS_READER "src/utils/procFsReaderQnx.cpp")
ENDIF()


SET (RESOURCE_MONITOR_PLUGIN_SRC
				${PROCFS_READER}
				src/resource-monitor-plugin.cpp
				src/resource-monitor-scheduler.cpp
	)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gen PROTO_GLUE_CODE)

ADD_LIBRARY (resource-monitor-plugin SHARED ${RESOURCE_MONITOR_PLUGIN_SRC} ${PROTO_GLUE_CODE})

 target_include_directories(resource-monitor-plugin 
    PUBLIC
        $<BUILD_INTERFACE:
					.${POCO_INCLUDE_DIR} 
		     ${PROTO_BUF_INCLUDE_DIR} 
		     ${COMMON_PLUGIN_INTERFACE_INCLUDE_DIR}
		     ${COMMON_PLUGIN_INTERFACE_GEN_DIR}
		     inc
		     inc/utils
		     gen
		>
        $<INSTALL_INTERFACE:inc>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    
IF(${CMAKE_SYSTEM_NAME} MATCHES "QNX")
  TARGET_LINK_LIBRARIES(resource-monitor-plugin ${PROTO_BUF_LIB} ${POCO_LIB_FOUNDATION})
ELSE()
  TARGET_LINK_LIBRARIES(resource-monitor-plugin ${PROTO_BUF_LIB} ${POCO_LIB_FOUNDATION} pthread)
ENDIF()	

INSTALL(TARGETS resource-monitor-plugin LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/plugins/)

IF (CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g -O0 -Wall -fprofile-arcs -ftest-coverage")
  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -O0 -Wall -fprofile-arcs -ftest-coverage")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
ENDIF()


IF(${TA_LOCAL_ENABLE_TESTING})

  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x -std=c++11")
  ADD_DEFINITIONS("-Wall -Wno-unknown-pragmas")
  SET(RESOURCE_MONITOR_TEST_LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test/resource-monitor-plugin-test.cpp)

  ADD_EXECUTABLE(resource-monitor-plugin-test ${RESOURCE_MONITOR_TEST_LIB_SRC})
 target_include_directories(resource-monitor-plugin-test
    PUBLIC
        $<BUILD_INTERFACE:
			 ${POCO_INCLUDE_DIR} 
		     ${PROTO_BUF_INCLUDE_DIR} 
		     ${COMMON_PLUGIN_INTERFACE_INCLUDE_DIR}
		     ${COMMON_PLUGIN_INTERFACE_GEN_DIR}
		     inc
		     inc/utils
		     gen
		     ${GTEST_INCLUDE_DIR}
		     ${GMOCK_INCLUDE_DIR}
		>
        $<INSTALL_INTERFACE:inc>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
  TARGET_LINK_LIBRARIES(
	  resource-monitor-plugin-test
	  ${PROTO_BUF_LIB}
	  ${POCO_LIB_FOUNDATION}
	  resource-monitor-plugin
	  ${GMOCK_LIB}
	  ${GTEST_LIB}
	  pthread
	  )
  
  add_test(
    NAME resource-monitor-plugin-test
    COMMAND resource-monitor-plugin-test
  )
  
      
ENDIF()
