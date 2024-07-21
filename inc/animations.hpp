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
 * Calculates the pixel number from a x,y coordinates. Origin is top left.
 * 
 */
#define pXY(x, y)  (LEDI_WIDTH) * (y) + (x)

/* --------------------------------------------------------------------------------------------
 * pXY_BtoT macro
 *
 * Calculates the pixel number from a x,y coordinates from Bottom Left 2 Top Right
 *
 */
#define pXY_BL(x, y)  LEDI_WIDTH * (LEDI_HEIGHT - 1) + (x) - ((y) * LEDI_WIDTH)

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
#define LED_TYPE rgb24
#endif /* LED_TYPE */

/* --------------------------------------------------------------------------------------------
 *  TYPES
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * AniTags type
 *
 * Specifies the general tags attached to an animation. Multiple tags can be included by
 * bitwise ORing together.
 */
typedef uint16_t AniTags;

/* Animation tag is unknown or unspecified. Should not be ORd with other tag values */
#define ANI_TAG_UNSPECIFIED                0x0000

/* Animation is audio reactive */
#define ANI_TAG_AUDIO_REACTIVE             0x0001

/* Animation plays a gif file on the SD card */
#define ANI_TAG_GIF                        0x0002

/* Animation remembers previously worked on LEDs and keeps track of them using a list */
#define ANI_TAG_REMEMBRANCE                0x0004

/* Animation displays text */
#define ANI_TAG_GRID_TEXTUAL               0x0008

/* Animation uses a mask and only writes to those pixels */
#define ANI_TAG_MASK                       0x0010

/* Animation is purely aesthetic to view */
#define ANI_TAG_VISUAL                     0x0100

/* Animation is can be be used for transitioning a new animation while phasing out
 * the previous animation(s).
 */
#define ANI_TAG_TRANSITION                 0x0200

/* Animation looks best for LED panels layed out in a grid */
#define ANI_TAG_GRID_OPTIMIZED             0x0400

/* Macro to identify an animation intended for audio analysis only only */
#define ANI_TAG_IS_AUDIO_ANALYSIS(t)      (((t) & (ANI_TAG_AUDIO_REACTIVE | ANI_TAG_VISUAL)) == \
                                            ANI_TAG_AUDIO_REACTIVE)

/* Macro to identify an animation intended for transitioning only */
#define ANI_TAG_IS_TRANSITION_ONLY(t)      (((t) & (ANI_TAG_TRANSITION | ANI_TAG_VISUAL)) == \
                                            ANI_TAG_TRANSITION)
/* End AniTags type */

/* --------------------------------------------------------------------------------------------
 * AniLayer type
 *
 * Defines the layer of an animation. Animations with a higher value layer have higher priority
 * over animations of a lower layer value, e.g. ANI_LAYER_PERSISTENT has a higher priority than
 * ANI_LAYER_HIGH. Animations with a higher priority essentially "claim" an LED pixel preventing
 * animations of lower priority from drawing to that pixel for a given draw frame.
 * In essence, this allows layered animations with low priority animations drawing to
 * the "background" and higher animations drawing to the "foreground".
 *
 */
typedef uint16_t AniLayer;

/* Unassigned animation. Unassigned animations are innactive when registered by
 * ANI_RegisterAnimation() and an animation uses the assigned type by
 * ANI_RegisterAnimation() when ANI_AddNextAnimation() is called.
 */
#define ANI_LAYER_UNASSIGNED               0x00F0

/* Writes to the "bottom" layer (background) */
#define ANI_LAYER_BOTTOM                   0x0100

/* Writes to the "middle" layer (middle ground) */
#define ANI_LAYER_MIDDLE                   0x0200

/* Writes to the "top" layer (foreground) */
#define ANI_LAYER_TOP                      0x0400

/* Persistent type. Higher priority than ANI_TYPE_TOP. This type keeps the pixel set
 * to the last value drawn until black is drawn by an animation that has this type, after
 * which it is free to be drawn by any animation. This type is useful for foreground
 * animations that don't write to a pixel on every draw frame such as when the animation's
 * FPS target is set lower than the refresh rate.
 */
#define ANI_LAYER_PERSISTENT               0x0401

/* Transition type. Highest priority. This type keeps the pixel set to the last value
 * drawn until black is drawn by an animation that has this type, or when an animation of
 * this type expires, after which it is free to be drawn by any animation. It is intended
 * to be used by animations that "transition" out a previous one with a new one.
 *
 * Tip: Set the RGB color to 0 (black) to let a pixel be drawn by a new animation on the same
 * draw frame.
 */
#define ANI_LAYER_TRANSITION               0x0800

/* End AniLayer type */

/* --------------------------------------------------------------------------------------------
 * AniMod type
 *
 * Specific modifications to some animations
 *
 */
typedef uint8_t AniMod;

#define ANI_MOD_1                   0x01
#define ANI_MOD_2                   0x02
#define ANI_MOD_3                   0x04
#define ANI_MOD_4                   0x08

/* End AniMod type */

/* --------------------------------------------------------------------------------------------
 * AniPixel type
 *
 * Desribes the state of an LED pixel.
 *
 */
typedef struct _AniPixel {
    /* ANI_TAG_REMEMBRANCE animations can set this node to their animation pack */
    ListNode    node;

    uint16_t    pixNum;
    uint16_t    crit; /* type AniCriteria */

    CRGB       color;
} AniPixel;

/* --------------------------------------------------------------------------------------------
 * AniParms type
 *
 * Animation options that configure the current playing animation. The type of option
 * depends on the animation playing
 *
 */
typedef struct _AniParms {

    /* The current Hue, saturation, and value (brightness) */
    CHSV    hsv;

    CRGB::HTMLColorCode color;

    /* Speed value */
    uint16_t speed;

    uint16_t offset;

    /* Scale. Used for certain animations */
    uint16_t scale;

    uint16_t counter;

    /* The max brightness */
    uint8_t size;

    uint8_t chance;

    /* Used by tag ANI_TAG_REMEMBRANCE */
    ListNode pixList;

    uint16_t fpsTarg;

    AniMod mod;


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
            /* This is a pointer to a buffer of size LI_NUM_LEDS filled out with the
             * mask the animation is permitted to write to.
             */
            AniLayer *maskType;

            /* Create a function pointer var that can be called to update the mask values */
            // void foo(maskType);
        } mask;

    } p;

    /* ~~~~ Internal Only fields ~~~~*/
    uint32_t startTime;
    /* Used to determine how long of a gap is needed before playing this animation */
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
typedef void (*AniFunc)(AniParms *Ap);

/* --------------------------------------------------------------------------------------------
 * AniPack type
 *
 * An animation "package". Contains a function pointer to an animation, the type of animation,
 * and parameters to apply to the animation
 *
 */
struct _AniPack {
    /* A list node that will be placed onto a list to be managed by this layer.
     * Internal use only. Don't modify.
     */
    ListNode     node; /* Must be first */

    /* Group: The following must be initialized prior to calling ANI_RegisterAnimation()
     * (In reality it can be delayed until ANI_SwapAnimation() is called but for simplicity
     * it should be filled out before registering the animation).
     */

    /* A pointer to an animation function */
    AniFunc      funcp;

    /* Different animation configuration options for the animation in aniFunc.
     * These parms allow for different affects to happen on an animation in real time.
     * The useful parms to modify depend on the animation tag. Most parms can be modified
     * between calls to ANI_DrawAnimationFrame().
     */
    AniParms     parms;

    /* Animation tag. */
    AniTags      tags;

    /* Group: The following is modified by ANI_RegisterAnimation() */

    /* Default layer of the animmation. The animation can be temporarily changed to a different
     * layer if one is specified in the ANI_AddNextAnimation() function otherwise the default
     * will be used.
     */
    AniLayer     defaultLayer;

    /* ~~ Internal Fields. ~~ They should not be set by the user */

    /* The current criteria of this animation */
    uint16_t      currCriteria; /* AniCriteria type */
};


/* --------------------------------------------------------------------------------------------
 *  PUBLIC FUNCTIONS
 * --------------------------------------------------------------------------------------------
 */

bool ANI_Init(void);

/* Register an unused AniPack to an animation list */
bool ANI_RegisterAnimation(AniPack *Ap, AniLayer DefaultLayer);

/* Unregisters an animation that was previously registered */
bool ANI_UnregisterAnimation(AniPack *Ap, bool Force);

/* Add animations that will become active when ANI_SwapAnimation() is called */
bool ANI_AddNextAnimation(AniPack *Ap, AniLayer OverrideLayer);

/* Add animations that will become active after ANI_SwapAnimation() is called. */
bool ANI_AddNextAnimationByFuncP(AniFunc FuncP, AniLayer OverrideLayer);

/* Add the queued animation to the active list */
void ANI_SwapAnimation(bool UseBlending);

/* Checks if the current animation is allowed to write to this pixel */
AniPixel *ANI_CheckPixNum(uint32_t PixNum);

/* Checks if the current animation is allowed to write to this pixel */
bool ANI_CheckPix(AniPixel *Pix);

// Writes the pixel to the PixNum LED if it has permission to
void ANI_WriteVerifiedPix(AniParms *Ap, AniPixel *Pix, const CRGB &RgbVal);

// Writes the pixel to the PixNum LED if it has permission to
void ANI_WritePixel(AniParms *Ap, uint32_t PixNum, const CRGB &RgbVal);

/* Fills out the LedBuff with animations :3 */
uint32_t ANI_DrawAnimationFrame(rgb24 *LedBuff);


/* Group: The following functions are animation functions of type AniFunc */

void AniWriteToBuffer(void);

/* Creates some noise in the animation that is fluid-like */
#if 0
void ANIFUNC_FillNoise8(AniParms *Ap);

void ANIFUNC_Glitter(AniParms *Ap);
#endif

void ANIFUNC_RainbowIris(AniParms *Ap);

void ANIFUNC_PlazInt(AniParms *Ap);
void ANIFUNC_Confetti(AniParms *Ap);


#endif /* _ANIMATIONS_HPP_ */