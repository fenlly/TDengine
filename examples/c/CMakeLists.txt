PROJECT(TDengine)

if(${BUILD_TEST})

IF(TD_LINUX)
  INCLUDE_DIRECTORIES(. ${TD_SOURCE_DIR}/src/inc ${TD_SOURCE_DIR}/src/client/inc ${TD_SOURCE_DIR}/inc)
  AUX_SOURCE_DIRECTORY(. SRC)

  add_executable(tmq "")
  add_executable(stream_demo "")
  add_executable(schemaless "")
  add_executable(prepare "")
  add_executable(demo "")
  add_executable(asyncdemo "")

  target_sources(tmq
    PRIVATE
    "tmq.c"
  )

  target_sources(stream_demo
    PRIVATE
    "stream_demo.c"
  )

  target_sources(schemaless
    PRIVATE
    "schemaless.c"
  )

  target_sources(prepare
    PRIVATE
    "prepare.c"
  )

  target_sources(demo
    PRIVATE
    "demo.c"
  )

  target_sources(asyncdemo
    PRIVATE
    "asyncdemo.c"
  )

  target_link_libraries(tmq
    ${TAOS_NATIVE_LIB}
  )

  target_link_libraries(stream_demo
    ${TAOS_NATIVE_LIB}
  )

  target_link_libraries(schemaless
    ${TAOS_NATIVE_LIB}
  )

  target_link_libraries(prepare
    ${TAOS_NATIVE_LIB}
  )

  target_link_libraries(demo
    ${TAOS_NATIVE_LIB}
  )

  target_link_libraries(asyncdemo
    ${TAOS_NATIVE_LIB}
  )

  SET_TARGET_PROPERTIES(tmq PROPERTIES OUTPUT_NAME tmq)
  SET_TARGET_PROPERTIES(stream_demo PROPERTIES OUTPUT_NAME stream_demo)
  SET_TARGET_PROPERTIES(schemaless PROPERTIES OUTPUT_NAME schemaless)
  SET_TARGET_PROPERTIES(prepare PROPERTIES OUTPUT_NAME prepare)
  SET_TARGET_PROPERTIES(demo PROPERTIES OUTPUT_NAME demo)
  SET_TARGET_PROPERTIES(asyncdemo PROPERTIES OUTPUT_NAME asyncdemo)
ENDIF()

IF(TD_DARWIN)
  INCLUDE_DIRECTORIES(. ${TD_SOURCE_DIR}/src/inc ${TD_SOURCE_DIR}/src/client/inc ${TD_SOURCE_DIR}/inc)
  AUX_SOURCE_DIRECTORY(. SRC)
ENDIF()

endif(${BUILD_TEST})