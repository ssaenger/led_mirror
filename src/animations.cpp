#include "../inc/sys/animations_i.h"


/* --------------------------------------------------------------------------------------------
 *  GLOBALS
 * --------------------------------------------------------------------------------------------
 */


static AniInfo  aniInfo;
static uint16_t numWritten;

/* --------------------------------------------------------------------------------------------
 *  PROTOTYPES
 * --------------------------------------------------------------------------------------------
 */
static void AniSetInactive(AniPack *Ap);
static void AniTransDone();
static bool AniCheckTranDel(AniParms *Ap);
static void writePixel(AniParms *Ap, AniType At, uint32_t PixNum, rgb24 *RgbVal);

/* --------------------------------------------------------------------------------------------
 *  PUBLIC FUNCTIONS
 * --------------------------------------------------------------------------------------------
 */


/* --------------------------------------------------------------------------------------------
 *                 ANI_Init()
 * --------------------------------------------------------------------------------------------
 * Description:    Initializes this layer. Must be the very first function to be called and
 *                 only call once or you can crash the system!
 *
 * Parameters:     None
 *
 * Returns:        void
 */
void ANI_Init()
{
    uint16_t i;
    InitList(&aniInfo.activeList);
    InitList(&aniInfo.mainWaitList);
    InitList(&aniInfo.transWaitList);
    InitList(&aniInfo.queueList);
    aniInfo.numMainWaiting = 0;
    aniInfo.numTransWaiting = 0;

    for (i = 0; i < SM_NUM_LEDS; i++) {
        aniInfo.owners[i] = ANI_TYPE_FREE;
    }
}

/* --------------------------------------------------------------------------------------------
 *                 ANI_AddAnimation()
 * --------------------------------------------------------------------------------------------
 * Description:    Adds a AniFuncInfo to the list of animations to draw from
 *
 * Parameters:     Ap - A pointer to a AniPack with "funcp" and "parms" filled out and "node"
 *                      initialized with InitNode().
 *                 At - The type of animation inside "Ap->funcp". 
 *                      Must be a ANI_TYPE_MAIN_OFFSET or ANI_TYPE_TRANS_OFFSET type animation
 *
 * Returns:        true it was added, false otherwise beceuse the type is bad or the AniPack
 *                 was already added to a list. It could also be that "Ap->node" was not iniitialized
 *                 with InitNode() yet.
 */
bool ANI_AddAnimation(AniPack *Ap, AniType At)
{
    if (IsNodeUsed(&Ap->node)) {
        /* Already added. */
        return false;
    }

    if (At & ANI_TYPE_MAIN_OFFSET) {
        /* Place this in the waiting main list */
        //Serial.printf("list addr: %p, F->%p, B->%p, node addr %p, F->%p, B->%p",
        //              &aniInfo.mainWaitList, aniInfo.mainWaitList.linkF, aniInfo.mainWaitList.linkB,
        //               &Ap->node, Ap->node.linkF, Ap->node.linkB);
        InsertTail(&aniInfo.mainWaitList, &Ap->node);
        aniInfo.numMainWaiting++;
    } else if (At & ANI_TYPE_TRANS_OFFSET) {
        /* Place this in the waiting transition list */
        InsertTail(&aniInfo.transWaitList, &Ap->node);
        aniInfo.numTransWaiting++;
    } else {
        return false;
    }

    Ap->type = At;
    return true;

}

/* --------------------------------------------------------------------------------------------
 *                 ANI_RemoveAnimation()
 * --------------------------------------------------------------------------------------------
 * Description:    Removes an AniPack that was added to the list
 *
 * Parameters:     AniPack - a pointer to a AniPack that was added earlier
 *
 * Returns:        void
 */
void ANI_RemoveAnimation(AniPack *Ap)
{
    if (IsNodeUsed(&Ap->node)) {
        RemoveNode(&Ap->node);
    }
}

/* --------------------------------------------------------------------------------------------
 *                 ANI_QueueAnimation()
 * --------------------------------------------------------------------------------------------
 * Description:    Queues 1 or more animations. Once ANI_SwapAnimation() is called, queued
 *                 animations will be moved to the active list. If any animations are transition
 *                 type animations, the previous main animations will be phased out over time.
 *                 If there are no transition type animations, then the previous main animations
 *                 will retire immediately.
 *
 * Parameters:     Ap - A pointer to a AniPack with that was added with ANI_AddAnimation(). This
 *                      animation will be added to the queue.
 *
 * Returns:        true if moved from waiting list to queue list. false if did not
 */
bool ANI_QueueAnimation(AniPack *Ap)
{
    if ((Ap->type & ANI_TYPE_MAIN_OFFSET) && IsNodeOnList(&aniInfo.mainWaitList, &Ap->node)) {
        RemoveNode(&Ap->node);
        aniInfo.numMainWaiting--;
        Serial.println("In Queue for main");
    } else if ((Ap->type & ANI_TYPE_TRANS_OFFSET) && IsNodeOnList(&aniInfo.transWaitList, &Ap->node)) {
        RemoveNode(&Ap->node);
        aniInfo.numTransWaiting--;
        Serial.println("In Queue for off");
    } else {
        return false;
    }

    /* Insert into queue list */
    InsertTail(&aniInfo.queueList, &Ap->node);

    return true;
}

/* --------------------------------------------------------------------------------------------
 *                 ANI_SwapAnimation()
 * --------------------------------------------------------------------------------------------
 * Description:    
 *
 * Parameters:     None
 *
 * Returns:        void
 */
void ANI_SwapAnimation()
{
    AniPack  *aniPack;
    AniPack  *aniPack2;
    ListNode *nodeItr;
    uint16_t  i;
    bool      inserted;
    bool      transPresent = false;


    /* Step 1: Check if any trans animations are present in the queue list */
    IterateList(aniInfo.queueList, aniPack, AniPack *) {
        //Serial.println("ANI_SwapAnimation: iter step 1");
        if (aniPack->type & ANI_TYPE_TRANS_OFFSET) {
            /* Insert before this node */
            transPresent = true;
            aniInfo.tranInProg = true;
            break;
        }
    }

    /* Step 2: Move main animations to old animations */
    //Serial.println("ANI_SwapAnimation: step 2");
    if (transPresent) {
        //Serial.println("ANI_SwapAnimation: tranpresent");
        for (i = 0; i < SM_NUM_LEDS; i++) {
            aniInfo.owners[i] &= ~ANI_TYPE_FREE_OFFSET;
        }
        //memset(aniInfo.owners, ANI_TYPE_OLD_FREE, SM_NUM_LEDS);
        nodeItr = &aniInfo.activeList;
        while ((nodeItr = GetNextNode(nodeItr)) != &aniInfo.activeList) {
            //Serial.println("ANI_SwapAnimation: 1");
            aniPack = (AniPack*)nodeItr;
            if (aniPack->type & ANI_TYPE_MAIN_OFFSET) {
                nodeItr = GetPriorNode(nodeItr);
                RemoveNode(&aniPack->node);

                /* Assign the "OLD" type */
                aniPack->type &= ~ANI_TYPE_MAIN_OFFSET;
                aniPack->type |= ANI_TYPE_OLD_OFFSET;

                /* Move this ahead of the active list to it's appropriate spot */
                transPresent = false;
                IterateList(aniInfo.activeList, aniPack2, AniPack *) {
                    if (aniPack->type <= aniPack2->type) {
                        /* Insert before this node */
                        InsertBefore(&aniPack2->node, &aniPack->node);
                        transPresent = true;
                        break; /* Must break out of IterateList() */
                    }
                }
                if (!transPresent) {
                    InsertTail(&aniInfo.activeList, &aniPack->node);
                }
            }
        }
    }

    /* Step 3: Move all animations from queue list to active list */
    while (!IsListEmpty(&aniInfo.queueList)) {
        aniPack = (AniPack*)GetHead(&aniInfo.queueList);
        //Serial.printf("ANI_SwapAnimation: queue type 0x%x\r\n", aniPack->type);

        RemoveNode(&aniPack->node);

        aniPack->parms.startTime = millis();
        aniPack->parms.delay = millis();
        aniPack->parms.value = 0;

        /* Insert into the active list in it's appropriate spot */
        inserted = false;
        IterateList(aniInfo.activeList, aniPack2, AniPack *) {
            if (aniPack->type <= aniPack2->type) {
                /* Insert before this node */
                InsertBefore(&aniPack2->node, &aniPack->node);
                inserted = true;
                break;
            }
        }
        if (!inserted) {
            InsertTail(&aniInfo.activeList, &aniPack->node);
        }
    }
}

/* --------------------------------------------------------------------------------------------
 *                 ANI_DrawAnimationFrame()
 * --------------------------------------------------------------------------------------------
 * Description:    Draw an animation frame funcs added earlier
 *
 * Parameters:     ledBuff - pointer to a LED buffer to fill
 *
 * Returns:        number of pixels written to the LedBuff. This counts pixels that were
 *                 written more than once.
 */
uint32_t ANI_DrawAnimationFrame(rgb24 *LedBuff)
{
    uint32_t tCount;
    AniPack *aniPack;
    AniPack *aniPack2;

    aniInfo.ledBuff = LedBuff;
    numWritten = 0;
    tCount = 0;

    /* Sort through each animation and check if it should be */
    //Serial.println("ANI_DrawAnimationFrame: Begin");

    IterateList(aniInfo.activeList, aniPack, AniPack*) {

        if (!AniCheckTranDel(&aniPack->parms)) {
            //Serial.prntlin("Skipping not time yet");
            if (aniPack->type & ANI_TYPE_TRANS_OFFSET) {
                tCount++;
            }
            continue;
        }
        aniPack->parms.delay = millis() + fps2Ms(aniPack->parms.fpsLimit);

        //Serial.printf("ANI_DrawAnimationFrame: type 0x%x. delay %lu\r\n", aniPack->type, aniPack->parms.delay);
        switch (aniPack->type) {
        case ANI_TYPE_MASK:
            break;

        case ANI_TYPE_BACKGROUND:
        case ANI_TYPE_FOREGROUND:
            aniPack->funcp(&aniPack->parms, aniPack->type);
            //Serial.printf("millis after: %lu\r\n", millis());
            break;

        case ANI_TYPE_FREE:
            break;

        case ANI_TYPE_OLD_FOREGROUND:
            aniPack->funcp(&aniPack->parms, aniPack->type);
            break;

        case ANI_TYPE_OLD_BACKGROUND:
            aniPack->funcp(&aniPack->parms, aniPack->type);
            break;

        case ANI_TYPE_TRANS_SWIPE:
        case ANI_TYPE_TRANS_TRANS:
            tCount++;
            aniPack->funcp(&aniPack->parms, aniPack->type);

            /* Check if transition is over */
            //Serial.printf("ElapsTime %lu\r\n", aniPack->parms.p.trans.transElapsTime);
            if ((millis() - aniPack->parms.startTime) >= aniPack->parms.p.trans.transTime) {
                aniPack2 = (AniPack*)GetPriorNode(&aniPack->node);
                AniSetInactive(aniPack);
                aniPack = aniPack2;
                tCount--;
            }

            break;

        
        }
    }

    if (aniInfo.tranInProg && tCount == 0) {
        AniTransDone();
    }

    return numWritten;
}

/* --------------------------------------------------------------------------------------------
 *                 ANIFUNC_FillNoise8()
 * --------------------------------------------------------------------------------------------
 * Description:    
 *
 * Parameters:     
 *
 * Returns:        
 */
void ANIFUNC_FillNoise8(AniParms *Ap, AniType At)
{
    uint16_t x;
    uint16_t y;
    uint16_t xOff;
    uint16_t yOff;
    uint16_t pixNum;
    CRGB     crgb;
    CHSV     chsv;
    static uint16_t xx = random16();
    static uint16_t yy = random16();
    static uint16_t zz = random16();
#if SM_WIDTH > SM_HEIGHT
    const uint32_t longestSide = SM_WIDTH;
    uint8_t noise[SM_WIDTH][SM_WIDTH];
#else
    const uint32_t longestSide = SM_HEIGHT;
    uint8_t noise[SM_HEIGHT][SM_HEIGHT];
#endif

    for (x= 0; x < longestSide; x++) {
        xOff = Ap->scale * x;
        for (y = 0; y < longestSide; y++) {
            yOff = Ap->scale * y;
            noise[x][y] = inoise8(xx + xOff, yy + yOff, zz);
        }
    }
    zz += Ap->speed;

    for (x = 0; x < SM_WIDTH; x++) {
        for (y = 0; y < SM_HEIGHT; y++) {
            // We use the value at the (x,y) coordinate in the noise
            // array for our brightness, and the flipped value from (y,x)
            // for our pixel's hue.
            pixNum = pXY(x, y);
            chsv.setHSV(Ap->hue + (noise[y][x]), 255, noise[x][y]);
            //chsv.setHSV(noise[y][x], 255, noise[x][y]);
            crgb = chsv;
            writePixel(Ap, At, pixNum, (rgb24*)&crgb);
        }
    }
}

/* --------------------------------------------------------------------------------------------
 *                 ANIFUNC_Glitter()
 * --------------------------------------------------------------------------------------------
 * Description:    
 *
 * Parameters:     
 *
 * Returns:        
 */
void ANIFUNC_Glitter(AniParms *Ap, AniType At)
{
    uint32_t totNum;
    uint32_t i, j;
    uint16_t x, y, pixNum;
    uint16_t maxItr;
    rgb24 rgb = rgb24(Ap->color);

    if (Ap->value != 0) {
        return;
    }
    
    uint32_t numToChange = SM_NUM_LEDS / (Ap->p.trans.transTime / 1000) / Ap->fpsLimit;
    numToChange += Ap->scale;

    maxItr = 0;
    totNum = numWritten;
    //Serial.printf("Numtochange %d\r\n", numToChange);
    for (i = 0; (i < numToChange) && (maxItr < numToChange * 2); i += numWritten - totNum - i) {
        maxItr++;
        x = random16(SM_WIDTH);
        y = random16(SM_HEIGHT);
        pixNum = pXY(x,y);
        for (j = 0; j < SM_NUM_LEDS; j++) {

            if ((aniInfo.owners[pixNum % SM_NUM_LEDS] & ANI_TYPE_FREE_OFFSET)) {
                /* Already changed this pix */
                pixNum = pixNum == SM_NUM_LEDS - 1 ? 0 : pixNum + 1;
            } else {
                //Serial.println("Found x,y");
                y = pixNum / SM_WIDTH;
                x = pixNum % SM_WIDTH;
                break;
            }
        }
        if (j == SM_NUM_LEDS) {
            Serial.println("Done");
            Ap->value = 1;
            return;
        }

        writePixel(Ap, At, pXY(x, y), &rgb);
        writePixel(Ap, At, pXY(x + 1, y), &rgb);
        writePixel(Ap, At, pXY(x - 1, y), &rgb);
        writePixel(Ap, At, pXY(x, y + 1), &rgb);
        writePixel(Ap, At, pXY(x, y - 1), &rgb);

    }
//    Serial.printf("printed %d. max iter %d\r\n", numWritten - totNum, maxItr);
}

/*
uint8_t const exp_gamma[256] =
{0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,3,3,3,3,3,
4,4,4,4,4,5,5,5,5,5,6,6,6,7,7,7,7,8,8,8,9,9,9,10,10,10,11,11,12,12,12,13,13,14,14,14,15,15,
16,16,17,17,18,18,19,19,20,20,21,21,22,23,23,24,24,25,26,26,27,28,28,29,30,30,31,32,32,33,
34,35,35,36,37,38,39,39,40,41,42,43,44,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,
61,62,63,64,65,66,67,68,70,71,72,73,74,75,77,78,79,80,82,83,84,85,87,89,91,92,93,95,96,98,
99,100,101,102,105,106,108,109,111,112,114,115,117,118,120,121,123,125,126,128,130,131,133,
135,136,138,140,142,143,145,147,149,151,152,154,156,158,160,162,164,165,167,169,171,173,175,
177,179,181,183,185,187,190,192,194,196,198,200,202,204,207,209,211,213,216,218,220,222,225,
227,229,232,234,236,239,241,244,246,249,251,253,254,255
};
*/

/* --------------------------------------------------------------------------------------------
 *                 ANIFUNC_PlazInt()
 * --------------------------------------------------------------------------------------------
 * Description:    Originally created by //Edmund "Skorn" Horn, adapted to use FastLED functions
 *                 instead.
 * 
 *                 Creates a cool plasma animation. Differerent waves can be seen.
 *
 * Parameters:     Ap - Pointer to animation parameters
 *                 At - The type of animation that is requesting this function create for
 *
 * Returns:        void
 */
void ANIFUNC_PlazInt(AniParms *Ap, AniType At)
{
    uint16_t x, y;
    uint16_t t;
    uint16_t t2;
    uint16_t t3;
    CRGB led;

    Ap->counter++;
    t  = cubicwave8((33 * Ap->counter)/100); // time displacement
    t2 = cubicwave8((11 * Ap->counter)/100); // fiddle with these
    t3 = cubicwave8((8 * Ap->counter)/100); // to change looks
    for (x = 0; x < SM_WIDTH; x++) {
        for (y = 0; y < SM_HEIGHT; y++) {
            //Calculate 3 seperate plasma waves, one for each color channel
            led.r = cubicwave8(((x << 3) + (t >> 1) +
                    cubicwave8((t2 + (y << 3)))));
            led.g = cubicwave8(((y << 3) + t +
                    cubicwave8(((t3 >> 2) + (x << 3)))));
            led.b = cubicwave8(((y << 3) + t2 +
                    cubicwave8((t + x + (led.g >> 2)))));
            led = applyGamma_video(led, 2.1);
            writePixel(Ap, At, pXY(x, y), (rgb24*)&led);

        }
    }
}

/* --------------------------------------------------------------------------------------------
 *                 ANIFUNC_Rainbow()
 * --------------------------------------------------------------------------------------------
 * Description:    
 *
 * Parameters:     
 *
 * Returns:        
 */
void ANIFUNC_Rainbow(AniParms *Ap, AniType At)
{
    // FastLED's built-in rainbow generator
    CHSV   hsv;
    CRGB   crgb;

    hsv.hue = Ap->hue;
    hsv.val = 255;
    hsv.sat = 240;
    for( int i = 0; i < SM_NUM_LEDS; ++i) {
        crgb = hsv;
        writePixel(Ap, At, i, (rgb24*)&crgb);
        hsv.hue += Ap->speed;
    }

    Ap->hue += Ap->speed;
}

/* --------------------------------------------------------------------------------------------
 *                 ANIFUNC_Confetti()
 * --------------------------------------------------------------------------------------------
 * Description:    
 *
 * Parameters:     
 *
 * Returns:        
 */
void ANIFUNC_Confetti(AniParms *Ap, AniType At)
{
    uint16_t written;
    uint16_t i;
    CRGB crgb = CHSV(Ap->hue + random8(64), 200, 255);

    // Random colored speckles that blink in and fade smoothly
    for (i = 0; i < SM_NUM_LEDS; i++) {
        if (aniInfo.owners[i] & Ap->type) {
            crgb.setRGB(aniInfo.ledBuff[i].red, aniInfo.ledBuff[i].green, aniInfo.ledBuff[i].blue);
            //Serial.printf("Before crgb r,b,g (%d,%d,%d)\r\n", crgb.r, crgb.b, crgb.g);
            crgb.nscale8(Ap->scale);
            //Serial.printf("After crgb r,b,g (%d,%d,%d)\r\n", crgb.r, crgb.b, crgb.g);
            writePixel(Ap, Ap->type, i, (rgb24*)&crgb);
            if (!crgb) {
                aniInfo.owners[i] &= 0x00FF;
            }
        }
    }
    
    written = numWritten;
    i = random16(SM_NUM_LEDS);
    crgb = CHSV(Ap->hue + random8(64), 200, 255);
    writePixel(Ap, At, i, (rgb24*)&crgb);
    if (numWritten > written) {
        /* Assign that to new value */
        aniInfo.owners[i] |= Ap->type;
    }

}
#if 0

void sinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy(leds, SM_NUM_LEDS, 5);
    int pos = beatsin16(13, 0, SM_NUM_LEDS - 1);
    leds[pos] += CHSV(gHue, 255, 192);
}

void bpm()
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette  = PartyColors_p;
    uint8_t beat           = beatsin8(BeatsPerMinute, 64, 255);
    for (int i = 0; i < SM_NUM_LEDS; i++) { // 9948
        leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
    }
}

void juggle()
{
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy(leds, SM_NUM_LEDS, 5);
    uint8_t dothue = 0;
    for (int i = 0; i < 8; i++) {
        leds[beatsin16(i + 7, 0, SM_NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
        dothue += 32;
    }
}
#endif

/* --------------------------------------------------------------------------------------------
 *                 AniSetInactive()
 * --------------------------------------------------------------------------------------------
 * Description:    
 *
 * Parameters:     
 *
 * Returns:        
 */
void AniSetInactive(AniPack *Ap)
{
    RemoveNode(&Ap->node);

    if (Ap->type & (ANI_TYPE_MAIN_OFFSET | ANI_TYPE_OLD_OFFSET)) {
        Serial.println("benging main/old");
        InsertTail(&aniInfo.mainWaitList, &Ap->node);
        aniInfo.numMainWaiting++;
    } else {
        Serial.println("benging trans");
        InsertTail(&aniInfo.transWaitList, &Ap->node);
        aniInfo.numTransWaiting++;
    }
}

/* --------------------------------------------------------------------------------------------
 *                 AniSetInactive()
 * --------------------------------------------------------------------------------------------
 * Description:    
 *
 * Parameters:     
 *
 * Returns:        
 */
void AniTransDone()
{
    AniPack  *aniPack;
    ListNode *nodeItr;
    uint16_t  i;

    aniInfo.tranInProg = false;

    Serial.println("In AniTransDone");

    nodeItr = &aniInfo.activeList;
    while ((nodeItr = GetNextNode(nodeItr)) != &aniInfo.activeList) {
        aniPack = (AniPack*)nodeItr;
        if (aniPack->type & ANI_TYPE_OLD_OFFSET) {
            nodeItr = GetPriorNode(nodeItr);

            /* Assign the "Main" type */
            aniPack->type &= ~ANI_TYPE_OLD_OFFSET;
            aniPack->type |= ANI_TYPE_MAIN_OFFSET;

            if (aniPack->parms.type != 0) {
                /* Need to check if any of these special types exists and change them to free */
                for (i = 0; i < SM_NUM_LEDS; i++) {
                    aniInfo.owners[i] &= ~aniPack->parms.type;
                }
            }
            AniSetInactive(aniPack);
        }
    }
}

/* --------------------------------------------------------------------------------------------
 *                 AniCheckTranDel()
 * --------------------------------------------------------------------------------------------
 * Description:    
 *
 * Parameters:     
 *
 * Returns:        
 */
bool AniCheckTranDel(AniParms *Ap)
{
    return (millis() > Ap->delay);
}

/* --------------------------------------------------------------------------------------------
 *                 writePixel()
 * --------------------------------------------------------------------------------------------
 * Description:    
 *
 * Parameters:     
 *
 * Returns:        
 */
void writePixel(AniParms *Ap, AniType At, uint32_t PixNum, rgb24 *RgbVal)
{
    if (PixNum >= SM_NUM_LEDS) {
        //Serial.println("Overbounds!");
        //PixNum = SM_NUM_LEDS - 1;
        return;
    }
    AniType pixOwner = aniInfo.owners[PixNum];
    //Serial.printf("owner at this pix %d is 0x%x\r\n", PixNum, aniInfo.owners[PixNum]);

    switch(At) {
    case ANI_TYPE_MASK:
        if ((Ap->p.mask.maskType[PixNum] == At) &&
            ((pixOwner == ANI_TYPE_FREE) ||
             (pixOwner == ANI_TYPE_MASK))) {
            goto write;
        }
        break;

    case ANI_TYPE_FOREGROUND:
    case ANI_TYPE_BACKGROUND:
        if (pixOwner == ANI_TYPE_FREE) {
            goto write;
        } else if ((pixOwner == ANI_TYPE_OLD_FREE) && !aniInfo.tranInProg) {
            aniInfo.owners[PixNum] = ANI_TYPE_FREE;
            goto write;
        }
        break;

    case ANI_TYPE_OLD_FOREGROUND:
    case ANI_TYPE_OLD_BACKGROUND:
        if ((pixOwner == ANI_TYPE_OLD_FREE)) {
            goto write;
        }
        break;

        case ANI_TYPE_TRANS_SWIPE:
        if (pixOwner != ANI_TYPE_FREE) {
            aniInfo.owners[PixNum] = ANI_TYPE_FREE;
            goto write;
        }
        break;

    case ANI_TYPE_TRANS_TRANS:
        if (pixOwner != ANI_TYPE_FREE) {
            aniInfo.owners[PixNum] = ANI_TYPE_FREE;
            goto skip;
        }
        break;

    default:
        if (pixOwner & At) {
            goto write;
        }
        break;
    }

    /* Don't write the pixel */
    return;

write:
    aniInfo.ledBuff[PixNum] = *RgbVal;
skip:
    //aniInfo.drawn[*PixIndx] = PixNum; /* commented because I don't think i need this anymore after optimizations. */
    numWritten++;
}

