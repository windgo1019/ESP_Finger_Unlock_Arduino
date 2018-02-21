//HardwareSerial Serial1(1);
HardwareSerial Serial1(2);
#include <WiFi.h>      // 引用程式庫
#include <mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
//#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
//#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
//#include "images.h"
//OLED pins to ESP32 GPIOs via this connecthin:
//OLED_SDA -- GPIO4
//OLED_SCL -- GPIO15

//Finger TX -- GPIO16
//Finger RX -- GPIO17
#define led 5
//SSD1306  display(0x3c, 4, 15);
//#define DEMO_DURATION 3000
//typedef void (*Demo)(void);
//int demoMode = 0;
//int counter = 1;
const char *ssid =  "windgo-asus";    // WIFI名稱
const char *password =  "ssmmno12";     // WIFI密碼
const char *mqtt_server = "192.168.31.184"; // your matt server ip
const char *mqtt_topic =  "finger";     // mqtt_topic for HA sensor name
const char *mqtt_unlock_payload =  "unlock";     // mqtt_unlock_payload for HA sensor state
const char *mqtt_lock_payload =  "lock";     // mqtt_lock_payload for HA sensor state
const char *ota_device_name =  "finger_esp32";     // OTA_device_name for upload firmware
//if you want to enable ota password, you should uncomment "//const char *ota_upload_password" and "//ArduinoOTA.setPassword(ota_upload_password)"
//要啟用OTA密碼功能的話要反註解下面的  "//const char *ota_upload_password" and //ArduinoOTA.setPassword(ota_upload_password)
//const char *ota_upload_password =  "ssmmno1";     // OTA_upload_password for upload firmware

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  pinMode(led,OUTPUT);
  digitalWrite(led, LOW);
  
 // pinMode(16,OUTPUT);
 // digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
//  delay(50); 
  //digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
   // Initialising the UI will init the display too.
  //display.init();
  //display.flipScreenVertically();
  //display.setFont(ArialMT_Plain_10);
  
  //Send command to connect finger device
  byte command[] = {0x55,0xAA,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01};
  Serial1.write(command, sizeof(command));
  Serial.print("Connected finger");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  //delay for wifi connected
  delay(1500);
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
  }
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(ota_device_name);

  // No authentication by default
  // ArduinoOTA.setPassword(ota_upload_password);

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(led, HIGH);
  delay(400);
  digitalWrite(led, LOW);
  delay(100);
  digitalWrite(led, HIGH);
  delay(100);
  digitalWrite(led, LOW);
  delay(100);
  digitalWrite(led, HIGH);
  delay(100);
  digitalWrite(led, LOW);
  randomSeed(micros());
  
  delay(150);
}
/*
void drawFontFaceDemo() {
    // Font Demo1
    // create more fonts at http://oleddisplay.squix.ch/
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "哈囉Hello");
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 10, "哈囉Hello");
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 26, "哈囉Hello");
}

void drawTextFlowDemo() {
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawStringMaxWidth(0, 0, 128,
      "測試一段中文chinese123Happy!" );
}

void drawTextAlignmentDemo() {
    // Text alignment demo
  display.setFont(ArialMT_Plain_10);

  // The coordinates define the left starting point of the text
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 10, "中文");

  // The coordinates define the center of the text
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 22, "中文chinese");

  // The coordinates define the right end of the text
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 33, "中文chinese123");
}

void drawRectDemo() {
      // Draw a pixel at given position
    for (int i = 0; i < 10; i++) {
      display.setPixel(i, i);
      display.setPixel(10 - i, i);
    }
    display.drawRect(12, 12, 20, 20);

    // Fill the rectangle
    display.fillRect(14, 14, 17, 17);

    // Draw a line horizontally
    display.drawHorizontalLine(0, 40, 20);

    // Draw a line horizontally
    display.drawVerticalLine(40, 0, 20);
}

void drawCircleDemo() {
  for (int i=1; i < 8; i++) {
    display.setColor(WHITE);
    display.drawCircle(32, 32, i*3);
    if (i % 2 == 0) {
      display.setColor(BLACK);
    }
    display.fillCircle(96, 32, 32 - i* 3);
  }
}

void drawProgressBarDemo() {
  int progress = (counter / 5) % 100;
  // draw the progress bar
  display.drawProgressBar(0, 32, 120, 10, progress);

  // draw the percentage as String
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 15, String(progress) + "%");
}

void drawImageDemo() {
    // see http://blog.squix.org/2015/05/esp8266-nodemcu-how-to-create-xbm.html
    // on how to create xbm files
    display.drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
}

Demo demos[] = {drawFontFaceDemo, drawTextFlowDemo, drawTextAlignmentDemo, drawRectDemo, drawCircleDemo, drawProgressBarDemo, drawImageDemo};
int demoLength = (sizeof(demos) / sizeof(Demo));
long timeSinceLastModeSwitch = 0;
*/
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
  WiFi.begin(ssid, password);
  //delay for wifi connected
  delay(1500);
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
  }
  }

 //Send command1 to check finger touch 
 byte command1[] = {0x55,0xAA,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0x01};
 Serial1.write(command1, sizeof(command1));
 int index = 0;
 byte inData[26];
 //clear inData byte array 
 //memset(inData, 0xFF, sizeof(inData));
    while (!Serial1.available()) {
    delay(5);
   }
 //delay(250);
 while (Serial1.available() > 0) {
    byte incomingByte = Serial1.read();
    //Serial.print(incomingByte,HEX);
    //Serial.print(' ');
    inData[index] = incomingByte;
    index++;
  }
 // display.clear();
  //display.setTextAlignment(TEXT_ALIGN_LEFT);
  //display.setFont(ArialMT_Plain_16);
  //display.drawString(0, 0, "Checking finger..");
  Serial.println("開始檢查指紋=========================================================");
 
  //check command1 result
  for (int i = 0 ; i<=25; i++){
   Serial.print(inData[i],HEX);
   Serial.print(" ");
  }
  Serial.println("");
  

 if (inData[8] == 0x00 && inData[24] == 0x22 ){
  //display.setTextAlignment(TEXT_ALIGN_LEFT);
  //display.setFont(ArialMT_Plain_16);
  //display.drawString(0, 22, "Find finger!");
   Serial.println("Find finger! 檢查到有手指放入!");
   //Send command2 to save finger pic to imagebuffer
   byte command2[] = {0x55,0xAA,0x00,0x00,0x60,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x61,0x01};
   Serial1.write(command2, sizeof(command2));
   int in = 0;
   byte inDa[26];
   while (!Serial1.available()) {
    delay(5);
   }
   //delay(500);
   while (Serial1.available() > 0) {
    byte incoming = Serial1.read();
    //Serial.print(incoming,HEX);
    //Serial.print(' ');
    inDa[in] = incoming;
    in++;
  }
    
    //check command2 result
    for (int i = 0 ; i<=25; i++){
     Serial.print(inDa[i],HEX);
     Serial.print(" ");
    }
    
   //clear inDa byte array 
   //memset(inDa, 0xFF, sizeof(inDa)); 
   //Send command3 to search imagebuff in finger database
   byte command3[] = {0x55,0xAA,0x00,0x00,0x63,0x00,0x06,0x00,0x00,0x00,0x01,0x00,0xF4,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x5E,0x02};
   Serial1.write(command3, sizeof(command3));
   in = 0;
   Serial.println();
   Serial.println("Finger check.... 比對指紋資料庫中....");
   while (!Serial1.available()) {
    delay(5);
   }
   //delay(250);
   while (Serial1.available() > 0) {
    byte incoming = Serial1.read();
    //Serial.print(incoming,HEX);
    //Serial.print(' ');
    inDa[in] = incoming;
    in++;
  }
    
  //check command3 result
   Serial.println("");
    for (int i = 0 ; i<=25; i++){
     Serial.print(inDa[i],HEX);
     Serial.print(" ");
    }
  
   if (inDa[4] == 0x63 && inDa[6] == 0x05 ){
     Serial.println();
     Serial.println("Your finger is in database! Open door! 已找到指紋!開門!");
    // display.setTextAlignment(TEXT_ALIGN_LEFT);
     //display.setFont(ArialMT_Plain_16);
     //display.drawString(0, 44, "Finger ok! Open door!");
     //delay(1000);
           //LED燈亮
      digitalWrite(led, HIGH);
      delay(50);
      // 開啟GPIO4的遙控器開鎖
      //if you want enable it , uncomment it 假如要使用遙控器開門,請拿掉3行註解
      //digitalWrite(door_open, HIGH);
      //delay(100);
      //digitalWrite(door_open, LOW);
      //MQTT publish to HA,Then use automation to open door 推送MQTT訊息到HA主機,然後使用自動化去開門
      if (!client.connected()) {
        String clientId = "finger-";
        clientId += String(random(0xffff), HEX);
        client.setServer(mqtt_server, 1883);
        client.connect(clientId.c_str());
        Serial.println();
        Serial.print(F("Send MQTT packet to HA 送出MQTT封包給HA..."));
        Serial.println();
      }
      // publish lock to HA for automation action
      client.publish(mqtt_topic, mqtt_unlock_payload);
      // Wait 0.5 sec to reset lock state to HA for next automation action
      delay(500);
      client.publish(mqtt_topic, mqtt_lock_payload);
      Serial.println("Wait 2 seconds for next one... 等待2秒後重新辨識指紋");
      delay(1500);
      digitalWrite(led, LOW);
   
   }
   else {
     //display.setTextAlignment(TEXT_ALIGN_LEFT);
     //display.setFont(ArialMT_Plain_16);
     //display.drawString(0, 44, "Finger not ok!");
     //delay(1000);
     Serial.println();
     Serial.println("Your finger is not in database! 你的指紋不在資料庫內!");
           digitalWrite(led, HIGH);
      delay(500);
      digitalWrite(led, LOW);
      delay(100);
      digitalWrite(led, HIGH);
      delay(100);
      digitalWrite(led, LOW);
      delay(100);
      digitalWrite(led, HIGH);
      delay(100);
      digitalWrite(led, LOW);
      delay(100);
      digitalWrite(led, HIGH);
      delay(100);
      digitalWrite(led, LOW);

   }
   }
 else{
  Serial.println("Find no finger! 找不到指紋!");
  //display.setTextAlignment(TEXT_ALIGN_LEFT);
  //display.setFont(ArialMT_Plain_16);
  //display.drawString(0, 44, "Find no finger!");
  //display.clear();
  }
 Serial.println("結束檢查指紋=========================================================");
 Serial.println("");
 ArduinoOTA.handle();
  //delay for Serial.read() has data to read
 delay(300);
// display.setTextAlignment(TEXT_ALIGN_LEFT);
// display.setFont(ArialMT_Plain_16);
// display.drawString(0, 44, "Finish check finger");
 //delay(100);
   // clear the display
  //display.clear();
  // draw the current demo method
 // demos[demoMode]();

  //display.setTextAlignment(TEXT_ALIGN_RIGHT);
  //display.drawString(10, 128, String(millis()));
  // write the buffer to the display
  //display.display();
 //delay(500);

 // if (millis() - timeSinceLastModeSwitch > DEMO_DURATION) {
 //   demoMode = (demoMode + 1)  % demoLength;
 //   timeSinceLastModeSwitch = millis();
 // }
 // counter++;
 // delay(10);
}
//MQTT broker檢查是否需要重連
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT packet to HA 送出MQTT封包給HA...");
    // Create a random client ID
    String clientId = "finger-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // publish lock to HA for automation action
      client.publish(mqtt_topic, mqtt_unlock_payload);
      // Wait 1 sec to reset lock state to HA for automation action
      delay(500);
      client.publish(mqtt_topic, mqtt_lock_payload);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 1 seconds");
      // Wait 0.5 seconds before retrying
      delay(200);
    }
  }
}
