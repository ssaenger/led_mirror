
/* ********************************************************************************************
 * gifDecoder.cpp
 *
 * Author: Shawn Saenger
 *
 * Created: Oct 1, 2023
 *
 * Description: ANIMartRIX implementation for this project. Modified code from
 *              Stefan Petrick https://github.com/StefanPetrick/animartrix
 *
 * ********************************************************************************************
 */

#include "../inc/animatrix.hpp"

/* --------------------------------------------------------------------------------------------
 *  MACROS
 * --------------------------------------------------------------------------------------------
 */
#define NOISE_FADE(t) ((t) * (t) * (t) * ((t) * ((t) * 6 - 15) + 10))
#define NOISE_LERP(t, a, b) ((a) + (t) * ((b) - (a)))
/* --------------------------------------------------------------------------------------------
 * NUM_OSCILLATORS define
 *
 * 
 */
#define NUM_OSCILLATORS 10
/* --------------------------------------------------------------------------------------------
 *  GLOBALS
 * --------------------------------------------------------------------------------------------
 */

static float *polar_theta;        // look-up table for polar angles
static float *distance;           // look-up table for polar distances

static unsigned long a, b, c;                  // for time measurements

typedef struct _RenderParameters {

    float center_x;                 // center of the matrix
    float center_y;
    float dist, angle;                
    float scale_x;                  // smaller values = zoom in
    float scale_y;
    float scale_z;       
    float offset_x, offset_y, offset_z;     
    float z;  
    float low_limit;                 // getting contrast by highering the black point
    float high_limit;                                            
} RenderParameters;

static RenderParameters animation;     // all animation parameters in one place

typedef struct _Oscillators {

    float master_speed;            // global transition speed
    float offset[NUM_OSCILLATORS]; // oscillators can be shifted by a time offset
    float ratio[NUM_OSCILLATORS];  // speed ratios for the individual oscillators                                  
} Oscillators;

static Oscillators timings;             // all speed settings in one place

typedef struct _Modulators {  

    float linear[NUM_OSCILLATORS];        // returns 0 to FLT_MAX
    float radial[NUM_OSCILLATORS];        // returns 0 to 2*PI
    float directional[NUM_OSCILLATORS];   // returns -1 to 1
    float noise_angle[NUM_OSCILLATORS];   // returns 0 to 2*PI        
} Modulators;

Modulators move;                 // all oscillator based movers and shifters at one place

typedef struct _AnimaxRgb {

  float red, green, blue;
} AnimaxRgb;
AnimaxRgb pixel;

static const byte p[] = {   151,160,137,91,90, 15,131, 13,201,95,96,
53,194,233, 7,225,140,36,103,30,69,142, 8,99,37,240,21,10,23,190, 6,
148,247,120,234,75, 0,26,197,62,94,252,219,203,117, 35,11,32,57,177,
33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,134,139,
48,27,166, 77,146,158,231,83,111,229,122, 60,211,133,230,220,105,92,
41,55,46,245,40,244,102,143,54,65,25,63,161, 1,216,80,73,209,76,132,
187,208, 89, 18,169,200,196,135,130,116,188,159, 86,164,100,109,198,
173,186, 3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,
212,207,206, 59,227, 47,16,58,17,182,189, 28,42,223,183,170,213,119,
248,152,2,44,154,163,70,221,153,101,155,167,43,172, 9,129,22,39,253,
19,98,108,110,79,113,224,232,178,185,112,104,218,246, 97,228,251,34,
242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,
150,254,138,236,205, 93,222,114, 67,29,24, 72,243,141,128,195,78,66,
215,61,156,180
};

/* --------------------------------------------------------------------------------------------
 *  PROTOTYPES
 * --------------------------------------------------------------------------------------------
 */
static void AnimaxCalculateOscillators(Oscillators &Timings);
static float AnimaxRenderValue(RenderParameters &Animation);
static void AnimaxRenderPolarLookupTable(float cx, float cy);
static float AnimaxMapFloat(float x, float in_min, float in_max, float out_min, float out_max);
static AnimaxRgb AnimaxRgbSanityCheck(AnimaxRgb &Pixel);

static float AnimaxNoiseGrad(int hash, float x, float y, float z);
static float AnimaxPnoise(float x, float y, float z);
static void AnimaxRunDefaultOscillators();
static void AnimaxReportPerformance();

/* --------------------------------------------------------------------------------------------
 *  PUBLIC FUNCTIONS
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 *                 ANIMAX_Init()
 * --------------------------------------------------------------------------------------------
 * Description:    Initializes this layer. Must be the very first function to be called and
 *                 only call once or you can crash the system!
 *
 * Parameters:     None
 *
 * Returns:        true if successful. false otherwise.
 */
bool ANIMAX_Init()
{
    polar_theta = (float*)malloc(sizeof(float) * LEDI_NUM_LEDS);
    if (polar_theta == 0) {
        Serial.println("Could not allocate memory for animatrix");
        return false;
    }
    distance = (float*)malloc(sizeof(float) * LEDI_NUM_LEDS);
    if (distance == 0) {
        Serial.println("Could not allocate memory for animatrix");
        return false;
    }

    animation.center_x = (LEDI_WIDTH / 2) - 0.5;
    animation.center_y = (LEDI_HEIGHT / 2) - 0.5;
    animation.dist = 0;
    animation.angle = 0;
    animation.scale_x = 0.1;  // smaller values = zoom in
    animation.scale_y = 0.1;
    animation.scale_z = 0.1;       
    animation.offset_x = 0;  
    animation.offset_y = 0;
    animation.offset_z = 0;
    animation.low_limit  = 0; // getting contrast by highering the black point
    animation.high_limit = 1;


    AnimaxRenderPolarLookupTable(animation.center_x, animation.center_y);

    return true;
}

/* --------------------------------------------------------------------------------------------
 *                 ANIMAX_Init()
 * --------------------------------------------------------------------------------------------
 * Description:    
 *
 * Parameters:     
 *
 * Returns:        void
 */
void ANIMAX_Lava1(AniParms *Ap)
{
    uint16_t x, y;

    timings.master_speed = 0.0015;    // speed ratios for the oscillators
    timings.ratio[0] = 4;         // higher values = faster transitions
    timings.ratio[1] = 1;
    timings.ratio[2] = 1;
    timings.ratio[3] = 0.05;
    timings.ratio[4] = 0.6;
    timings.offset[0] = 0;
    timings.offset[1] = 100;
    timings.offset[2] = 200;
    timings.offset[3] = 300;
    timings.offset[4] = 400;
    
    AnimaxCalculateOscillators(timings);     // get linear movers and oscillators going

    for (x = 0; x < LEDI_WIDTH; x++) {
      for (y = 0; y < LEDI_HEIGHT; y++) {
    
        // describe and render animation layers
        animation.dist       = distance[pXY(x, y)] * 0.8;
        animation.angle      = polar_theta[pXY(x, y)];
        animation.scale_x    = 0.15;// + (move.directional[0] + 2)/100;
        animation.scale_y    = 0.12;// + (move.directional[1] + 2)/100;
        animation.scale_z    = 0.01;
        animation.offset_y   = -move.linear[0];
        animation.offset_x   = 0;
        animation.offset_z   = 0;
        animation.z          = 30;
        float show1          = AnimaxRenderValue(animation);

        animation.offset_y   = -move.linear[1];
        animation.scale_x    = 0.15;// + (move.directional[0] + 2)/100;
        animation.scale_y    = 0.12;// + (move.directional[1] + 2)/100;
        animation.offset_x   = show1 / 100;
        animation.offset_y   += show1/100;
      
        float show2          = AnimaxRenderValue(animation);

        animation.offset_y   = -move.linear[2];
        animation.scale_x    = 0.15;// + (move.directional[0] + 2)/100;
        animation.scale_y    = 0.12;// + (move.directional[1] + 2)/100;
        animation.offset_x   = show2 / 100;
        animation.offset_y   += show2/100;
      
        float show3         = AnimaxRenderValue(animation);

        // colormapping
        float linear = (y)/(LEDI_HEIGHT-1.f);  // radial mask

        pixel.red = linear*show2;
        pixel.green = 0.1*linear*(show2-show3);
        
        pixel = AnimaxRgbSanityCheck(pixel);

        ANI_WritePixel(Ap, pXY(x, y), CRGB(pixel.red, pixel.green, pixel.blue));

      }
    }
}

void ANIMAX_ChasingSpirals(AniParms *Ap) {

  timings.master_speed = 0.01;    // speed ratios for the oscillators
  timings.ratio[0] = 0.1;         // higher values = faster transitions
  timings.ratio[1] = 0.13;
  timings.ratio[2] = 0.16;
  
  timings.offset[1] = 10;
  timings.offset[2] = 20;
  timings.offset[3] = 30;
  
  AnimaxCalculateOscillators(timings);     // get linear movers and oscillators going

  for (int x = 0; x < LEDI_WIDTH; x++) {
    for (int y = 0; y < LEDI_HEIGHT; y++) {
  
      // describe and render animation layers
      animation.angle      = 3 * polar_theta[pXY(x, y)] +  move.radial[0] - distance[pXY(x, y)]/3;
      animation.dist       = distance[pXY(x, y)];
      animation.scale_z    = 0.1;  
      animation.scale_y    = 0.1;
      animation.scale_x    = 0.1;
      animation.offset_x   = move.linear[0];
      animation.offset_y   = 0;
      animation.offset_z   = 0;
      animation.z          = 0;
      float show1          = AnimaxRenderValue(animation);

      animation.angle      = 3 * polar_theta[pXY(x, y)] +  move.radial[1] - distance[pXY(x, y)]/3;
      animation.dist       = distance[pXY(x, y)];
      animation.offset_x   = move.linear[1];
      float show2          = AnimaxRenderValue(animation);

      animation.angle      = 3 * polar_theta[pXY(x, y)] +  move.radial[2] - distance[pXY(x, y)]/3;
      animation.dist       = distance[pXY(x, y)];
      animation.offset_x   = move.linear[2];
      float show3          = AnimaxRenderValue(animation);

      // colormapping
      float radius = 10;
      float radial_filter = (radius - distance[pXY(x, y)]) / radius;

      pixel.red   = 3*show1 * radial_filter;
      pixel.green = show2 * radial_filter / 2;
      pixel.blue  = show3 * radial_filter / 4;

      pixel = AnimaxRgbSanityCheck(pixel);

      ANI_WritePixel(Ap, pXY(x, y), CRGB(pixel.red, pixel.green, pixel.blue));
    }
  }

}

void ANIMAX_Caleido1(AniParms *Ap)
{

  timings.master_speed = 0.003;    // speed ratios for the oscillators
  timings.ratio[0] = 0.02;         // higher values = faster transitions
  timings.ratio[1] = 0.03;
  timings.ratio[2] = 0.04;
  timings.ratio[3] = 0.05;
  timings.ratio[4] = 0.6;
  timings.offset[0] = 0;
  timings.offset[1] = 100;
  timings.offset[2] = 200;
  timings.offset[3] = 300;
  timings.offset[4] = 400;
  
  AnimaxCalculateOscillators(timings);     // get linear movers and oscillators going

  for (int x = 0; x < LEDI_WIDTH; x++) {
    for (int y = 0; y < LEDI_HEIGHT; y++) {
  
      // describe and render animation layers
      animation.dist       = distance[pXY(x, y)] * (2 + move.directional[0]) / 3;
      animation.angle      = 3 * polar_theta[pXY(x, y)] + 3 * move.noise_angle[0] + move.radial[4];
      animation.scale_x    = 0.1;
      animation.scale_y    = 0.1;
      animation.scale_z    = 0.1;
      animation.offset_y   = 2 * move.linear[0];
      animation.offset_x   = 0;
      animation.offset_z   = 0;
      animation.z          = move.linear[0];
      float show1          = AnimaxRenderValue(animation);

      animation.dist       = distance[pXY(x, y)] * (2 + move.directional[1]) / 3;
      animation.angle      = 4 * polar_theta[pXY(x, y)] + 3 * move.noise_angle[1] + move.radial[4];
      animation.offset_x   = 2 * move.linear[1];
      animation.z          = move.linear[1];
      float show2          = AnimaxRenderValue(animation);

      animation.dist       = distance[pXY(x, y)] * (2 + move.directional[2]) / 3;
      animation.angle      = 5 * polar_theta[pXY(x, y)] + 3 * move.noise_angle[2] + move.radial[4];
      animation.offset_y   = 2 * move.linear[2];
      animation.z          = move.linear[2];
      float show3          = AnimaxRenderValue(animation);

      animation.dist       = distance[pXY(x, y)] * (2 + move.directional[3]) / 3;
      animation.angle      = 4 * polar_theta[pXY(x, y)] + 3 * move.noise_angle[3] + move.radial[4];
      animation.offset_x   = 2 * move.linear[3];
      animation.z          = move.linear[3];
      float show4          = AnimaxRenderValue(animation);
      
      // colormapping
      pixel.red   = show1;
      pixel.green = show3 * distance[pXY(x, y)] / 10;
      pixel.blue  = (show2 + show4) / 2;

      pixel = AnimaxRgbSanityCheck(pixel);

      ANI_WritePixel(Ap, pXY(x, y), CRGB(pixel.red, pixel.green, pixel.blue));
    }
  }
}

void ANIMAX_Caleido2(AniParms *Ap)
{

  timings.master_speed = 0.002;    // speed ratios for the oscillators
  timings.ratio[0] = 0.02;         // higher values = faster transitions
  timings.ratio[1] = 0.03;
  timings.ratio[2] = 0.04;
  timings.ratio[3] = 0.05;
  timings.ratio[4] = 0.6;
  timings.offset[0] = 0;
  timings.offset[1] = 100;
  timings.offset[2] = 200;
  timings.offset[3] = 300;
  timings.offset[4] = 400;
  
  AnimaxCalculateOscillators(timings);     // get linear movers and oscillators going

  for (int x = 0; x < LEDI_WIDTH; x++) {
    for (int y = 0; y < LEDI_HEIGHT; y++) {
  
      // describe and render animation layers
      animation.dist       = distance[pXY(x, y)] * (2 + move.directional[0]) / 3;
      animation.angle      = 2 * polar_theta[pXY(x, y)] + 3 * move.noise_angle[0] + move.radial[4];
      animation.scale_x    = 0.1;
      animation.scale_y    = 0.1;
      animation.scale_z    = 0.1;
      animation.offset_y   = 2 * move.linear[0];
      animation.offset_x   = 0;
      animation.offset_z   = 0;
      animation.z          = move.linear[0];
      float show1          = AnimaxRenderValue(animation);

      animation.dist       = distance[pXY(x, y)] * (2 + move.directional[1]) / 3;
      animation.angle      = 2 * polar_theta[pXY(x, y)] + 3 * move.noise_angle[1] + move.radial[4];
      animation.offset_x   = 2 * move.linear[1];
      animation.z          = move.linear[1];
      float show2          = AnimaxRenderValue(animation);

      animation.dist       = distance[pXY(x, y)] * (2 + move.directional[2]) / 3;
      animation.angle      = 2 * polar_theta[pXY(x, y)] + 3 * move.noise_angle[2] + move.radial[4];
      animation.offset_y   = 2 * move.linear[2];
      animation.z          = move.linear[2];
      float show3          = AnimaxRenderValue(animation);

      animation.dist       = distance[pXY(x, y)] * (2 + move.directional[3]) / 3;
      animation.angle      = 2 * polar_theta[pXY(x, y)] + 3 * move.noise_angle[3] + move.radial[4];
      animation.offset_x   = 2 * move.linear[3];
      animation.z          = move.linear[3];
      float show4          = AnimaxRenderValue(animation);
      
      // colormapping
      pixel.red   = show1;
      pixel.green = show3 * distance[pXY(x, y)] / 10;
      pixel.blue  = (show2 + show4) / 2;

      pixel = AnimaxRgbSanityCheck(pixel);

      ANI_WritePixel(Ap, pXY(x, y), CRGB(pixel.red, pixel.green, pixel.blue));
    }
  }
}

void ANIMAX_Caleido3(AniParms *Ap)
{

a = micros();                   // for time measurement in report_performance()

  timings.master_speed = 0.004;    // speed ratios for the oscillators
  timings.ratio[0] = 0.02;         // higher values = faster transitions
  timings.ratio[1] = 0.03;
  timings.ratio[2] = 0.04;
  timings.ratio[3] = 0.05;
  timings.ratio[4] = 0.6;
  timings.offset[0] = 0;
  timings.offset[1] = 100;
  timings.offset[2] = 200;
  timings.offset[3] = 300;
  timings.offset[4] = 400;
  
  AnimaxCalculateOscillators(timings);     // get linear movers and oscillators going

  for (int x = 0; x < LEDI_WIDTH; x++) {
    for (int y = 0; y < LEDI_HEIGHT; y++) {
  
      // describe and render animation layers
      animation.dist       = distance[pXY(x, y)] * (2 + move.directional[0]) / 3;
      animation.angle      = 2 * polar_theta[pXY(x, y)] + 3 * move.noise_angle[0] + move.radial[4];
      animation.scale_x    = 0.1;// + (move.directional[0] + 2)/100;
      animation.scale_y    = 0.1;// + (move.directional[1] + 2)/100;
      animation.scale_z    = 0.1;
      animation.offset_y   = 2 * move.linear[0];
      animation.offset_x   = 2 * move.linear[1];
      animation.offset_z   = 0;
      animation.z          = move.linear[0];
      float show1          = AnimaxRenderValue(animation);

      animation.dist       = distance[pXY(x, y)] * (2 + move.directional[1]) / 3;
      animation.angle      = 2 * polar_theta[pXY(x, y)] + 3 * move.noise_angle[1] + move.radial[4];
      animation.offset_x   = 2 * move.linear[1];
      animation.offset_y   = show1 / 20.0;
      animation.z          = move.linear[1];
      float show2          = AnimaxRenderValue(animation);

      animation.dist       = distance[pXY(x, y)] * (2 + move.directional[2]) / 3;
      animation.angle      = 2 * polar_theta[pXY(x, y)] + 3 * move.noise_angle[2] + move.radial[4];
      animation.offset_y   = 2 * move.linear[2];
      animation.offset_x   = show2 / 20.0;
      animation.z          = move.linear[2];
      float show3          = AnimaxRenderValue(animation);

      animation.dist       = distance[pXY(x, y)] * (2 + move.directional[3]) / 3;
      animation.angle      = 2 * polar_theta[pXY(x, y)] + 3 * move.noise_angle[3] + move.radial[4];
      animation.offset_x   = 2 * move.linear[3];
      animation.offset_y   = show3 / 20.0;
      animation.z          = move.linear[3];
      float show4          = AnimaxRenderValue(animation);
      
      // colormapping
      float radius = 8;  // radial mask

      pixel.red   = show1 * (y+1) / LEDI_HEIGHT;
      pixel.green = show3 * distance[pXY(x, y)] / 10;
      pixel.blue  = (show2 + show4) / 2;
      if (distance[pXY(x, y)] > radius) {
        pixel.red = 0;
        pixel.green = 0;
        pixel.blue = 0;
      }

      pixel = AnimaxRgbSanityCheck(pixel);

      ANI_WritePixel(Ap, pXY(x, y), CRGB(pixel.red, pixel.green, pixel.blue));
    }
  }
}

void ANIMAX_Scaledemo1(AniParms *Ap)
{


  timings.master_speed = 0.00003;    // speed ratios for the oscillators
  timings.ratio[0] = 4;         // higher values = faster transitions
  timings.ratio[1] = 3.2;
  timings.ratio[2] = 10;
  timings.ratio[3] = 0.05;
  timings.ratio[4] = 0.6;
  timings.offset[0] = 0;
  timings.offset[1] = 100;
  timings.offset[2] = 200;
  timings.offset[3] = 300;
  timings.offset[4] = 400;
  
  AnimaxCalculateOscillators(timings);     // get linear movers and oscillators going

  for (int x = 0; x < LEDI_WIDTH; x++) {
    for (int y = 0; y < LEDI_HEIGHT; y++) {
  
      // describe and render animation layers
      animation.dist       = 0.3*distance[pXY(x, y)] * 0.8;
      animation.angle      = 3*polar_theta[pXY(x, y)] + move.radial[2];
      animation.scale_x    = 0.1 + (move.noise_angle[0])/10;
      animation.scale_y    = 0.1 + (move.noise_angle[1])/10;// + (move.directional[1] + 2)/100;
      animation.scale_z    = 0.01;
      animation.offset_y   = 0;
      animation.offset_x   = 0;
      animation.offset_z   = 100*move.linear[0];
      animation.z          = 30;
      float show1          = AnimaxRenderValue(animation);

      animation.angle      = 3;
      float show2          = AnimaxRenderValue(animation);

      float dist = (10-distance[pXY(x, y)])/ 10;
      pixel.red = show1*dist;
      pixel.green = (show1-show2)*dist*0.3;
      pixel.blue = (show2-show1)*dist;

      if (distance[pXY(x, y)] > 8) {
         pixel.red = 0;
         pixel.green = 0;
         pixel.blue = 0;

      }
      
      pixel = AnimaxRgbSanityCheck(pixel);

      ANI_WritePixel(Ap, pXY(x, y), CRGB(pixel.red, pixel.green, pixel.blue));
    }
  }

}


void ANIMAX_Yves(AniParms *Ap)
{


  timings.master_speed = 0.001;    // speed ratios for the oscillators
  timings.ratio[0] = 3;         // higher values = faster transitions
  timings.ratio[1] = 2;
  timings.ratio[2] = 1;
  timings.ratio[3] = 0.13;
  timings.ratio[4] = 0.15;
  timings.ratio[5] = 0.03;
  timings.ratio[6] = 0.025;
  timings.offset[0] = 0;
  timings.offset[1] = 100;
  timings.offset[2] = 200;
  timings.offset[3] = 300;
  timings.offset[4] = 400;
  timings.offset[5] = 500;
  timings.offset[6] = 600;
  
  AnimaxCalculateOscillators(timings);     // get linear movers and oscillators going

  for (int x = 0; x < LEDI_WIDTH; x++) {
    for (int y = 0; y < LEDI_HEIGHT; y++) {
      
      animation.dist       = distance[pXY(x, y)] ;
      animation.angle      = polar_theta[pXY(x, y)] + 2*PI + move.noise_angle[5];
      animation.scale_x    = 0.08;
      animation.scale_y    = 0.08;
      animation.scale_z    = 0.08;
      animation.offset_y   = -move.linear[0];
      animation.offset_x   = 0;
      animation.offset_z   = 0;
      animation.z          = 0;
      float show1          = AnimaxRenderValue(animation);

      animation.dist       = distance[pXY(x, y)];
      animation.angle      = polar_theta[pXY(x, y)] + 2*PI + move.noise_angle[6];;
      animation.scale_x    = 0.08;
      animation.scale_y    = 0.08;
      animation.scale_z    = 0.08;
      animation.offset_y   = -move.linear[1];
      animation.offset_x   = 0;
      animation.offset_z   = 0;
      animation.z          = 0;
      float show2          = AnimaxRenderValue(animation);

      animation.angle      = polar_theta[pXY(x, y)] + show1/100 + move.noise_angle[3] + move.noise_angle[4];
      animation.dist       = distance[pXY(x, y)] + show2/50;
      animation.offset_y   = -move.linear[2];

      animation.offset_y   += show1/100;
      animation.offset_x   += show2/100;

      float show3          = AnimaxRenderValue(animation);

      animation.offset_y   = 0;
      animation.offset_x   = 0;

      float show4          = AnimaxRenderValue(animation);
      
     
      pixel.red   = show3;
      pixel.green = show3*show4/255;
      pixel.blue  = 0;
      
      pixel = AnimaxRgbSanityCheck(pixel);
      ANI_WritePixel(Ap, pXY(x, y), CRGB(pixel.red, pixel.green, pixel.blue));
    }
  }
}

void ANIMAX_Spiralus(AniParms *Ap)
{

  timings.master_speed = 0.0011;    // speed ratios for the oscillators
  timings.ratio[0] = 1.5;         // higher values = faster transitions
  timings.ratio[1] = 2.3;
  timings.ratio[2] = 3;
  timings.ratio[3] = 0.05;
  timings.ratio[4] = 0.2;
  timings.ratio[5] = 0.03;
  timings.ratio[6] = 0.025;
  timings.ratio[7] = 0.021;
  timings.ratio[8] = 0.027;
  timings.offset[0] = 0;
  timings.offset[1] = 100;
  timings.offset[2] = 200;
  timings.offset[3] = 300;
  timings.offset[4] = 400;
  timings.offset[5] = 500;
  timings.offset[6] = 600;
  
  AnimaxCalculateOscillators(timings);     // get linear movers and oscillators going

  for (int x = 0; x < LEDI_WIDTH; x++) {
    for (int y = 0; y < LEDI_HEIGHT; y++) {
      
      animation.dist       = distance[pXY(x, y)] ;
      animation.angle      = 2*polar_theta[pXY(x, y)] + move.noise_angle[5] + move.directional[3] * move.noise_angle[6]* animation.dist/10;
      animation.scale_x    = 0.08;
      animation.scale_y    = 0.08;
      animation.scale_z    = 0.02;
      animation.offset_y   = -move.linear[0];
      animation.offset_x   = 0;
      animation.offset_z   = 0;
      animation.z          = move.linear[1];
      float show1          = AnimaxRenderValue(animation);

      animation.angle      = 2*polar_theta[pXY(x, y)] + move.noise_angle[7] + move.directional[5] * move.noise_angle[8]* animation.dist/10;
      animation.offset_y   = -move.linear[1];
      animation.z          = move.linear[2];
            
      float show2          = AnimaxRenderValue(animation);

      animation.angle      = 2*polar_theta[pXY(x, y)] + move.noise_angle[6] + move.directional[6] * move.noise_angle[7]* animation.dist/10;
      animation.offset_y   = move.linear[2];
      animation.z          = move.linear[0];
      float show3          = AnimaxRenderValue(animation);
      
      
      float f =  (20-distance[pXY(x, y)])/20;
     
      pixel.red   = f*(show1+show2);
      pixel.green = f*(show1-show2);
      pixel.blue  = f*(show3-show1);
      
      pixel = AnimaxRgbSanityCheck(pixel);
      ANI_WritePixel(Ap, pXY(x, y), CRGB(pixel.red, pixel.green, pixel.blue));
    }
  }
}

void ANIMAX_Spiralus2(AniParms *Ap)
{
  timings.master_speed = 0.0011;    // speed ratios for the oscillators
  timings.ratio[0] = 1.5;         // higher values = faster transitions
  timings.ratio[1] = 2.3;
  timings.ratio[2] = 3;
  timings.ratio[3] = 0.05;
  timings.ratio[4] = 0.2;
  timings.ratio[5] = 0.03;
  timings.ratio[6] = 0.025;
  timings.ratio[7] = 0.021;
  timings.ratio[8] = 0.027;
  timings.offset[0] = 0;
  timings.offset[1] = 100;
  timings.offset[2] = 200;
  timings.offset[3] = 300;
  timings.offset[4] = 400;
  timings.offset[5] = 500;
  timings.offset[6] = 600;
  
  AnimaxCalculateOscillators(timings);     // get linear movers and oscillators going

  for (int x = 0; x < LEDI_WIDTH; x++) {
    for (int y = 0; y < LEDI_HEIGHT; y++) {
      
      animation.dist       = distance[pXY(x, y)] ;
      animation.angle      = 2*polar_theta[pXY(x, y)] + move.noise_angle[5] + move.directional[3] * move.noise_angle[6]* animation.dist/10;
      animation.scale_x    = 0.08;
      animation.scale_y    = 0.08;
      animation.scale_z    = 0.02;
      animation.offset_y   = -move.linear[0];
      animation.offset_x   = 0;
      animation.offset_z   = 0;
      animation.z          = move.linear[1];
      float show1          = AnimaxRenderValue(animation);

      animation.angle      = 3*polar_theta[pXY(x, y)] + move.noise_angle[7] + move.directional[5] * move.noise_angle[8]* animation.dist/10;
      animation.offset_y   = -move.linear[1];
      animation.z          = move.linear[2];
            
      float show2          = AnimaxRenderValue(animation);

      animation.angle      = 4*polar_theta[pXY(x, y)] + move.noise_angle[6] + move.directional[6] * move.noise_angle[7]* animation.dist/10;
      animation.offset_y   = move.linear[2];
      animation.z          = move.linear[0];
      animation.dist       = distance[pXY(x, y)] *0.8;
      float show3          = AnimaxRenderValue(animation);
      
      
      float f =  (20-distance[pXY(x, y)])/20;
     
      pixel.red   = f*(show1+show2);
      pixel.green = f*(show1-show2);
      pixel.blue  = f*(show3-show1);
      
      pixel = AnimaxRgbSanityCheck(pixel);
      ANI_WritePixel(Ap, pXY(x, y), CRGB(pixel.red, pixel.green, pixel.blue));
    }
  }
}

void Animax_HotBlob(AniParms *Ap)
{ // nice one
  c = micros(); // for time measurement in AnimaxReportPerformance()
  EVERY_N_MILLIS(500) AnimaxReportPerformance();   // check serial monitor for report
  a = micros();                   

  AnimaxRunDefaultOscillators();

  for (int x = 0; x < LEDI_WIDTH; x++) {
    for (int y = 0; y < LEDI_HEIGHT; y++) {
      
      animation.dist       = distance[pXY(x, y)] ;
      animation.angle      = polar_theta[pXY(x, y)];
      
      animation.scale_x    = 0.07 + move.directional[0]*0.002;
      animation.scale_y    = 0.07;
      
      animation.offset_y   = -move.linear[0];
      animation.offset_x   = 0;
      animation.offset_z   = 0;
      
      animation.z          = 0;
      animation.low_limit  = -1;
      float show1          = AnimaxRenderValue(animation);

      animation.offset_y   = -move.linear[1];
      float show3          = AnimaxRenderValue(animation);

      animation.offset_x   = show3/20;
      animation.offset_y   = -move.linear[0]/2 + show1/70;
      animation.low_limit  = 0;
      float show2          = AnimaxRenderValue(animation);

      animation.offset_x   = show3/20;
      animation.offset_y   = -move.linear[0]/2 + show1/70;
      animation.z          = 100;
      float show4          = AnimaxRenderValue(animation);

      float radius = 11;   // radius of a radial brightness filter
      float radial = (radius-animation.dist)/animation.dist;

      float linear = (y+1)/(LEDI_HEIGHT-1.f);
      
      pixel.red   = radial  * show2;
      pixel.green   = linear * radial* 0.3* (show2-show4);
      
      
      pixel = AnimaxRgbSanityCheck(pixel);
      ANI_WritePixel(Ap, pXY(x, y), CRGB(pixel.red, pixel.green, pixel.blue));
    }
  }
  b = micros(); // for time measurement in report_performance()
}



void ANIMAX_Zoom(AniParms *Ap)
{ // nice one

  AnimaxRunDefaultOscillators();
  timings.master_speed = 0.003;
  AnimaxCalculateOscillators(timings); 

  for (int x = 0; x < LEDI_WIDTH; x++) {
    for (int y = 0; y < LEDI_HEIGHT; y++) {
      
      animation.dist       = distance[pXY(x, y)] * distance[pXY(x, y)];
      animation.angle      = polar_theta[pXY(x, y)];
      
      animation.scale_x    = 0.01;
      animation.scale_y    = 0.01;
      
      animation.offset_y   = -10*move.linear[0];
      animation.offset_x   = 0;
      animation.offset_z   = 0;
      
      animation.z          = 0;
      animation.low_limit  = 0;
      float show1          = AnimaxRenderValue(animation);

      float linear = (y+1)/(LEDI_HEIGHT-1.f);
      
      pixel.red   = show1*linear;
      pixel.green   = 0;
      
      
      pixel = AnimaxRgbSanityCheck(pixel);
      ANI_WritePixel(Ap, pXY(x, y), CRGB(pixel.red, pixel.green, pixel.blue));
    }
  }
}

void ANIMAX_Rings(AniParms *Ap)
{

  timings.master_speed = 0.01;    // speed ratios for the oscillators
  timings.ratio[0] = 1;         // higher values = faster transitions
  timings.ratio[1] = 1.1;
  timings.ratio[2] = 1.2;
  
  timings.offset[1] = 100;
  timings.offset[2] = 200;
  timings.offset[3] = 300;
  
  AnimaxCalculateOscillators(timings);     // get linear movers and oscillators going

  for (int x = 0; x < LEDI_WIDTH; x++) {
    for (int y = 0; y < LEDI_HEIGHT; y++) {
  
      // describe and render animation layers
      animation.angle      = 5;
      animation.scale_x    = 0.2;
      animation.scale_y    = 0.2;
      animation.scale_z    = 1;
      animation.dist       = distance[pXY(x,y)];
      animation.offset_y   = -move.linear[0];
      animation.offset_x   = 0;
      float show1          = AnimaxRenderValue(animation);

       // describe and render animation layers
      animation.angle      = 10;
      
      animation.dist       = distance[pXY(x,y)];
      animation.offset_y   = -move.linear[1];
      float show2          = AnimaxRenderValue(animation);

       // describe and render animation layers
      animation.angle      = 12;
      
      animation.dist       = distance[pXY(x,y)];
      animation.offset_y   = -move.linear[2];
      float show3          = AnimaxRenderValue(animation);

      // colormapping
      pixel.red   = show1;
      pixel.green = show2 / 4;
      pixel.blue  = show3 / 4;

      pixel = AnimaxRgbSanityCheck(pixel);

      ANI_WritePixel(Ap, pXY(x, y), CRGB(pixel.red, pixel.green, pixel.blue));
    }
  }
}

void ANIMAX_Waves(AniParms *Ap)
{

a = micros();                   // for time measurement in AnimaxReportPerformance()

  timings.master_speed = 0.01;    // speed ratios for the oscillators
  timings.ratio[0] = 2;         // higher values = faster transitions
  timings.ratio[1] = 2.1;
  timings.ratio[2] = 1.2;
  
  timings.offset[1] = 100;
  timings.offset[2] = 200;
  timings.offset[3] = 300;
  
  AnimaxCalculateOscillators(timings);     // get linear movers and oscillators going

  for (int x = 0; x < LEDI_WIDTH; x++) {
    for (int y = 0; y < LEDI_HEIGHT; y++) {
  
      // describe and render animation layers
      animation.angle      = polar_theta[pXY(x,y)];
      animation.scale_x    = 0.1;
      animation.scale_y    = 0.1;
      animation.scale_z    = 0.1;
      animation.dist       = distance[pXY(x,y)];
      animation.offset_y   = 0;
      animation.offset_x   = 0;
      animation.z          = 2*distance[pXY(x,y)] - move.linear[0];
      float show1          = AnimaxRenderValue(animation);

      animation.angle      = polar_theta[pXY(x,y)];
      animation.dist       = distance[pXY(x,y)];
      animation.z          = 2*distance[pXY(x,y)] - move.linear[1];
      float show2          = AnimaxRenderValue(animation);

  
      // colormapping
      pixel.red   = show1;
      pixel.green = 0;
      pixel.blue  = show2;

      pixel = AnimaxRgbSanityCheck(pixel);

      ANI_WritePixel(Ap, pXY(x, y), CRGB(pixel.red, pixel.green, pixel.blue));
    }
  }
}

void ANIMAX_CenterField(AniParms *Ap)
{

  timings.master_speed = 0.01;    // speed ratios for the oscillators
  timings.ratio[0] = 1;         // higher values = faster transitions
  timings.ratio[1] = 1.1;
  timings.ratio[2] = 1.2;
  
  timings.offset[1] = 100;
  timings.offset[2] = 200;
  timings.offset[3] = 300;
  
  AnimaxCalculateOscillators(timings);     // get linear movers and oscillators going

  for (int x = 0; x < LEDI_WIDTH; x++) {
    for (int y = 0; y < LEDI_HEIGHT; y++) {
  
      // describe and render animation layers
      animation.angle      = polar_theta[pXY(x,y)];
      animation.scale_x    = 0.07;
      animation.scale_y    = 0.07;
      animation.scale_z    = 0.1;
      animation.dist       = 5*sqrtf(distance[pXY(x,y)]);
      animation.offset_y   = move.linear[0];
      animation.offset_x   = 0;
      animation.z          = 0;
      float show1          = AnimaxRenderValue(animation);

      animation.angle      = polar_theta[pXY(x,y)];
      animation.scale_x    = 0.07;
      animation.scale_y    = 0.07;
      animation.scale_z    = 0.1;
      animation.dist       = 4*sqrtf(distance[pXY(x,y)]);
      animation.offset_y   = move.linear[0];
      animation.offset_x   = 0;
      animation.z          = 0;
      float show2          = AnimaxRenderValue(animation);

     

  
      // colormapping
      pixel.red   = show1;
      pixel.green = show2;
      pixel.blue  = 0;

      pixel = AnimaxRgbSanityCheck(pixel);

      ANI_WritePixel(Ap, pXY(x, y), CRGB(pixel.red, pixel.green, pixel.blue));
    }
  }

}



/* --------------------------------------------------------------------------------------------
 *  PRIVATE FUNCTIONS
 * --------------------------------------------------------------------------------------------
 */


void AnimaxCalculateOscillators(Oscillators &Timings)
{

  double runtime = millis() * Timings.master_speed;  // global anaimation speed

  for (int i = 0; i < NUM_OSCILLATORS; i++) {
    
    move.linear[i]      = (runtime + Timings.offset[i]) * Timings.ratio[i];     // continously rising offsets, returns              0 to max_float
    
    move.radial[i]      = fmodf(move.linear[i], 2 * PI);                        // angle offsets for continous rotation, returns    0 to 2 * PI
    
    move.directional[i] = sinf(move.radial[i]);                                 // directional offsets or factors, returns         -1 to 1
    
    move.noise_angle[i] = PI * (1 + AnimaxPnoise(move.linear[i], 0, 0));              // noise based angle offset, returns                0 to 2 * PI
    
  }
}

// Convert the 2 polar coordinates back to cartesian ones & also apply all 3d transitions.
// Calculate the noise value at this point based on the 5 dimensional manipulation of 
// the underlaying coordinates.

float AnimaxRenderValue(RenderParameters &Animation)
{

  // convert polar coordinates back to cartesian ones

  float newx = (animation.offset_x + animation.center_x - (cosf(animation.angle) * animation.dist)) * animation.scale_x;
  float newy = (animation.offset_y + animation.center_y - (sinf(animation.angle) * animation.dist)) * animation.scale_y;
  float newz = (animation.offset_z + animation.z) * animation.scale_z;

  // render noisevalue at this new cartesian point

  float raw_noise_field_value = AnimaxPnoise(newx, newy, newz);
  
  // A) enhance histogram (improve contrast) by setting the black and white point (low & high_limit)
  // B) scale the result to a 0-255 range (assuming you want 8 bit color depth per rgb chanel)
  // Here happens the contrast boosting & the brightness mapping

  if (raw_noise_field_value < animation.low_limit)  raw_noise_field_value =  animation.low_limit;
  if (raw_noise_field_value > animation.high_limit) raw_noise_field_value = animation.high_limit;

  float scaled_noise_value = AnimaxMapFloat(raw_noise_field_value, animation.low_limit, animation.high_limit, 0, 255);

  return scaled_noise_value;
}



// given a static polar origin we can precalculate 
// the polar coordinates

void AnimaxRenderPolarLookupTable(float cx, float cy)
{
    int xx, yy;
  for (xx = 0; xx < LEDI_WIDTH; xx++) {
    for (yy = 0; yy < LEDI_HEIGHT; yy++) {

      float dx = xx - cx;
      float dy = yy - cy;

      distance[pXY(xx, yy)]    = hypotf(dx, dy);
      polar_theta[pXY(xx, yy)] = atan2f(dy, dx); 
    }
  }
}

// float mapping maintaining 32 bit precision
// we keep values with high resolution for potential later usage

float AnimaxMapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{ 
  
  float result = (x-in_min) * (out_max-out_min) / (in_max-in_min) + out_min;
  if (result < out_min) result = out_min;
  if( result > out_max) result = out_max;

  return result; 
}


// Avoid any possible color flicker by forcing the raw RGB values to be 0-255.
// This enables to play freely with random equations for the colormapping
// without causing flicker by accidentally missing the valid target range.

AnimaxRgb AnimaxRgbSanityCheck(AnimaxRgb &Pixel)
{

   
    // discard everything above the valid 8 bit colordepth 0-255 range
    if (Pixel.red   > 0xFF)   Pixel.red = 0xFF;
    if (Pixel.green > 0xFF) Pixel.green = 0xFF;
    if (Pixel.blue  > 0xFF)  Pixel.blue = 0xFF;

    return Pixel;
}

float AnimaxPnoise(float x, float y, float z)
{
  
int   X = (int)floorf(x) & 0xFF,             /* FIND UNIT CUBE THAT */
      Y = (int)floorf(y) & 0xFF,             /* CONTAINS POINT.     */
      Z = (int)floorf(z) & 0xFF;
x -= floorf(x);                             /* FIND RELATIVE X,Y,Z */
y -= floorf(y);                             /* OF POINT IN CUBE.   */
z -= floorf(z);
float  u = NOISE_FADE(x),                         /* COMPUTE FADE CURVES */
       v = NOISE_FADE(y),                         /* FOR EACH OF X,Y,Z.  */
       w = NOISE_FADE(z);
int  A = p[X & 0xFF]+Y, 
     AA = p[(A) & 0xFF]+Z, 
     AB = p[(A+1) & 0xFF]+Z,                         /* HASH COORDINATES OF */
     B = p[(X+1) & 0xFF]+Y, 
     BA = p[(B) & 0xFF]+Z, 
     BB = p[(B+1) & 0xFF]+Z;                         /* THE 8 CUBE CORNERS, */

return NOISE_LERP(w,NOISE_LERP(v,NOISE_LERP(u, AnimaxNoiseGrad(p[AA & 0xFF], x, y, z),    /* AND ADD */
                          AnimaxNoiseGrad(p[BA & 0xFF], x-1, y, z)),    /* BLENDED */
              NOISE_LERP(u, AnimaxNoiseGrad(p[AB & 0xFF], x, y-1, z),         /* RESULTS */
                   AnimaxNoiseGrad(p[BB & 0xFF], x-1, y-1, z))),        /* FROM  8 */
            NOISE_LERP(v, NOISE_LERP(u, AnimaxNoiseGrad(p[(AA+1) & 0xFF], x, y, z-1),   /* CORNERS */
                 AnimaxNoiseGrad(p[(BA+1) & 0xFF], x-1, y, z-1)),           /* OF CUBE */
              NOISE_LERP(u, AnimaxNoiseGrad(p[(AB+1) & 0xFF], x, y-1, z-1),
                   AnimaxNoiseGrad(p[(BB+1) & 0xFF], x-1, y-1, z-1))));
}

float AnimaxNoiseGrad(int hash, float x, float y, float z)
{
    int    h = hash & 15;          /* CONVERT LO 4 BITS OF HASH CODE */
    float  u = h < 8 ? x : y,      /* INTO 12 GRADIENT DIRECTIONS.   */
          v = h < 4 ? y : h==12||h==14 ? x : z;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v); 
}

void AnimaxRunDefaultOscillators()
{

  timings.master_speed = 0.005;    // master speed

  timings.ratio[0] = 1;           // speed ratios for the oscillators, higher values = faster transitions
  timings.ratio[1] = 2;
  timings.ratio[2] = 3;
  timings.ratio[3] = 4;
  timings.ratio[4] = 5;
  timings.ratio[5] = 6;
  timings.ratio[6] = 7;
  timings.ratio[7] = 8;
  timings.ratio[8] = 9;
  timings.ratio[9] = 10;

  
  timings.offset[0] = 000;
  timings.offset[1] = 100;
  timings.offset[2] = 200;
  timings.offset[3] = 300;
  timings.offset[4] = 400;
  timings.offset[5] = 500;
  timings.offset[6] = 600;
  timings.offset[7] = 700;
  timings.offset[8] = 800;
  timings.offset[9] = 900;

  AnimaxCalculateOscillators(timings);  
}

void AnimaxReportPerformance()
{
  
  float calc  = b - a;                         // rendering time
  float push  = c - b;                         // time to initialize led update
  float total = c - a;                         // time per frame
  int fps  = 1000000 / total;                // frames per second
  int kpps = (fps * LEDI_WIDTH * LEDI_HEIGHT) / 1000;   // kilopixel per second

  Serial.print(fps);                         Serial.print(" fps  ");
  Serial.print(kpps);                        Serial.print(" kpps @");
  Serial.print(LEDI_WIDTH*LEDI_HEIGHT);                 Serial.print(" LEDs  ");  
  Serial.print(round(total));                Serial.print(" µs per frame  Rendering: ");
  Serial.print(round((calc * 100) / total)); Serial.print("%  Sending data: ");
  Serial.print(round((push * 100) / total)); Serial.print("%  (");
  Serial.print(round(calc));                 Serial.print(" + ");
  Serial.print(round(push));                 Serial.println(" µs) ");
}