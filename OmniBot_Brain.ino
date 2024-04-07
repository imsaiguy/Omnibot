/*
  OmniBot esp32 Brain
  Printed circuit version
  ImsaiGuy 2024

  to be used with esp32 OmniBot Controller
  This is a list of MAC addresses for esp32 modules I have
1:  EC:94:CB:54:B8:64
2:  34:86:5D:3A:35:30
3:  34:86:5D:3A:0E:A8
4:  E8:DB:84:F8:31:8C
5:  34:85:18:45:FD:20     elecrow display esp32-S3

Functionality:
   pair to fixed MAC address (need to hardcode below...)
   initialize and send first data
   Key_status variable holds current key state
   Key_status = 0   no keys pressed
   if a key is pressed Key_status is transmitted
   when the key is released Key_status = 0 is transmitted
==========================================================================================
  
*/

#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// pin number assignments
#define Status_LED 13
#define Speaker    12

#define PCA9557_ADDRESS 0x18  // I2C address for PCA9557
#define PCA9557_CONTROL 0x03  // Control register address for configuring as output
#define PCA9557_OUTPUT  0x01  // Output register address

// Motor H-bridge settings
#define Motor_idle    B00000000
#define Motor_enable  B01000010
#define Motor_forward B01011010
#define Motor_reverse B01100110
#define Motor_left    B01010110
#define Motor_right   B01101010
#define PCA9557_out   B10000000

int Key_status = 0;       // global Key Status value

// Declaration for SSD1306 display connected using I2C
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     // Reset pin
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  char a[32];
  int Key_status;
  float c;
  bool d;
} struct_message;

// // Create a struct_message called myData
struct_message myData;

// // callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
}

//============================================================================
void setup() {
  // Configure all pins of PCA9557 for output
  Wire.begin();
  Wire.beginTransmission(PCA9557_ADDRESS);
  Wire.write(PCA9557_CONTROL);
  Wire.write(0x00);  // Set all 8 bits as outputs
  Wire.endTransmission();
  Wire.beginTransmission(PCA9557_ADDRESS);
  Wire.write(PCA9557_OUTPUT);
  Wire.write(Motor_idle);
  Wire.endTransmission();

  pinMode(Status_LED, OUTPUT);
  digitalWrite(Status_LED, LOW);
  
  // OLED display
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 28);
  display.println("OmniBot Brain");
  display.display();

  Flash_LED(4, 100);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  // Init ESP-NOW
  esp_now_init();
  // set recieve interupt code
  esp_now_register_recv_cb(OnDataRecv);

  delay(3000);
}

//============================================================================
void Flash_LED(int times, int waits) {
  for (int i = 0; i < times; i++) {
    digitalWrite(Status_LED, HIGH);
    delay(waits);
    digitalWrite(Status_LED, LOW);
    delay(waits);
  }
}

//============================================================================
void loop() {
  Key_status = myData.Key_status;
  if (Key_status == 0) {
    Wire.beginTransmission(PCA9557_ADDRESS);
    Wire.write(PCA9557_OUTPUT);
    Wire.write(Motor_idle);
    Wire.endTransmission();

    digitalWrite(Status_LED, LOW);
    display.clearDisplay();
    display.setCursor(0, 28);
    display.println("Idle");
    display.display();
    delay(1000);
  }

  if (Key_status == 1) {
    digitalWrite(Status_LED, HIGH);
    display.clearDisplay();
    display.setCursor(0, 28);
    display.println("Forward");
    display.display();

    Wire.beginTransmission(PCA9557_ADDRESS);
    Wire.write(PCA9557_OUTPUT);
    Wire.write(Motor_forward);
    Wire.endTransmission();
  }
  if (Key_status == 3) {
    digitalWrite(Status_LED, HIGH);
    display.clearDisplay();
    display.setCursor(0, 28);
    display.println("Reverse");
    display.display();

    Wire.beginTransmission(PCA9557_ADDRESS);
    Wire.write(PCA9557_OUTPUT);
    Wire.write(Motor_reverse);
    Wire.endTransmission();
  }
  if (Key_status == 4) {
    digitalWrite(Status_LED, HIGH);
    display.clearDisplay();
    display.setCursor(0, 28);
    display.println("Left");
    display.display();

    Wire.beginTransmission(PCA9557_ADDRESS);
    Wire.write(PCA9557_OUTPUT);
    Wire.write(Motor_left);
    Wire.endTransmission();
  }
  if (Key_status == 2) {
    digitalWrite(Status_LED, HIGH);
    display.clearDisplay();
    display.setCursor(0, 28);
    display.println("Right");
    display.display();

    Wire.beginTransmission(PCA9557_ADDRESS);
    Wire.write(PCA9557_OUTPUT);
    Wire.write(Motor_right);
    Wire.endTransmission();
  }
}
//============================================================================