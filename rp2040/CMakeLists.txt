set(CMAKE_SYSTEM_NAME Generic)
# User configuration section starts here

# Some boards, such as the first Adafruit revisions, may have trouble booting properly
# due to bad componets used in the design.
# Turn this setting to ON if your rp2040 is not detected after firmware upload and reset
set(BOOT_WORKAROUND OFF)

# Default output data pin for the non-SPI LED strips (only for sk6812/ws2812b)
set(OUTPUT_DATA_PIN 14)

# -----------------------------------------------------------------------
# Multi-segment support (up to 4 segments)
#
# To enable additional segments, set their START_INDEX to the cumulative
# LED count at which that segment begins, and set their DATA_PIN.
# Set REVERSED to ON if that segment runs in the opposite direction.
#
# Example — 4 segments of 150 LEDs each (600 LEDs total):
#   SECOND_SEGMENT_INDEX  = 150   DATA_PIN = 15   REVERSED = OFF
#   THIRD_SEGMENT_INDEX   = 300   DATA_PIN = 16   REVERSED = ON
#   FOURTH_SEGMENT_INDEX  = 450   DATA_PIN = 17   REVERSED = OFF
#
# In HyperHDR configure a single continuous strip of 600 LEDs.
# Segments 2-4 must each have a HIGHER start index than the previous one.
# -----------------------------------------------------------------------

# Second segment
set(SECOND_SEGMENT_INDEX 220)
set(SECOND_SEGMENT_DATA_PIN 15)
set(SECOND_SEGMENT_REVERSED OFF)

# Third segment (requires SECOND_SEGMENT_INDEX to also be set)
set(THIRD_SEGMENT_INDEX 344)
set(THIRD_SEGMENT_DATA_PIN 16)
set(THIRD_SEGMENT_REVERSED OFF)

# Fourth segment (requires SECOND_SEGMENT_INDEX and THIRD_SEGMENT_INDEX to also be set)
set(FOURTH_SEGMENT_INDEX 564)
set(FOURTH_SEGMENT_DATA_PIN 17)
set(FOURTH_SEGMENT_REVERSED OFF)

# HyperSPI communication interface with Raspberry Pi
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

# Validate segment ordering
if (THIRD_SEGMENT_INDEX AND NOT SECOND_SEGMENT_INDEX)
	message(FATAL_ERROR "THIRD_SEGMENT_INDEX requires SECOND_SEGMENT_INDEX to be set")
endif()
if (FOURTH_SEGMENT_INDEX AND NOT THIRD_SEGMENT_INDEX)
	message(FATAL_ERROR "FOURTH_SEGMENT_INDEX requires THIRD_SEGMENT_INDEX to be set")
endif()

cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/HyperSerialPico/sdk/pico)
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/HyperSerialPico/sdk/freertos)
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(HyperSPI_Pico C CXX ASM)
pico_sdk_init()

set(PICO_PROGRAM_MAIN_ENTRY "../hyperspi.cpp")
set(DISABLE_SPI_LEDS ON)

# Always pass SPI interface defines
set(SEGMENT_DEFINES
	-DSPI_INTERFACE=${OUTPUT_SPI_INTERFACE}
	-DSPI_DATA_PIN=${OUTPUT_SPI_DATA_PIN}
	-DSPI_CLOCK_PIN=${OUTPUT_SPI_CLOCK_PIN}
	-DSPI_CHIP_SELECT=${OUTPUT_SPI_CHIP_SELECT}
)

# Second segment
if (SECOND_SEGMENT_INDEX)
	message(STATUS "${GreenColor}Second segment enabled: start=${SECOND_SEGMENT_INDEX} pin=${SECOND_SEGMENT_DATA_PIN} reversed=${SECOND_SEGMENT_REVERSED}${ColorReset}")
	list(APPEND SEGMENT_DEFINES
		-DSECOND_SEGMENT_START_INDEX=${SECOND_SEGMENT_INDEX}
		-DSECOND_SEGMENT_DATA_PIN=${SECOND_SEGMENT_DATA_PIN}
	)
	if (SECOND_SEGMENT_REVERSED)
		list(APPEND SEGMENT_DEFINES -DSECOND_SEGMENT_REVERSED)
	endif()
endif()

# Third segment
if (THIRD_SEGMENT_INDEX)
	message(STATUS "${GreenColor}Third segment enabled: start=${THIRD_SEGMENT_INDEX} pin=${THIRD_SEGMENT_DATA_PIN} reversed=${THIRD_SEGMENT_REVERSED}${ColorReset}")
	list(APPEND SEGMENT_DEFINES
		-DTHIRD_SEGMENT_START_INDEX=${THIRD_SEGMENT_INDEX}
		-DTHIRD_SEGMENT_DATA_PIN=${THIRD_SEGMENT_DATA_PIN}
	)
	if (THIRD_SEGMENT_REVERSED)
		list(APPEND SEGMENT_DEFINES -DTHIRD_SEGMENT_REVERSED)
	endif()
endif()

# Fourth segment
if (FOURTH_SEGMENT_INDEX)
	message(STATUS "${GreenColor}Fourth segment enabled: start=${FOURTH_SEGMENT_INDEX} pin=${FOURTH_SEGMENT_DATA_PIN} reversed=${FOURTH_SEGMENT_REVERSED}${ColorReset}")
	list(APPEND SEGMENT_DEFINES
		-DFOURTH_SEGMENT_START_INDEX=${FOURTH_SEGMENT_INDEX}
		-DFOURTH_SEGMENT_DATA_PIN=${FOURTH_SEGMENT_DATA_PIN}
	)
	if (FOURTH_SEGMENT_REVERSED)
		list(APPEND SEGMENT_DEFINES -DFOURTH_SEGMENT_REVERSED)
	endif()
endif()

add_definitions(${SEGMENT_DEFINES})
add_subdirectory(HyperSerialPico)
