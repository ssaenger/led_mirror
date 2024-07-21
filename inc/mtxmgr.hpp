/* ********************************************************************************************
 * mtxmgr.hpp
 *
 * Author: Shawn Saenger
 *
 * Created: Sep 4, 2023
 *
 * Description: Matrix Manager Class
 *
 * ********************************************************************************************
 */

#ifndef _MTX_MGR_HPP_
#define _MTX_MGR_HPP_

#include "sys/mtxmgr_i.h"
#include "overide.h"
#include "animations.hpp"

/* --------------------------------------------------------------------------------------------
 *  DEFINITIONS
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * dfdf define
 *
 * Defines the max amount of time waiting for an IP Address to be assigned from the gateway in
 * milliseconds when syncInit() is called.
 *
 * Default is 15 seconds
 */
#ifndef __DD_
#define __D_D_ 15000
#endif /* dfssdfs */


/* --------------------------------------------------------------------------------------------
 *  TYPES
 * --------------------------------------------------------------------------------------------
 */


/* --------------------------------------------------------------------------------------------
 *  CLASSES
 * --------------------------------------------------------------------------------------------
 */


// A singleton class
class MtxMgr
{
public:
    static MtxMgr &getInstance();
    #if 0
    static void NETMGR_LineSwipeR(AniParms *Ap);
    static void NETMGR_LineSwipeL(AniParms *Ap);
    static void NETMGR_LineSwipeU(AniParms *Ap);
    static void NETMGR_LineSwipeD(AniParms *Ap);

    static void NETMGR_CircleCenter(AniParms *Ap);
#endif
    /* Init function */
    bool syncInit();
    void run();

    /* Methods we don't want to exist */
    MtxMgr(MtxMgr const &)         = delete;
    void operator=(MtxMgr const &) = delete;

protected:
    rgb24    *ledBuff;
    //rgb24    *ledBuff2;

private:
    MtxMgr();
    ~MtxMgr();
    MtxMgrFlags flags;
    uint8_t     val;
    AniPack    *aniPackArray;
    uint8_t     aniPackNum;
};

#endif /* _MTX_MGR_HPP_ */