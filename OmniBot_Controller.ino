/*
  OmniBot controller using Crowbot Controller
  ImsaiGuy 2024

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
*/

#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/5, /* data=*/4, /* reset=*/U8X8_PIN_NONE);  // Adafruit Feather ESP8266/32u4 Boards + FeatherWing OLED

#define SDA 4                    // I2C
#define SCL 5                    // I2C
#define Keys4 0                  // analog input for 4 keys ABCD
#define LeftButton 8             // digital input left button
#define RightButton 10           // digital input right button
#define Vib 7                    // vibrator motor
#define Xjoy 1                   // analog input X axis
#define Yjoy 3                   // analog input Y axis
#define PUSHjoy 18               // digital input joy stick push button
const int Buzzer_pin = 6;        // PWM output pin for the buzzer
const int PWM_channel = 0;       // PWM PWM_channel (0-15)
const int PWM_resolution = 8;    // PWM resolution (1-16), 8 means 8-bit resolution
const int PWM_frequency = 1000;  // Frequency of the tone in Hz

uint8_t broadcastAddress[] = { 0x34, 0x86, 0x5D, 0x3A, 0x0E, 0xA8 };  // MAC address to pair to
int Key_status = 0;                                                   // global Key Status value

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  char a[32];
  int Key_status;
  float c;
  bool d;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //  Serial.print("\r\nLast Packet Send Status:\t");
  //  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

//=================================================================================
// Setup everything
//=================================================================================
void setup() {
  pinMode(Keys4, INPUT);
  pinMode(LeftButton, INPUT);
  pinMode(RightButton, INPUT);
  pinMode(Xjoy, INPUT);
  pinMode(Yjoy, INPUT);
  pinMode(PUSHjoy, INPUT_PULLUP);
  pinMode(Vib, OUTPUT);
  ledcSetup(PWM_channel, PWM_frequency, PWM_resolution);
  ledcAttachPin(Buzzer_pin, PWM_channel);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // OLED display
  u8g2.begin();
  u8g2.enableUTF8Print();  // enable UTF8 support for the Arduino print() function
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.setFontDirection(0);
  for (int i = 128; i > -90; i -= 14) {
    u8g2.firstPage();
    do {
      u8g2.drawStr(i, 25, "OmniBot");
      delay(4);
    } while (u8g2.nextPage());
  }

  if (esp_now_init() != ESP_OK) {
    //   Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    //  Serial.println("Failed to add peer");
    return;
  }
  Update_status();  // transmit and display
}


//=================================================================================
// main loop
//=================================================================================
void loop() {

  int read1 = Get_keys();
  delay(30);
  int read2 = Get_keys();
  delay(30);
  int read3 = Get_keys();
  if (read1 - read2 - read3) Key_status = read1;

  if (Key_status != 0) {  // key pressed
    Update_status();      // transmit and display

    Wait_on_key(Key_status);  // wait for key to be released
    Key_status = 0;

    Update_status();  // transmit and display
  }
}


//=================================================================================
// Read the keys and return Key_status
// 0 - no keys
// 1 - A key
// 2 - B key
// 3 - C key
// 4 - D key
// 11 - left button
// 12 - right button
//=================================================================================
int Get_keys() {
  int voltage = analogRead(Keys4);  // Read voltage on pin D0

  // no button pressed: 4095
  // A button: 2360
  // B button: 22
  // C button: 3220
  // D button: 820

  if (voltage >= 3400) {
    Key_status = 0;
  } else if (voltage >= 3000) {
    Key_status = 3;
  } else if (voltage >= 2100) {
    Key_status = 1;
  } else if (voltage >= 400) {
    Key_status = 4;
  } else if (voltage < 400) {
    Key_status = 2;
  }

  if (!digitalRead(LeftButton)) {
    Key_status = 11;
  }
  if (!digitalRead(RightButton)) {
    Key_status = 12;
  }

  return Key_status;
}

//=================================================================================
// wait until key is released
//=================================================================================
void Wait_on_key(int Current_key) {
  while (Get_keys() == Current_key) {
  }
}

//=================================================================================
// Update, send Key_status to WIFI and display
//=================================================================================
void Update_status() {
  // Set values to send
  strcpy(myData.a, "ImsaiGuy");
  myData.Key_status = Key_status;
  myData.c = ++myData.c;
  myData.d = false;

  // Send message via ESP-NOW
  esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setCursor(0, 20);
    u8g2.print("Key Status: ");
    u8g2.print(Key_status);  // Display Key_status
  } while (u8g2.nextPage());
}

//=================================================================================
// End of file
//=================================================================================
