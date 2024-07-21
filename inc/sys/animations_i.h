/* ********************************************************************************************
 * animations_i.h
 *
 * Author: Shawn Saenger
 *
 * Created: Sep 8, 2023
 *
 * Description: Internal Header file for the animations
 *
 * ********************************************************************************************
 */

#ifndef _ANIMATIONS_I_H_
#define _ANIMATIONS_I_H_

#include "../animations.hpp"


/* --------------------------------------------------------------------------------------------
 * AniCriteria type
 *
 * Criteria needed to draw to a pixel. Used internally.
 */
typedef uint16_t AniCriteria;

/* Persistent flag */
#define ANI_CRIT_PERSISTENT                0x0001

/* Any animations being phased out will have at least 1 bit set in this */
#define ANI_CRIT_BELOW_ANY                 0x00F0

/* Animations that are being transitioned out (phased out) by 1 or more new ones. */
#define ANI_CRIT_BELOW_LOW                 0x0010

/* Animations that are being transitioned out (phased out) by 1 or more new ones. */
#define ANI_CRIT_BELOW_MEDIUM              0x0020

/* Animations that are being transitioned out (phased out) by 1 or more new ones. */
#define ANI_CRIT_BELOW_HIGH                0x0040

/* Animations that are being transitioned out (phased out) by 1 or more new ones. */
#define ANI_CRIT_BELOW_HIGH_PERSISTENT     (ANI_CRIT_BELOW_HIGH | ANI_CRIT_PERSISTENT)

/* Any animations that is active and not a transition animation will have at least 1 bit set */
#define ANI_CRIT_ACTIVE_ANY                0x0F00

/* Animations with ANI_PRIORITY_LOW or higher can draw to this pixel */
#define ANI_CRIT_DEFAULT                   0x0100
#define ANI_CRIT_LOW                       ANI_CRIT_DEFAULT

/* Animations with ANI_PRIORITY_MEDIUM or higher can draw to this pixel */
#define ANI_CRIT_MEDIUM                    0x0200

/* Animations with ANI_PRIORITY_HIGH or higher can draw to this pixel */
#define ANI_CRIT_HIGH                      0x0400

/* Animations with ANI_PRIORITY_PERSISTENT or higher can draw to this pixel */
#define ANI_CRIT_HIGH_PERSISTENT           (ANI_CRIT_HIGH | ANI_CRIT_PERSISTENT)

/* Animations with ANI_PRIORITY_TRANSITION or higher can draw to this pixel */
#define ANI_CRIT_TRANSITION                0x0800

/* Special type blend animation between a new and old animation being transitioned out */
#define ANI_CRIT_BLEND                     0x1000

/* End AniCriteria type */

/* --------------------------------------------------------------------------------------------
 * AniInfo type
 *
 * A collection of animations
 *
 */
typedef struct _AniInfo {
    /* List of AniPack. Active list */
    ListNode    activeList;

    /* List of aniPack. Queue list waiting to be moved to the activeList */
    ListNode    queueList;

    /* List of AniPack. All the main animation types are placed here. Once activated, they will
    * move to the activeList and move back when they are deactivated
     */
    ListNode    mainWaitList;
    uint8_t     numMainWaiting;

    /* List of AniPack. All the trans animation types are placed here. Once activated, they will
     * move to the activeList and move back when they are deactivated.
     */
    ListNode    transWaitList;
    uint8_t     numTransWaiting;

    bool        tranInProg;

    /* The LED pixel number that was recently drawn to */
    AniPixel   *pix;

    uint16_t    fpsTarg;
    uint32_t    msDelay;

    /* Pointer to the current drawing buffer of size LEDI_NUM_LEDS */
    LED_TYPE   *drawBuff;

} AniInfo;

#endif /* _ANIMATIONS_I_H_ */