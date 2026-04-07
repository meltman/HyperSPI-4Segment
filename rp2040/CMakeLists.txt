set(CMAKE_SYSTEM_NAME Generic)
# User configuration section starts here

set(BOOT_WORKAROUND ON)

# Segment 0: 220 LEDs on GPIO 16
set(OUTPUT_DATA_PIN 2)

# Segment 1: 124 LEDs — starts at index 220
set(SECOND_SEGMENT_INDEX 220)
set(SECOND_SEGMENT_REVERSED ON)

# Segment 2: 220 LEDs — starts at index 344
set(THIRD_SEGMENT_INDEX 344)
set(THIRD_SEGMENT_REVERSED ON)

# Segment 3: 124 LEDs — starts at index 564
set(FOURTH_SEGMENT_INDEX 564)
set(FOURTH_SEGMENT_REVERSED ON)

# Per-segment data pins (used in data_pin defines passed to compiler)
set(SECOND_SEGMENT_DATA_PIN 17)
set(THIRD_SEGMENT_DATA_PIN  18)
set(FOURTH_SEGMENT_DATA_PIN 19)

# SPI interface for Scorpio (overridden by GitHub Action for Scorpio build)
set(OUTPUT_SPI_DATA_PIN 4)
set(OUTPUT_SPI_CLOCK_PIN 2)
set(OUTPUT_SPI_CHIP_SELECT 5)
set(OUTPUT_SPI_INTERFACE spi0)

# User configuration section ends here
# Usually you don't need to change anything below this section

if(NOT CMAKE_HOST_WIN32)
	string(ASCII 27 EscChar)
	set(ColorReset  "${EscChar}[m")
	set(GreenColor  "${EscChar}[32m")
	set(YellowColor "${EscChar}[33m")
endif()

if (OVERRIDE_DATA_PIN)
	set(OUTPUT_DATA_PIN ${OVERRIDE_DATA_PIN})
	message( STATUS "${YellowColor}Overriding output Neopixel Data GPIO: ${OUTPUT_DATA_PIN}${ColorReset}")
endif()

if (OVERRIDE_SPI_DATA_PIN)
	set(OUTPUT_SPI_DATA_PIN ${OVERRIDE_SPI_DATA_PIN})
	message( STATUS "${YellowColor}Overriding input SPI Data GPIO: ${OUTPUT_SPI_DATA_PIN}${ColorReset}")
endif()

if (OVERRIDE_SPI_CLOCK_PIN)
	set(OUTPUT_SPI_CLOCK_PIN ${OVERRIDE_SPI_CLOCK_PIN})
	message( STATUS "${YellowColor}Overriding input SPI Clock GPIO: ${OUTPUT_SPI_CLOCK_PIN}${ColorReset}")
endif()

if (OVERRIDE_SPI_CHIP_SELECT)
	set(OUTPUT_SPI_CHIP_SELECT ${OVERRIDE_SPI_CHIP_SELECT})
	message( STATUS "${YellowColor}Overriding input SPI chip select GPIO: ${OUTPUT_SPI_CHIP_SELECT}${ColorReset}")
endif()

if (OVERRIDE_SPI_INTERFACE)
	set(OUTPUT_SPI_INTERFACE ${OVERRIDE_SPI_INTERFACE})
	message( STATUS "${YellowColor}Overriding input SPI Interface: ${OUTPUT_SPI_INTERFACE}${ColorReset}")
endif()

message( STATUS "---------------------------")
message( STATUS "Neopixel Data GPIO: ${GreenColor}${OUTPUT_DATA_PIN}${ColorReset}")
message( STATUS "SPI Data GPIO: ${GreenColor}${OUTPUT_SPI_DATA_PIN}${ColorReset}")
message( STATUS "SPI Clock GPIO: ${GreenColor}${OUTPUT_SPI_CLOCK_PIN}${ColorReset}")
message( STATUS "SPI Select GPIO: ${GreenColor}${OUTPUT_SPI_CHIP_SELECT}${ColorReset}")
message( STATUS "SPI Interface: ${GreenColor}${OUTPUT_SPI_INTERFACE}${ColorReset}")
message( STATUS "Boot workaround: ${GreenColor}${BOOT_WORKAROUND}${ColorReset}")
if (SECOND_SEGMENT_INDEX)
	message( STATUS "2nd segment start: ${GreenColor}${SECOND_SEGMENT_INDEX}${ColorReset}")
endif()
if (THIRD_SEGMENT_INDEX)
	message( STATUS "3rd segment start: ${GreenColor}${THIRD_SEGMENT_INDEX}${ColorReset}")
endif()
if (FOURTH_SEGMENT_INDEX)
	message( STATUS "4th segment start: ${GreenColor}${FOURTH_SEGMENT_INDEX}${ColorReset}")
endif()
message( STATUS "---------------------------")

cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/HyperSerialPico/sdk/pico)
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/HyperSerialPico/sdk/freertos)
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(HyperSPI_Pico C CXX ASM)
pico_sdk_init()

set(PICO_PROGRAM_MAIN_ENTRY "../hyperspi.cpp")
set(DISABLE_SPI_LEDS ON)

# Pass SPI and all segment defines to the compiler.
# SECOND/THIRD/FOURTH_SEGMENT_START_INDEX are also passed via
# target_compile_definitions in HyperSerialPico/CMakeLists.txt,
# but add_definitions ensures hyperspi.cpp also sees them.
add_definitions(
	-DSPI_INTERFACE=${OUTPUT_SPI_INTERFACE}
	-DSPI_DATA_PIN=${OUTPUT_SPI_DATA_PIN}
	-DSPI_CLOCK_PIN=${OUTPUT_SPI_CLOCK_PIN}
	-DSPI_CHIP_SELECT=${OUTPUT_SPI_CHIP_SELECT}
)

if (SECOND_SEGMENT_INDEX)
	add_definitions(
		-DSECOND_SEGMENT_START_INDEX=${SECOND_SEGMENT_INDEX}
		-DSECOND_SEGMENT_DATA_PIN=${SECOND_SEGMENT_DATA_PIN}
	)
	if (SECOND_SEGMENT_REVERSED)
		add_definitions(-DSECOND_SEGMENT_REVERSED)
	endif()
endif()

if (THIRD_SEGMENT_INDEX)
	add_definitions(
		-DTHIRD_SEGMENT_START_INDEX=${THIRD_SEGMENT_INDEX}
		-DTHIRD_SEGMENT_DATA_PIN=${THIRD_SEGMENT_DATA_PIN}
	)
	if (THIRD_SEGMENT_REVERSED)
		add_definitions(-DTHIRD_SEGMENT_REVERSED)
	endif()
endif()

if (FOURTH_SEGMENT_INDEX)
	add_definitions(
		-DFOURTH_SEGMENT_START_INDEX=${FOURTH_SEGMENT_INDEX}
		-DFOURTH_SEGMENT_DATA_PIN=${FOURTH_SEGMENT_DATA_PIN}
	)
	if (FOURTH_SEGMENT_REVERSED)
		add_definitions(-DFOURTH_SEGMENT_REVERSED)
	endif()
endif()

add_subdirectory(HyperSerialPico)
