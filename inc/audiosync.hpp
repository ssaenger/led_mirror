/* ********************************************************************************************
 * audiosync.hpp
 *
 * Author: Shawn Saenger
 *
 * Created: Sep 20, 2023
 *
 * Description: Header file for the PJRC Audio library and FFT code
 *
 * ********************************************************************************************
 */

#ifndef _AUDIOSYNC_HPP_
#define _AUDIOSYNC_HPP_

#include "overide.h"

// All these libraries are required for the Teensy Audio Library
#include <Audio.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <Wire.h>

#include "../inc/animations.hpp"

/* --------------------------------------------------------------------------------------------
 *  CONSTANTS
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * AS_FFT_BIN_COUNT define
 *
 * The PJRC AudioAnalyzeFFT1024 is a 1024 point FFT at a sampled at a 44.1 kHz sampling rate so
 * the bin count is half at 512. Each bin is 43 Hz apart (43.0664 Hz).
 */
#define AS_FFT_BIN_COUNT 512

/* --------------------------------------------------------------------------------------------
 *  DEFINITIONS
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * AS_MAX_LEVEL define
 *
 * 1.0 = max, lower is more "sensitive"
 *
 * Default is 0.30
 */
#ifndef AS_MAX_LEVEL
#define AS_MAX_LEVEL 0.30f
#endif /* AS_MAX_LEVEL */

/* --------------------------------------------------------------------------------------------
 * AS_DYNAMIC_RANGE define
 *
 * Smaller number = harder to overcome init thresh
 *
 * Default is 40.0
 */
#ifndef AS_DYNAMIC_RANGE
#define AS_DYNAMIC_RANGE 40.0f
#endif /* AS_DYNAMIC_RANGE */

/* --------------------------------------------------------------------------------------------
 * AS_LINEAR_BLEND define
 *
 * Useful range is 0 to 0.7
 *
 * Default is 0.5
 */
#ifndef AS_LINEAR_BLEND
#define AS_LINEAR_BLEND 0.5f
#endif /* AS_LINEAR_BLEND */

/* --------------------------------------------------------------------------------------------
 *  PUBLIC FUNCTIONS
 * --------------------------------------------------------------------------------------------
 */

void AS_Init();
void AS_CalcFftBins(uint16_t Bands);
void AS_CalcLevelThresh(float MaxLevel, float DynamicRange, float LinearBlend);

void AS_PlotFftTop(AniParms *Ap, AniType At);
void AS_PlotFftBottom(AniParms *Ap, AniType At);

#endif /* _AUDIOSYNC_HPP_ */