set(TARGET_NAME bayan)
add_executable(${TARGET_NAME} main.h main.cpp 
    settings.h cmdparser.h cmdparser.cpp
    walker.h walker.cpp
    comparer.h comparer.cpp
    hashes.h hashes.cpp
)

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Boost REQUIRED COMPONENTS system locale filesystem program_options)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        ${Boost_LIBRARIES}
)

include(GNUInstallDirs)
install(
    TARGETS ${TARGET_NAME} COMPONENT app
    PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
