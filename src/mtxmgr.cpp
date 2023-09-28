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

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, SM_WIDTH, SM_HEIGHT, SM_REFRESH_DEPTH, SM_DMA_BUFF_ROWS, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, SM_WIDTH, SM_HEIGHT, SM_COLOR_DEPTH, kBackgroundLayerOptions);

// For larger fonts, you need to allocate enough memory to hold the entire scrolling text bitmap (the bounds of all the characters in the string) in memory
// and this may be larger than kMatrixWidth*kMatrixHeight.  Use the SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER macro, which allows for setting the character bitmap
// to different size than the matrix bounds.  We allocate several times the default amount of memory here:
SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(scrollingLayer, SM_WIDTH, SM_HEIGHT, SM_WIDTH * 2, SM_HEIGHT, SM_COLOR_DEPTH, kScrollingLayerOptions);
AniType aniTypeArray[SM_NUM_LEDS];


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

    ANI_Init(aniTypeArray);
    AS_Init();


    //matrix.setRefreshRate(60);
    matrix.addLayer(&backgroundLayer);
    //matrix.addLayer(&foregroundLayer);
    matrix.addLayer(&scrollingLayer);
    matrix.begin();
    delay(10);

    matrix.setBrightness(SM_INITIAL_BRIGHTNESS);

    val = 0;

    //backgroundLayer.setDrawnPixelBuffer(ledBuff);
    //memset((void *)aniI.owners, ANI_TYPE_FREE, sizeof(aniI.owners));
    Serial.printf("aniTypeArray before: 0x%x\r\n", aniTypeArray[0]);
    ANI_Init(aniTypeArray);
    Serial.printf("aniTypeArray after: 0x%x\r\n", aniTypeArray[0]);

    InitNode(&aniPackArray[0].node);
    aniPackArray[0].funcp = ANIFUNC_Confetti;
    aniPackArray[0].type = ANI_TYPE_FOREGROUND;
    aniPackArray[0].parms.hue = 190;
    aniPackArray[0].parms.scale = 240;
    aniPackArray[0].parms.type = ANI_TYPE_SPECIAL_1;
    aniPackArray[0].parms.fpsLimit = 35;

    InitNode(&aniPackArray[1].node);
    aniPackArray[1].funcp = ANIFUNC_PlazInt;
    aniPackArray[1].type = ANI_TYPE_FOREGROUND;
    aniPackArray[1].parms.counter = 100;
    aniPackArray[1].parms.fpsLimit = 60;
    aniPackArray[1].parms.type = 0;

    InitNode(&aniPackArray[2].node);
    aniPackArray[2].funcp = ANIFUNC_Glitter;
    aniPackArray[2].type = ANI_TYPE_TRANS_SWIPE;
    aniPackArray[2].parms.color = CRGB::Black;
    aniPackArray[2].parms.p.trans.transTime = 3000;
    aniPackArray[2].parms.fpsLimit = 40;
    aniPackArray[2].parms.counter = 0;
    aniPackArray[2].parms.scale = 10;
    aniPackArray[2].parms.type = 0;

    InitNode(&aniPackArray[3].node);
    aniPackArray[3].funcp = [](AniParms *Ap, AniType At) {
        NETMGR_LineSwipeR(Ap, At);
    };
    //aniPackArray[3].funcp =         NETMGR_LineSwipeR;
    aniPackArray[3].type = ANI_TYPE_TRANS_SWIPE;
    aniPackArray[3].parms.hue = 0;
    aniPackArray[3].parms.fpsLimit = SM_WIDTH / 2; // about 2 seconds
    aniPackArray[3].parms.p.trans.transTime = 4000;
    aniPackArray[3].parms.speed = 3;
    aniPackArray[3].parms.scale = 0;
    aniPackArray[3].parms.type = 0;

    InitNode(&aniPackArray[4].node);
    aniPackArray[4].funcp = [](AniParms *Ap, AniType At) {
        AS_PlotFftTop(Ap, At);
    };
    aniPackArray[4].type = ANI_TYPE_FOREGROUND;
    aniPackArray[4].parms.hue = 0;
    aniPackArray[4].parms.speed = 1;
    aniPackArray[4].parms.fpsLimit = matrix.getRefreshRate();
    aniPackArray[4].parms.type = 0;

    InitNode(&aniPackArray[5].node);
    aniPackArray[5].funcp = AS_PlotFftBottom;
    aniPackArray[5].type = ANI_TYPE_FOREGROUND;
    aniPackArray[5].parms.hue = 0;
    aniPackArray[5].parms.speed = 1;
    aniPackArray[5].parms.maxBright = 200;
    aniPackArray[5].parms.aff = ANI_EFFECT_2 | ANI_EFFECT_3;
    aniPackArray[5].parms.fpsLimit = matrix.getRefreshRate();
    aniPackArray[5].parms.type = 0;

    InitNode(&aniPackArray[6].node);
    aniPackArray[6].funcp = AS_PlotFftMid;
    aniPackArray[6].type = ANI_TYPE_FOREGROUND;
    aniPackArray[6].parms.hue = 0;
    aniPackArray[6].parms.speed = 1;
    aniPackArray[6].parms.maxBright = 100;
    aniPackArray[6].parms.aff = ANI_EFFECT_3;
    aniPackArray[6].parms.fpsLimit = matrix.getRefreshRate();
    aniPackArray[6].parms.type = 0;

    ANI_AddAnimation(&aniPackArray[0], ANI_TYPE_FOREGROUND);
    ANI_AddAnimation(&aniPackArray[1], ANI_TYPE_FOREGROUND);
    //ANI_AddAnimation(&aniPackArray[2], ANI_TYPE_TRANS_SWIPE);
    ANI_AddAnimation(&aniPackArray[3], aniPackArray[3].type);
    ANI_AddAnimation(&aniPackArray[4], aniPackArray[4].type);
    ANI_AddAnimation(&aniPackArray[5], aniPackArray[5].type);
    ANI_AddAnimation(&aniPackArray[6], aniPackArray[6].type);

    ANI_QueueAnimation(&aniPackArray[6]);
    ANI_SwapAnimation();

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
#if 0
    while(backgroundLayer.isSwapPending());
    ledBuff = backgroundLayer.backBuffer();
    //ledBuff2 = foregroundLayer.backBuffer();
    Serial.println("Here0");
    memset(ledBuff, 0, sizeof(RGB) * SM_NUM_LEDS);
    //memset(ledBuff2, 0, sizeof(RGB) * SM_NUM_LEDS);
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
    ledBuff = backgroundLayer.backBuffer();

    if (ANI_DrawAnimationFrame(ledBuff) != 0) {
        backgroundLayer.swapBuffers();
        matrix.countFPS();      // print the loop() frames per second to Serial
    }
#if 0
    EVERY_N_MILLIS(8'000) {
        //val = (val + 1) % 2;
        //Serial.printf("Swapping!! %d\n\r", val);
        ANI_QueueAnimation(&aniPackArray[4]);
        ANI_QueueAnimation(&aniPackArray[3]);
        ANI_SwapAnimation();
    }
#endif
#endif
}


void MtxMgr::NETMGR_LineSwipeR(AniParms *Ap, AniType At)
{
    CHSV   hsv;
    CRGB   crgb;

    backgroundLayer.registerWriteCallback(Ap, At, [](void *ctx, uint16_t ctxVal, uint16_t pixNum, const rgb24 &color) {
        writePixel((AniParms*)ctx, ctxVal, pixNum, color);
    });

    if (Ap->value < SM_WIDTH) {
        hsv.hue = Ap->hue;
        hsv.val = Ap->scale;
        hsv.sat = 240;
        crgb = hsv;
        backgroundLayer.drawFastVLine(Ap->value, 0, SM_HEIGHT, (rgb24)crgb);
        Ap->value++;
        Ap->hue += Ap->speed;
    }

    backgroundLayer.unregisterWriteCallback();
}

