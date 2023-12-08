set_source_files_properties(examples/VLCB_4in4out/VLCB_4in4out.ino PROPERTIES LANGUAGE CXX)
add_executable(VLCB_4in4out
  examples/VLCB_4in4out/VLCB_4in4out.ino
  examples/VLCB_4in4out/LEDControl.cpp
)
target_link_libraries(VLCB_4in4out PUBLIC hardware_library)
target_link_libraries(VLCB_4in4out PUBLIC core_library)
target_link_libraries(VLCB_4in4out PUBLIC ArduinoFlags)
target_link_libraries(VLCB_4in4out PUBLIC ArduinoCore)
target_link_libraries(VLCB_4in4out PUBLIC ArduinoLibs)
target_link_libraries(VLCB_4in4out PUBLIC ArduinoBounce2)

arduino_avr_hex(VLCB_4in4out)
arduino_avr_upload(VLCB_4in4out ${ARDUINO_PORT})
