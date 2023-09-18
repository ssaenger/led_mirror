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

    /* Init function */
    bool syncInit();
    void run();

    /* Methods we don't want to exist */
    MtxMgr(MtxMgr const &)         = delete;
    void operator=(MtxMgr const &) = delete;

protected:
    rgb24    *ledBuff;

private:
    MtxMgr();
    ~MtxMgr();
    MtxMgrFlags flags;
    uint8_t     val;
    AniPack aniPackArray[20];
};

#endif /* _MTX_MGR_HPP_ */