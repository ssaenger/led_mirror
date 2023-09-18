/* ********************************************************************************************
 * smartmtxconfig.h
 *
 * Author: Shawn Saenger
 *
 * Created: Sep 5, 2023
 *
 * Description: Configuration options for the smart matrix library
 *
 * ********************************************************************************************
 */

#ifndef _SMARTMTXCONFIG_H_
#define _SMARTMTXCONFIG_H_

#include "../inc/overide.h"


#define USE_ADAFRUIT_GFX_LAYERS
#define INCLUDE_FASTLED_BACKGROUND

#include <MatrixHardware_Teensy4_ShieldV5.h> // SmartLED Shield for Teensy 4 (V5)
#include <SmartMatrix.h>
#include <FastLED.h>

#include <Fonts/FreeMono12pt7b.h>
//#include <Fonts/FreeMonoBold18pt7b.h>
//#include <Fonts/FreeMonoBoldOblique24pt7b.h>
//#include <Fonts/FreeSansOblique24pt7b.h>
//#include <Fonts/FreeSerif24pt7b.h>
//#include <Fonts/FreeSerifItalic9pt7b.h>
//#include <Fonts/Org_01.h>
//#include <Fonts/Picopixel.h>
//#include <Fonts/Tiny3x3a2pt7b.h>
//#include <Fonts/TomThumb.h>

/* --------------------------------------------------------------------------------------------
 *  DEFINITIONS
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * SM_COLOR_DEPTH define
 *
 * Color depth used for storing pixels in the layers: 24 or 48 (24 is good for
 * most sketches - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24)
 */
#ifndef SM_COLOR_DEPTH
#define SM_COLOR_DEPTH          24
#endif /* SM_COLOR_DEPTH */

/* --------------------------------------------------------------------------------------------
 * SM_HEIGHT define
 *
 * Height of the HUB75 display, multiple of 8
 */
#ifndef SM_HEIGHT
#define SM_HEIGHT               32
#endif /* SM_HEIGHT */

/* --------------------------------------------------------------------------------------------
 * SM_WIDTH define
 *
 * Width of the HUB75 display, multiple of 8
 */
#ifndef SM_WIDTH
#define SM_WIDTH                64
#endif /* SM_WIDTH */

/* --------------------------------------------------------------------------------------------
 * SM_REFRESH_DEPTH define
 *
 * Tradeoff of color quality vs refresh rate, max brightness, and RAM usage. 
 * 36 is typically good, drop down to 24 if you need to.  On Teensy, multiples of 3, up
 * to 48: 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48.  On ESP32: 24, 36, 48
 */
#ifndef SM_REFRESH_DEPTH
#define SM_REFRESH_DEPTH 36
#endif /* SM_REFRESH_DEPTH */

/* --------------------------------------------------------------------------------------------
 * SM_DMA_BUFF_ROWS define
 *
 * Known working: 2-4, use 2 to save RAM, more to keep from dropping frames and automatically
 * lowering refresh rate.  (This isn't used on ESP32, leave as default)
 */
#ifndef SM_DMA_BUFF_ROWS
#define SM_DMA_BUFF_ROWS 4
#endif /* SM_DMA_BUFF_ROWS */

/* --------------------------------------------------------------------------------------------
 * SM_INITIAL_BRIGHTNESS define
 *
 * Initial smart matrix brightness
 */
#ifndef SM_INITIAL_BRIGHTNESS
#define SM_INITIAL_BRIGHTNESS 100
#endif /* SM_DMA_BUFF_ROWS */

/* --------------------------------------------------------------------------------------------
 *  CONST
 * --------------------------------------------------------------------------------------------
 */
#define SM_NUM_LEDS                    (SM_WIDTH * SM_HEIGHT)

const uint8_t kPanelType              = SM_PANELTYPE_HUB75_64ROW_MOD32SCAN; // Choose the configuration that matches your panels.  See more details in MatrixCommonHub75.h and the docs: https://github.com/pixelmatix/SmartMatrix/wiki
const uint32_t kMatrixOptions         = (SM_HUB75_OPTIONS_NONE);            // see docs for options: https://github.com/pixelmatix/SmartMatrix/wiki
const uint8_t kScrollingLayerOptions  = (SM_SCROLLING_OPTIONS_NONE);
#if defined(INCLUDE_FASTLED_BACKGROUND) && defined(USE_ADAFRUIT_GFX_LAYERS)
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_GFX_OPTIONS_NONE);
#else
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
#endif

#endif /* _SMARTMTXCONFIG_H_ */