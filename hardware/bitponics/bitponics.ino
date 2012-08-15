/*
 * Bitponics Controller
 * 
 * Ted Hayes / Limina.Studio 2012
 * ted.hayes@liminastudio.com
 * 
 * [License]
 * 
 */

#include <SPI.h>
#include <WiFly.h>
#include <EEPROM.h>
#include <Button.h>

#define PIN_LED_STATUS          9
#define PIN_BUTTON_RESET        11    // Used to 'factory reset' the system
#define RESET_BUTTON_TIME       3000  // ms, time to hold down button

#define ADHOC_SSID "Bitponics"

/////// Globals ////////

WiFlyServer server(80);
Button resetButton = Button(PIN_BUTTON_RESET);
String scanlist;
boolean wifiSet;

void setup() {
  // Serial ports
  Serial1.begin(9600);
  WiFly.setUart(&Serial1);
  WiFly.begin();

  Serial.begin(9600);
  // Serial.print("IP: ");
  // Serial.println(WiFly.ip());
  
  server.begin();

  // set up factory reset button
  resetButton.holdHandler(factoryReset, RESET_BUTTON_TIME);
  if(resetButton.isPressed()){
    factoryReset(resetButton);
  }

  // Is the unit in Setup or Normal mode?
  wifiSet = EEPROM.read(0);
  if(wifiSet){
    // The WiFi module is set up for wifi, so try to connect to the Bitponics server
    Serial.println("WiFi Mode");
  } else {
    // The WiFi module is NOT set up for wifi and should already be in Adhoc mode, so start web server
    Serial.println("Adhoc Mode");
    // startServer();
  }

  // Misc
  pinMode(PIN_LED_STATUS, OUTPUT);
}

void factoryReset(Button& b){
  // Serial.print("Held for at least 1 second: ");
  // Serial.println(b.pin);
//  Serial.print("factoryReset... ");
  boolean adhocOK = setAdhocMode();
  Serial.println(adhocOK);
  restart();
}

void restart(){
  // wifi.reboot();
  delay(300); // wait for reboot to continue
  // setup();    // will this do?
  wifiSet = 1; // because loop will continue running
}

boolean setAdhocMode(){
  // sets up the wifi module for Adhoc mode
  // only called by the "Factory Reset" function
  boolean sendOK = false;

  // sendOK = wifi.SendCommand("set wlan join 4",">", bufBody, BODY_BUFFER_SIZE) << endl;
  // sendOK = wifi.SendCommand("set wlan ssid Bitponics",">", bufBody, BODY_BUFFER_SIZE) << endl;
  // sendOK = wifi.SendCommand("set wlan chan 1",">", bufBody, BODY_BUFFER_SIZE) << endl;
  // // TODO: clear previously set password
  // sendOK = wifi.SendCommand("set ip address 169.254.1.1",">", bufBody, BODY_BUFFER_SIZE) << endl;
  // sendOK = wifi.SendCommand("set ip netmask 255.255.0.0",">", bufBody, BODY_BUFFER_SIZE) << endl;
  // sendOK = wifi.SendCommand("set ip dhcp 0",">", bufBody, BODY_BUFFER_SIZE) << endl;
  // sendOK = wifi.SendCommand("save",">", bufBody, BODY_BUFFER_SIZE) << endl;

  if(sendOK){
    // set EEPROM value & reboot trigger
    EEPROM.write(0, 0); // means that the device is NOT configured for WiFi, so upon start, perform the correct actions
  } else {
//    Serial.println(F("*** setAdhocMode error"));
  }
  return sendOK;
}

void postBitponicsData(){
  // take readings and send to server


}

void returnScanlist(WiFlyClient client){
  client.println(scanlist);
}

boolean setupWifi(String ssid, String pass){
  boolean sendOK = false;
  
  // char ssidc[ssid.length()];
  // char passc[pass.length()];
  // ssid.toCharArray(ssidc, ssid.length());
  // pass.toCharArray(passc, pass.length());
//      wifi.setSSID(wifiSSID);
//      wifi.setPassphrase(wifiPass);
  // sendOK = WFSEthernet.configure(WIFLY_AUTH_WPA2_PSK, WIFLY_JOIN_AUTO, WIFLY_DHCP_ON);
  // sendOK = WFSEthernet.credentials(ssidc, passc);
  // sendOK = wifi.SendCommand("set ip netmask 255.255.255.0",">", bufBody, BODY_BUFFER_SIZE) << endl;
  // sendOK = wifi.setChannel("0");
  // sendOK = wifi.SendCommand("save",">", bufBody, BODY_BUFFER_SIZE) << endl;

if(sendOK){
    // set EEPROM value & reboot trigger
    EEPROM.write(0, 1); // means that the device is configured for WiFi, so upon start, perform the correct actions
    // needsReboot = true;
  } else {
//    Serial.println(F("*** setupWifi error"));
  }

  return sendOK;
}

void configRequest(WiFlyClient client, char* requestBuffer){
  String rBuffer = requestBuffer;
  int startIndex = rBuffer.indexOf("?name=") + 6;
  int endIndex = rBuffer.indexOf("&pass=");
  // Serial.println("CONFIG params: ");
  String wifiSSID = rBuffer.substring(startIndex,endIndex);
  String wifiPass = rBuffer.substring(endIndex+6);
  // now set new parameters, reset module, try to connect to server, and show green light
  setupWifi(wifiSSID, wifiPass);
}

void append(char* s, char c){
  byte len = strlen(s);
  s[len] = c;
  s[len + 1] = '\0';
}

void processServer(){
  WiFlyClient client = server.available();
  if (client) {
    char requestBuffer[50] = "";
    Serial.println("Got a client");
    // an http request ends with a blank line
    boolean current_line_is_blank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // requestBuffer += c;
        append(requestBuffer, c);
        Serial.print(c);

        if (c == '\n' && current_line_is_blank) {
          // got two newlines, process request
          char get_url[30];
          sscanf(requestBuffer, "GET %s HTTP/1.1", get_url);

          if(get_url == "/scanlist"){
            returnScanlist(client);
          } else if(strstr(requestBuffer, "/config")){
            configRequest(client, requestBuffer);
          }
          break;
        }
        if (c == '\n') {
          // we're starting a new line
          current_line_is_blank = true;
        } else if (c != '\r') {
          // we've gotten a character on the current line
          current_line_is_blank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1000);
    client.stop();
  }
}

void loop() {
  resetButton.process();
  if(wifiSet == 1){
    postBitponicsData();
  } else {
    processServer();
  }
}
