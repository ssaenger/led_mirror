/* ********************************************************************************************
 * gifDecoder.hpp
 *
 * Author: Shawn Saenger
 *
 * Created: Oct 1, 2023
 *
 * Description: Header file the gif decoder. Relies on smartMatrix and AnimatedGIF libraries.
 *              - https://github.com/pixelmatix/SmartMatrix
 *              - https://github.com/bitbank2/AnimatedGIF        
 *
 * ********************************************************************************************
 */

#include "overide.h"
#include "../inc/smartmtxconfig.h"
#include "../inc/animations.hpp"
#include <SD.h>

/* --------------------------------------------------------------------------------------------
 *  CONSTANTS
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * GIFDEC_SD_CS define
 *
 * Teensy onboard mircorSD CS pin
 */
#define GIFDEC_SD_CS BUILTIN_SDCARD

/* --------------------------------------------------------------------------------------------
 *  DEFINITIONS
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * GIFDEC_DIRECTORY define
 *
 * Gifs directory in the SD card. Teensy SD library requires a trailing slash in the directory
 * name.
 *
 * Default is "/gifs/"
 */
#ifndef GIFDEC_DIRECTORY
#define GIFDEC_DIRECTORY "/gifs/"
#endif /* GIFDEC_DIRECTORY */

/* --------------------------------------------------------------------------------------------
 *  PUBLIC FUNCTIONS
 * --------------------------------------------------------------------------------------------
 */


bool GIFDEC_Init();

void GIFDEC_Play(AniParms *Ap, AniType At);