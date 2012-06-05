// Uses the Webduino library: https://github.com/sirleech/Webduino

#include "SPI.h"
#include "Ethernet.h"
#include "WebServer.h"

#define NAMELEN 32
#define VALUELEN 32

static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static uint8_t ip[] = { 10, 0, 25, 80 };

#define PREFIX ""
WebServer webserver(PREFIX, 80);

String scanlist;

void scanRequestHandler(WebServer &server, WebServer::ConnectionType type, char *, bool){
  server.httpSuccess();

  if (type != WebServer::HEAD) {
    server.println("scanlist");
  }
}

void netCredsHandler(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete){
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  int  name_len;
  char value[VALUELEN];
  int value_len;

  server.httpSuccess();

  if (type != WebServer::HEAD) {
    Serial.println("netCredsHandler");
    if (strlen(url_tail)) {
      //server.printP(Parsed_tail_begin);
      while (strlen(url_tail)) {
        rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
        if (rc == URLPARAM_EOS){
          // done
          server.print("thanks");
        } else {
          Serial.println(name);
          Serial.println(value);
        }
      }
    }
  }
}

void setup() {
  /* initialize wifly module */
  Ethernet.begin(mac, ip);
  Serial.begin(9600);
  webserver.setDefaultCommand(&scanRequestHandler);
  webserver.addCommand("netCreds.html", &netCredsHandler);
  webserver.begin();
}

void loop() {
  char buff[64];
  int len = 64;

  /* process incoming connections one at a time forever */
  webserver.processConnection(buff, &len);
}