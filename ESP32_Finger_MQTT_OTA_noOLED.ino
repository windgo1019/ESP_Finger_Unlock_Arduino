/*
  This is a sample for finger search in database 這是一個將放入的指紋與已存入資料庫的指紋做比對的範例
  you can follow this post to get the latest code 你可以追蹤這篇文章來取得最新的程式碼
  https://bbs.hassbian.com/thread-2768-1-1.html

  Hardware list硬體購買清單:
  FPC1020AM+QS808 UART輸出輸入指紋模組
  https://item.taobao.com/item.htm?id=545528806282
  ESP32 WIFIKit32模組
  https://item.taobao.com/item.htm?id=555572682989https://detail.tmall.com/item.htm?id=44766400092
  USB-TTL:非必須，但可以用來在PC上連接指紋模組好註冊指紋進模組內給未來搜尋資料庫使用
  https://detail.tmall.com/item.htm?id=536700034613
  Dupont Line 杜邦線
  https://detail.tmall.com/item.htm?id=41065178536
  220歐姆電阻
  https://item.taobao.com/item.htm?id=5653998305
  LED
  https://item.taobao.com/item.htm?id=38869571577

  Reference參考資料:
  指紋模組FPC1020AM_算法板QS808_IDWD1020用户使用手册
  https://wenku.baidu.com/view/c174958f4a7302768f993900.html
  MQTT教學（六）：使用PubSubClient程式庫開發Arduino MQTT應用
  https://swf.com.tw/?p=1021
  使用USBpcap抓取USB封包內容
  https://www.anquanke.com/post/id/85218
  額外參考：Mifare RFID-RC522模組實驗（四）：Mifare RFID的門禁系統實驗
  https://swf.com.tw/?p=1027

  Finger指紋+算法板連接WIFIKit32的接腳方式(務必看手冊去接線，接錯模組可能會燒掉)
  https://raw.githubusercontent.com/lvidarte/esp8266/master/nodemcu_pins.png
  Finger   WIFIKit32
  TX     = GPIO 16
  RX     = GPIO 17
  GND    = GND
  3.3V   = 3.3V , 不能接5V,接錯模組會燒掉!

  LED      WIFIKit32
  +      = GPIO 5
  GND    = GND

  下面為透過usbpcap抓包得到的指紋模組檢查的流程與回應數據
  #command
  HOST发送 CMD_TEST_CONNECTION指令及模块的响应
  HOST命令：  55 AA 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01
  Target响应：AA 55 01 00 01 00 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 01

  #command1
  发送采集指纹图像后模块检测到手指的命令及响应   <-- 連續偵測到有檢測到手指
  Host命令：  55 AA 00 00 20 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 1F 01
  Target响应：AA 55 01 00 20 00 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 22 01

  发送采集滑动指纹图像后结果超时（FP TimeOut）的命令及响应
  CMD_GET_IMAGE ：55 AA 00 00 20 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 1F 01
  ERR_TIME_OUT：  AA 55 01 00 20 00 02 00 23 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 4A 01

  #command2
  从ImageBuffer中生成模板数据保存在RamBuffer0中
  Host命令包：  55 AA 00 00 60 00 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 61 01
  Target响应包：  AA 55 01 00 60 00 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 62 01

  #command3
  暂存在RamBuffer0中的指纹模板与1-500编号范围内的指纹比对，返回比对结果 响应內的01=已註冊在指紋資料庫的Finger ID
  Host命令： 55 AA 00 00 63 00 06 00 00 00 01 00 F4 01 00 00 00 00 00 00 00 00 00 00 5E 02
  Target响应：AA 55 01 00 63 00 05 00 00 00 01 00 01 00 00 00 00 00 00 00 00 00 00 00 6A 01
*/
//Finger TX -- GPIO16 HardwareSerial finger(1); or GPIO 09 HardwareSerial finger(2)
//Finger RX -- GPIO17 HardwareSerial finger(1); or GPIO 10 HardwareSerial finger(2)
HardwareSerial finger(1);
//HardwareSerial finger(2);
#include <WiFi.h>      // 引用程式庫
#include <mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

#define led 5
//Finger command to connect finger device 第一次跟指紋模組溝通
const byte command[] = {0x55, 0xAA, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
//Finger command1 to check finger touch 檢查是否有放入手指
const byte command1[] = {0x55, 0xAA, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x01};
//Finger command2 to save finger pic to imagebuffer 將指紋掃描結果存入暫存檔
const byte command2[] = {0x55, 0xAA, 0x00, 0x00, 0x60, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, 0x01};
//Finger command3 to search imagebuff in finger database 將暫存檔與指紋資料庫做比對
const byte command3[] = {0x55, 0xAA, 0x00, 0x00, 0x63, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0xF4, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5E, 0x02};
const char *ssid =  "your_wifi_ssid";    // WIFI名稱
const char *password =  "your_wifi_password";     // WIFI密碼
const char *mqtt_server = "your matt server ip"; // your matt server ip
const char *mqtt_topic =  "finger";     // mqtt_topic for HA sensor name
const char *mqtt_unlock_payload =  "unlock";     // mqtt_unlock_payload for HA sensor state
const char *mqtt_lock_payload =  "lock";     // mqtt_lock_payload for HA sensor state
const char *ota_device_name =  "finger_esp32";     // OTA_device_name for upload firmware
//if you want to enable ota password, you should uncomment "//const char *ota_upload_password" and "//ArduinoOTA.setPassword(ota_upload_password)"
//要啟用OTA密碼功能的話要反註解下面的  "//const char *ota_upload_password" and //ArduinoOTA.setPassword(ota_upload_password)
//const char *ota_upload_password =  "your_ota_password";     // OTA_upload_password for upload firmware

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  finger.begin(115200);
  pinMode(led,OUTPUT);
  digitalWrite(led, LOW);

  //Send command to connect finger device
  //byte command[] = {0x55,0xAA,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01};
  finger.write(command, sizeof(command));
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

void loop() {
  //initial value
  int index;
  byte inData[26];
  byte incomingByte;
  // reconnect wifi
  if (WiFi.status() != WL_CONNECTED) {
  WiFi.begin(ssid, password);
  //delay for wifi connected
  delay(1500);
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
  }
  }
  // Starting OTA
  ArduinoOTA.handle();
  
 //Send command1 to check finger touch 
 //byte command1[] = {0x55,0xAA,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0x01};
 finger.write(command1, sizeof(command1));
 index = 0;
 while (!finger.available()) {
    delay(5);
   }
 while (finger.available() > 0) {
    incomingByte = finger.read();
    //Serial.print(incomingByte,HEX);
    //Serial.print(' ');
    inData[index] = incomingByte;
    index++;
  }
  Serial.println("開始檢查指紋=========================================================");
 
  //check command1 result
  for (int i = 0 ; i<=25; i++){
   Serial.print(inData[i],HEX);
   Serial.print(" ");
  }
  Serial.println("");
  
 if (inData[8] == 0x00 && inData[24] == 0x22 ){
   Serial.println("Find finger! 檢查到有手指放入!");
   //Send command2 to save finger pic to imagebuffer
   //byte command2[] = {0x55,0xAA,0x00,0x00,0x60,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x61,0x01};
   finger.write(command2, sizeof(command2));
   int index = 0;
   while (!finger.available()) {
    delay(5);
   }
   while (finger.available() > 0) {
    byte incomingByte = finger.read();
    //Serial.print(incomingByte,HEX);
    //Serial.print(' ');
    inData[index] = incomingByte;
    index++;
  }
    //check command2 result
    for (int i = 0 ; i<=25; i++){
     Serial.print(inData[i],HEX);
     Serial.print(" ");
    }
   //Send command3 to search imagebuff in finger database
   //byte command3[] = {0x55,0xAA,0x00,0x00,0x63,0x00,0x06,0x00,0x00,0x00,0x01,0x00,0xF4,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x5E,0x02};
   finger.write(command3, sizeof(command3));
   index = 0;
   Serial.println();
   Serial.println("Finger check.... 比對指紋資料庫中....");
   while (!finger.available()) {
    delay(5);
   }
   while (finger.available() > 0) {
    byte incomingByte = finger.read();
    //Serial.print(incomingByte,HEX);
    //Serial.print(' ');
    inData[index] = incomingByte;
    index++;
  }
   /*  
  //check command3 result

   Serial.println("");
    for (int i = 0 ; i<=25; i++){
     Serial.print(inData[i],HEX);
     Serial.print(" ");
    }
  */
   if (inData[4] == 0x63 && inData[6] == 0x05 ){
     Serial.println();
     Serial.println("Your finger is in database! Open door! 已找到指紋!開門!");
     //delay(1000);
      //LED燈亮
      digitalWrite(led, HIGH);
      delay(50);
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
  }
 Serial.println("結束檢查指紋=========================================================");
 Serial.println("");
  //delay for Serial.read() has data to read
 delay(500);
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
