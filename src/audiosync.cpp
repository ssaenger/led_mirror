/* ********************************************************************************************
 * audiosync.cpp
 *
 * Author: Shawn Saenger
 *
 * Created: Sep 20, 2023
 *
 * Description: Audio Sync Implementation. Uses the PJRC audio library to analyze an audio
 * stream using FFT and plotting the analysis to an LED matrix.
 *
 * ********************************************************************************************
 */
#include "../inc/audiosync.hpp"
#include "../inc/smartmtxconfig.h"
#include <Arduino.h>

#include <math.h>

#define PRINT_FFT_DEBUG 1

/* --------------------------------------------------------------------------------------------
 *  GLOBALS
 * --------------------------------------------------------------------------------------------
 */

// FFT definitions. Audio comes from computer through USB
static AudioInputUSB        usb1;
static AudioOutputI2S       i2s1; // Needed
static AudioMixer4          mixer1;
static AudioAnalyzePeak     peak;
static AudioAnalyzeFFT1024  fft1024;
static AudioConnection patchCord1(usb1, 0, mixer1, 0);
static AudioConnection patchCord2(usb1, 1, mixer1, 1);
static AudioConnection patchCord3(mixer1, fft1024);
static AudioConnection patchCord4(mixer1, peak);

static uint16_t fftBins[LEDI_WIDTH];
static float levelThreshVert[LEDI_HEIGHT];
//static float levelThreshHoriz[LEDI_WIDTH];
static uint8_t shownBinLevel[LEDI_WIDTH];

/* --------------------------------------------------------------------------------------------
 *  PROTOTYPES
 * --------------------------------------------------------------------------------------------
 */

static float FindE(uint16_t Bands, uint16_t Bins);

/* --------------------------------------------------------------------------------------------
 *  PUBLIC FUNCTIONS
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 *                 AS_Init()
 * --------------------------------------------------------------------------------------------
 * Description:    Allocates memory and calls AS_CalcFftBins() to calculate the bins for the
 *                 matrix width. Calls AS_CalcLevelThresh() afterwards. Must only be called
 *                 once.
 *
 * Parameters:     void
 *
 * Returns:        void
 */
void AS_Init()
{
    uint16_t i;
    AudioMemory(18);
    AS_CalcFftBins(LEDI_WIDTH);
    AS_CalcLevelThresh(AS_MAX_LEVEL, AS_DYNAMIC_RANGE, AS_LINEAR_BLEND);
    for (i = 0; i < LEDI_WIDTH; i++) {
        shownBinLevel[i] = 0;
    }
}

/* --------------------------------------------------------------------------------------------
 *                 AS_CalcFftBins()
 * --------------------------------------------------------------------------------------------
 * Description:    Calculates the Bin selection for a given number of Bands.
 *                 See https://forum.pjrc.com/threads/32677
 *
 * Parameters:     Bands - The number of bands to distribute the bins into. Can not
 *                         not exceed the number of columns in the LED matrix
 *
 * Returns:        void
 */
void AS_CalcFftBins(uint16_t Bands)
{
    float e, n;
    int b, count = 0, d;

    if ((Bands > LEDI_WIDTH) || (Bands < 1)) {
        Serial.println("Invalid Bands level");
        return;
    }

    e = FindE(Bands, AS_FFT_BIN_COUNT + 20);  // Find calculated E value
    if (e) {                             // If a value was returned continue
        Serial.printf("E = %4.4f\n", e); // Print calculated E value
        for (b = 0; b < Bands; b++) {    // Test and print the bins from the calculated E
            n = pow(e, b);
            d = int(n + 0.5);
            count += d - 1;
            count++;
            fftBins[b] = count - 1;
#if PRINT_FFT_DEBUG == 1
            Serial.printf("band: %4d -> # of bins %4d. Total bins: %4d -> %fHz\r\n", b,
                                                        (b == 0 ? fftBins[b] : fftBins[b] - fftBins[b - 1]),
                                                        fftBins[b],
                                                        (43.0664) * fftBins[b]); // Print high bin
#endif /* PRINT_FFT_DEBUG == 1 */
        }
        if (fftBins[b - 1] > 511) { fftBins[b - 1] = 511; }
    } else {
        Serial.println("Error finding E..."); // Error, something happened
    }
}

/* --------------------------------------------------------------------------------------------
 *                 AS_CalcLevelThresh()
 * --------------------------------------------------------------------------------------------
 * Description:    Computes the vertical and horizontal levels.
 *
 * Parameters:     MaxLevel - 1.0 is max, lower is more "sensitive"
 *                 DynamicRange - Smaller number means harder to overcome init thresh
 *                 LinearBlend - useful range is 0 to 0.7
 *
 * Returns:        void
 */
void AS_CalcLevelThresh(float MaxLevel, float DynamicRange, float LinearBlend)
{
    uint8_t y;
    float n, logLevel, linearLevel;

#if PRINT_FFT_DEBUG == 1
    Serial.print("maxLevel = ");
    Serial.println(MaxLevel);
    Serial.print("dynamicRange = ");
    Serial.println(DynamicRange);
    Serial.print("linearBlend = ");
    Serial.println(LinearBlend);
    Serial.println("---------------------");
#endif /* PRINT_FFT_DEBUG == 1 */

    // Compute vertical values (for when LED_state is showing bars going up-down)
    for (y = 0; y < LEDI_HEIGHT; y++) {
        n                                  = (float)y / (float)(LEDI_HEIGHT - 1);
        logLevel                           = pow10f(n * -1.0 * (DynamicRange / 20.0));
        linearLevel                        = 1.0 - n;
        linearLevel                        = linearLevel * LinearBlend;
        logLevel                           = logLevel * (1.0 - LinearBlend);
        levelThreshVert[LEDI_HEIGHT - y - 1] = (logLevel + linearLevel) * MaxLevel;
#if PRINT_FFT_DEBUG == 1
        Serial.printf("Thresh: y %2d: %f\n\r", LEDI_HEIGHT - y - 1,
                    levelThreshVert[LEDI_HEIGHT - y - 1]);
#endif /* PRINT_FFT_DEBUG == 1 */
    }
#if 0
    // Compute horizontal values (for when LED_state is showing bars going
    // side-to-side)
    for (x = 0; x < LEDI_WIDTH; x++) {
        n                                  = (float)x / (float)(LEDI_WIDTH - 1);
        logLevel                           = pow10f(n * -1.0 * (DynamicRange / 20.0));
        linearLevel                        = 1.0 - n;
        linearLevel                        = linearLevel * LinearBlend;
        logLevel                           = logLevel * (1.0 - LinearBlend);
        levelThreshHoriz[LEDI_WIDTH - x - 1] = (logLevel + linearLevel) * MaxLevel;
#if PRINT_FFT_DEBUG == 1
        Serial.printf("Thresh: x %2d: %f\n\r", LEDI_WIDTH - x - 1,
                    levelThreshHoriz[LEDI_WIDTH - x - 1]);
#endif /* PRINT_FFT_DEBUG == 1 */
    }
#endif
}

/* --------------------------------------------------------------------------------------------
 *                 AS_PlotFftTop()
 * --------------------------------------------------------------------------------------------
 * Description:
 *
 * Parameters:
 *
 * Returns:
 */
void AS_PlotFftTop(AniParms *Ap)
{
    float level;
    int16_t x, y;
    uint16_t prevFreqBin;
    uint8_t i;
    uint8_t j, w;
    uint8_t hue;
    uint16_t speed;

    CHSV hsvX;
    CHSV hsvY;
    CRGB crgb;
    const CRGB crgbBlack = CRGB::Black;

    Ap->counter = (Ap->counter >= Ap->fpsTarg) ? 0 : Ap->counter + Ap->scale;
    if (Ap->counter == 0) {
        Ap->hsv.hue += 1;
    }
    hue = Ap->hsv.h;
    hsvX = Ap->hsv;
    hsvY = hsvX;
    speed = Ap->speed;
    w = ((Ap->mod & ANI_MOD_4) == ANI_MOD_4) ? Ap->size : 1;

    if (fft1024.available()) {

        prevFreqBin = 0;
        for (x = 0; x < LEDI_WIDTH; x += w) {
            //Serial.printf("Reading %d, %d\r\n",prevFreqBin, fftBins[x]);
            level = fft1024.read(prevFreqBin, fftBins[x]);

            if (Ap->mod & ANI_MOD_3) {
                /* Overide max level if previous value was greater */
                level = max(levelThreshVert[((shownBinLevel[x] >= 3) ? shownBinLevel[x] - 3 : 0)], level);
            }

            if (Ap->mod & ANI_MOD_2) {
                hsvX.hue = hue + (x * speed);
            }
            crgb = hsvX;
            /* First row is always written */
            //ANI_WritePixel(Ap, pXY(x, 0), crgb);
            for (j = 0; j < w; j++) { ANI_WritePixel(Ap, pXY(x + j, 0), crgb); }
            for (y = 1; y < LEDI_HEIGHT; y++) {

                if (level >= levelThreshVert[y]) {
                    if (Ap->mod & ANI_MOD_1) {
                        hsvY.hue = hsvX.hue + ((y) * speed);
                    }
                    crgb = hsvY;
                    //ANI_WritePixel(Ap, pXY(x, y), crgb);
                    for (j = 0; j < w; j++) { ANI_WritePixel(Ap, pXY(x + j, y), crgb); }
                } else {
                    /* Black out this Pixel. All pixels above are already blacked out so exit loop */
                    /* Need to black out all remain pixels since the last fft draw */
                    for (i = y; i <= shownBinLevel[x]; i++) {
                        //ANI_WritePixel(Ap, pXY(x, i), crgbBlack);
                        for (j = 0; j < w; j++) { ANI_WritePixel(Ap, pXY(x + j, i), crgbBlack); }
                    }
                    break;
                }
            }
            shownBinLevel[x] = (y > 1) ? y - 1 : 0;
            prevFreqBin = fftBins[x] + 1;

        }
    }
}

/* --------------------------------------------------------------------------------------------
 *                 AS_PlotFftBottom()
 * --------------------------------------------------------------------------------------------
 * Description: Reads FFT data from an audio stream and displays the frequencies shooting
 *              up from the bottom of the matrix
 *
 * Parameters:  Ap - Pointer to AniParms data where:
 *                   hue: specifies hue color (rgb color from the color wheel)
 *                   aff: ANI_MOD_1 - increase hue color along y-axis Can't be used with 2
 *                        ANI_MOD_2 - increase hue color along x-axis. Can't be used with 1.
 *                        ANI_MOD_3 - subtly apply a "fall" affect to each bin. This means
 *                                    instead of just removing a drawn bar in a bin when
 *                                    the fft data doesn't meet a certian threshold, the drawn
 *                                    bar will decrease in size until it goes to 0 or until
 *                                    fft data exceeds the size of the drawn bar.
 *                   maxBright: brightness of each drawn bar
 *                   speed: speed the hue color cycles. 0 means static hue color
 *              At - Type of animation (Recommendation: ANI_TYPE_FOREGROUND)
 *
 * Returns:     void
 */
void AS_PlotFftBottom(AniParms *Ap)
{
    float level;
    int16_t x, y, i;
    uint16_t prevFreqBin;
    uint8_t hue;

    CHSV hsv;
    CRGB crgb;
    const CRGB crgbBlack = CRGB::Black;

    hue = Ap->hsv.h;
    hsv = Ap->hsv;

    if (fft1024.available()) {
        hue += Ap->speed;

        prevFreqBin = 0;
        for (x = 0; x < LEDI_WIDTH; x++) {
            //Serial.printf("Reading %d, %d\r\n",prevFreqBin, fftBins[x]);
            level = fft1024.read(prevFreqBin, fftBins[x]);

            if (Ap->mod & ANI_MOD_3) {
                /* Overide max level if previous value was greater */
                level = max(levelThreshVert[((shownBinLevel[x] >= 3) ? shownBinLevel[x] - 3 : 0)], level);
            }

            if (Ap->mod & ANI_MOD_2) {
                hsv.hue = hue + x;
            } else {
                hsv.hue = hue;
            }
            crgb = hsv;
            /* First row is always written */
            ANI_WritePixel(Ap, pXY_BL(x, 0), crgb);
            for (y = 1; y < LEDI_HEIGHT; y++) {

                if (level >= levelThreshVert[y]) {
                    if (Ap->mod & ANI_MOD_1) {
                        hsv.hue = hue + y;
                        crgb = hsv;
                    }
                    ANI_WritePixel(Ap, pXY_BL(x, y), crgb);
                } else {
                    /* Black out this Pixel. All pixels above are already blacked out so exit loop */
                        /* Need to black out all remain pixels since the last fft draw */
                    for (i = y; i <= shownBinLevel[x]; i++) {
                        ANI_WritePixel(Ap, pXY_BL(x, i), crgbBlack);
                    }
                    break;
                }
            }
            shownBinLevel[x] = (y > 1) ? y - 1 : 0;
            prevFreqBin = fftBins[x] + 1;

        }
    }
}

/* --------------------------------------------------------------------------------------------
 *                 AS_PlotFftMid()
 * --------------------------------------------------------------------------------------------
 * Description: Reads FFT data from an audio stream and displays the frequencies shooting
 *              from the center vertically, up and down
 *
 * Parameters:  Ap - Pointer to AniParms data where:
 *                   hue: specifies hue color (rgb color from the color wheel)
 *                   speed: Combined with ANI_MOD_1 and ANI_MOD_2. Determines how
 *                          much the color changes along the x and y-axis.
 *                   scale: A higher scale means a change in color over time. This is done
 *                          by increasing the hue value. This leads to a changing rainbow
 *                          effect. Set to 0 to keep hue the same.
 *                   counter: Internal counter. Set to 1. Used with scale.
 *                   maxBright: brightness of each drawn bar
 *                   aff: ANI_MOD_1 - increase hue color along y-axis
 *                        ANI_MOD_2 - increase hue color along x-axis.
 *                        ANI_MOD_3 - subtly apply a "fall" affect to each bin. This means
 *                                    instead of just removing a drawn bar in a bin when
 *                                    the fft data doesn't meet a certian threshold, the drawn
 *                                    bar will decrease in size until it goes to 0 or until
 *                                    fft data exceeds the size of the drawn bar.
 *              At - Type of animation (Recommendation: ANI_TYPE_FOREGROUND)
 *
 * Returns:     void
 */
void AS_PlotFftMid(AniParms *Ap)
{
    float level;
    int16_t x, y, i;
    uint16_t prevFreqBin;
    uint16_t offset, speed;
    uint8_t hue;

    CHSV hsvX;
    CHSV hsvY;
    CRGB crgb;
    const CRGB crgbBlack = CRGB::Black;

    // Roughly Ap->hue goes up by 1 every Ap->scale per second
    Ap->counter = (Ap->counter >= Ap->fpsTarg) ? 0 : Ap->counter + Ap->scale;
    if (Ap->counter == 0) {
        Ap->hsv.hue += 1;
    }
    hue = Ap->hsv.h;
    speed = Ap->speed;
    offset = Ap->offset;
    hsvX = Ap->hsv;
    hsvY = hsvX;

    if (fft1024.available()) {

        prevFreqBin = 0;
        for (x = 0; x < LEDI_WIDTH; x++) {
            //Serial.printf("Reading %d, %d\r\n",prevFreqBin, fftBins[x]);
            level = fft1024.read(prevFreqBin, fftBins[x]);

            if (Ap->mod & ANI_MOD_3) {
                /* Overide max level if previous value was greater */
                level = max(levelThreshVert[((shownBinLevel[x] >= 2) ? ((shownBinLevel[x] - 2) * 2) : 0)], level);
            }

            if (Ap->mod & ANI_MOD_2) {
                hsvX.hue = hue + (x * speed);
            }
            crgb = hsvX;

            // Mid 2 rows are always on
            ANI_WritePixel(Ap, pXY(x, LEDI_HEIGHT / 2), crgb);
            ANI_WritePixel(Ap, pXY_BL(x, LEDI_HEIGHT / 2), crgb);
            for (y = 1; (y < (offset + 1)) && (y < LEDI_HEIGHT / 2) ; y++) {
                ANI_WritePixel(Ap, pXY(x, y + LEDI_HEIGHT / 2), crgbBlack);
                ANI_WritePixel(Ap, pXY_BL(x, y + LEDI_HEIGHT / 2), crgbBlack);
            }
            for (y = 1; y < LEDI_HEIGHT / 2 - offset; y++) {

                if (level >= levelThreshVert[y * 2 + offset]) {
                    if (Ap->mod & ANI_MOD_1) {
                        hsvY.hue = hsvX.hue + ((y * 2) * speed);
                    }
                    crgb = hsvY;

                    ANI_WritePixel(Ap, pXY(x, y + LEDI_HEIGHT / 2 + offset), crgb);
                    ANI_WritePixel(Ap, pXY_BL(x, y + LEDI_HEIGHT / 2 + offset), crgb);
                } else {
                    for (i = y; i <= shownBinLevel[x]; i++) {
                        ANI_WritePixel(Ap, pXY(x, i + LEDI_HEIGHT / 2 + offset), crgbBlack);
                        ANI_WritePixel(Ap, pXY_BL(x, i + LEDI_HEIGHT / 2 + offset), crgbBlack);
                    }
                    break;
                }
            }
            prevFreqBin = fftBins[x] + 1;
            shownBinLevel[x] = ((y > 1) ? y - 1 : 0);
        }
    }
}

/* --------------------------------------------------------------------------------------------
 *                 AS_RainbowIris()
 * --------------------------------------------------------------------------------------------
 * Description:    
 *
 * Parameters:     
 *
 * Returns:        
 */
void AS_RainbowIris(AniParms *Ap)
{
    // FastLED's built-in rainbow generator
    CHSV   hsvX, hsvY;
    CRGB   crgb;
    uint16_t x, y;
    uint16_t scale = Ap->scale;
    float  peakVal = 0;
    uint8_t peakU8 = 0;

    if (peak.available()) {
        hsvX = Ap->hsv;
        hsvY = hsvX;
        peakVal = peak.read();
        peakU8 = peakVal * 10.0;

        for (x = 0; x < LEDI_WIDTH / 2; x++) {
            if ((x % 3) == 0) {
                scale++;
            }
            //hsvX.h += scale >> 2;
            hsvY = hsvX;
            for (y = 0; y < LEDI_HEIGHT / 2; y++) {
                if ((y % 3) == 0) {
                    hsvY.h += scale;
                }                
                crgb = hsvY;
                ANI_WritePixel(Ap, pXY(x, y), crgb);
                ANI_WritePixel(Ap, pXY(x, LEDI_HEIGHT - 1 - y), crgb);
                ANI_WritePixel(Ap, pXY(LEDI_WIDTH - 1 - x, y), crgb);
                ANI_WritePixel(Ap, pXY(LEDI_WIDTH - 1 - x, LEDI_HEIGHT - 1 - y), crgb);
            }
        }
    }
    EVERY_N_MILLISECONDS(40) {
        Serial.println(peakU8);
        if (peakU8 < 1) {
            Ap->hsv.h += 1;    
        } else {
            Ap->hsv.h += peakU8 << 1;
        }
    }
}

/* --------------------------------------------------------------------------------------------
 *  PRIVATE FUNCTIONS
 * --------------------------------------------------------------------------------------------
 */

float FindE(uint16_t Bands, uint16_t Bins)
{
    float increment = 0.1, eTest, n;
    int b, count, d;

    for (eTest = 1; eTest < Bins; eTest += increment) { // Find E through brute force calculations
        count = 0;
        for (b = 0; b < Bands; b++) { // Calculate full log values
            n = pow(eTest, b);
            d = int(n + 0.5);
            count += d;
        }
        if (count > Bins) {         // We calculated over our last bin
            eTest -= increment;     // Revert back to previous calculation increment
            increment /= 10.0;      // Get a finer detailed calculation & increment a decimal point lower
        } else if (count == Bins) { // We found the correct E
            return eTest;           // Return calculated E
        }
        if (increment < 0.0000001) { // Ran out of calculations. Return previous E. Last bin will be lower than (bins-1)
            return (eTest - increment);
        }
    }
    return 0; // Return error 0
}
