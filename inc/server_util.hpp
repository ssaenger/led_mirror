
#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include <AsyncWebServer_Teensy41.hpp>

/* --------------------------------------------------------------------------------------------
 *  DEFINITIONS
 * --------------------------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------------------------
 * HTTP_MAX_MESSAGE_LEN define
 *
 * The maximum HTTP length for any web page
 */
#define HTTP_MAX_MESSAGE_LEN           512


/* --------------------------------------------------------------------------------------------
 *  UTILITY FUNCTION DECLARATIONS
 * --------------------------------------------------------------------------------------------
 */

void handleRoot(AsyncWebServerRequest *request);

void SendStatusPage(AsyncWebServerRequest *Request, const char *Message, const int Code = 400);

void handleNotFound(AsyncWebServerRequest *request);

void sendOtaResponse(AsyncWebServerRequest *UploadRequest, bool Success);

#endif /* _SERVER_HPP_ */