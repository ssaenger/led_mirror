/* ********************************************************************************************
 * server_util.cpp
 *
 * Author: Shawn Saenger
 *
 * Created: Aug 31, 2023
 *
 * Description: Provides utility functions to send responses back to the client
 *              https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers
 *              https://developer.mozilla.org/en-US/docs/Web/HTTP/Status
 *
 * ********************************************************************************************
 */
#include "../inc/server_util.hpp"

/* --------------------------------------------------------------------------------------------
 *  FUNCTIONS
 * --------------------------------------------------------------------------------------------
 */

void handleRoot(AsyncWebServerRequest *request)
{

    char temp[400];
    int sec = millis() / 1000;
    int min = sec / 60;
    int hr  = min / 60;
    int day = hr / 24;

    snprintf(temp, 400 - 1,
             "<html>\
  <head>\
  <meta http-equiv='refresh' content='5'/>\
  <title>AsyncWebServer-%s</title>\
  <style>\
  body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
  </style>\
  </head>\
  <body>\
  <h2>AsyncWebServer_Teensy41!</h2>\
  <h3>running on %s</h3>\
  <p>Uptime: %d d %02d:%02d:%02d</p>\
  <img src=\"/test.svg\" />\
  </body>\
  </html>",
             BOARD_NAME, BOARD_NAME, day, hr % 24, min % 60, sec % 60);

    request->send(200, "text/html", temp);
}

/* --------------------------------------------------------------------------------------------
 *                 SendStatusPage()
 * --------------------------------------------------------------------------------------------
 * Description:    Sends a basic status response message
 *
 * Parameters:     Request - A client request
 *                 Message - Status message
 *                 Code = 200 - HTTP response status codes
 *                              https://developer.mozilla.org/en-US/docs/Web/HTTP/Status
 *
 * Returns:        void
 */
void SendStatusPage(AsyncWebServerRequest *Request, const char *Message, const int Code)
{
    char pageOut[HTTP_MAX_MESSAGE_LEN];
    unsigned int len;

    len = snprintf(pageOut, HTTP_MAX_MESSAGE_LEN, "<html><head><meta http-equiv=\"refresh\" content=\"10\"></head>");
    len += snprintf(pageOut + len, HTTP_MAX_MESSAGE_LEN - len, "<body><h1>%s</h1></body></html>", Message);

    Request->send(Code, "text/html", pageOut);
}

/* --------------------------------------------------------------------------------------------
 *                 handleNotFound()
 * --------------------------------------------------------------------------------------------
 * Description:    Sends a basic 404 error response
 *
 * Parameters:     Request - A client request
 *
 * Returns:        void
 */
void handleNotFound(AsyncWebServerRequest *Request)
{
    String message = "File Not Found\n\n";
    uint8_t i;

    message += "URI: ";
    message += Request->url();
    message += "\nMethod: ";
    message += (Request->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += Request->args();
    message += "\n";

    for (i = 0; i < Request->args(); i++) {
        message += " " + Request->argName(i) + ": " + Request->arg(i) + "\n";
    }

    Request->send(404, "text/plain", message);
}

/* --------------------------------------------------------------------------------------------
 *                 sendOtaResponse()
 * --------------------------------------------------------------------------------------------
 * Description:    Sends a response to an upload
 *
 * Parameters:     UploadRequest - A client upload request
 *                 Success - Whether that upload request succeeded or not
 *
 * Returns:        void
 */
void sendOtaResponse(AsyncWebServerRequest *UploadRequest, bool Success)
{
    AsyncWebServerResponse *response;
    char pageOut[HTTP_MAX_MESSAGE_LEN];
    unsigned int len;

    len = snprintf(pageOut, HTTP_MAX_MESSAGE_LEN, "<html><head><meta http-equiv=\"refresh\" content=\"20\"></head>");
    len += snprintf(pageOut + len, HTTP_MAX_MESSAGE_LEN - len,
                    "<body><h1>%s</h1></body></html>",
                    (Success) ? "OTA Failed..." : "OTA Success! Rebooting...");

    response = UploadRequest->beginResponse(Success ? 500 : 200, "text/html", pageOut);
    response->addHeader("Connection", "close");
    response->addHeader("Access-Control-Allow-Origin", "*");
    UploadRequest->send(response);
}