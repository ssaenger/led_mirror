/* ********************************************************************************************
 * gifDecoder.cpp
 *
 * Author: Shawn Saenger
 *
 * Created: Oct 1, 2023
 *
 * Description: Manages reading the SD card for gifs to play
 *
 * ********************************************************************************************
 */

#include "../inc/gifDecoder.hpp"
#include "../inc/FilenameFunctions.hpp"
#include <GifDecoder.h>

#define NUMBER_FULL_CYCLES   2

/* --------------------------------------------------------------------------------------------
 *  GLOBALS
 * --------------------------------------------------------------------------------------------
 */


GifDecoder<LEDI_WIDTH, LEDI_HEIGHT, 12> decoder;

//static AniParms *ap_g;

static int numGifs;

static CRGB *prevFrame;

// these variables keep track of when we're done displaying the last frame and are ready for a new frame
static uint32_t lastFrameDisplayTime = 0;
static unsigned int currentFrameDelay = 0;

// this is used for the profiling code unique to this sketch
static unsigned long cycleStartTime_millis;

// these variables keep track of when it's time to play a new GIF
static unsigned long displayStartTime_millis;

/* --------------------------------------------------------------------------------------------
 *  PROTOTYPES
 * --------------------------------------------------------------------------------------------
 */
static void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue);
static void updateScreenCallback(void);
static void screenClearCallback(void);

/* --------------------------------------------------------------------------------------------
 *  PUBLIC FUNCTIONS
 * --------------------------------------------------------------------------------------------
 */

bool GIFDEC_Init()
{
    decoder.setScreenClearCallback(screenClearCallback);
    decoder.setUpdateScreenCallback(updateScreenCallback);
    decoder.setDrawPixelCallback(drawPixelCallback);

    decoder.setFileSeekCallback(fileSeekCallback);
    decoder.setFilePositionCallback(filePositionCallback);
    decoder.setFileReadCallback(fileReadCallback);
    decoder.setFileReadBlockCallback(fileReadBlockCallback);
    decoder.setFileSizeCallback(fileSizeCallback);

    if(initFileSystem(GIFDEC_SD_CS) < 0) {
        Serial.println("No SD card");
        return false;
    }

        // Determine how many animated GIF files exist
    numGifs = enumerateGIFFiles(GIFDEC_DIRECTORY, true);

    if(numGifs < 0) {
        Serial.println("No gifs directory");
        return false;
    }

    if(numGifs == 0) {
        Serial.println("Empty gifs directory");
        return false;
    }

    prevFrame = (CRGB*)malloc(sizeof(CRGB) * LEDI_NUM_LEDS);
    if (prevFrame == 0) {
        Serial.println("Could not allocate memory for gifs");
        return false;
    }

    cycleStartTime_millis = 0;
    displayStartTime_millis = 0;

    return true;
}

void GIFDEC_Play(AniParms *Ap)
{
    uint16_t i;
    unsigned long now = millis();

    //ap_g = Ap;
    if((now - displayStartTime_millis) > (17 * 1000) || decoder.getCycleNumber() > NUMBER_FULL_CYCLES) {
        Ap->counter++;
        Serial.println("Swapping gifs");
    }

    if (Ap->counter != Ap->last) {
        /* Upper layer desires a new gif */
        Serial.println("calling openGifFilenameByIndex");
         if (openGifFilenameByIndex(GIFDEC_DIRECTORY, Ap->counter % numGifs) >= 0) {
            Serial.println("Called openGifFilenameByIndex");
            // start decoding, skipping to the next GIF if there's an error
            if(decoder.startDecoding() < 0) {
                Serial.printf("Could not select gif %d\r\n", Ap->counter);
                Ap->counter++;
                return;
            }

            // Calculate time in the future to terminate animation
            displayStartTime_millis = now;
            cycleStartTime_millis = now;
            Ap->last = Ap->counter;
        }     
    }

    //if((millis() - lastFrameDisplayTime) > currentFrameDelay) {
    EVERY_N_MILLISECONDS(95) {
        // we completed one pass of the GIF, print some stats
        if(decoder.getCycleNumber() > 0 && decoder.getFrameNumber() == 0) {
            // Print the stats for this GIF      
            char buf[80];
            int32_t frames       = decoder.getFrameCount();
            int32_t cycle_design = decoder.getCycleTime();  // Intended duration
            int32_t cycle_actual = now - cycleStartTime_millis;       // Actual duration
            int32_t percent = 100 * cycle_design / cycle_actual;
            sprintf(buf, "[%ld frames = %ldms] actual: %ldms speed: %ld%%",
                    frames, cycle_design, cycle_actual, percent);
            Serial.println(buf);

            cycleStartTime_millis = now;
        }
        if(decoder.decodeFrame(false) < 0) {
            Serial.printf("Could not play gif. Going to first\r\n");
            Ap->counter = 0;
        }
        lastFrameDisplayTime = now;
        currentFrameDelay = decoder.getFrameDelay_ms();
    }

    for (i = 0; i < LEDI_NUM_LEDS; i++) {
        ANI_WritePixel(Ap, i, prevFrame[i]);
    }

}

/* --------------------------------------------------------------------------------------------
 *  PRIVATE FUNCTIONS
 * --------------------------------------------------------------------------------------------
 */
void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue)
{
    //ANI_WritePixel(ap_g, pXY(x, y), CRGB(red, green, blue));
    prevFrame[pXY(x, y)] = CRGB(red, green, blue);
}

void updateScreenCallback(void)
{
}

void screenClearCallback(void)
{
}