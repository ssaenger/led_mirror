/* ********************************************************************************************
 * netmgr.cpp
 *
 * Author: Shawn Saenger
 *
 * Created: Sep 2, 2023
 *
 * Description: Network Manager implementation. Responsible for handling events for initializing
 *              and managing events to/from the web server.
 *
 * ********************************************************************************************
 */

#include "../inc/netmgr.hpp"
#include "../inc/server_util.hpp"
#include <QNEthernet.h>

/* --------------------------------------------------------------------------------------------
 *                 NetMgr()
 * --------------------------------------------------------------------------------------------
 * Description:    Private NetMgr constructor
 *
 * Parameters:     void
 *
 * Returns:        void
 */
NetMgr::NetMgr()
{
}

/* --------------------------------------------------------------------------------------------
 *                 ~NetMgr()
 * --------------------------------------------------------------------------------------------
 * Description:    Private NetMgr deconstructor
 *
 * Parameters:     void
 *
 * Returns:        void
 */
NetMgr::~NetMgr()
{
    delete webServer;
}

/* --------------------------------------------------------------------------------------------
 *                 getInstance()
 * --------------------------------------------------------------------------------------------
 * Description:    Grabs a reference to the singleton NetMgr object
 *
 * Parameters:     void
 *
 * Returns:        A pointer to a NetMgr object
 */
NetMgr& NetMgr::getInstance()
{
    static NetMgr netmgr;
    return netmgr;
}

/* --------------------------------------------------------------------------------------------
 *                 syncInit()
 * --------------------------------------------------------------------------------------------
 * Description:    A blocking call that initializes NetMgr and starts the web server.
 *
 * Parameters:     void
 *
 * Returns:        true if successful, false othewise
 */
bool NetMgr::syncInit(uint16_t Port)
{
    webServer = new AsyncWebServer(80);

#if LWIP_NETIF_HOSTNAME
    Serial.printf("Setting hostname to %s\r\n", NETMGR_HOSTNAME);
    Ethernet.setHostname(NETMGR_HOSTNAME);
#endif

    // Attempt to establish a connection with the network
#if NETMGR_USING_DHCP
    // Start the Ethernet connection, using DHCP
    Serial.print("Initialize Ethernet using DHCP => ");
    Ethernet.begin();
#else
    // Start the Ethernet connection, using static IP
    Serial.print("Initialize Ethernet using static IP => ");
    myIP.fromString(NETMGR_STATIC_IP_ADDR_STRNG);
    myGW.fromString(NETMGR_GATEWAY_IP_ADDR_STRING);
    myNetmask.fromString(NETMGR_SUBNET_MASK_STRING);
    mydnsServer.fromString(NETMGR_DNS_SERVER_STRING);
    Ethernet.begin(myIP, myNetmask, myGW);
    Ethernet.setDNSServerIP(mydnsServer);
#endif

    if (!Ethernet.waitForLocalIP(100/*NETMGR_WAIT_FOR_LOCAL_IP_WAIT_TIME*/)) {
        Serial.println(F("Failed to configure Ethernet"));

        if (!Ethernet.linkStatus()) {
            Serial.println(F("Ethernet cable is not connected."));
        }
        flags = NETMGR_FLAG_IDLE;
        return false;
    } else {
        Serial.print(F("Connected! IP address:"));
        Serial.println(Ethernet.localIP());
    }

    webServer->on("/", HTTP_GET, handleRoot);
    webServer->onNotFound(handleNotFound);

    tOtaUpdater = new TeensyOtaUpdater(webServer, NETMGR_OTA_URL_PATH);
    if (tOtaUpdater->isUpdateReady()) {
        Serial.println("ready to update!");
    }

    webServer->begin();
    flags |= NETMGR_FLAG_CONN;
    
    return true;
}