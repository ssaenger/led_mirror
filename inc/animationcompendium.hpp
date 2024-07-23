/* ********************************************************************************************
 * animationcompendium.hpp
 *
 * Author: Shawn Saenger
 *
 * Created: July 13, 2024
 *
 * Description: Animation Compendium header file.
 *
 * ********************************************************************************************
 */

#ifndef _ANIMATIONCOMPENDIUM_HPP_
#define _ANIMATIONCOMPENDIUM_HPP_

#include "../inc/animations.hpp"

#include "../inc/audiosync.hpp"
#include "../inc/gifDecoder.hpp"
#include "../inc/animatrix.hpp"

/* --------------------------------------------------------------------------------------------
 *  FORWARD DEFS
 * --------------------------------------------------------------------------------------------
 */
typedef struct _AniPack AniPack;

/* --------------------------------------------------------------------------------------------
 *  MACROS
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * pXY macro
 *
 * Calculates the pixel number from a x,y coordinates. Origin is top left.
 * 
 */
#define pXY(x, y)  (LEDI_WIDTH) * (y) + (x)

/* --------------------------------------------------------------------------------------------
 *  DEFINITIONS
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * LED_TYPE define
 *
 * The number of animations that are saved by this layer.
 * 
 */
#ifndef ANICOMP_NUM_ANIMATIONS
#define ANICOMP_NUM_ANIMATIONS 30
#endif /* ANICOMP_NUM_ANIMATIONS */

#if ANICOMP_NUM_ANIMATIONS > 254
#error "ANICOMP_NUM_ANIMATIONS can't be greater than 254"
#endif /* ANICOMP_NUM_ANIMATIONS > 254 */


/* --------------------------------------------------------------------------------------------
 *  TYPES
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * AniCompCategory type
 *
 * Criteria needed to draw to a pixel. Used internally.
 */
typedef uint16_t AniCompCategory;

/* Animations that are being transitioned out (phased out) by 1 or more new ones. */
#define ANI_CRIT_BELOW                     0x0001

/* Animations with ANI_PRIORITY_LOW or higher can draw to this pixel */
#define ANI_CRIT_DEFAULT                   0x0002
#define ANI_CRIT_LOW                       ANI_CRIT_DEFAULT

/* Animations with ANI_PRIORITY_MEDIUM or higher can draw to this pixel */
#define ANI_CRIT_MEDIUM                    0x0004

/* Animations with ANI_PRIORITY_HIGH or higher can draw to this pixel */
#define ANI_CRIT_HIGH                      0x0008

/* Animations with ANI_PRIORITY_PERSISTENT or higher can draw to this pixel */
#define ANI_CRIT_HIGH_PERSISTENT           0x0010

/* Animations with ANI_PRIORITY_TRANSITION or higher can draw to this pixel */
#define ANI_CRIT_TRANSITION                0x0020

/* Special type blend animation between a new and old animation being transitioned out */
#define ANI_CRIT_BLEND                     0x0040

/* End AniCriteria type */

/* --------------------------------------------------------------------------------------------
 *  PUBLIC FUNCTIONS
 * --------------------------------------------------------------------------------------------
 */
bool ANICOMP_Init(AniPack *Animations, uint8_t *NumAnimations);

#endif /* _ANIMATIONCOMPENDIUM_HPP_ */
