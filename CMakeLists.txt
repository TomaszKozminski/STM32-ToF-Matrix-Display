cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME LED_controller)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here

    ./modules/LED_matrix_driver/Src/Font.c
    ./modules/LED_matrix_driver/Src/Canvas.c
    ./modules/LED_matrix_driver/Src/LedMatrix.c
    ./modules/LED_matrix_driver/Src/Frame.c
    ./modules/LED_matrix_driver/Src/lib_Backgrounds.c
    ./modules/LED_matrix_driver/Src/lib_MovingItems.c
    ./modules/LED_matrix_driver/Src/ImageBackground.c
    ./modules/LED_matrix_driver/Src/ImageMovingItem.c

    ./modules/VL53L7CX/Src/ObjectTracker.c
    ./modules/VL53L7CX/Src/platform.c
    ./modules/VL53L7CX/Src/VL53L7CX_Adapter.c
    ./modules/VL53L7CX/Src/vl53l7cx_api.c

    # SD Card driver
    ./modules/SD_driver/Src/sd_spi.c
    # Animation reader
    ./modules/SD_driver/Src/sd_animation_reader.c
    # Animation 2D mapper
    ./modules/SD_driver/Src/animation_mapper.c


)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ./modules/LED_matrix_driver/Inc
    ./modules/VL53L7CX/Inc
    ./modules/SD_driver/Inc
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
    
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
