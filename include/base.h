/* base.h
*
*  MIT License
*
*  Copyright (c) 2021-2026 awawa-dev
*
*  https://github.com/awawa-dev/HyperSPI
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in all
*  copies or substantial portions of the Software.

*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*  SOFTWARE.
 */

#ifndef BASE_H
#define BASE_H

#if defined(ARDUINO_ARCH_ESP32)
	#include "freertos/semphr.h"
#endif

// Segment prerequisite checks
#if defined(SECOND_SEGMENT_START_INDEX)
	#if !defined(SECOND_SEGMENT_DATA_PIN)
		#error "Please define SECOND_SEGMENT_DATA_PIN for second segment"
	#elif !defined(SECOND_SEGMENT_CLOCK_PIN) && !defined(NEOPIXEL_RGBW) && !defined(NEOPIXEL_RGB)
		#error "Please define SECOND_SEGMENT_CLOCK_PIN and SECOND_SEGMENT_DATA_PIN for second segment"
	#endif
#endif

#if defined(THIRD_SEGMENT_START_INDEX)
	#if !defined(SECOND_SEGMENT_START_INDEX)
		#error "THIRD_SEGMENT_START_INDEX requires SECOND_SEGMENT_START_INDEX to be defined"
	#endif
	#if !defined(THIRD_SEGMENT_DATA_PIN)
		#error "Please define THIRD_SEGMENT_DATA_PIN for third segment"
	#elif !defined(THIRD_SEGMENT_CLOCK_PIN) && !defined(NEOPIXEL_RGBW) && !defined(NEOPIXEL_RGB)
		#error "Please define THIRD_SEGMENT_CLOCK_PIN and THIRD_SEGMENT_DATA_PIN for third segment"
	#endif
#endif

#if defined(FOURTH_SEGMENT_START_INDEX)
	#if !defined(THIRD_SEGMENT_START_INDEX)
		#error "FOURTH_SEGMENT_START_INDEX requires THIRD_SEGMENT_START_INDEX to be defined"
	#endif
	#if !defined(FOURTH_SEGMENT_DATA_PIN)
		#error "Please define FOURTH_SEGMENT_DATA_PIN for fourth segment"
	#elif !defined(FOURTH_SEGMENT_CLOCK_PIN) && !defined(NEOPIXEL_RGBW) && !defined(NEOPIXEL_RGB)
		#error "Please define FOURTH_SEGMENT_CLOCK_PIN and FOURTH_SEGMENT_DATA_PIN for fourth segment"
	#endif
#endif

class Base
{
	// LED strip number
	int ledsNumber = 0;
	// NeoPixelBusLibrary segment objects
	LED_DRIVER*  ledStrip1 = nullptr;
	LED_DRIVER2* ledStrip2 = nullptr;
	LED_DRIVER3* ledStrip3 = nullptr;
	LED_DRIVER4* ledStrip4 = nullptr;
	// frame is set and ready to render
	bool readyToRender = false;

	public:
		// static data buffer for the loop
		uint8_t buffer[MAX_BUFFER + 1] = {0};
		// handle to tasks
		TaskHandle_t processDataHandle = nullptr;
		TaskHandle_t processSerialHandle = nullptr;

		// semaphore to synchronize them
		xSemaphoreHandle i2sXSemaphore;

		// current queue position
		volatile int queueCurrent = 0;
		// queue end position
		volatile int queueEnd = 0;

		inline int getLedsNumber()
		{
			return ledsNumber;
		}

		inline LED_DRIVER* getLedStrip1()
		{
			return ledStrip1;
		}

		inline LED_DRIVER2* getLedStrip2()
		{
			return ledStrip2;
		}

		inline LED_DRIVER3* getLedStrip3()
		{
			return ledStrip3;
		}

		inline LED_DRIVER4* getLedStrip4()
		{
			return ledStrip4;
		}

		void initLedStrip(int count)
		{
			// Clean up all existing strip instances
			if (ledStrip1 != nullptr) { delete ledStrip1; ledStrip1 = nullptr; }
			if (ledStrip2 != nullptr) { delete ledStrip2; ledStrip2 = nullptr; }
			if (ledStrip3 != nullptr) { delete ledStrip3; ledStrip3 = nullptr; }
			if (ledStrip4 != nullptr) { delete ledStrip4; ledStrip4 = nullptr; }

			ledsNumber = count;

			// ----------------------------------------------------------------
			// Segment size calculation:
			//   seg1: [0,                    SECOND_SEGMENT_START_INDEX)
			//   seg2: [SECOND_SEGMENT_START_INDEX, THIRD_SEGMENT_START_INDEX)
			//   seg3: [THIRD_SEGMENT_START_INDEX,  FOURTH_SEGMENT_START_INDEX)
			//   seg4: [FOURTH_SEGMENT_START_INDEX, ledsNumber)
			//
			// Each lower-numbered segment define implies all higher-numbered
			// ones are absent if their own define is missing.
			// ----------------------------------------------------------------

			#if defined(FOURTH_SEGMENT_START_INDEX)
			{
				// Four-segment mode
				int seg1Size = SECOND_SEGMENT_START_INDEX;
				int seg2Size = THIRD_SEGMENT_START_INDEX  - SECOND_SEGMENT_START_INDEX;
				int seg3Size = FOURTH_SEGMENT_START_INDEX - THIRD_SEGMENT_START_INDEX;
				int seg4Size = ledsNumber                 - FOURTH_SEGMENT_START_INDEX;

				ledStrip1 = new LED_DRIVER(seg1Size, DATA_PIN);
				ledStrip1->Begin();

				ledStrip2 = new LED_DRIVER2(seg2Size, SECOND_SEGMENT_DATA_PIN);
				ledStrip2->Begin();

				ledStrip3 = new LED_DRIVER3(seg3Size, THIRD_SEGMENT_DATA_PIN);
				ledStrip3->Begin();

				ledStrip4 = new LED_DRIVER4(seg4Size, FOURTH_SEGMENT_DATA_PIN);
				ledStrip4->Begin();
			}
			#elif defined(THIRD_SEGMENT_START_INDEX)
			{
				// Three-segment mode
				int seg1Size = SECOND_SEGMENT_START_INDEX;
				int seg2Size = THIRD_SEGMENT_START_INDEX - SECOND_SEGMENT_START_INDEX;
				int seg3Size = ledsNumber                - THIRD_SEGMENT_START_INDEX;

				ledStrip1 = new LED_DRIVER(seg1Size, DATA_PIN);
				ledStrip1->Begin();

				ledStrip2 = new LED_DRIVER2(seg2Size, SECOND_SEGMENT_DATA_PIN);
				ledStrip2->Begin();

				ledStrip3 = new LED_DRIVER3(seg3Size, THIRD_SEGMENT_DATA_PIN);
				ledStrip3->Begin();
			}
			#elif defined(SECOND_SEGMENT_START_INDEX)
			{
				// Two-segment mode (original behaviour preserved exactly)
				if (ledsNumber > SECOND_SEGMENT_START_INDEX)
				{
					#if defined(NEOPIXEL_RGBW) || defined(NEOPIXEL_RGB)
						ledStrip1 = new LED_DRIVER(SECOND_SEGMENT_START_INDEX, DATA_PIN);
						ledStrip1->Begin();
						ledStrip2 = new LED_DRIVER2(ledsNumber - SECOND_SEGMENT_START_INDEX, SECOND_SEGMENT_DATA_PIN);
						ledStrip2->Begin();
					#else
						ledStrip1 = new LED_DRIVER(SECOND_SEGMENT_START_INDEX);
						ledStrip1->Begin(CLOCK_PIN, 12, DATA_PIN, 15);
						ledStrip2 = new LED_DRIVER2(ledsNumber - SECOND_SEGMENT_START_INDEX);
						ledStrip2->Begin(SECOND_SEGMENT_CLOCK_PIN, 12, SECOND_SEGMENT_DATA_PIN, 15);
					#endif
				}
			}
			#endif

			// Fall back to single-segment if seg1 was not yet created
			if (ledStrip1 == nullptr)
			{
				#if defined(ARDUINO_ARCH_ESP32)
					ledStrip1 = new LED_DRIVER(ledsNumber, DATA_PIN);
					ledStrip1->Begin();
				#else
					ledStrip1 = new LED_DRIVER(ledsNumber);
					ledStrip1->Begin();
				#endif
			}
		}

		/**
		 * @brief Check if there is already a prepared frame to display
		 */
		inline bool hasLateFrameToRender()
		{
			return readyToRender;
		}

		inline void dropLateFrame()
		{
			readyToRender = false;
		}

		inline void renderLeds(bool newFrame)
		{
			if (newFrame)
				readyToRender = true;

			// All active strips must be ready before we push
			bool strip1Ready = (ledStrip1 != nullptr && ledStrip1->CanShow());
			bool strip2Ready = !(ledStrip2 != nullptr && !ledStrip2->CanShow());
			bool strip3Ready = !(ledStrip3 != nullptr && !ledStrip3->CanShow());
			bool strip4Ready = !(ledStrip4 != nullptr && !ledStrip4->CanShow());

			if (readyToRender && strip1Ready && strip2Ready && strip3Ready && strip4Ready)
			{
				statistics.increaseShow();
				readyToRender = false;

				ledStrip1->Show(false);
				if (ledStrip2 != nullptr) ledStrip2->Show(false);
				if (ledStrip3 != nullptr) ledStrip3->Show(false);
				if (ledStrip4 != nullptr) ledStrip4->Show(false);
			}
		}

		inline bool setStripPixel(uint16_t pix, ColorDefinition &inputColor)
		{
			if (pix < ledsNumber)
			{
				#if defined(FOURTH_SEGMENT_START_INDEX)
				{
					if (pix < SECOND_SEGMENT_START_INDEX)
					{
						ledStrip1->SetPixelColor(pix, inputColor);
					}
					else if (pix < THIRD_SEGMENT_START_INDEX)
					{
						uint16_t localPix = pix - SECOND_SEGMENT_START_INDEX;
						#if defined(SECOND_SEGMENT_REVERSED)
							int seg2Size = THIRD_SEGMENT_START_INDEX - SECOND_SEGMENT_START_INDEX;
							ledStrip2->SetPixelColor(seg2Size - localPix - 1, inputColor);
						#else
							ledStrip2->SetPixelColor(localPix, inputColor);
						#endif
					}
					else if (pix < FOURTH_SEGMENT_START_INDEX)
					{
						uint16_t localPix = pix - THIRD_SEGMENT_START_INDEX;
						#if defined(THIRD_SEGMENT_REVERSED)
							int seg3Size = FOURTH_SEGMENT_START_INDEX - THIRD_SEGMENT_START_INDEX;
							ledStrip3->SetPixelColor(seg3Size - localPix - 1, inputColor);
						#else
							ledStrip3->SetPixelColor(localPix, inputColor);
						#endif
					}
					else
					{
						uint16_t localPix = pix - FOURTH_SEGMENT_START_INDEX;
						#if defined(FOURTH_SEGMENT_REVERSED)
							int seg4Size = ledsNumber - FOURTH_SEGMENT_START_INDEX;
							ledStrip4->SetPixelColor(seg4Size - localPix - 1, inputColor);
						#else
							ledStrip4->SetPixelColor(localPix, inputColor);
						#endif
					}
				}
				#elif defined(THIRD_SEGMENT_START_INDEX)
				{
					if (pix < SECOND_SEGMENT_START_INDEX)
					{
						ledStrip1->SetPixelColor(pix, inputColor);
					}
					else if (pix < THIRD_SEGMENT_START_INDEX)
					{
						uint16_t localPix = pix - SECOND_SEGMENT_START_INDEX;
						#if defined(SECOND_SEGMENT_REVERSED)
							int seg2Size = THIRD_SEGMENT_START_INDEX - SECOND_SEGMENT_START_INDEX;
							ledStrip2->SetPixelColor(seg2Size - localPix - 1, inputColor);
						#else
							ledStrip2->SetPixelColor(localPix, inputColor);
						#endif
					}
					else
					{
						uint16_t localPix = pix - THIRD_SEGMENT_START_INDEX;
						#if defined(THIRD_SEGMENT_REVERSED)
							int seg3Size = ledsNumber - THIRD_SEGMENT_START_INDEX;
							ledStrip3->SetPixelColor(seg3Size - localPix - 1, inputColor);
						#else
							ledStrip3->SetPixelColor(localPix, inputColor);
						#endif
					}
				}
				#elif defined(SECOND_SEGMENT_START_INDEX)
				{
					// Original two-segment behaviour preserved exactly
					if (pix < SECOND_SEGMENT_START_INDEX)
						ledStrip1->SetPixelColor(pix, inputColor);
					else
					{
						#if defined(SECOND_SEGMENT_REVERSED)
							ledStrip2->SetPixelColor(ledsNumber - pix - 1, inputColor);
						#else
							ledStrip2->SetPixelColor(pix - SECOND_SEGMENT_START_INDEX, inputColor);
						#endif
					}
				}
				#else
				{
					ledStrip1->SetPixelColor(pix, inputColor);
				}
				#endif
			}

			return (pix + 1 < ledsNumber);
		}
} base;

#endif
