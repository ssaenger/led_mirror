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
static AudioInputUSB       usb1;
static AudioOutputI2S      i2s1; // Needed
static AudioMixer4         mixer1;
static AudioAnalyzeFFT1024 fft1024;
static AudioConnection patchCord1(usb1, 0, mixer1, 0);
static AudioConnection patchCord2(usb1, 1, mixer1, 1);
static AudioConnection patchCord3(mixer1, fft1024);

static uint16_t fftBins[SM_WIDTH];
static float levelThreshVert[SM_HEIGHT];
static float levelThreshHoriz[SM_WIDTH];
static uint8_t shownBinLevel[SM_WIDTH];

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
    AS_CalcFftBins(SM_WIDTH);
    AS_CalcLevelThresh(AS_MAX_LEVEL, AS_DYNAMIC_RANGE, AS_LINEAR_BLEND);
    for (i = 0; i < SM_WIDTH; i += 2) {
        shownBinLevel[i] = 0;
        shownBinLevel[i + 1] = 0;
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

    if ((Bands > SM_WIDTH) || (Bands < 1)) {
        Serial.println("Invalid Bands level");
        return;
    }

    e = FindE(Bands, AS_FFT_BIN_COUNT);  // Find calculated E value
    if (e) {                             // If a value was returned continue
        Serial.printf("E = %4.4f\n", e); // Print calculated E value
        for (b = 0; b < Bands; b++) {    // Test and print the bins from the calculated E
            n = pow(e, b);
            d = int(n + 0.5);
#if PRINT_FFT_DEBUG == 1
            Serial.printf("%4d ", b); // Print low bin
#endif /* PRINT_FFT_DEBUG == 1 */
            count += d - 1;
            count++;
            fftBins[b] = count - 1;
#if PRINT_FFT_DEBUG == 1
            Serial.printf("%4d\r\n", fftBins[b]); // Print high bin
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
    uint8_t x, y;
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
    for (y = 0; y < SM_HEIGHT; y++) {
        n                                  = (float)y / (float)(SM_HEIGHT - 1);
        logLevel                           = pow10f(n * -1.0 * (DynamicRange / 20.0));
        linearLevel                        = 1.0 - n;
        linearLevel                        = linearLevel * LinearBlend;
        logLevel                           = logLevel * (1.0 - LinearBlend);
        levelThreshVert[SM_HEIGHT - y - 1] = (logLevel + linearLevel) * MaxLevel;
#if PRINT_FFT_DEBUG == 1
        Serial.print(SM_HEIGHT - y - 1);
        Serial.print(": ");
        Serial.print(levelThreshVert[SM_HEIGHT - y - 1], 5);
        Serial.print("\r\n");
#endif /* PRINT_FFT_DEBUG == 1 */
    }

    // Compute horizontal values (for when LED_state is showing bars going
    // side-to-side)
    for (x = 0; x < SM_WIDTH; x++) {
        n                                  = (float)x / (float)(SM_WIDTH - 1);
        logLevel                           = pow10f(n * -1.0 * (DynamicRange / 20.0));
        linearLevel                        = 1.0 - n;
        linearLevel                        = linearLevel * LinearBlend;
        logLevel                           = logLevel * (1.0 - LinearBlend);
        levelThreshHoriz[SM_WIDTH - x - 1] = (logLevel + linearLevel) * MaxLevel;
#if PRINT_FFT_DEBUG == 1
        Serial.print(SM_WIDTH - x - 1);
        Serial.print(": ");
        Serial.print(levelThreshHoriz[SM_WIDTH - x - 1], 5);
        Serial.print("\r\n");
#endif /* PRINT_FFT_DEBUG == 1 */
    }
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
void AS_PlotFftTop(AniParms *Ap, AniType At)
{
    float level;
    int16_t x, y;
    uint16_t prevFreqBin;
    uint8_t i;

    CHSV hsv;
    CRGB crgb;
    const CRGB crgbBlack = CRGB::Black;

    hsv.hue = Ap->hue;
    hsv.val = 255;
    hsv.sat = Ap->maxBright;
    crgb = hsv;

    if (fft1024.available()) {
        Ap->hue += Ap->speed;

        prevFreqBin = 0;
        for (x = 0; x < SM_WIDTH; x++) {
            //Serial.printf("Reading %d, %d\r\n",prevFreqBin, fftBins[x]);
            level = fft1024.read(prevFreqBin, fftBins[x]);

            if (Ap->aff & ANI_EFFECT_2) {
                hsv.hue = Ap->hue + x;
                crgb = hsv;
            } else {
                if (Ap->aff & ANI_EFFECT_3) {
                    /* Overide max level if previous value was greater */
                    level = max(levelThreshVert[((shownBinLevel[x] >= 3) ? shownBinLevel[x] - 3 : 0)], level);
                }
                hsv.hue = Ap->hue;
                crgb = hsv;
            }
            /* First row is always written */
            writePixel(Ap, At, pXY(x, 0), crgb);
            for (y = 1; y < SM_HEIGHT; y++) {

                if (level >= levelThreshVert[y]) {
                    if (Ap->aff & ANI_EFFECT_1) {
                        hsv.hue = Ap->hue + y;
                        crgb = hsv;
                    }
                    if (x == 0) {
                        //Serial.printf("writing to y = %d\n\r", y);
                    }
                    writePixel(Ap, At, pXY(x, y), crgb);
                } else {
                    /* Black out this Pixel. All pixels above are already blacked out so exit loop */
                        /* Need to black out all remain pixels since the last fft draw */
                    for (i = y; i <= shownBinLevel[x]; i++) {
                        writePixel(Ap, At, pXY(x, i), crgbBlack);
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
 *                   aff: ANI_EFFECT_1 - increase hue color along y-axis Can't be used with 2
 *                        ANI_EFFECT_2 - increase hue color along x-axis. Can't be used with 1.
 *                        ANI_EFFECT_3 - subtly apply a "fall" affect to each bin. This means
 *                                       instead of just removing a drawn bar in a bin when
 *                                       the fft data doesn't meet a certian threshold, the drawn
 *                                       bar will decrease in size until it goes to 0 or until
 *                                       fft data exceeds the size of the drawn bar.
 *                   maxBright: brightness of each drawn bar
 *                   speed: speed the hue color cycles. 0 means static hue color
 *              At - Type of animation (Recommendation: ANI_TYPE_FOREGROUND)
 *
 * Returns:     void
 */
void AS_PlotFftBottom(AniParms *Ap, AniType At)
{
    float level;
    int16_t x, y;
    uint16_t prevFreqBin;
    uint8_t i;

    CHSV hsv;
    CRGB crgb;
    const CRGB crgbBlack = CRGB::Black;

    hsv.hue = Ap->hue;
    hsv.val = 255;
    hsv.sat = Ap->maxBright;
    crgb = hsv;

    if (fft1024.available()) {
        Ap->hue += Ap->speed;

        prevFreqBin = 0;
        for (x = 0; x < SM_WIDTH; x++) {
            //Serial.printf("Reading %d, %d\r\n",prevFreqBin, fftBins[x]);
            level = fft1024.read(prevFreqBin, fftBins[x]);

            if (Ap->aff & ANI_EFFECT_2) {
                hsv.hue = Ap->hue + x;
                crgb = hsv;
            } else {
                if (Ap->aff & ANI_EFFECT_3) {
                    /* Overide max level if previous value was greater */
                    level = max(levelThreshVert[((shownBinLevel[x] >= 3) ? shownBinLevel[x] - 3 : 0)], level);
                }
                hsv.hue = Ap->hue;
                crgb = hsv;
            }
            /* First row is always written */
            writePixel(Ap, At, pXY_BL(x, 0), crgb);
            for (y = 1; y < SM_HEIGHT; y++) {

                if (level >= levelThreshVert[y]) {
                    if (Ap->aff & ANI_EFFECT_1) {
                        hsv.hue = Ap->hue + y;
                        crgb = hsv;
                    }
                    if (x == 0) {
                        //Serial.printf("writing to y = %d\n\r", y);
                    }
                    writePixel(Ap, At, pXY_BL(x, y), crgb);
                } else {
                    /* Black out this Pixel. All pixels above are already blacked out so exit loop */
                        /* Need to black out all remain pixels since the last fft draw */
                    for (i = y; i <= shownBinLevel[x]; i++) {
                        writePixel(Ap, At, pXY_BL(x, i), crgbBlack);
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
 *                 AS_PlotFftMidSpace()
 * --------------------------------------------------------------------------------------------
 * Description:
 *
 * Parameters:
 *
 * Returns:
 */
void AS_PlotFftMidSpace(AniParms *Ap, AniType At)
{
    float level;
    int16_t x, y;
    uint16_t prevFreqBin;

    CHSV hsv;
    CRGB crgb;
    const CRGB crgbBlack = CRGB::Black;

    hsv.hue = Ap->hue;
    hsv.val = 255;
    hsv.sat = 240;
    crgb    = hsv;
    Ap->hue += Ap->speed;

    if (fft1024.available()) {

        prevFreqBin = 0;
        for (x = 0; x < SM_WIDTH; x++) {
            //Serial.printf("Reading %d, %d\r\n",prevFreqBin, fftBins[x]);
            level = fft1024.read(prevFreqBin, fftBins[x]);
            for (y = 1; y < SM_HEIGHT / 2 - 1; y++) {

                if (level >= levelThreshVert[y * 2 + 1]) {
                    writePixel(Ap, At, pXY(x, y + SM_HEIGHT / 2 + 1), crgb);
                    writePixel(Ap, At, pXY_BL(x, y + SM_HEIGHT / 2 + 1), crgb);
                } else {
                    writePixel(Ap, At, pXY(x, y + SM_HEIGHT / 2 + 1), crgbBlack);
                    writePixel(Ap, At, pXY_BL(x, y + SM_HEIGHT / 2 + 1), crgbBlack);
                }
            }
            // fix always on LEDs
            writePixel(Ap, At, pXY(x, SM_HEIGHT / 2), crgb);
            writePixel(Ap, At, pXY_BL(x, SM_HEIGHT / 2), crgb);
            prevFreqBin = fftBins[x] + 1;

        }
    } else {
        writePixel(Ap, At, pXY_BL(Ap->hue % SM_WIDTH, SM_HEIGHT - 1), crgb);
    }
}

/* --------------------------------------------------------------------------------------------
 *                 AS_PlotFftMid()
 * --------------------------------------------------------------------------------------------
 * Description:
 *
 * Parameters:
 *
 * Returns:
 */
void AS_PlotFftMid(AniParms *Ap, AniType At)
{
    float level;
    int16_t x, y;
    uint16_t prevFreqBin;
    uint8_t yMax;

    CHSV hsv;
    CRGB crgb;
    const CRGB crgbBlack = CRGB::Black;

    hsv.hue = Ap->hue;
    hsv.val = 255;
    hsv.sat = Ap->maxBright;
    crgb = hsv;

    if (fft1024.available()) {
        Ap->hue += Ap->speed;

        prevFreqBin = 0;
        for (x = 0; x < SM_WIDTH; x++) {
            //Serial.printf("Reading %d, %d\r\n",prevFreqBin, fftBins[x]);
            level = fft1024.read(prevFreqBin, fftBins[x]);
            yMax = 1;

            // fix always on LEDs
            writePixel(Ap, At, pXY(x, SM_HEIGHT / 2), crgb);
            writePixel(Ap, At, pXY_BL(x, SM_HEIGHT / 2), crgb);
            for (y = 1; y < SM_HEIGHT / 2; y++) {

                if (level >= levelThreshVert[y * 2] || y <= shownBinLevel[x]) {
                    writePixel(Ap, At, pXY(x, y + SM_HEIGHT / 2), crgb);
                    writePixel(Ap, At, pXY_BL(x, y + SM_HEIGHT / 2), crgb);
                    yMax = y;
                } else {
                    writePixel(Ap, At, pXY(x, y + SM_HEIGHT / 2), crgbBlack);
                    writePixel(Ap, At, pXY_BL(x, y + SM_HEIGHT / 2), crgbBlack);
                    break;
                }
            }
            prevFreqBin = fftBins[x] + 1;
            shownBinLevel[x] = (Ap->aff & ANI_EFFECT_3) ? yMax - 1 : 0;

        }
    } else {
        writePixel(Ap, At, pXY_BL(Ap->hue % SM_WIDTH, SM_HEIGHT - 1), crgb);
        if (Ap->aff & ANI_EFFECT_3) {
            /* No FFT data, but lower the bin levels for that subtle fall affect. */
            for (x = 0; x < SM_WIDTH; x++) {
                writePixel(Ap, At, pXY(x, shownBinLevel[x] + SM_HEIGHT / 2 + 1), crgbBlack);
                writePixel(Ap, At, pXY_BL(x, shownBinLevel[x] + SM_HEIGHT / 2 + 1), crgbBlack);
                shownBinLevel[x] = shownBinLevel[x] > 0 ? shownBinLevel[x] - 1 : shownBinLevel[x];
            }
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
