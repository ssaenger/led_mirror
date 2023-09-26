/* ********************************************************************************************
 * overide.h
 *
 * Author: Shawn Saenger
 *
 * Created: Sep 2, 2023
 *
 * Description: Overide file for changing configurable defines. Configurable defines are the ones
 * that are encapsulated between #ifndef - #endif. It seems this file must be incuded at the top
 * of every file that you wish to configure when working with the Arduino compiler.
 *
 * ********************************************************************************************
 */

#ifndef _OVERIDE_H_
#define _OVERIDE_H_


/* MTXMGR overrides */

/* NetMgr overrides */
#define NETMGR_HOSTNAME "LED_Mirror_Teensy"
//#define NETMGR_STATIC_IP_ADDR_STRNG "192.168.1.111"
//#define NETMGR_USING_DHCP false

/* smartmtxconfig.h */
//#define SM_COLOR_DEPTH 24
#define SM_HEIGHT 64
#define SM_WIDTH 128
//#define SM_REFRESH_DEPTH 36

/* audiosync.hpp */
#define AS_MAX_LEVEL 0.20f
#define AS_DYNAMIC_RANGE 30.0f
#define AS_LINEAR_BLEND 0.5f

#endif /* _OVERIDE_H_ */