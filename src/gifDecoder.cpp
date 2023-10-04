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

/* --------------------------------------------------------------------------------------------
 *  GLOBALS
 * --------------------------------------------------------------------------------------------
 */


GifDecoder<SM_WIDTH, SM_HEIGHT, 12> decoder;

static AniParms *ap_g;
static AniType at_g;

static int numGifs;


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

    return true;
}

void GIFDEC_Play(AniParms *Ap, AniType At)
{
    ap_g = Ap;
    at_g = At;

    if (Ap->counter != Ap->last) {
        /* Upper layer desires a new gif */
         if (openGifFilenameByIndex(GIFDEC_DIRECTORY, Ap->counter % numGifs) >= 0) {

            // start decoding, skipping to the next GIF if there's an error
            if(decoder.startDecoding() < 0) {
                Serial.printf("Could not select gif %d\r\n", Ap->counter);
                Ap->counter++;
                return;
            }

            Ap->last = Ap->counter;
        }     
    }

    if(decoder.decodeFrame(false) < 0) {
        Serial.printf("Could not play gif. Going to first\r\n");
        Ap->counter = 0;
    }
}

/* --------------------------------------------------------------------------------------------
 *  PRIVATE FUNCTIONS
 * --------------------------------------------------------------------------------------------
 */
void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue)
{
    writePixel(ap_g, at_g, pXY(x, y), {red, green, blue});
}

void updateScreenCallback(void)
{
}

void screenClearCallback(void)
{
}