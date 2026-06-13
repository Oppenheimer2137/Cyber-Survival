cmake_minimum_required(VERSION 3.21)

project(CyberSurvival LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK ON CACHE BOOL "" FORCE)
set(SFML_BUILD_AUDIO ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    SFML
    URL https://github.com/SFML/SFML/archive/refs/tags/2.6.2.zip
)

FetchContent_MakeAvailable(SFML)

add_executable(CyberSurvival
    main.cpp
    Game.cpp
    Constants.cpp
    Utils.cpp
    Upgrades.cpp
    SoundManager.cpp
)

target_include_directories(CyberSurvival PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(CyberSurvival PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    sfml-network
)

if(WIN32)
    add_custom_command(TARGET CyberSurvival POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:CyberSurvival>
        $<TARGET_FILE_DIR:CyberSurvival>
        COMMAND_EXPAND_LISTS
    )
endif()
