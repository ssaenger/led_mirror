/* ********************************************************************************************
 * animationcompendium.cpp
 *
 * Author: Shawn Saenger
 *
 * Created: July 13, 2024
 *
 * Description: A collection of Animations
 *
 * ********************************************************************************************
 */

#include "../inc/animationcompendium.hpp"

/* --------------------------------------------------------------------------------------------
 *  GLOBALS
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 *  PROTOTYPES
 * --------------------------------------------------------------------------------------------
 */


/* --------------------------------------------------------------------------------------------
 *  PUBLIC FUNCTIONS
 * --------------------------------------------------------------------------------------------
 */


/* --------------------------------------------------------------------------------------------
 *                 ANICOMP_GetCategories()
 * --------------------------------------------------------------------------------------------
 * Description:    Initializes the AniPack array with recommended settings
 *
 * Parameters:     AniPtr - A pointer to receive a filled array of AniPacks
 *                 NumAnimations - Number of elements that are filled in AniPtr.
 *                     The maximum number of elements is controlled by ANICOMP_NUM_ANIMATIONS.
 *
 * Returns:        void
 */
bool ANICOMP_Init(AniPack *Animations, uint8_t *NumAnimations)
{
    uint8_t i;
    uint8_t numAnimations;
    AniLayer defaultLayer = ANI_LAYER_MIDDLE;
    HSVHue basicHues[8] = {HUE_RED, HUE_ORANGE, HUE_YELLOW, HUE_GREEN,
                           HUE_AQUA, HUE_BLUE, HUE_PURPLE, HUE_PINK};

    if  (*NumAnimations < ANICOMP_NUM_ANIMATIONS) {
        return false;
    }
    memset((void*)Animations, 0, sizeof(Animations));


    i = 0;
    Animations[i].funcp = ANIFUNC_Confetti;
    Animations[i].tags = (ANI_TAG_REMEMBRANCE | ANI_TAG_VISUAL | ANI_TAG_TRANSITION);
    Animations[i].parms.scale = 233;
    Animations[i].parms.fpsTarg = 20; /* 1000 means max capable refresh rate */
    i++;
#if 0
    Animations[1].funcp = ANIFUNC_PlazInt;
    Animations[1].defaultLayer = ANI_TYPE_BACKGROUND;
    Animations[1].parms.counter = 100;
    Animations[1].parms.fpsTarg = 60;
    Animations[1].parms.type = 0;

    InitNode(&Animations[2].node);
    Animations[2].funcp = ANIFUNC_Glitter;
    Animations[2].defaultLayer = ANI_TYPE_TRANS_SWIPE;
    Animations[2].parms.color = CRGB::Black;
    Animations[2].parms.p.trans.transTime = 3000;
    Animations[2].parms.fpsTarg = 40;
    Animations[2].parms.counter = 0;
    Animations[2].parms.scale = 10;
    Animations[2].parms.type = 0;

    InitNode(&Animations[3].node);
    Animations[3].funcp = [](AniParms *Ap) {
        NETMGR_LineSwipeR(Ap, At);
    };
    //Animations[3].funcp =         NETMGR_LineSwipeR;
    Animations[3].defaultLayer = ANI_TYPE_TRANS_SWIPE;
    Animations[3].parms.hue = 0;
    Animations[3].parms.fpsTarg = LEDI_WIDTH / 2; // about 2 seconds
    Animations[3].parms.p.trans.transTime = 4000;
    Animations[3].parms.speed = 3;
    Animations[3].parms.scale = 0;
    Animations[3].parms.type = 0;
    Animations[2].parms.counter = 4;
    Animations[3].parms.x0 = 0;
    Animations[3].parms.y0 = 0;
    Animations[3].parms.x1 = 0;
    Animations[3].parms.y1 = LEDI_HEIGHT - 1;
#endif

    //Animations[4].funcp = [](AniParms *Ap) {
    //    AS_PlotFftTop(Ap, At);
    //};
    Animations[i].funcp = AS_PlotFftTop;
    Animations[i].tags = (ANI_TAG_AUDIO_REACTIVE | ANI_TAG_GRID_OPTIMIZED);
    Animations[i].parms.speed = 1;
    Animations[i].parms.scale = 10;
    Animations[i].parms.fpsTarg = 90; /* No use going higher. Data won't be available */
    Animations[i].parms.mod = ANI_MOD_3 | ANI_MOD_2 | ANI_MOD_1; /* If ANI_MOD_4, set size */
    /* Must be divisible by LEDI_WIDTH. For 128, that's 2, 4, 8, 16... Only use 2 and 4 */
    Animations[i].parms.size = 2;
    i++;
    Animations[i].funcp = AS_PlotFftBottom;
    Animations[i].tags = (ANI_TAG_AUDIO_REACTIVE | ANI_TAG_GRID_OPTIMIZED);
    Animations[i].parms.speed = 0;
    Animations[i].parms.mod = ANI_MOD_3;
    Animations[i].parms.fpsTarg = 90;
    i++;
    Animations[i].funcp = AS_PlotFftMid;
    Animations[i].tags = (ANI_TAG_AUDIO_REACTIVE | ANI_TAG_GRID_OPTIMIZED);
    Animations[i].parms.speed = 4;
    Animations[i].parms.scale = 20;
    Animations[i].parms.offset = 0;
    Animations[i].parms.counter = 0;
    Animations[i].parms.mod = ANI_MOD_3 | ANI_MOD_2 | ANI_MOD_1;
    Animations[i].parms.fpsTarg = 90;
    i++;
    Animations[i].funcp = AS_RainbowIris;
    Animations[i].tags = (ANI_TAG_AUDIO_REACTIVE | ANI_TAG_VISUAL);
    Animations[i].parms.scale = 0; /* How much the rainbow color changes per frame */
    Animations[i].parms.fpsTarg = 100;
    i++;
    Animations[i].funcp = ANIFUNC_RainbowIris;
    Animations[i].tags = (ANI_TAG_VISUAL);
    Animations[i].parms.speed = 1; /* How fast it changes between frames */
    Animations[i].parms.scale = 0; /* How much the rainbow color changes per frame */
    Animations[i].parms.fpsTarg = 40;
    i++;
    Animations[i].funcp = ANIFUNC_PlazInt;
    Animations[i].tags = (ANI_TAG_VISUAL);
    Animations[i].parms.fpsTarg = 80;
    i++;
    Animations[i].funcp = GIFDEC_Play;
    Animations[i].tags = (ANI_TAG_VISUAL | ANI_TAG_GRID_OPTIMIZED | ANI_TAG_GIF);
    Animations[i].parms.fpsTarg = 20;
    Animations[i].parms.last = 1;
    i++;
    Animations[i].funcp = ANIMAX_Lava1;
    Animations[i].tags = (ANI_TAG_VISUAL);
    Animations[i].parms.fpsTarg = 400;
    i++;
    Animations[i].funcp = ANIMAX_ChasingSpirals;
    Animations[i].tags = (ANI_TAG_VISUAL);
    Animations[i].parms.fpsTarg = 400;
    i++;
    Animations[i].funcp = ANIMAX_Caleido1;
    Animations[i].tags = (ANI_TAG_VISUAL);
    Animations[i].parms.fpsTarg = 400;
    i++;
    Animations[i].funcp = ANIMAX_Zoom;
    Animations[i].tags = (ANI_TAG_VISUAL);
    Animations[i].parms.fpsTarg = 400;
    i++;
    Animations[i].funcp = ANIMAX_Rings;
    Animations[i].tags = (ANI_TAG_VISUAL);
    Animations[i].parms.fpsTarg = 400;
    i++;
    Animations[i].funcp = ANIMAX_Waves;
    Animations[i].tags = (ANI_TAG_VISUAL);
    Animations[i].parms.fpsTarg = 400;
    i++;
    Animations[i].funcp = ANIMAX_CenterField;
    Animations[i].tags = (ANI_TAG_VISUAL);
    Animations[i].parms.fpsTarg = 400;
    i++;
    Animations[i].funcp = ANIMAX_Caleido2;
    Animations[i].tags = (ANI_TAG_VISUAL);
    Animations[i].parms.fpsTarg = 400;
    i++;
    Animations[i].funcp = ANIMAX_Caleido3;
    Animations[i].tags = (ANI_TAG_VISUAL);
    Animations[i].parms.fpsTarg = 400;
    i++;
    Animations[i].funcp = ANIMAX_Scaledemo1;
    Animations[i].tags = (ANI_TAG_VISUAL);
    Animations[i].parms.fpsTarg = 400;
    i++;
    Animations[i].funcp = ANIMAX_Yves;
    Animations[i].tags = (ANI_TAG_VISUAL);
    Animations[i].parms.fpsTarg = 400;
    i++;
    Animations[i].funcp = ANIMAX_Spiralus;
    Animations[i].tags = (ANI_TAG_VISUAL);
    Animations[i].parms.fpsTarg = 400;
    i++;
    Animations[i].funcp = ANIMAX_Spiralus2;
    Animations[i].tags = (ANI_TAG_VISUAL);
    Animations[i].parms.fpsTarg = 400;
    i++;
    Animations[i].funcp = Animax_HotBlob;
    Animations[i].tags = (ANI_TAG_VISUAL);
    Animations[i].parms.fpsTarg = 400;
    i++;
    
#if 0
    InitNode(&Animations[8].node);
    Animations[8].funcp = GIFDEC_Play;
    Animations[8].defaultLayer = ANI_TYPE_BACKGROUND;
    Animations[8].parms.counter = 7;
    Animations[8].parms.fpsTarg = 12;

    InitNode(&Animations[9].node);
    Animations[9].funcp = [](AniParms *Ap) {
        AS_PlotFftBottom(Ap, At);
    };
    Animations[9].defaultLayer = ANI_TYPE_FOREGROUND;
    Animations[9].parms.hue = HUE_GREEN;
    Animations[9].parms.speed = 0;
    Animations[9].parms.maxBright = 200;
    Animations[9].parms.mod = 0;
    Animations[9].parms.fpsTarg = 100; /* No use going higher. Data won't be available */
    Animations[9].parms.type = 0;
    Animations[9].parms.mod = ANI_MOD_2;
#endif

    numAnimations = i;
    for (i = 0; i < numAnimations; i++) {
        InitNode(&Animations[i].node);
        /* Initialize list head for all Animations (but only used by ANI_TAG_REMEMBRANCE) */
        InitList(&Animations[i].parms.pixList);
       
        Animations[i].parms.hsv.setHSV(basicHues[i % 8], 0xFF, 0xFF);
        /* Set layer */
        if (Animations[i].defaultLayer == 0) {
            if (ANI_TAG_IS_TRANSITION_ONLY(Animations[i].tags)) {
                Animations[i].defaultLayer = ANI_LAYER_TRANSITION;
            } else if (ANI_TAG_IS_AUDIO_ANALYSIS(Animations[i].tags)) {
                Animations[i].defaultLayer = ANI_LAYER_PERSISTENT;
            } else if (Animations[i].tags & ANI_TAG_AUDIO_REACTIVE) {
                Animations[i].defaultLayer = defaultLayer;
                Animations[i].parms.hsv.v = 130;
            } else {
                Animations[i].defaultLayer = defaultLayer;
                Animations[i].parms.hsv.v = 130;
            }
        }
    }
    //Serial.println("Exiting ANICOMP_Init");

    *NumAnimations = numAnimations;
    return true;
}
