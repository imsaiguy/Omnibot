// Complete Instructions to Get and Change ESP MAC Address: https://RandomNerdTutorials.com/get-change-esp32-esp8266-mac-address-arduino/
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
 
void setup(){
  Serial.begin(115200);
  //WiFi.mode(WIFI_MODE_STA);
  Serial.print("The MAC address is: ");
  Serial.println(WiFi.macAddress());
}
 
void loop(){

}