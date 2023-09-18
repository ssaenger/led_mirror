/* ********************************************************************************************
 * netmgr_i.h
 *
 * Author: Shawn Saenger
 *
 * Created: Sep 3, 2023
 *
 * Description: Network Manager internal header file
 *
 * ********************************************************************************************
 */

#ifndef _NET_MGR_HPP_I_
#define _NET_MGR_HPP_I_

/* --------------------------------------------------------------------------------------------
 *  TYPES
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * NetMgrFlags type
 *
 * Bitmask flags for the different states the NetMgr can be in.
 */
typedef uint16_t NetMgrFlags;

/* Uninitialized */
#define NETMGR_FLAG_IDLE    0x0000

/* NetMgr is connected with the network */
#define NETMGR_FLAG_CONN    0x0001

/*  */
#define NETMGR_FLAG_IDLE1   0x0002

/*  */
#define NETMGR_FLAG_IDLE2   0x0004

/*  */
#define NETMGR_FLAG_IDLE3   0x0008

/*  */
#define NETMGR_FLAG_IDLE4   0x0010

/* End NetMgrFlags type */

#endif /* _NET_MGR_HPP_I_ */