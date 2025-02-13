set(TARGET_NAME bayan_tests)
add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE
    test_bayan.cpp 
    ../src/settings.h ../src/cmdparser.cpp ../src/cmdparser.h
    ../src/walker.h ../src/walker.cpp
    ../src/comparer.h ../src/comparer.cpp
    ../src/hashes.h ../src/hashes.cpp
)
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

find_package(GTest REQUIRED)
find_package(Boost REQUIRED COMPONENTS system program_options)

target_compile_definitions(${TARGET_NAME} PRIVATE VIRTUAL_TESTS)

target_link_libraries(${TARGET_NAME} 
    PRIVATE 
        ${Boost_LIBRARIES}
        GTest::gtest_main
)

if(MSVC) # for vs debugging
    set_property(TARGET ${TARGET_NAME} PROPERTY
        VS_DEBUGGER_WORKING_DIRECTORY $(OutDir)
    )
endif()

add_test(BayanTest "${CMAKE_INSTALL_PREFIX}/tests/bayan-tests")

include(GNUInstallDirs)
install(
    TARGETS ${TARGET_NAME} COMPONENT tests
    PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    RUNTIME DESTINATION tests
)

# boost shared libraries for windows
if(MSVC) 
  foreach(lib_target ${Boost_LIBRARIES})
      get_target_property(lib_location ${lib_target} LOCATION)
      install(FILES ${lib_location} DESTINATION ${CMAKE_INSTALL_PREFIX}/tests)
      
      if(CMAKE_BUILD_TYPE STREQUAL "Debug") # run from vs debug
        install(FILES ${lib_location} DESTINATION tests)
      endif()
  endforeach() 

endif()