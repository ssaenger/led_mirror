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

// All these libraries are required for the Teensy Audio Library
#include <Audio.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <Wire.h>

#include "../inc/mtxmgr.hpp"
#include "../inc/smartmtxconfig.h"

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, SM_WIDTH, SM_HEIGHT, SM_REFRESH_DEPTH, SM_DMA_BUFF_ROWS, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, SM_WIDTH, SM_HEIGHT, SM_COLOR_DEPTH, kBackgroundLayerOptions);

// For larger fonts, you need to allocate enough memory to hold the entire scrolling text bitmap (the bounds of all the characters in the string) in memory
// and this may be larger than kMatrixWidth*kMatrixHeight.  Use the SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER macro, which allows for setting the character bitmap
// to different size than the matrix bounds.  We allocate several times the default amount of memory here:
SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(scrollingLayer, SM_WIDTH, SM_HEIGHT, SM_WIDTH * 2, SM_HEIGHT, SM_COLOR_DEPTH, kScrollingLayerOptions);

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
    //matrix.setRefreshRate(60);
    matrix.addLayer(&backgroundLayer);
    matrix.addLayer(&scrollingLayer);
    matrix.begin();
    delay(10);

    matrix.setBrightness(SM_INITIAL_BRIGHTNESS);

    val = 0;

    //backgroundLayer.setDrawnPixelBuffer(ledBuff);
    //memset((void *)aniI.owners, ANI_TYPE_FREE, sizeof(aniI.owners));
    ANI_Init();

    InitNode(&aniPackArray[0].node);
    aniPackArray[0].funcp = ANIFUNC_Confetti;
    aniPackArray[0].type = ANI_TYPE_FOREGROUND;
    aniPackArray[0].parms.hue = 190;
    aniPackArray[0].parms.scale = 240;
    aniPackArray[0].parms.type = ANI_TYPE_SPECIAL_1;
    aniPackArray[0].parms.fpsLimit = 100;

    InitNode(&aniPackArray[1].node);
    aniPackArray[1].funcp = ANIFUNC_PlazInt;
    aniPackArray[1].type = ANI_TYPE_FOREGROUND;
    aniPackArray[1].parms.counter = 100;
    aniPackArray[1].parms.fpsLimit = 35;
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

    ANI_AddAnimation(&aniPackArray[0], ANI_TYPE_FOREGROUND);
    ANI_AddAnimation(&aniPackArray[1], ANI_TYPE_FOREGROUND);
    ANI_AddAnimation(&aniPackArray[2], ANI_TYPE_TRANS_SWIPE);

    ANI_QueueAnimation(&aniPackArray[val]);
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
    while(backgroundLayer.isSwapPending());
    ledBuff = backgroundLayer.backBuffer();

    if (ANI_DrawAnimationFrame(ledBuff) != 0) {
        backgroundLayer.swapBuffers();
        matrix.countFPS();      // print the loop() frames per second to Serial
    }

    EVERY_N_MILLIS(8'000) {
        val = (val + 1) % 2;
        Serial.printf("Swapping!! %d\n\r", val);
        ANI_QueueAnimation(&aniPackArray[val]);
        ANI_QueueAnimation(&aniPackArray[2]);
        ANI_SwapAnimation();
    }
}


