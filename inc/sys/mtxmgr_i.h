/* ********************************************************************************************
 * mtxmgr_i.h
 *
 * Author: Shawn Saenger
 *
 * Created: Sep 4, 2023
 *
 * Description: Matrix Manager internal header file
 *
 * ********************************************************************************************
 */

#ifndef _MTX_MGR_HPP_I_
#define _MTX_MGR_HPP_I_

#include <Arduino.h>

/* --------------------------------------------------------------------------------------------
 *  TYPES
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * MtxMgrFlags type
 *
 * Bitmask flags for the different states the MtxMgrFlags can be in.
 */
typedef uint16_t MtxMgrFlags;

/* Uninitialized */
#define MTXMGR_FLAG_IDLE    0x0000

/* End MtxMgrFlags type */

#endif /* _NET_MGR_HPP_I_ */