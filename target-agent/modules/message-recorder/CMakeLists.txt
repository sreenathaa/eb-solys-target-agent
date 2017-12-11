
cmake_minimum_required(VERSION 2.8.5)

# Source files
SET (MESSAGE_RECORDER_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET (MESSAGE_RECORDER_LIB_SRC 	${MESSAGE_RECORDER_SOURCE_PATH}/MessageRecorder.cpp
								${MESSAGE_RECORDER_SOURCE_PATH}/OfflineRecorder.cpp
								${MESSAGE_RECORDER_SOURCE_PATH}/BufferingHandler.cpp
								${MESSAGE_RECORDER_SOURCE_PATH}/Serializer.cpp
								${MESSAGE_RECORDER_SOURCE_PATH}/PathGenerator.cpp
								)
		
				

SET (MESSAGE_RECORDER_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/inc)
SET (MESSAGE_RECORDER_LIB_INC	${MESSAGE_RECORDER_INCLUDE_PATH}/MessageRecorder.hpp)

include(FindProtobuf)
find_package(Protobuf REQUIRED)
find_package(Poco REQUIRED Foundation Util Net XML JSON)



ADD_LIBRARY(message-recorder STATIC ${MESSAGE_RECORDER_LIB_SRC} ${MESSAGE_RECORDER_LIB_INC})
TARGET_LINK_LIBRARIES(message-recorder 
    ${PROTOBUF_LIBRARY} 
    Poco::Foundation
    Poco::Util
    Poco::Net
    Poco::XML
    Poco::JSON
    ) 

target_include_directories(message-recorder
    PUBLIC
        $<BUILD_INTERFACE:
					${CMAKE_CURRENT_SOURCE_DIR}/inc 
					${CMAKE_CURRENT_SOURCE_DIR}/../target-agent-runtime/inc
					${CMAKE_CURRENT_SOURCE_DIR}/../target-agent-runtime/gen
		>
        $<INSTALL_INTERFACE:inc>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

# Message_Recorder_Test begin
IF(${TA_LOCAL_ENABLE_TESTING})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	IF (CMAKE_COMPILER_IS_GNUCC)
	  SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g -O0 -Wall -fprofile-arcs -ftest-coverage")
	  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -O0 -Wall -fprofile-arcs -ftest-coverage")
	  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
	ENDIF()
	INCLUDE_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR}/../logger/test
	                     ${CMAKE_CURRENT_SOURCE_DIR}/../target-agent-runtime/gen
						 )
	SET (MESSAGE_RECORDER_TEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/test)
	
	
	SET (SERIALIZER_TEST_LIB_SRC	${CMAKE_CURRENT_SOURCE_DIR}/test/SerializerTest.cpp
										${CMAKE_CURRENT_SOURCE_DIR}/test/config_mock.cpp
										${CMAKE_CURRENT_SOURCE_DIR}/../plugin-provider/src/protocol_message.cpp
										${CMAKE_CURRENT_SOURCE_DIR}/src/Serializer.cpp
										)

	ADD_EXECUTABLE(serializer-test ${SERIALIZER_TEST_LIB_SRC})
	
        TARGET_LINK_LIBRARIES(serializer-test ${GTEST_MAIN_LIB} ${GTEST_LIB} ${GMOCK_LIB} ${POCO_LIB_FOUNDATION} ${ADDITIONAL_LIBRARY} ${POCO_LIB_NET} ${PROTOBUF_LIBRARY}
															plugin-provider communicator)

	add_test(
		NAME serializer-test
		COMMAND serializer-test
	)
	
	
	
	SET (PATHHANDLER_TEST_LIB_SRC	    ${CMAKE_CURRENT_SOURCE_DIR}/test/PathGeneratorTest.cpp
								#		${CMAKE_CURRENT_SOURCE_DIR}/test/config_mock.cpp
										${CMAKE_CURRENT_SOURCE_DIR}/src/PathGenerator.cpp
										${CMAKE_CURRENT_SOURCE_DIR}/../logger/test/config_mock.cpp
										)

	ADD_EXECUTABLE(pathGenerator-test ${PATHHANDLER_TEST_LIB_SRC})
	
        TARGET_LINK_LIBRARIES(pathGenerator-test ${GTEST_MAIN_LIB} ${GTEST_LIB} ${GMOCK_LIB} ${POCO_LIB_FOUNDATION} ${ADDITIONAL_LIBRARY} ${POCO_LIB_NET} ${PROTO_BUF_LIBRARY}
															plugin-provider communicator)

	add_test(
		NAME pathGenerator-test
		COMMAND pathGenerator-test
	)
	
ENDIF()


#ADD_LIBRARY(message-recorder${DEBUG_SUFFIX} STATIC ${MESSAGE_RECORDER_LIB_SRC} ${MESSAGE_RECORDER_LIB_INC})
#TARGET_LINK_LIBRARIES(message-recorder${DEBUG_SUFFIX} ${POCO_LIB_FOUNDATION})
#ADD_DEPENDENCIES (message-recorder${DEBUG_SUFFIX} plugin-provider${DEBUG_SUFFIX} communicator${DEBUG_SUFFIX})
