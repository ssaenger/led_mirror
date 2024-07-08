/* ********************************************************************************************
 * animations.hpp
 *
 * Author: Shawn Saenger
 *
 * Created: Sep 8, 2023
 *
 * Description: Header file for the animations. APIs are not Reentrant!! Can only be called by
 *              a single thread
 *
 * ********************************************************************************************
 */

#ifndef _ANIMATIONS_HPP_
#define _ANIMATIONS_HPP_

#include "smartmtxconfig.h"
extern "C" {
#include "lists.h"
}

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
 * Calculates the pixel number from a x,y coordinates
 * 
 */
#define pXY(x, y)  (SM_WIDTH) * (y) + (x)

/* --------------------------------------------------------------------------------------------
 * pXY_BtoT macro
 *
 * Calculates the pixel number from a x,y coordinates from Bottom Left 2 Top Right
 *
 */
#define pXY_BL(x, y)  SM_WIDTH * (SM_HEIGHT - 1) + (x) - ((y) * SM_WIDTH)

// #define SM_HEIGHT 64
// #define SM_WIDTH 128
// NUM LED          8,192

/* --------------------------------------------------------------------------------------------
 * fps2Ms macro
 *
 * Calculates the ms window given the fps target. For example, if fps is 100, then there are
 * 10ms for each frame
 * 
 */
#define fps2Ms(fps)  (1000 / fps)
/* --------------------------------------------------------------------------------------------
 *  DEFINITIONS
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * LED_TYPE define
 *
 * The LED type used when passing in
 * 
 */
#ifndef LED_TYPE
#define LED_TYPE rgb24*
#endif /* LED_TYPE */

/* --------------------------------------------------------------------------------------------
 *  TYPES
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * AniType type
 *
 * Ties an animation to a type. Animation types values are ordered by their draw order.
 * There are a few rules to these types and their assignment.
 *      1. An animation function assigned the type ANI_TYPE_INACTIVE means that func is just skipped.
 *      2. The three main animation types are ANI_TYPE_MASK, ANI_TYPE_FOREGROUND, and ANI_TYPE_BACKGROUND
 *      3. Once a function is assigned either ANI_TYPE_TRANS_SWIPE or ANI_TYPE_TRANS_TO_FREE,
 *         types ANI_TYPE_FOREGROUND/ANI_TYPE_BACKGROUND are assigned
 *         types ANI_TYPE_TRANS_BEFORE_FOREGROUND/ANI_TYPE_TRANS_BEFORE_BACKGROUND.
 *      4. Transition type animation functions will become ANI_TYPE_FREE during the current
 *         frame or right after
 *      5. ANI_TYPE_FOREGROUND and ANI_TYPE_BACKGROUND cannot overwrite either
 *         ANI_TYPE_TRANS_BEFORE_FOREGROUND or ANI_TYPE_TRANS_BEFORE_BACKGROUND
 *
 * Generally the lifecylce of animation functions are:
 *                        ANI_TYPE_FOREGROUND/ANI_TYPE_BACKGROUND
 *                                           |
 *                                           v
 *           ANI_TYPE_TRANS_BEFORE_FOREGROUND/ANI_TYPE_TRANS_BEFORE_BACKGROUND
 *                                           |
 *                                           v
 *                       ANI_TYPE_TRANS_SWIPE/ANI_TYPE_TRANS_TO_FREE
 *                                           |
 *                                           v
 *                                     ANI_TYPE_FREE
 *                                           |
 *                                           v
 *                        ANI_TYPE_FOREGROUND/ANI_TYPE_BACKGROUND
 *
 * Pixels get assigned to an animation type, so only the animation function types that follow
 * the rules above are permitted to writing to that pixel.
 */
typedef uint16_t AniType;

/* Group: The following type is free for use by old animations  */
#define ANI_TYPE_OLDFREE_OFFSET           0x0000

/* Available for use immediately in either ANI_TYPE_OLD_OFFSET type animations
 * These are not assignable. Must be assigned a value of 0. These are not assignable.
 */
#define ANI_TYPE_OLD_FREE                 (ANI_TYPE_OLDFREE_OFFSET + 0x00)

/* Group: The following types are old animations. The animation is limited to the mask in AniParms.
* These are not assignable.
*/
#define ANI_TYPE_OLD_OFFSET               0x0010

/* Old animation mask transitioning away! ANI_TYPE_MASK is assigned this when a transition starts */
#define ANI_TYPE_OLD_MASK                 (ANI_TYPE_OLD_OFFSET + 0x00)

/* Old background animation transitioning away! ANI_TYPE_BACKGROUND is assigned this type when
 * a transition has started.
 */
#define ANI_TYPE_OLD_BACKGROUND           (ANI_TYPE_OLD_OFFSET + 0x01)

/* Old foreground animation transitioning away! ANI_TYPE_FOREGROUND is assigned this type when
 * a transition has started. Can overwrite ANI_TYPE_OLD_BACKGROUND
 */
#define ANI_TYPE_OLD_FOREGROUND           (ANI_TYPE_OLD_OFFSET + 0x02)

/* Group: The following types are main animations */
#define ANI_TYPE_MAIN_OFFSET              0x0020

/* Main animation mask. The animation is limited to the mask in AniParms */
#define ANI_TYPE_MASK                     (ANI_TYPE_MAIN_OFFSET + 0x00)

/* Main background animation. Can overwrite ANI_TYPE_FREE */
#define ANI_TYPE_BACKGROUND               (ANI_TYPE_MAIN_OFFSET + 0x01)

/* Main foreground animation. Can overwrite ANI_TYPE_FREE or ANI_TYPE_BACKGROUND */
#define ANI_TYPE_FOREGROUND               (ANI_TYPE_MAIN_OFFSET + 0x02)

/* Group: The following types are transition animations */
#define ANI_TYPE_TRANS_OFFSET             0x0040

/* Transition swipe animation. Pixels written during the current frame are then assigned to the
 * ANI_TYPE_FREE type after the current frame.
 */
#define ANI_TYPE_TRANS_SWIPE              (ANI_TYPE_TRANS_OFFSET + 0x00)

/* Transparent Transition. Pixels are not written but still get assigned to the ANI_TYPE_FREE type
 * during the current frame.
 */
#define ANI_TYPE_TRANS_TRANS              (ANI_TYPE_TRANS_OFFSET + 0x01)

/* Group: The following types are miscellaneous animations. */
#define ANI_TYPE_FREE_OFFSET              0x0080

/* Available for use immediately in either ANI_TYPE_MAIN_OFFSET type animations
 * These are not assignable.
 */
#define ANI_TYPE_FREE                     (ANI_TYPE_FREE_OFFSET + 0x00)

#define ANI_TYPE_SPECIAL_1                0x0100

#define ANI_TYPE_SPECIAL_2                0x0200

#define ANI_TYPE_SPECIAL_3                0x0400

#define ANI_TYPE_SPECIAL_4                0x0800


/* End AniType type */

/* --------------------------------------------------------------------------------------------
 * AniEffect type
 *
 * Bitmask of different effects to apply to an animation. Each animation will describe what
 * each value means
 */
typedef uint8_t AniEffect;

/* Apply no special affects to the animation */
#define ANI_EFFECT_NONE          0x00

/* Apply special effect 1 */
#define ANI_EFFECT_1             0x01

/* Apply special effect 2 */
#define ANI_EFFECT_2             0x02

/* Apply special effect 3 */
#define ANI_EFFECT_3             0x04

/* Apply special effect 4 */
#define ANI_EFFECT_4             0x08

/* End AniEffect type */


/* --------------------------------------------------------------------------------------------
 * AniParms type
 *
 * Animation options that configure the current playing animation. The type of option
 * depends on the animation playing
 *
 */
typedef struct _AniParms {

    /* The current Hue value */
    uint8_t hue;

    CRGB::HTMLColorCode color;

    /* Speed value */
    uint16_t speed;

    /* Scale. Used for certain animations */
    uint16_t scale;

    uint16_t counter;

    /* The max brightness */
    uint8_t maxBright;

    uint8_t chance;

    uint16_t fpsTarg;

    AniType type;

    AniEffect aff;

    uint16_t x0;
    uint16_t y0;
    uint16_t x1;
    uint16_t y1;


    /* An additional animation function pointer that is used to blend animations together */
    //AniFunc aniBlendFunc;

    union {

        /* Valid when animation type is ANI_TYPE_TRANS_SWIPE or ANI_TYPE_TRANS_TO_FREE */
        struct {
            /* Time executing animation */
            //uint32_t transStartTime;
            uint32_t transTime;
            uint32_t transElapsTime;
            //uint32_t delay;

        } trans;

        /* Valid when animation type is ANI_TYPE_MASK. */     
        struct {
            /* This is a pointer to a buffer of size SM_NUM_LEDS filled out with the
             * mask the animation is permitted to write to.
             */
            AniType *maskType;

            /* Create a function pointer var that can be called to update the mask values */
            // void foo(maskType);
        } mask;

    } p;

    /* ~~~~ Internal Only fields ~~~~*/
    uint32_t startTime;
    uint32_t delay;
    uint16_t last;
    uint8_t  value;

} AniParms;

/* --------------------------------------------------------------------------------------------
 * AniFunc type
 *
 * Function pointer to an animated LED sequence
 *
 */
typedef void (*AniFunc)(AniParms *Ap, AniType At);

/* --------------------------------------------------------------------------------------------
 * AniPack type
 *
 * An animation "package". Contains a function pointer to an animation, the type of animation,
 * and parameters to apply to the animation
 *
 */
struct _AniPack {
    /* A node to be placed onto a list. Initialize the node with InitNode().
     * Must be first member in this struct.
     */
    ListNode     node; /* Must be first */

    /* A pointer to an animation function. To be filled prior to calling ANI_AddAnimation() */
    AniFunc      funcp;

    /* Different animation configuration options for the animation in aniFunc.
     * To be filled prior to calling ANI_AddAnimation().
     */
    AniParms     parms;

    /* ~~ Internal Fields. ~~ They should not be set by the user */
    /* The type of animation to treat the animation in funcp. Can be read. It gets assigned the
     * type that was passed in ANI_AddAnimation().
     */
    AniType      type;
};


/* --------------------------------------------------------------------------------------------
 *  PUBLIC FUNCTIONS
 * --------------------------------------------------------------------------------------------
 */

void ANI_Init(AniType *OwnerArray);

/* Adds an unused AniPack to an animation list of the type "At" */
bool ANI_AddAnimation(AniPack *Ap, AniType At);

/* Fills out the LedBuff with animations :3 */
void ANI_RemoveAnimation(AniPack *Ap);

/* Queue animations */
bool ANI_QueueAnimation(AniPack *Ap);

/* Add the queued animation to the active list */
void ANI_SwapAnimation();

/* Fills out the LedBuff with animations :3 */
uint32_t ANI_DrawAnimationFrame(rgb24 *LedBuff);

// Writes the pixel to the PixNum LED if it has permission to
void writePixel(AniParms *Ap, AniType At, uint32_t PixNum, const rgb24 &RgbVal);

/* Group: The following functions are animation functions of type AniFunc */

/* Creates some noise in the animation that is fluid-like */
void ANIFUNC_FillNoise8(AniParms *Ap, AniType At);

void ANIFUNC_Glitter(AniParms *Ap, AniType At);

void ANIFUNC_Rainbow(AniParms *Ap, AniType At);

void ANIFUNC_PlazInt(AniParms *Ap, AniType At);

void ANIFUNC_Confetti(AniParms *Ap, AniType At);


#endif /* _ANIMATIONS_HPP_ */