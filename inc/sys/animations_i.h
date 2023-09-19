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

    /* Which of the three functions is permitted to writing to the indexed LED */
    AniType     *owners;

    /* The LED pixel number that was recently drawn to */
    //uint32_t    drawn[SM_NUM_LEDS];

    /* Pointer to the backgroundLayer's backBuffer (drawing buffer) */
    rgb24      *ledBuff;

} AniInfo;

#endif /* _ANIMATIONS_I_H_ */