/* ********************************************************************************************
 * animations.cpp
 *
 * Author: Shawn Saenger
 *
 * Created: Sep 8, 2023
 *
 * Description: Holds the logic to performing advance animations. Manages a list of animation
 *              function pointers and fills out an RGB buffer. This file also contains
 *              some animations available for use.
 *
 * ********************************************************************************************
 */

#include "../inc/sys/animations_i.h"


/* --------------------------------------------------------------------------------------------
 *  MACROS
 * --------------------------------------------------------------------------------------------
 */

#define RETURN_IS_VALID_RIGHT(p)   {                                          \
                                        if ((currAc >= (p)->crit) ||          \
                                            ((p)->crit == ANI_CRIT_BLEND)) {  \
                                            return true;                      \
                                        }                                     \
                                        return false;                         \
                                    }

/* --------------------------------------------------------------------------------------------
 *  GLOBALS
 * --------------------------------------------------------------------------------------------
 */


static AniInfo     aniInfo;
static uint16_t    numWritten;
static AniCriteria currAc;

/* --------------------------------------------------------------------------------------------
 *  PROTOTYPES
 * --------------------------------------------------------------------------------------------
 */
static void AniSetInactive(AniPack *Ap);
static void AniTransDone();
static bool AniCheckTranDel(AniParms *Ap);

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
 * Returns:        true if successful. false otherwise.
 */
bool ANI_Init(void)
{
    uint16_t i;

    // Sanity check. Check values between two types are equal
    if (!((ANI_LAYER_UNASSIGNED == ANI_CRIT_BELOW_ANY) &&
        (ANI_LAYER_BOTTOM == ANI_CRIT_LOW) &&
        (ANI_LAYER_MIDDLE == ANI_CRIT_MEDIUM) &&
        (ANI_LAYER_TOP == ANI_CRIT_HIGH) &&
        (ANI_LAYER_PERSISTENT == ANI_CRIT_HIGH_PERSISTENT) &&
        (ANI_LAYER_TRANSITION == ANI_CRIT_TRANSITION))) {

        return false;
    }

    InitList(&aniInfo.activeList);
    InitList(&aniInfo.mainWaitList);
    InitList(&aniInfo.transWaitList);
    InitList(&aniInfo.queueList);
    aniInfo.numMainWaiting = 0;
    aniInfo.numTransWaiting = 0;

    aniInfo.pix = (AniPixel*)malloc(sizeof(AniPixel) * LEDI_NUM_LEDS);
    if (aniInfo.pix == 0) {
        Serial.println("Could not allocate memory for animations");
        return false;
    }

    for (i = 0; i < LEDI_NUM_LEDS; i++) {
        InitNode(&aniInfo.pix[i].node);
        aniInfo.pix[i].pixNum = i;
        aniInfo.pix[i].color.setRGB(0, 0, 0);
        aniInfo.pix[i].crit = ANI_CRIT_DEFAULT;
    }

    return true;
}

/* --------------------------------------------------------------------------------------------
 *                 ANI_AddAnimation()
 * --------------------------------------------------------------------------------------------
 * Description:    Registeres a AniPack to a list of available animations. Available animations
 *                 can be added to a pending list through ANI_AddNextAnimation() and then
 *                 are active once ANI_SwapAnimation() is called.
 *
 * Parameters:     Ap - A pointer to a AniPack with the fields filled out according to its
 *                      description.
 *                 DefaultLayer - The type of animation inside "Ap->funcp". 
 *                     
 *
 * Returns:        true it was added, false otherwise beceuse the type is bad or the AniPack
 *                 was already added to a list. It could also be that "Ap->node" was not iniitialized
 *                 with InitNode() yet.
 */
bool ANI_RegisterAnimation(AniPack *Ap, AniLayer DefaultLayer)
{
    Serial.printf("activeList.F 0x%x, activeList.B 0x%x, Ap->node.F 0x%x, Ap->node.B 0x%x ",
    aniInfo.activeList.linkF, aniInfo.activeList.linkB, Ap->node.linkF, Ap->node.linkB);
    //if ((IsNodeOnList(&aniInfo.activeList, &Ap->node))) {
    // ||
        //(IsNodeOnList(&aniInfo.transWaitList, &Ap->node)) || 
        //(IsNodeOnList(&aniInfo.mainWaitList, &Ap->node))) {
        // Node is already registered
    //    Serial.println("ANI_RegisterAnimation returning false");
    //    return false;
    //}
    delay(10);
    Serial.println("ANI_RegisterAnimation 1");
    InitNode(&Ap->node);
    Ap->defaultLayer = DefaultLayer;

    if (DefaultLayer & ANI_LAYER_TRANSITION) {
        /* Place this in the waiting main list */
        InsertTail(&aniInfo.transWaitList, &Ap->node);
        aniInfo.numTransWaiting++;
    } else {
        InsertTail(&aniInfo.mainWaitList, &Ap->node);
        aniInfo.numMainWaiting++;
    }
    Serial.println("ANI_RegisterAnimation 2");
    delay(10);
    return true;
}

/* --------------------------------------------------------------------------------------------
 *                 ANI_UnregisterAnimation()
 * --------------------------------------------------------------------------------------------
 * Description:    Unregistered an AniPack that was previously registered
 *
 * Parameters:     AniPack - a pointer to a AniPack that was added earlier
 *                 Force - forcefully remove an animation even if it is actively being used.
 *                         If true, this function is guaranteed to return true as well.
 *
 * Returns:        true if it was removed or never registered. false if it cannot be removed
 */
bool ANI_UnregisterAnimation(AniPack *Ap, bool Force)
{
    /* Note: calling IsNodeUsed() here can be problematic if caller never called
     * ANI_RegisterAnimation() in case Ap->node is uninitialized. So a work-around
     * is to check if the node is on a list, not just if the node is being used.
    */
    if (IsNodeOnList(&aniInfo.activeList, &Ap->node)) {
        if (!Force) {
            /* Currently in use */
            return false;
        }
        AniSetInactive(Ap);
    }

    if (IsNodeOnList(&aniInfo.mainWaitList, &Ap->node)) {
        aniInfo.numMainWaiting--;
        RemoveNode(&Ap->node);
    } else if (IsNodeOnList(&aniInfo.transWaitList, &Ap->node)) {
        aniInfo.numTransWaiting--;
        RemoveNode(&Ap->node);
    }

    return true;
}

/* --------------------------------------------------------------------------------------------
 *                 ANI_AddNextAnimation()
 * --------------------------------------------------------------------------------------------
 * Description:    Queues an animations. Once ANI_SwapAnimation() is called, queued
 *                 animations will be moved to the active list. If any animations are transition
 *                 type animations, the previous main animations will be phased out over time.
 *                 If there are no transition type animations, then the previous main animations
 *                 will retire immediately, unless the "blend" option is chosen in
 *                 ANI_SwapAnimation()
 *
 * Parameters:     Ap - A pointer to a AniPack with that was added with ANI_RegisterAnimation().
 *                      This animation will be added to the queue.
 *                 OverrideLayer - Select a different layer to play this animation different
 *                                  than the default. Set to 0 to keep the default layer.
 *
 * Returns:        true if moved from waiting list to queue list. false if did not
 */
bool ANI_AddNextAnimation(AniPack *Ap, AniLayer OverrideLayer)
{
    Serial.println("ANI_AddNextAnimation 1");
    if (IsNodeOnList(&aniInfo.transWaitList, &Ap->node)) {
        RemoveNode(&Ap->node);
        aniInfo.numTransWaiting--;
        Serial.println("ANI_AddNextAnimation 2");
    } else if (IsNodeOnList(&aniInfo.mainWaitList, &Ap->node)) {
        RemoveNode(&Ap->node);
        aniInfo.numMainWaiting--;
        Serial.println("ANI_AddNextAnimation 3");
    } else {
        return false;
    }

    Serial.printf("ANI_AddNextAnimation: OverrideLayer 0x%x, Ap->defaultLayer 0x%x\r\n",
                    OverrideLayer, Ap->defaultLayer);
    if (OverrideLayer > ANI_LAYER_UNASSIGNED) {
        Serial.println("ANI_AddNextAnimation 3.1");
        Ap->currCriteria = (AniCriteria)OverrideLayer;
    } else {
        Serial.println("ANI_AddNextAnimation 3.2");
        Ap->currCriteria = (AniCriteria)Ap->defaultLayer;
    }

    /* Insert into queue list */
    InsertTail(&aniInfo.queueList, &Ap->node);
    Serial.printf("ANI_AddNextAnimation 4. Ap->currCriteria == 0x%x\n\r", Ap->currCriteria);

    return true;
}

/* --------------------------------------------------------------------------------------------
 *                 ANI_AddNextAnimationByFuncP()
 * --------------------------------------------------------------------------------------------
 * Description:    Queue an animation by providing its function pointer. The first registered
 *                 animation that matches the function pointer will be added, unless it is
 *                 already added, in which case it will attempt the to add the next one if
 *                 one exists. Once ANI_SwapAnimation() is called, queued
 *                 animations will be moved to the active list. If any animations are transition
 *                 type animations, the previous main animations will be phased out over time.
 *                 If there are no transition type animations, then the previous main animations
 *                 will retire immediately, unless the "blend" option is chosen in
 *                 ANI_SwapAnimation()
 *
 * Parameters:     FuncP - A pointer to the animation function. Must have already been registered
 *                         via ANI_RegisterAnimation().
 *                 OverrideLayer - Select a different layer to play this animation different
 *                                  than the default. Set to 0 to keep the default layer.
 *
 * Returns:        true if moved from waiting list to queue list. false if did not
 */
bool ANI_AddNextAnimationByFuncP(AniFunc FuncP, AniLayer OverrideLayer)
{
    AniPack *ap;

    IterateList(aniInfo.mainWaitList, ap, AniPack*) {
        if (FuncP == ap->funcp) {
            /* Found a matching animation */
            aniInfo.numMainWaiting--;
            goto found;
        }
    }
    IterateList(aniInfo.transWaitList, ap, AniPack*) {
        if (FuncP == ap->funcp) {
            /* Found a matching transition animation */
            aniInfo.numTransWaiting--;
            goto found;
        }
    }

    /* Function is not registered or is already added to queue */
    return false;

found:
    if (OverrideLayer > ANI_LAYER_UNASSIGNED) {
        ap->currCriteria = (AniCriteria)OverrideLayer;
    } else {
        ap->currCriteria = (AniCriteria)ap->defaultLayer;
    }

    /* Insert into queue list */
    MoveNodeBefore(&aniInfo.queueList, &ap->node);
    Serial.printf("ANI_AddNextAnimation 4. Ap->currCriteria == 0x%x\n\r", ap->currCriteria);

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
void ANI_SwapAnimation(bool UseBlending)
{
    AniPack  *aniPack;
    AniPack  *aniPack2;
    ListNode *nodeItr;
    uint16_t  i;
    bool      inserted;
    bool      transPresent = false;


    /* Step 1: Check if any trans animations are present in the queue list */
    IterateList(aniInfo.queueList, aniPack, AniPack *) {
        Serial.println("ANI_SwapAnimation: iter step 1");
        if (aniPack->currCriteria & ANI_CRIT_TRANSITION) {
            if (UseBlending) {
                AniSetInactive(aniPack);
            } else {
                transPresent = true;
                aniInfo.tranInProg = true;
                break;
            }
        }
    }

    /* Step 2: Set active animation criteria to below normal */
    Serial.println("ANI_SwapAnimation: step 2");
    if (transPresent) {
        //Serial.println("ANI_SwapAnimation: transpresent");
        for (i = 0; i < LEDI_NUM_LEDS; i++) {
            aniInfo.pix[i].crit = ANI_CRIT_BELOW_LOW;
        }
        /* Like IterateListSafely(), this while loop lets you safely remove
         * a node from a list while iterating through it.
         */
        nodeItr = &aniInfo.activeList;
        while ((nodeItr = GetNextNode(nodeItr)) != &aniInfo.activeList) {
            Serial.println("ANI_SwapAnimation: 1");
            aniPack = (AniPack*)nodeItr;
            if (aniPack->currCriteria == ANI_CRIT_TRANSITION) {
                nodeItr = GetPriorNode(nodeItr); /* Get prior node to prevent breaking list */
                /* Remove old transition */
                AniSetInactive(aniPack); /* Removes node from list */
                continue;
            }

            /* Change the active criteria to its old criteria preserving the persistent flag */
            aniPack->currCriteria = ((aniPack->currCriteria >> 4) | (aniPack->currCriteria & ANI_CRIT_PERSISTENT));
        }
    } else {
        while (!IsListEmpty(&aniInfo.activeList)) {
            Serial.println("ANI_SwapAnimation: 1");
            aniPack = (AniPack*)GetHead(&aniInfo.activeList);
            AniSetInactive(aniPack);
        }
        /* Since no transaction, black out all the pixels */
        for (i = 0; i < LEDI_NUM_LEDS; i++) {
            aniInfo.pix[i].color = 0;
        }
    }

    /* Step 3: Move all animations from queue list to active list */
    aniInfo.fpsTarg = 0;
    while (!IsListEmpty(&aniInfo.queueList)) {
        aniPack = (AniPack*)GetHead(&aniInfo.queueList);
        Serial.printf("ANI_SwapAnimation: queue type 0x%x\r\n", aniPack->currCriteria);

        aniPack->parms.startTime = millis();
        aniPack->parms.delay = millis();
        aniPack->parms.value = 0;
        aniInfo.fpsTarg = max(aniInfo.fpsTarg, aniPack->parms.fpsTarg);

        /* Insert into the active list in its appropriate spot */
        inserted = false;
        IterateList(aniInfo.activeList, aniPack2, AniPack *) {
            if (aniPack->currCriteria >= aniPack2->currCriteria) {
                /* Insert before this node */
                MoveNodeBefore(&aniPack2->node, &aniPack->node);
                inserted = true;
                break;
            }
        }
        if (!inserted) {
            RemoveNode(&aniPack->node);
            InsertTail(&aniInfo.activeList, &aniPack->node);
        }
    }
//    IterateList(aniInfo.activeList, aniPack2, AniPack *) {
//        Serial.println(aniPack2->currCriteria);
//    }
}

/* --------------------------------------------------------------------------------------------
 *                 ANI_CheckPixNum()
 * --------------------------------------------------------------------------------------------
 * Description:    
 *
 * Parameters:     
 *
 * Returns:        
 */
AniPixel *ANI_CheckPixNum(uint32_t PixNum)
{
    AniPixel *pix;
    if (PixNum >= LEDI_NUM_LEDS) {
        return 0;
    }
    pix = &aniInfo.pix[PixNum];

    if ((currAc >= pix->crit) || ((pix)->crit == ANI_CRIT_BLEND)) {
        switch (currAc) {
        case ANI_CRIT_BELOW_LOW:
        case ANI_CRIT_BELOW_MEDIUM:
        case ANI_CRIT_BELOW_HIGH:
        case ANI_CRIT_BELOW_HIGH_PERSISTENT:
            // Only the animations being transitioned out can write to
            // this pixel
            if ((pix->crit & ANI_CRIT_BELOW_ANY) != 0) {
                Serial.println("ANI_CheckPixNum returns 0 first");
                return 0;
            }
            break;
        
        default:
            if (pix->crit & ANI_CRIT_BELOW_ANY) {
                // Anything that is not an exclusive can't write
                // to this pixel
                Serial.println("ANI_CheckPixNum returns 0 second");
                return 0;
            }
        }
        return pix;
    }
    return 0;
}

/* --------------------------------------------------------------------------------------------
 *                 ANI_CheckPix()
 * --------------------------------------------------------------------------------------------
 * Description:    
 *
 * Parameters:     
 *
 * Returns:        
 */
bool ANI_CheckPix(AniPixel *Pix)
{
    if ((currAc >= Pix->crit) || (Pix->crit == ANI_CRIT_BLEND)) {
        switch (currAc) {
        case ANI_CRIT_BELOW_LOW:
        case ANI_CRIT_BELOW_MEDIUM:
        case ANI_CRIT_BELOW_HIGH:
        case ANI_CRIT_BELOW_HIGH_PERSISTENT:
            // Only the animations being transitioned out can write to
            // this pixel
            if ((Pix->crit & ANI_CRIT_BELOW_ANY) != 0) {
                return 0;
            }
            break;
        
        default:
            if (Pix->crit & ANI_CRIT_BELOW_ANY) {
                // Anything that is not an exclusive can't write
                // to this pixel
                return 0;
            }
        }
        return true;
    }
    return false;
}

/* --------------------------------------------------------------------------------------------
 *                 ANI_WriteVerifiedPix()
 * --------------------------------------------------------------------------------------------
 * Description:    
 *
 * Parameters:     
 *
 * Returns:        
 */
void ANI_WriteVerifiedPix(AniParms *Ap, AniPixel *Pix, const CRGB &RgbVal)
{
    AniCriteria  currCrit = currAc;
    if (Pix->crit == ANI_CRIT_BLEND) {

    } else {

        switch (currCrit) {
        case ANI_CRIT_BELOW_HIGH_PERSISTENT:
            if (!RgbVal) {
                currCrit = ANI_CRIT_BELOW_LOW;
            }
            break;

        case ANI_CRIT_HIGH_PERSISTENT:
        case ANI_CRIT_TRANSITION:
            if (!RgbVal) {
                currCrit = ANI_CRIT_DEFAULT;
            }
            break;

        default:
            break;
        }
        // Save this criteria and color
        Pix->crit = currCrit;
        Pix->color = RgbVal;
        numWritten++;
    }
}

/* --------------------------------------------------------------------------------------------
 *                 ANI_WritePixel()
 * --------------------------------------------------------------------------------------------
 * Description:    
 *
 * Parameters:     
 *
 * Returns:        
 */
void ANI_WritePixel(AniParms *Ap, uint32_t PixNum, const CRGB &RgbVal)
{
    AniPixel *pix;
    AniCriteria  currCrit = currAc;

    if (PixNum >= LEDI_NUM_LEDS) {
        Serial.println("Overbounds!");
        //PixNum = LEDI_NUM_LEDS - 1;
        return;
    }
    pix = &aniInfo.pix[PixNum];
    //Serial.printf("owner at this pix %d is 0x%x\r\n", PixNum, aniInfo.owners[PixNum]);

    if (currCrit >= pix->crit) {

        switch (currCrit) {
        case ANI_CRIT_BELOW_LOW:
        case ANI_CRIT_BELOW_MEDIUM:
        case ANI_CRIT_BELOW_HIGH:
        case ANI_CRIT_BELOW_HIGH_PERSISTENT:
            // Only the animations being transitioned out can write to
            // this pixel
            if ((currCrit & ANI_CRIT_BELOW_ANY) != 0) {
                return;
            } else if ((currCrit == ANI_CRIT_BELOW_HIGH_PERSISTENT) && !RgbVal) {
                currCrit = ANI_CRIT_BELOW_LOW;
            }
            break;

        case ANI_CRIT_HIGH_PERSISTENT:
        case ANI_CRIT_TRANSITION:
            if (!RgbVal) { 
                currCrit = ANI_CRIT_LOW;
            }
            break;

        default:
            if (pix->crit & ANI_CRIT_BELOW_ANY) {
                // Anything that is not a transition can't write
                // to this pixel
                return;
            }
            break;
        }
        // Save this criteria and color
        pix->crit = currCrit;
        pix->color = RgbVal;
        numWritten++;
    } else if (pix->crit == ANI_CRIT_BLEND) {
        
    }
}

/* --------------------------------------------------------------------------------------------
 *                 ANI_DrawAnimationFrame()
 * --------------------------------------------------------------------------------------------
 * Description:    Draw an animation frame funcs added earlier
 *
 * Parameters:     drawBuff - pointer to a LED buffer to fill
 *
 * Returns:        number of pixels written to the LedBuff. This counts pixels that were
 *                 written more than once.
 */
uint32_t ANI_DrawAnimationFrame(rgb24 *LedBuff)
{
    uint32_t tCount;
    uint32_t now;
    AniPack *aniPack;
    AniPack *aniPack2;
    

    aniInfo.drawBuff = LedBuff;
    numWritten = 0;
    tCount = 0;
    now = millis();

    /* Sort through each animation and check if it should be */
    //Serial.println("ANI_DrawAnimationFrame: Begin");
    if (now < aniInfo.msDelay) {
        /* Not yet time to draw a frame */
        return 0;
    }

    /* Time to draw a frame! Get the next delay needed before we can draw the next frame */
    aniInfo.msDelay = now + fps2Ms(aniInfo.fpsTarg);

    IterateList(aniInfo.activeList, aniPack, AniPack*) {

        //Serial.printf("ANI_DrawAnimationFrame: criteria 0x%x. delay %lu\r\n", aniPack->currCriteria, aniPack->parms.delay);
        currAc = aniPack->currCriteria;
        switch (currAc) {
        case ANI_CRIT_TRANSITION:
            tCount++;
            aniPack->funcp(&aniPack->parms);

            /* Check if transition is over */
            //Serial.printf("ElapsTime %lu\r\n", aniPack->parms.p.trans.transElapsTime);
            if ((millis() - aniPack->parms.startTime) >= aniPack->parms.p.trans.transTime) {
                aniPack2 = (AniPack*)GetPriorNode(&aniPack->node);
                AniSetInactive(aniPack);
                aniPack = aniPack2;
                tCount--;
            }
            break;

        default:
            aniPack->funcp(&aniPack->parms);
            break;
        }
    }

    if (aniInfo.tranInProg && tCount == 0) {
        AniTransDone();
    }

    if (numWritten > 0) {
        AniWriteToBuffer();
        return LEDI_NUM_LEDS;
    }
    return 0;
}

#if 0
/* --------------------------------------------------------------------------------------------
 *                 ANIFUNC_FillNoise8()
 * --------------------------------------------------------------------------------------------
 * Description:    
 *
 * Parameters:     
 *
 * Returns:        
 */
void ANIFUNC_FillNoise8(AniParms *Ap)
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
    const uint32_t longestSide = LEDI_WIDTH;
    uint8_t noise[LEDI_WIDTH][LEDI_WIDTH];
#else
    const uint32_t longestSide = LEDI_HEIGHT;
    uint8_t noise[LEDI_HEIGHT][LEDI_HEIGHT];
#endif

    for (x= 0; x < longestSide; x++) {
        xOff = Ap->scale * x;
        for (y = 0; y < longestSide; y++) {
            yOff = Ap->scale * y;
            noise[x][y] = inoise8(xx + xOff, yy + yOff, zz);
        }
    }
    zz += Ap->speed;

    for (x = 0; x < LEDI_WIDTH; x++) {
        for (y = 0; y < LEDI_HEIGHT; y++) {
            // We use the value at the (x,y) coordinate in the noise
            // array for our brightness, and the flipped value from (y,x)
            // for our pixel's hue.
            pixNum = pXY(x, y);
            chsv.setHSV(Ap->hue + (noise[y][x]), 255, noise[x][y]);
            //chsv.setHSV(noise[y][x], 255, noise[x][y]);
            crgb = chsv;
            writePixel(Ap, At, pixNum, crgb);
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
void ANIFUNC_Glitter(AniParms *Ap)
{
    uint32_t totNum;
    uint32_t i, j;
    uint16_t x, y, pixNum;
    uint16_t maxItr;
    rgb24 rgb = rgb24(Ap->color);

    if (Ap->value != 0) {
        return;
    }
    
    uint32_t numToChange = LEDI_NUM_LEDS / (Ap->p.trans.transTime / 1000) / Ap->fpsTarg;
    numToChange += Ap->scale;

    maxItr = 0;
    totNum = numWritten;
    //Serial.printf("Numtochange %d\r\n", numToChange);
    for (i = 0; (i < numToChange) && (maxItr < numToChange * 2); i += numWritten - totNum - i) {
        maxItr++;
        x = random16(LEDI_WIDTH);
        y = random16(LEDI_HEIGHT);
        pixNum = pXY(x,y);
        for (j = 0; j < LEDI_NUM_LEDS; j++) {

            if ((aniInfo.owners[pixNum % LEDI_NUM_LEDS] & ANI_TYPE_FREE_OFFSET)) {
                /* Already changed this pix */
                pixNum = pixNum == LEDI_NUM_LEDS - 1 ? 0 : pixNum + 1;
            } else {
                //Serial.println("Found x,y");
                y = pixNum / LEDI_WIDTH;
                x = pixNum % LEDI_WIDTH;
                break;
            }
        }
        if (j == LEDI_NUM_LEDS) {
            Serial.println("Done");
            Ap->value = 1;
            return;
        }

        writePixel(Ap, At, pXY(x, y), rgb);
        writePixel(Ap, At, pXY(x + 1, y), rgb);
        writePixel(Ap, At, pXY(x - 1, y), rgb);
        writePixel(Ap, At, pXY(x, y + 1), rgb);
        writePixel(Ap, At, pXY(x, y - 1), rgb);

    }
//    Serial.printf("printed %d. max iter %d\r\n", numWritten - totNum, maxItr);
}

#endif

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
void ANIFUNC_PlazInt(AniParms *Ap)
{
    uint16_t x, y;
    uint16_t t;
    uint16_t t2;
    uint16_t t3;
    CRGB led;
    AniPixel *currAniPix;

    Ap->counter++;
    t  = cubicwave8((33 * Ap->counter)/100); // time displacement
    t2 = cubicwave8((8 * Ap->counter)/100); // fiddle with these
    t3 = cubicwave8((15 * Ap->counter)/100); // to change looks
    for (x = 0; x < LEDI_WIDTH; x++) {
        for (y = 0; y < LEDI_HEIGHT; y++) {
            if ((currAniPix = ANI_CheckPixNum(pXY(x, y))) != 0) {
                //Calculate 3 seperate plasma waves, one for each color channel
                led.r = cubicwave8(((x << 3) + (t >> 1) +
                        cubicwave8((t2 + (y << 3)))));
                led.g = cubicwave8(((y << 3) + t +
                        cubicwave8(((t3 >> 2) + (x << 3)))));
                led.b = triwave8(((y << 3) + t2 +
                        triwave8((t + x + (led.g >> 2)))));
#if 1 /* Faster, more memory */
                led.b = exp_gamma[led.b];
                led.g = exp_gamma[led.g];
                led.r = exp_gamma[led.r];
#else
                led = applyGamma_video(led, 2.1);
#endif
                ANI_WriteVerifiedPix(Ap, currAniPix, led);
            }
        }
    }
}

/* --------------------------------------------------------------------------------------------
 *                 ANIFUNC_RainbowIris()
 * --------------------------------------------------------------------------------------------
 * Description:    
 *
 * Parameters:     
 *
 * Returns:        
 */
void ANIFUNC_RainbowIris(AniParms *Ap)
{
    // FastLED's built-in rainbow generator
    CHSV   hsvX, hsvY;
    CRGB   crgb;
    uint16_t x, y;
    uint16_t scale = Ap->scale;

    hsvX = Ap->hsv;
    hsvY = hsvX;
    for (x = 0; x < LEDI_WIDTH / 2; x++) {
        if ((x % 3) == 0) {
            scale++;
        }
        //hsvX.h += scale >> 2;
        hsvY = hsvX;
        for (y = 0; y < LEDI_HEIGHT / 2; y++) {
            hsvY.h += scale >> 1;
            crgb = hsvY;
            ANI_WritePixel(Ap, pXY(x, y), crgb);
            ANI_WritePixel(Ap, pXY(x, LEDI_HEIGHT - 1 - y), crgb);
            ANI_WritePixel(Ap, pXY(LEDI_WIDTH - 1 - x, y), crgb);
            ANI_WritePixel(Ap, pXY(LEDI_WIDTH - 1 - x, LEDI_HEIGHT - 1 - y), crgb);
        }
    }

    Ap->hsv.h += Ap->speed;
}
/* --------------------------------------------------------------------------------------------
 *                 ANIFUNC_Confetti()
 * --------------------------------------------------------------------------------------------
 * Description: Pixels blink and fade out by randomly adding 1 pixel in a draw frame and reducing
 *              pixels that have been drawn in previous frames until they are are blacked out.
 *
 * Parameters:  Ap - Pointer to AniParms data where:
 *                   hsv: starting hue,sat,val
 *                   scale: Defines how fast the pixel fades to 0. A higher scale value means
 *                          the pixel fades slower.
 *              At - Type of animation (Recommendation: ANI_TYPE_FOREGROUND)
 *
 * Returns:     void      
 */
void ANIFUNC_Confetti(AniParms *Ap)
{
    AniPixel *currAniPix;
    AniPixel *nextAniPix;
    CRGB crgb;
    
    //Serial.println("ANIFUNC_Confetti 1");
    // Random colored speckles that blink in and fade smoothly
    IterateListSafely(Ap->pixList, currAniPix, nextAniPix, AniPixel*) {
        // Wrote to this in the past. Check if we still can
        if (ANI_CheckPix(currAniPix)) {
            crgb = currAniPix->color;
            //Serial.printf("Before crgb r,b,g (%d,%d,%d)\r\n", crgb.r, crgb.b, crgb.g);
            crgb.nscale8(Ap->scale);
            //Serial.printf("After crgb r,b,g (%d,%d,%d)\r\n", crgb.r, crgb.b, crgb.g);
            ANI_WriteVerifiedPix(Ap, currAniPix, crgb);
            if (!crgb) {
                // Pixel faded to black, so remove from list */
                RemoveNode(&currAniPix->node);
            }
        } else {
            // A higher layer animation wrote to this pixel. Since we lost the color
            // value we wrote, we will just remove this from the list now.
            RemoveNode(&currAniPix->node);
        }
    }
    //Serial.println("ANIFUNC_Confetti 2");
    if ((currAniPix = ANI_CheckPixNum(random16(LEDI_NUM_LEDS))) != 0) {
        // Make sure this pixel is not already on a list. Adding it to a list
        // (whether its the same list or different one), will corrupt the previous list.
        if (!IsNodeUsed(&currAniPix->node)) {
            crgb.setHue(Ap->hsv.hue + random8(64));
            //Serial.println("ANIFUNC_Confetti 2.2");
            ANI_WriteVerifiedPix(Ap, currAniPix, crgb);
            InsertTail(&Ap->pixList, &currAniPix->node);
        }
    }

    //Serial.println("ANIFUNC_Confetti 3");
}
#if 0

void sinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy(leds, LEDI_NUM_LEDS, 5);
    int pos = beatsin16(13, 0, LEDI_NUM_LEDS - 1);
    leds[pos] += CHSV(gHue, 255, 192);
}

void bpm()
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette  = PartyColors_p;
    uint8_t beat           = beatsin8(BeatsPerMinute, 64, 255);
    for (int i = 0; i < LEDI_NUM_LEDS; i++) { // 9948
        leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
    }
}

void juggle()
{
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy(leds, LEDI_NUM_LEDS, 5);
    uint8_t dothue = 0;
    for (int i = 0; i < 8; i++) {
        leds[beatsin16(i + 7, 0, LEDI_NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
        dothue += 32;
    }
}
#endif

// --------------------------------------------------------------------------
//  Internal Functions
// --------------------------------------------------------------------------

/* --------------------------------------------------------------------------------------------
 *                 AniWriteToBuffer()
 * --------------------------------------------------------------------------------------------
 * Description:   TODO this can be improved if we swap between two different lists. Items on
 *                list are pixels that haven't been drawn yet. 
 *
 * Parameters:     
 *
 * Returns:        
 */
void AniWriteToBuffer(void)
{
    uint16_t i;
    for (i = 0; i < LEDI_NUM_LEDS; i++) {
        aniInfo.drawBuff[i] = LED_TYPE(aniInfo.pix[i].color);
        if ((aniInfo.pix[i].crit & ANI_CRIT_PERSISTENT) == 0) {
            if (aniInfo.pix[i].crit & ANI_CRIT_BELOW_ANY) {
                aniInfo.pix[i].crit = ANI_CRIT_BELOW_LOW;
            } else if (aniInfo.pix[i].crit & ANI_CRIT_ACTIVE_ANY) {
                aniInfo.pix[i].crit = ANI_CRIT_LOW;
            }
        }
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
void AniSetInactive(AniPack *Ap)
{
    ListNode   *node;

    // Remove animation from the active list
    RemoveNode(&Ap->node);

    // Remove any held pixels if there are any
    while(!IsListEmpty(&Ap->parms.pixList)) {
        node = GetHead(&Ap->parms.pixList);
        RemoveNode(node);
    }

    if (Ap->defaultLayer & (ANI_LAYER_TRANSITION)) {
        //Serial.println("Deactivating trans animation");
        InsertTail(&aniInfo.transWaitList, &Ap->node);
        aniInfo.numTransWaiting++;
    } else {
        //Serial.println("Deactivating regular animation");
        InsertTail(&aniInfo.mainWaitList, &Ap->node);
        aniInfo.numMainWaiting++;
    }
}

/* --------------------------------------------------------------------------------------------
 *                 AniTransDone()
 * --------------------------------------------------------------------------------------------
 * Description:    Transitioning has completed
 *
 * Parameters:     
 *
 * Returns:        
 */
void AniTransDone()
{
    AniPack  *aniPack;
    ListNode *nodeItr;

    aniInfo.tranInProg = false;

    Serial.println("In AniTransDone");

    nodeItr = &aniInfo.activeList;
    while ((nodeItr = GetNextNode(nodeItr)) != &aniInfo.activeList) {
        aniPack = (AniPack*)nodeItr;
        if (aniPack->currCriteria & ANI_CRIT_BELOW_ANY) {
            nodeItr = GetPriorNode(nodeItr);
            AniSetInactive(aniPack);
        }
    }
}

/* --------------------------------------------------------------------------------------------
 *                 AniCheckTranDel()
 * --------------------------------------------------------------------------------------------
 * Description:    Checks the elapsed time since the last frame draw for this animation is longer
 *                 than the fps target
 *
 * Parameters:     
 *
 * Returns:        
 */
bool AniCheckTranDel(AniParms *Ap)
{
    return (millis() > Ap->delay);
}