

SET (CMAKE_VERBOSE_MAKEFILE off)

cmake_minimum_required(VERSION 2.8)

SET (CLOGGER_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET (CLOGGER_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/inc)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Source files
SET (CLOGGER_LIB_SRC 	${CLOGGER_SOURCE_PATH}/CLogger.cpp ${CLOGGER_SOURCE_PATH}/CConnector.cpp)
SET (CLOGGER_LIB_INC	${CLOGGER_INCLUDE_PATH}/CLogger.hpp)


ADD_LIBRARY(logger STATIC ${CLOGGER_LIB_SRC})
TARGET_LINK_LIBRARIES(logger  ${PROTO_BUF_LIB} ${POCO_LIB_NET} ${POCO_LIB_UTIL}  ${POCO_LIB_XML}  ${POCO_LIB_FOUNDATION} ${POCO_LIB_JASON}  ${POCO_LIB_UTIL})

target_include_directories(logger
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc:
						  ${POCO_INCLUDE_DIR}
						  ${CLOGGER_INCLUDE_PATH}
						  ${CMAKE_CURRENT_SOURCE_DIR}/../target-agent-runtime/gen
		 >
        $<INSTALL_INTERFACE:inc>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
    

IF(${TA_LOCAL_ENABLE_TESTING})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../target-agent-runtime/gen ${POCO_INCLUDE_DIR})
	IF (CMAKE_COMPILER_IS_GNUCC)
	  SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g -O0 -Wall -fprofile-arcs -ftest-coverage")
	  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -O0 -Wall -fprofile-arcs -ftest-coverage")
	  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
	ENDIF()

	SET (LOGGER_TEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/test)
	SET (LOGGER_TEST_LIB_SRC	 ${LOGGER_TEST_PATH}/config_mock.cpp
									${LOGGER_TEST_PATH}/logger_test.cpp
									${CLOGGER_SOURCE_PATH}/CLogger.cpp
									${CMAKE_CURRENT_SOURCE_DIR}/src/CConnector.cpp
										)

	ADD_EXECUTABLE(logger-test ${LOGGER_TEST_LIB_SRC})
	
	TARGET_LINK_LIBRARIES(logger-test ${GTEST_MAIN_LIB}
													 ${GTEST_LIB} ${GMOCK_LIB}		
													 ${POCO_LIB_FOUNDATION} ${POCO_LIB_UTIL}
													 ${POCO_LIB_NET} 
															)

	add_test(
		NAME logger-test
		COMMAND logger-test
	)
	
	

	SET (CONNECTOR_TEST_LIB_SRC	${CMAKE_CURRENT_SOURCE_DIR}/test/connector_test.cpp
									${CMAKE_CURRENT_SOURCE_DIR}/src/CConnector.cpp
										)

	ADD_EXECUTABLE(connector${DEBUG_SUFFIX}-test ${CONNECTOR_TEST_LIB_SRC})
	
	TARGET_LINK_LIBRARIES(connector${DEBUG_SUFFIX}-test ${GTEST_MAIN_LIB}
													 ${GTEST_LIB} ${GMOCK_LIB}		
													 ${POCO_LIB_FOUNDATION} ${POCO_LIB_UTIL}
													 ${POCO_LIB_NET} 
															)

	add_test(
		NAME connector${DEBUG_SUFFIX}-test
		COMMAND connector${DEBUG_SUFFIX}-test
	)
	
ENDIF()
