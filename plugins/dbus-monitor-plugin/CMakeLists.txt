cmake_minimum_required(VERSION 2.8)

PROJECT ( DbusMonitorPlugin )

#Platform compatibility matrix
#	    QNX 	     WINDOWS	      Linux
#ARM	     x		|	x	|	x	|
#i686	     x		|	x	|	x	|
#x86_64	     x		|	x	|	x	|
#SH2   	     x		|	x	|	x	|
#SH4	     x		|	x	|	x	|
#AMD64	     x		|	x	|	x	|
set(dbus_monitor_platformSupportMatrix  "0 0 1|0 0 1|0 0 1|0 0 0|0 0 0|0 0 0" PARENT_SCOPE)

	set(dbus_monitor_Capabilities "\t\t<${plugin}>\n\t\t\t<Path attr=\"./plugins/${LINUX_LIB_PREFIX}${plugin}${DEBUG_SUFFIX}${LIBRARY_EXTENSION}\"/>
		\n\t\t\t<!-- DBUS_BUS: bus to be observed\; must be one of: DBUS_BUS_SESSION, DBUS_BUS_SYSTEM -->
		\n\t\t\t<DBUS_BUS attr=\"DBUS_BUS_SESSION\"/>
		\n\t\t\t<!-- Add your config Item here: <ConfigItem1 attr=\"value1\"/> -->
		\n\t\t\t<!-- Add your config Item here: <ConfigItem1 attr=\"value1\"/> -->
		\n\t\t</${plugin}>" PARENT_SCOPE)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x -std=c++11")
ADD_DEFINITIONS("-Wall -Wno-unknown-pragmas")
IF (CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g -O0 -Wall -fprofile-arcs -ftest-coverage")
  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -O0 -Wall -fprofile-arcs -ftest-coverage")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
ENDIF()

# Find DBUS
find_package(PkgConfig REQUIRED)
pkg_search_module(DBUS dbus-1)
if(DBUS_FOUND)
   # nop
else()
set( DBUS_INCLUDE_DIRS ${CMAKE_SYSROOT_PATH}/usr/include/dbus-1.0
                       ${CMAKE_SYSROOT_PATH}/usr/lib/dbus-1.0/include
                       ${CMAKE_SYSROOT_PATH}/lib/dbus-1.0/include
                       /usr/lib/i386-linux-gnu/dbus-1.0/include
   )
endif()

INCLUDE_DIRECTORIES (${POCO_INCLUDE_DIR} 
		     ${PROTO_BUF_INCLUDE_DIR} 
		     ${COMMON_PLUGIN_INTERFACE_INCLUDE_DIR}
		     ${COMMON_PLUGIN_INTERFACE_GEN_DIR}
		     ${DBUS_INCLUDE_DIRS}
		     inc
		     gen)
		     
LINK_DIRECTORIES  (${PROTO_BUF_LIB_PATH} )

SET (DBUS_MONITOR_PLUGIN_SRC    src/dbus-monitor-plugin.cpp 
                                src/dbus_handler.cpp
                                src/dbus_producer.cpp  
                                src/dbus_data_pool.cpp
                                src/dbus_consumer.cpp                                      
)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gen PROTO_GLUE_CODE)

ADD_LIBRARY (dbus-monitor-plugin SHARED ${DBUS_MONITOR_PLUGIN_SRC} ${PROTO_GLUE_CODE})

SET_TARGET_PROPERTIES(dbus-monitor-plugin PROPERTIES COMPILE_FLAGS "-std=c++0x")
TARGET_LINK_LIBRARIES(dbus-monitor-plugin  ${PROTO_BUF_LIB}  ${POCO_LIB_FOUNDATION} ${POCO_LIB_UTIL} ${POCO_LIB_XML} pthread dbus-1)

INSTALL(FILES ${CMAKE_BINARY_DIR}/bin/${BUILD_FOLDER_NAME}/${LINUX_LIB_PREFIX}dbus-monitor-plugin${DEBUG_SUFFIX}${LIBRARY_EXTENSION} DESTINATION ${CMAKE_INSTALL_PREFIX}/plugins/)

IF(${TA_LOCAL_ENABLE_TESTING})
  INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR} ${GMOCK_INCLUDE_DIR}    ${COMMON_PLUGIN_INTERFACE_GEN_DIR})
  
  SET(RESOURCE_MONITOR_TEST_LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test/dbus-monitor-plugin-test.cpp)

  ADD_EXECUTABLE(dbus-monitor-plugin${DEBUG_SUFFIX}-test ${RESOURCE_MONITOR_TEST_LIB_SRC})

  TARGET_LINK_LIBRARIES(
	  dbus-monitor-plugin${DEBUG_SUFFIX}-test
	  ${PROTO_BUF_LIB}
	  ${POCO_LIB_FOUNDATION}
	  ${GMOCK_LIB}
	  ${GTEST_LIB}
	  pthread
	  dbus-1 )
  
  add_test(
    NAME dbus-monitor-plugin${DEBUG_SUFFIX}-test
    COMMAND dbus-monitor-plugin${DEBUG_SUFFIX}-test
  )
ENDIF()
