/* ********************************************************************************************
 * netmgr.hpp
 *
 * Author: Shawn Saenger
 *
 * Created: Aug 30, 2023
 *
 * Description: Network Manager Class
 *
 * ********************************************************************************************
 */

#ifndef _NETMGR_HPP_
#define _NETMGR_HPP_

#include <teensyupdater.hpp>
#include "sys/netmgr_i.h"
#include "overide.h"

/* --------------------------------------------------------------------------------------------
 *  DEFINITIONS
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * NETMGR_WAIT_FOR_LOCAL_IP_WAIT_TIME define
 *
 * Defines the max amount of time waiting for an IP Address to be assigned from the gateway in
 * milliseconds when syncInit() is called.
 *
 * Default is 15 seconds
 */
#ifndef NETMGR_WAIT_FOR_LOCAL_IP_WAIT_TIME
#define NETMGR_WAIT_FOR_LOCAL_IP_WAIT_TIME 15000
#endif /* NETMGR_WAIT_FOR_LOCAL_IP_WAIT_TIME */

/* --------------------------------------------------------------------------------------------
 * NETMGR_HOSTNAME define
 *
 * Sets the hostname. By default this is "teensy-lwip".
 *
 * Requires LWIP_NETIF_HOSTNAME to be defined. See QNEthernet/src/Iwipopts.h.
 *
 * Default is "MyTeensyLittleProj"
 */
#ifndef NETMGR_HOSTNAME
#define NETMGR_HOSTNAME "MyTeensyLittleProj"
#endif /* NETMGR_HOSTNAME */

/* --------------------------------------------------------------------------------------------
 * NETMGR_USING_DHCP define
 *
 * Defines whether to use DHCP to get an IP address
 *
 * Default is true
 */
#ifndef NETMGR_USING_DHCP
#define NETMGR_USING_DHCP true
#endif /* NETMGR_USING_DHCP */

/* --------------------------------------------------------------------------------------------
 * NETMGR_STATIC_IP_ADDR_STRNG define
 *
 * Defines the static IP address to use when USING_DHCP is false
 *
 * Requires USING_DHCP = false
 *
 * Default is "192.168.1.111" but you should really set your own if going static
 */
#ifndef NETMGR_STATIC_IP_ADDR_STRNG
#define NETMGR_STATIC_IP_ADDR_STRNG "192.168.1.111"
#endif /* NETMGR_STATIC_IP_ADDR_STRNG */

/* --------------------------------------------------------------------------------------------
 * NETMGR_GATEWAY_IP_ADDR_STRING define
 *
 * Defines the gateway ip address mask to use when USING_DHCP is false
 *
 * Requires USING_DHCP = false
 *
 * Default is "192.168.1.1". Most likely what most users will use.
 */
#ifndef NETMGR_GATEWAY_IP_ADDR_STRING
#define NETMGR_GATEWAY_IP_ADDR_STRING "192.168.1.1"
#endif /* NETMGR_GATEWAY_IP_ADDR_STRING */

/* --------------------------------------------------------------------------------------------
 * NETMGR_SUBNET_MASK_STRING define
 *
 * Defines the subnet mask to use when USING_DHCP is false.
 *
 * Requires USING_DHCP = false
 *
 * Default is "255.255.255.0"
 */
#ifndef NETMGR_SUBNET_MASK_STRING
#define NETMGR_SUBNET_MASK_STRING "255.255.255.0"
#endif /* NETMGR_SUBNET_MASK_STRING */

/* --------------------------------------------------------------------------------------------
 * NETMGR_DNS_SERVER_STRING define
 *
 * Defines the DNS server to use when USING_DHCP is false. Some examples are 1.1.1.1 from
 * Cloudflare and 8.8.8.8 from Google.
 *
 * Requires USING_DHCP = false
 *
 * Default is "1.1.1.1"
 */
#ifndef NETMGR_DNS_SERVER_STRING
#define NETMGR_DNS_SERVER_STRING "1.1.1.1"
#endif /* NETMGR_DNS_SERVER_STRING */

/* --------------------------------------------------------------------------------------------
 * NETMGR_OTA_URL_PATH define
 *
 * Defines the path to the OTA updater web page. Must be prepended by a slash character
 *
 * Default is "/update"
 */
#ifndef NETMGR_OTA_URL_PATH
#define NETMGR_OTA_URL_PATH "/update"
#endif /* NETMGR_OTA_URL_PATH */

/* --------------------------------------------------------------------------------------------
 *  TYPES
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 *  CLASSES
 * --------------------------------------------------------------------------------------------
 */


using namespace qindesign::network;

// A singleton class
class NetMgr
{
public:
    static NetMgr& getInstance();
    
    /* Init function */
    bool syncInit(uint16_t Port = 80);

    /* Methods we don't want to exist */
    NetMgr(NetMgr const&)         = delete;
    void operator=(NetMgr const&) = delete;

protected:
    AsyncWebServer   *webServer;
    TeensyOtaUpdater *tOtaUpdater;

#if !NETMGR_USING_DHCP
    IPAddress myIP;
    IPAddress myNetmask;
    IPAddress myGW;
    IPAddress mydnsServer;
#endif /* !NETMGR_USING_DHCP */

private:
    NetMgr();
    ~NetMgr();

    NetMgrFlags flags;

};

#endif /* _NETMGR_HPP_ */