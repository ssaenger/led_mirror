#include "inc/mtxmgr.hpp"
//#include "inc/netmgr.hpp"
#include "inc/overide.h"
#include <Arduino.h>
#ifdef IS_TEENSY_BOARD
#include <AsyncWebServer_Teensy41.h> /* Only to be included once here to avoid multiple redefinitions */
#endif

void setup()
{
    //NetMgr &netMgr = NetMgr::getInstance();
    MtxMgr &mtxMgr = MtxMgr::getInstance();

    Serial.begin(115200);

    while (!Serial  && millis() < 5000 )
        ;

    delay(2000);

    Serial.printf("\nInitializing LED_Mirror Project on %s...\n\r", BOARD_NAME);

    mtxMgr.syncInit();
    //netMgr.syncInit();
}

void loop()
{
    MtxMgr &mtxMgr = MtxMgr::getInstance();
    mtxMgr.run();
}