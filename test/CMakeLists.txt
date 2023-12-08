add_library(test_library OBJECT
        ArduinoMock.cpp
        TestTools.cpp
        MockUserInterface.cpp
        MockTransport.cpp
        MockStorage.cpp
        MockCanTransport.cpp
        VlcbCommon.cpp
      )
target_link_libraries(test_library PUBLIC ArduinoFlags)
target_link_libraries(test_library PUBLIC ArduinoMock)
target_link_libraries(test_library PUBLIC core_library)

add_executable(testAll
        testAll.cpp
        testArduino.cpp
        testMinimumNodeService.cpp
        testNodeVariableService.cpp
        testCanService.cpp
        testEventProducerService.cpp
        testEventConsumerService.cpp
        testEventTeachingService.cpp
        testConsumeOwnEventsService.cpp
        testLongMessageService.cpp
	testGridConnect.cpp
        )

target_link_libraries(testAll PUBLIC ArduinoFlags)
target_link_libraries(testAll PUBLIC ArduinoMock)
target_link_libraries(testAll PUBLIC core_library)
target_link_libraries(testAll PUBLIC test_library)
