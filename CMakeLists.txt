cmake_minimum_required(VERSION 3.22)
project(ZMusicPlayer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(3rd/EUI-NEO)

add_executable(ZMusicPlayer
    src/main.cpp
    src/app_state.cpp
    src/audio_player.cpp
    src/music_library.cpp
)

# The app plays audio through miniaudio directly (volume + looping), reusing the
# implementation that EUI-NEO already compiles into the static eui_neo library.
# Only the declarations are needed here, so this is an include path, not a
# second implementation.
target_include_directories(ZMusicPlayer PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/3rd/EUI-NEO/3rd"
)

eui_neo_configure_app(ZMusicPlayer)
