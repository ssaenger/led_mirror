/* ********************************************************************************************
 * ledinfo.h
 *
 * Author: Shawn Saenger
 *
 * Created: July 13, 2024
 *
 * Description: Information of the LED panel or string
 *
 * ********************************************************************************************
 */

#ifndef _LEDINFO_H_
#define _LEDINFO_H_

#include "../inc/overide.h"

/* --------------------------------------------------------------------------------------------
 * LEDI_HEIGHT define
 *
 * Number of LEDs in the y-axis
 */
#ifndef LEDI_HEIGHT
#define LEDI_HEIGHT               32
#endif /* LEDI_HEIGHT */

#if LEDI_HEIGHT == 0
#error "LEDI_HEIGHT can't be 0"
#endif

/* --------------------------------------------------------------------------------------------
 * LEDI_WIDTH define
 *
 * Number of LEDs in the x-axis
 */
#ifndef LEDI_WIDTH
#define LEDI_WIDTH                64
#endif /* LEDI_WIDTH */

#if LEDI_WIDTH == 0
#error "LEDI_WIDTH can't be 0"
#endif

/* --------------------------------------------------------------------------------------------
 * LEDI_GRID_LAYOUT define
 *
 * Number of LEDs in the x-axis
 */
#ifndef LEDI_GRID_LAYOUT
#if (LEDI_HEIGHT == 1) || (LEDI_WIDTH == 1)
#define LEDI_GRID_LAYOUT                0
#else
#define LEDI_GRID_LAYOUT                1
#endif /* (LEDI_HEIGHT == 1) || (LEDI_WIDTH == 1) */
#endif /* LEDI_GRID_LAYOUT */


/* --------------------------------------------------------------------------------------------
 *  CONST
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * LEDI_NUM_LEDS define
 *
 * Defines the total number of pixels. It is the product of the number of pixels in x and y
 * direction. For example, if LEDI_WIDTH is 128 and LEDI_HEIGHT is 96, the total number of pixels
 * is 12,288.
 */
#define LEDI_NUM_LEDS                    (LEDI_WIDTH * LEDI_HEIGHT)


#endif /* _LEDINFO_H_ */