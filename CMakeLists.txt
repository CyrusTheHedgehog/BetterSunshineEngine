cmake_minimum_required(VERSION 3.8)

project(BetterSunshineEngine VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)

option(SMS_FIX_CRASHES "Repair supported crashes in Super Mario Sunshine" ON)
option(SMS_FIX_BUGS "Repair supported bugs in Super Mario Sunshine" ON)
option(SMS_FIX_DOWNWARP "Changes OOB behavior so downwarping ceases" ON)
option(SMS_FIX_YOSHI_NOZZLES "Enhances nozzle behavior while riding Yoshi" ON)
option(SMS_INCLUDE_WIDESCREEN "Includes extended render distance" ON)
option(SMS_INCLUDE_MUSIC "Includes streamed music features" ON)
option(SMS_INCLUDE_EXTENDED_SHINES "Includes support for 999 shines" ON)
option(SMS_INCLUDE_EXTENDED_OBJECTS "Includes support for custom objects" ON)
option(SMS_INCLUDE_EXTENDED_COLLISION "Includes extended collision types" ON)
option(SMS_INCLUDE_EXTENDED_VISIBILITY "Includes extended render distance" ON)
option(SMS_INCLUDE_LZ_BUTTON_SWAP "Functionally swaps L and Z buttons for movesets. (Ground Pound remains mapped to L)" ON)
option(SMS_INCLUDE_EXTENDED_MOVESET "Includes extended moves such as 
Burst, Hover Slide, and Long Jumping" ON)
option(SMS_INCLUDE_GREEN_YOSHI "Includes Green Yoshi and the free egg system" ON)
option(SMS_INCLUDE_UNDERWATER_FRUIT "Includes non-perishing underwater fruit" ON)
option(SMS_INCLUDE_SLOT_B_SUPPORT "Includes Slot B memcard support" ON)
option(SMS_INCLUDE_BMG_EXTENSION "Includes extended BMG opcodes" ON)
option(SMS_INCLUDE_SHADOW_MARIO_HEALTH "Includes the visibility of Shadow Mario's health" ON)
option(SMS_INCLUDE_DYNAMIC_FALL_DAMAGE "Includes dynamic fall damage in SMS" ON)
option(SMS_INCLUDE_EXCEPTION_HANDLER "Includes the exception handler information" ON)

list(APPEND BETTER_SMS_CONFIG_DEFINES "KURIBO_NO_TYPES" "BETTER_SMS_VERSION=\"v1.0\"")

if(SMS_FIX_CRASHES)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_CRASHFIXES=1")
endif()

if(SMS_FIX_BUGS)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_BUGFIXES=1")
endif()

if(SMS_FIX_DOWNWARP)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_NO_DOWNWARP=1")
endif()

if(SMS_FIX_YOSHI_NOZZLES)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_YOSHI_SAVE_NOZZLES=1")
endif()

if(SMS_INCLUDE_WIDESCREEN)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_WIDESCREEN=1")
endif()

if(SMS_INCLUDE_MUSIC)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_CUSTOM_MUSIC=1")
endif()

if(SMS_INCLUDE_EXTENDED_SHINES)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_EXTRA_SHINES=1")
endif()

if(SMS_INCLUDE_EXTENDED_OBJECTS)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_EXTRA_OBJECTS=1")
endif()

if(SMS_INCLUDE_EXTENDED_COLLISION)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_EXTRA_COLLISION=1")
endif()

if(SMS_INCLUDE_BMG_EXTENSION)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_EXTRA_BMG_COMMANDS=1")
endif()

if(SMS_INCLUDE_EXTENDED_VISIBILITY)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_EXTENDED_RENDER_DISTANCE=1")
endif()

if(SMS_INCLUDE_LZ_BUTTON_SWAP)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_SWAP_LZ_BUTTONS=1")
endif()

if(SMS_INCLUDE_EXTENDED_MOVESET)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_BACK_FLIP=1" "BETTER_SMS_LONG_JUMP=1" "BETTER_SMS_MULTI_JUMP=1" "BETTER_SMS_HOVER_BURST=1" "BETTER_SMS_HOVER_SLIDE=1" "BETTER_SMS_ROCKET_DIVE=1")
endif()

if(SMS_INCLUDE_GREEN_YOSHI)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_GREEN_YOSHI=1")
endif()

if(SMS_INCLUDE_UNDERWATER_FRUIT)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_UNDERWATER_FRUIT=1")
endif()

if(SMS_INCLUDE_SLOT_B_SUPPORT)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_SLOT_B_SUPPORT=1")
endif()

if(SMS_INCLUDE_SHADOW_MARIO_HEALTH)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_SHADOW_MARIO_HEALTHBAR=1")
endif()

if(SMS_INCLUDE_DYNAMIC_FALL_DAMAGE)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_DYNAMIC_FALL_DAMAGE=1")
endif()

if(SMS_INCLUDE_EXCEPTION_HANDLER)
    list(APPEND BETTER_SMS_CONFIG_DEFINES "BETTER_SMS_EXCEPTION_HANDLER=1")
endif()

set(SMS_KURIBO_CONVERTER_PATH ${PROJECT_SOURCE_DIR}/tools/KuriboConverter.exe)
set(SMS_LINKER_MAP_PATH ${PROJECT_SOURCE_DIR}/maps/${SMS_REGION}.map)

add_subdirectory(lib/sms_interface)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    file(GLOB BETTERSMS_SRC
        "src/*.c"
        "src/*.cpp"
        "src/asm/*.s"
        "src/camera/*.c"
        "src/camera/*.cpp"
        "src/collision/*.c"
        "src/collision/*.cpp"
        "src/cstd/*.c"
        "src/cstd/*.cpp"
        "src/debug/*.c"
        "src/debug/*.cpp"
        "src/libs/*.cpp"
        "src/libs/dolphin/*.c"
        "src/moveset/*.c"
        "src/moveset/*.cpp"
        "src/objects/*.c"
        "src/objects/*.cpp"
        "src/patches/*.c"
        "src/patches/*.cpp"
        "src/physics/*.c"
        "src/physics/*.cpp"
        "src/shine/*.c"
        "src/shine/*.cpp"
        "src/sunscript/*.c"
        "src/sunscript/*.cpp"
        "src/yoshi/*.c"
        "src/yoshi/*.cpp"
        "include/BetterSMS/*.h"
        "include/BetterSMS/*.hxx"
        "include/BetterSMS/collision/*.h"
        "include/BetterSMS/collision/*.hxx"
        "include/BetterSMS/cstd/*.h"
        "include/BetterSMS/cstd/*.hxx"
        "include/BetterSMS/libs/*.h"
        "include/BetterSMS/libs/*.hxx"
        "include/BetterSMS/objects/*.h"
        "include/BetterSMS/objects/*.hxx"
    )

    add_executable(BetterSunshineEngine ${BETTERSMS_SRC})
    target_link_libraries(BetterSunshineEngine PUBLIC
                          Dolphin
                          JSystem
                          SMS
                          Kamek
                          Kuribo
                          ${PROJECT_SOURCE_DIR}/lib/libKuriboClang/libKuriboClang.a
    )
    target_include_directories(BetterSunshineEngine PUBLIC "include/BetterSMS" "lib/sms_interface/include")
    target_include_directories(BetterSunshineEngine PRIVATE "src")
    target_compile_options(BetterSunshineEngine PUBLIC ${SMS_COMPILE_FLAGS})
    target_link_options(BetterSunshineEngine PUBLIC ${SMS_LINK_FLAGS})
    target_compile_definitions(BetterSunshineEngine PUBLIC ${BETTER_SMS_CONFIG_DEFINES})
else()
    add_library(BetterSunshineEngine INTERFACE)
    target_link_libraries(BetterSunshineEngine INTERFACE
                          Dolphin
                          JSystem
                          SMS
                          Kamek
                          Kuribo
                          ${PROJECT_SOURCE_DIR}/lib/libKuriboClang/libKuriboClang.a
    )
    target_include_directories(BetterSunshineEngine INTERFACE "include" "lib/sms_interface/include")
    target_compile_options(BetterSunshineEngine INTERFACE ${SMS_COMPILE_FLAGS})
    target_link_options(BetterSunshineEngine INTERFACE ${SMS_LINK_FLAGS})
    target_compile_definitions(BetterSunshineEngine INTERFACE ${BETTER_SMS_CONFIG_DEFINES})
endif()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_custom_target(${PROJECT_NAME}.kxe ALL DEPENDS ${PROJECT_NAME})
    add_custom_command(
        TARGET ${PROJECT_NAME}.kxe
        COMMAND ${PROJECT_SOURCE_DIR}/tools/KuriboConverter.exe ARGS ${PROJECT_NAME} ${PROJECT_NAME}.kxe ${PROJECT_SOURCE_DIR}/maps/${SMS_REGION}.map
    )
endif()

unset(SMS_FIX_CRASHES CACHE)
unset(SMS_FIX_BUGS CACHE)
unset(SMS_FIX_DOWNWARP CACHE)
unset(SMS_FIX_YOSHI_NOZZLES CACHE)
unset(SMS_INCLUDE_WIDESCREEN CACHE)
unset(SMS_INCLUDE_MUSIC CACHE)
unset(SMS_INCLUDE_EXTENDED_SHINES CACHE)
unset(SMS_INCLUDE_EXTENDED_OBJECTS CACHE)
unset(SMS_INCLUDE_EXTENDED_COLLISION CACHE)
unset(SMS_INCLUDE_EXTENDED_VISIBILITY CACHE)
unset(SMS_INCLUDE_LZ_BUTTON_SWAP CACHE)
unset(SMS_INCLUDE_EXTENDED_MOVESET CACHE)
unset(SMS_INCLUDE_GREEN_YOSHI CACHE)
unset(SMS_INCLUDE_UNDERWATER_FRUIT CACHE)
unset(SMS_INCLUDE_SLOT_B_SUPPORT CACHE)
unset(SMS_INCLUDE_BMG_EXTENSION CACHE)
unset(SMS_INCLUDE_SHADOW_MARIO_HEALTH CACHE)
unset(SMS_INCLUDE_DYNAMIC_FALL_DAMAGE CACHE)
unset(SMS_INCLUDE_EXCEPTION_HANDLER CACHE)