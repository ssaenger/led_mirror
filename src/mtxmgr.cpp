/* ********************************************************************************************
 * mtxmgr.cpp
 *
 * Author: Shawn Saenger
 *
 * Created: Sep 4, 2023
 *
 * Description: Matrix Manager implementation. Responsible for coordinating between the different
 * layers driving the LED matrix
 *
 * ********************************************************************************************
 */

#include "../inc/mtxmgr.hpp"
#include "../inc/smartmtxconfig.h"
#include "../inc/audiosync.hpp"
#include "../inc/gifDecoder.hpp"
#include "../inc/animationcompendium.hpp"

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, LEDI_WIDTH, LEDI_HEIGHT, SM_REFRESH_DEPTH, SM_DMA_BUFF_ROWS, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, LEDI_WIDTH, LEDI_HEIGHT, SM_COLOR_DEPTH, kBackgroundLayerOptions);

// For larger fonts, you need to allocate enough memory to hold the entire scrolling text bitmap (the bounds of all the characters in the string) in memory
// and this may be larger than kMatrixWidth*kMatrixHeight.  Use the SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER macro, which allows for setting the character bitmap
// to different size than the matrix bounds.  We allocate several times the default amount of memory here:
SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(scrollingLayer, LEDI_WIDTH, LEDI_HEIGHT, LEDI_WIDTH * 2, LEDI_HEIGHT, SM_COLOR_DEPTH, kScrollingLayerOptions);

/* --------------------------------------------------------------------------------------------
 *  GLOBALS
 * --------------------------------------------------------------------------------------------
 */
HSVHue hueArr[8] = {HUE_RED, HUE_ORANGE, HUE_YELLOW,
                    HUE_GREEN, HUE_AQUA, HUE_BLUE,
                    HUE_PURPLE, HUE_PINK};


static AniPack animations[ANICOMP_NUM_ANIMATIONS];

/* --------------------------------------------------------------------------------------------
 *                 MtxMgr()
 * --------------------------------------------------------------------------------------------
 * Description:    Private MtxMgr constructor
 *
 * Parameters:     void
 *
 * Returns:        void
 */
MtxMgr::MtxMgr()
{
    SM_RGB pix;
    pix.red = 0x00;
    pix.green = 0x00;
    pix.blue = 0x00;
    
    // just showing different ways
    // backgroundLayer.fillScreen(rgb24(0xff, 0xff, 0xff));
    // backgroundLayer.fillScreen(SM_RGB(0xff, 0xff, 0xff));
    // backgroundLayer.fillScreen(CRGB(CHSV(0xff, 0xff, 0xff)));
    // backgroundLayer.fillScreen({0xff, 0xff, 0xff});
    // backgroundLayer.fillScreen(CRGB(0x2000));
    // backgroundLayer.fillScreen(CRGB(0x200000));
    // backgroundLayer.drawPixel(1, 1, CRGB(0x200000));
    // SM_RGB color = CRGB(CHSV(5 * 15, 255, 255));
}

/* --------------------------------------------------------------------------------------------
 *                 ~MtxMgr()
 * --------------------------------------------------------------------------------------------
 * Description:    Private MtxMgr deconstructor
 *
 * Parameters:     void
 *
 * Returns:        void
 */
MtxMgr::~MtxMgr()
{
}

/* --------------------------------------------------------------------------------------------
 *                 getInstance()
 * --------------------------------------------------------------------------------------------
 * Description:    Grabs a reference to the singleton MtxMgr object
 *
 * Parameters:     void
 *
 * Returns:        A pointer to a MtxMgr object
 */
MtxMgr& MtxMgr::getInstance()
{
    static MtxMgr netmgr;
    return netmgr;
}

/* --------------------------------------------------------------------------------------------
 *                 syncInit()
 * --------------------------------------------------------------------------------------------
 * Description:    A blocking call that initializes MtxMgr.
 *
 * Parameters:     void
 *
 * Returns:        true if successful, false othewise
 */
bool MtxMgr::syncInit()
{
    uint8_t i;

    //matrix.setRefreshRate(60);
    matrix.addLayer(&backgroundLayer);
    //matrix.addLayer(&foregroundLayer);
    //matrix.addLayer(&scrollingLayer);
    matrix.begin();
    delay(10);

    matrix.setBrightness(SM_INITIAL_BRIGHTNESS);

    val = 0;

    //backgroundLayer.setDrawnPixelBuffer(ledBuff);
    //memset((void *)aniI.owners, ANI_TYPE_FREE, sizeof(aniI.owners));
    if (!ANI_Init()) {
        Serial.println("Can't initialize ANI_Init");
        return false;
    }
    if (!ANIMAX_Init()) {
        Serial.println("Could not animartrix");
        return false;
    }
    //if (!GIFDEC_Init()) {
    //    // TODO: prevent adding gif animation
    //    Serial.println("Could not initialize gif");
    //}

    //Serial.printf("Before ANICOMP_Init() 0x%x.\n\r", animations);
    aniPackNum = ANICOMP_NUM_ANIMATIONS;
    if (!ANICOMP_Init(animations, &aniPackNum)) {
        Serial.println("Could not init anicomp!");
        return false;
    }
    //Serial.printf("After ANICOMP_Init() 0x%x.\n\r", animations);
    //Serial.printf("Before ANI_RegisterAnimation() %d.\n\r", aniPackNum);
    for (i = 0; i < aniPackNum; i++) {
        Serial.printf("ANI_RegisterAnimation()...func 0x%x, tags 0x%x, defaultlayer 0x%x\n\r", animations[i].funcp, animations[i].tags, animations[i].defaultLayer);
        ANI_RegisterAnimation(&animations[i], animations[i].defaultLayer);

    }
    //AS_Init();
    //Serial.printf("After ANI_RegisterAnimation() %d.\n\r", aniPackNum);
    //ANI_AddNextAnimation(&animations[3]);
    //if (!ANI_AddNextAnimation(&animations[0], 0)) {
    /*
    if (!ANI_AddNextAnimationByFuncP(AS_RainbowIris, 0)) {
        Serial.println("Could not add animation!");
        return false;
    }
    if (!ANI_AddNextAnimationByFuncP(AS_PlotFftTop, 0)) {
        Serial.println("Could not add animation!");
        return false;
    }
    */
    //if (!ANI_AddNextAnimationByFuncP(GIFDEC_Play, 0)) {
    //    Serial.println("Could not add animation!");
    //    return false;
    //}
    if (!ANI_AddNextAnimationByFuncP(Animax_HotBlob, 0)) {
        Serial.println("Could not add animation!");
        return false;
    }
    

    ANI_SwapAnimation(false);

    Serial.println(matrix.getRefreshRate());
    return true;
}

/* --------------------------------------------------------------------------------------------
 *                 run()
 * --------------------------------------------------------------------------------------------
 * Description:    Runs through MtxMgr tasks
 *
 * Parameters:     void
 *
 * Returns:        void
 */
void MtxMgr::run()
{
#if 1
    static int val = 0;
#endif
    static uint16_t pixCount = 0;
#if 0
    while(backgroundLayer.isSwapPending());
    ledBuff = backgroundLayer.backBuffer();
    //ledBuff2 = foregroundLayer.backBuffer();
    Serial.println("Here0");
    memset(ledBuff, 0, sizeof(RGB) * LEDI_NUM_LEDS);
    //memset(ledBuff2, 0, sizeof(RGB) * LEDI_NUM_LEDS);
    Serial.println("Here1");
    backgroundLayer.drawCircle(64, 32, 10, 0x555555);
    //foregroundLayer.drawCircle(64, 32, 10, 0x8810FF);
    Serial.println("Here1.1");
    backgroundLayer.swapBuffers(false);
    //foregroundLayer.swapBuffers(false);
    Serial.println("Here2");
    while(1) {
        while(backgroundLayer.isSwapPending());
        //while(foregroundLayer.isSwapPending());


        backgroundLayer.drawCircle(64, 32, 10, 0x555555);
        //foregroundLayer.drawCircle(64, 32, 10, 0x8810FF);

        backgroundLayer.swapBuffers(false);
        //foregroundLayer.swapBuffers(false);
        matrix.countFPS();      // print the loop() frames per second to Serial
    }
#endif
#if 1
    while(backgroundLayer.isSwapPending());
    ledBuff = backgroundLayer.getRealBackBuffer();

    if ((pixCount = ANI_DrawAnimationFrame(ledBuff)) != 0) {

        //Serial.println("Swapping");
        //rgb24 rgbcolor = ledBuff[60];
        //Serial.printf("g=%d\r\n", rgbcolor.green);
        if (pixCount >= LEDI_NUM_LEDS) {
            /* All pixels were drawn. Assume this means that the next
             * will be filled as well
             */
            backgroundLayer.swapBuffers(false);
        } else {
            /* Copy the current frame to the next frame */
            backgroundLayer.swapBuffers(true);
        }
        matrix.countFPS();      // print the loop() frames per second to Serial
    }
#if 1
    EVERY_N_SECONDS(10) {
        val = (val + 1) % 7;
        Serial.printf("Swapping!! val = %d\n\r", val);
        if (val == 0) {
            if (!ANI_AddNextAnimationByFuncP(Animax_HotBlob, 0)) {
                Serial.println("Could not add animation!");
            }
        } else if (val == 1) {
            if (!ANI_AddNextAnimationByFuncP(ANIMAX_Rings, 0)) {
                Serial.println("Could not add animation!");
            }
        } else if (val == 2) {
            if (!ANI_AddNextAnimationByFuncP(ANIMAX_Scaledemo1, 0)) {
                Serial.println("Could not add animation!");
            }
        } else if (val == 3) {
            if (!ANI_AddNextAnimationByFuncP(ANIMAX_Yves, 0)) {
                Serial.println("Could not add animation!");
            }
        } else if (val == 4) {
            if (!ANI_AddNextAnimationByFuncP(ANIMAX_Spiralus, 0)) {
                Serial.println("Could not add animation!");
            }
        } else if (val == 5) {
            if (!ANI_AddNextAnimationByFuncP(ANIMAX_Caleido1, 0)) {
                Serial.println("Could not add animation!");
            }
        } else if (val == 6) {
            if (!ANI_AddNextAnimationByFuncP(ANIMAX_Spiralus2, 0)) {
                Serial.println("Could not add animation!");
            }
        }
        ANI_SwapAnimation(false);
    }
#endif
#endif
}

#if 0
void MtxMgr::NETMGR_LineSwipeR(AniParms *Ap)
{
    CHSV   hsv;
    uint16_t y;
    CRGB   crgb;

    backgroundLayer.registerWriteCallback(Ap, At, [](void *ctx, uint16_t ctxVal, uint16_t pixNum, const rgb24 &color) {
        writePixel((AniParms*)ctx, ctxVal, pixNum, color);
    });

    for (y = 0; y < LEDI_HEIGHT; y++) {

    }

    if (Ap->x0 < LEDI_WIDTH) {
        hsv.hue = Ap->hue;
        hsv.val = Ap->scale;
        hsv.sat = 240;
        crgb = hsv;
        backgroundLayer.fillRectangle(Ap->x0, Ap->y0, Ap->x1, Ap->y1, (rgb24)crgb);
        Ap->x1++;
        Ap->x0 = Ap->x1 - Ap->counter;
        Ap->hue += Ap->speed;
    }


    backgroundLayer.unregisterWriteCallback();
}

    void NETMGR_LineSwipeL(AniParms *Ap)
    {

    }

    void NETMGR_LineSwipeU(AniParms *Ap)
    {

    }

    void NETMGR_LineSwipeD(AniParms *Ap)
    {

    }

    void NETMGR_CircleCenter(AniParms *Ap)
    {

    }
#endif
