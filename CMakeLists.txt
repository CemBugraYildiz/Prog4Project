cmake_minimum_required(VERSION 3.26)
project(Minigin)

if(EMSCRIPTEN)
  message("Using Emscripten!")
endif()

# Minigin Engine Library
add_library(Minigin STATIC
  Minigin/Font.h
  Minigin/Font.cpp
  Minigin/GameObject.cpp
  Minigin/GameObject.h
  Minigin/InputManager.cpp
  Minigin/InputManager.h
  Minigin/Minigin.cpp
  Minigin/Renderer.cpp
  Minigin/ResourceManager.cpp
  Minigin/Scene.cpp
  Minigin/SceneManager.cpp
  Minigin/TextObject.cpp
  Minigin/Texture2D.cpp
  Minigin/Transform.cpp
  Minigin/RenderComponent.cpp
  Minigin/TextComponent.cpp
  Minigin/FPSComponent.cpp
  Minigin/FPSComponent.h
  Minigin/OrbitComponent.h
  Minigin/OrbitComponent.cpp
  Minigin/Command.h
  Minigin/Component.h
  Minigin/Controller.h
  Minigin/Controller.cpp
  Minigin/MoveCommand.h
  Minigin/Event.h
  Minigin/IEventListener.h
  Minigin/EventQueue.h
  Minigin/EventQueue.cpp
  Minigin/HealthComponent.h
  Minigin/HealthComponent.cpp
  Minigin/ScoreComponent.h
  Minigin/ScoreComponent.cpp
  Minigin/LivesDisplayComponent.h
  Minigin/LivesDisplayComponent.cpp
  Minigin/ScoreDisplayComponent.h
  Minigin/ScoreDisplayComponent.cpp
  Minigin/Singleton.h
  Minigin/DamageCommand.h
  Minigin/AddScoreCommand.h
  Minigin/WinnerAchievementComponent.h
  Minigin/WinnerAchievementComponent.cpp
  Minigin/BenchmarkPi.h
  Minigin/BenchmarkPi.cpp
  Minigin/SoundSystem.h
  Minigin/ServiceLocator.h
  Minigin/SDLSoundSystem.h
  Minigin/SDLSoundSystem.cpp
  Minigin/SoundIds.h
  Minigin/PlaySoundCommand.h 
   "Minigin/EngineTime.h")

target_compile_features(Minigin PUBLIC cxx_std_20)

include(FetchContent)

#rapidjson
FetchContent_Declare(
  rapidjson
  GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
  GIT_TAG master
)
FetchContent_MakeAvailable(rapidjson)

# GLM
find_package(glm CONFIG QUIET)
if(NOT glm_FOUND)
  FetchContent_Declare(
    glm
    URL https://github.com/g-truc/glm/releases/download/1.0.3/glm-1.0.3.zip
    DOWNLOAD_NO_PROGRESS ON
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/downloads
  )
  FetchContent_MakeAvailable(glm)
endif()

# SDL3
find_package(SDL3 3.4 CONFIG QUIET)
if(NOT SDL3_FOUND)
  if (WIN32)
    FetchContent_Declare(
      SDL3
      URL https://www.libsdl.org/release/SDL3-devel-3.4.0-VC.zip
      DOWNLOAD_NO_PROGRESS ON
      DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/downloads
    )
    FetchContent_MakeAvailable(SDL3)
    list(PREPEND CMAKE_PREFIX_PATH "${sdl3_SOURCE_DIR}")
    find_package(SDL3 CONFIG REQUIRED)
  else()
    FetchContent_Declare(
      SDL3
      GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
      GIT_TAG release-3.4.0
      GIT_SHALLOW TRUE
      GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(SDL3)
  endif()
endif()

# SDL3_ttf
find_package(SDL3_ttf 3.2.2 CONFIG QUIET)
if(NOT SDL3_ttf_FOUND)
  if (WIN32)
    FetchContent_Declare(
      SDL3_ttf
      URL https://www.libsdl.org/projects/SDL_ttf/release/SDL3_ttf-devel-3.2.2-VC.zip
      DOWNLOAD_NO_PROGRESS ON
      DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/downloads
    )
    FetchContent_MakeAvailable(SDL3_ttf)
    list(PREPEND CMAKE_PREFIX_PATH "${sdl3_ttf_SOURCE_DIR}")
    find_package(SDL3_ttf CONFIG REQUIRED)
  else()
    FetchContent_Declare(
      SDL3_ttf
      GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
      GIT_TAG release-3.2.2
      GIT_SHALLOW TRUE
      GIT_PROGRESS TRUE
    )
    set(SDL3TTF_INSTALL OFF)
    if(EMSCRIPTEN)
      list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/emscripten")
      set(SDL3TTF_USE_SYSTEM_FREETYPE ON)
      set(SDL3TTF_VENDORED OFF)
      set(SDLTTF_PLUTOSVG OFF)
      add_compile_options(-sUSE_FREETYPE=1 -sUSE_HARFBUZZ=1)
      add_link_options(-sUSE_FREETYPE=1 -sUSE_HARFBUZZ=1)
    endif()
    FetchContent_MakeAvailable(SDL3_ttf)
  endif()
endif()

# SDL3_mixer
find_package(SDL3_mixer CONFIG QUIET)
if(NOT SDL3_mixer_FOUND)
  if (WIN32)
    FetchContent_Declare(
      SDL3_mixer
      URL https://github.com/libsdl-org/SDL_mixer/releases/download/release-3.2.0/SDL3_mixer-devel-3.2.0-VC.zip
      DOWNLOAD_NO_PROGRESS ON
      DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/downloads
    )
    FetchContent_MakeAvailable(SDL3_mixer)
    list(PREPEND CMAKE_PREFIX_PATH "${sdl3_mixer_SOURCE_DIR}")
    find_package(SDL3_mixer CONFIG REQUIRED)
  else()
    FetchContent_Declare(
      SDL3_mixer
      GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
      GIT_TAG release-3.2.0
      GIT_SHALLOW TRUE
      GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(SDL3_mixer)
  endif()
endif()

# Dear ImGui
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG v1.92.5-docking
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
 )

target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
)

target_link_libraries(imgui PUBLIC SDL3::SDL3)
target_compile_features(imgui PUBLIC cxx_std_20)

# Minigin Library Configuration
target_include_directories(Minigin PUBLIC 
  ${CMAKE_SOURCE_DIR}/Minigin/
  ${glm_SOURCE_DIR}
  ${sdl3_SOURCE_DIR}/include
  ${sdl3_ttf_SOURCE_DIR}/include
  ${sdl3_mixer_SOURCE_DIR}/include
  ${imgui_SOURCE_DIR}
  ${VLD_INCLUDE_DIR}
  ${rapidjson_SOURCE_DIR}/include
)

target_link_libraries(Minigin PUBLIC 
  SDL3::SDL3 
  SDL3_ttf::SDL3_ttf
  SDL3_mixer::SDL3_mixer
  imgui
  glm::glm
  ${VLD_LIBRARIES}
)

if (WIN32)
  find_package(VLD CONFIG)
endif()

# Add steam if available
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/steam")
find_package(Steamworks QUIET)

if(MSVC)
  target_compile_options(Minigin PRIVATE /W4 /WX)
else()
  target_compile_options(Minigin PRIVATE -Wall -Wextra -Werror)
endif()

# BurgerTime Game Project
add_executable(BurgerTime
  "BurgerTime/Game.cpp"
  "BurgerTime/Enemy.h" 
  "BurgerTime/Enemy.cpp"
 "BurgerTime/PlayerState.h"  "BurgerTime/Player.h" "BurgerTime/Player.cpp" "BurgerTime/PlayerState.cpp"  "BurgerTime/AnimationComponent.h" "BurgerTime/AnimationComponent.cpp" "BurgerTime/UsePepperCommand.h" "BurgerTime/LevelData.h" "BurgerTime/LevelLoader.h" "BurgerTime/LevelLoader.cpp" "BurgerTime/LevelManager.h" "BurgerTime/LevelManager.cpp" "BurgerTime/BurgerPiece.h" "BurgerTime/BurgerPiece.cpp" "BurgerTime/GameConfig.h"  )

target_link_libraries(BurgerTime PRIVATE Minigin)
target_include_directories(BurgerTime PRIVATE
  ${CMAKE_SOURCE_DIR}/Minigin
  ${CMAKE_SOURCE_DIR}/BurgerTime
)

target_compile_features(BurgerTime PUBLIC cxx_std_20)

if(MSVC)
  target_compile_options(BurgerTime PRIVATE /W4)
endif()

# Platform-Specific Settings

if(EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")

  set_target_properties(BurgerTime PROPERTIES
    COMPILE_FLAGS "-sUSE_FREETYPE=1 -sUSE_HARFBUZZ=1 -g"
    LINK_FLAGS "-sUSE_FREETYPE=1 -sUSE_HARFBUZZ=1 -g -sALLOW_MEMORY_GROWTH --preload-file \"${CMAKE_SOURCE_DIR}/Data@/\""
  )

  add_custom_command(TARGET BurgerTime POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory 
    "${CMAKE_CURRENT_SOURCE_DIR}/web" 
    "$<TARGET_FILE_DIR:BurgerTime>/"
  )

elseif(WIN32)
  # Windows
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT BurgerTime)
  
  set_target_properties(BurgerTime PROPERTIES 
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:BurgerTime>"
  )

  # SDL DLLs
  set(SDL_RUNTIME_DLLS
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    SDL3_mixer::SDL3_mixer
  )

  foreach(dllTarget IN LISTS SDL_RUNTIME_DLLS)
    add_custom_command(
      TARGET BurgerTime POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "$<TARGET_FILE:${dllTarget}>"
          "$<TARGET_FILE_DIR:BurgerTime>"
      VERBATIM
    )
  endforeach()

  # Steamworks
  if(Steamworks_FOUND)
    target_link_libraries(BurgerTime PUBLIC Steamworks::Steamworks)
    target_compile_definitions(BurgerTime PRIVATE USE_STEAMWORKS)
    add_custom_command(TARGET BurgerTime POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy "${Steamworks_RUNTIME_DLL}" "$<TARGET_FILE_DIR:BurgerTime>"
    )
    add_custom_command(TARGET BurgerTime POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E echo "480" > "$<TARGET_FILE_DIR:BurgerTime>/steam_appid.txt"
      COMMENT "Generating steam_appid.txt"
    )
  endif()

  add_custom_command(TARGET BurgerTime POST_BUILD
     COMMAND "${CMAKE_COMMAND}" -E echo "Copying Data folder..."
    COMMAND "${CMAKE_COMMAND}" -E remove_directory "$<TARGET_FILE_DIR:BurgerTime>/Data"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory 
        "${CMAKE_CURRENT_SOURCE_DIR}/Data" 
        "$<TARGET_FILE_DIR:BurgerTime>/Data"
    COMMENT "Force updating Data folder (including JSON levels)"
    VERBATIM
  )

elseif(APPLE)
  set_target_properties(BurgerTime PROPERTIES
    MACOSX_BUNDLE TRUE
    INSTALL_RPATH "@executable_path/../Frameworks"
    BUILD_WITH_INSTALL_RPATH TRUE
  )

else()
  if(Steamworks_FOUND)
    target_link_libraries(BurgerTime PUBLIC Steamworks::Steamworks)
    target_compile_definitions(BurgerTime PRIVATE USE_STEAMWORKS)
  endif()

  add_custom_command(TARGET BurgerTime POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory 
    "${CMAKE_CURRENT_SOURCE_DIR}/Data" 
    "$<TARGET_FILE_DIR:BurgerTime>/Data"
  )
endif()