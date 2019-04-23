/**
    lockcode test

          Hcreak 2018.10.24

*/

#include <ESP8266WiFi.h>
//#include <ESP8266HTTPClient.h>

#include <EEPROM.h>
#include "FS.h"

#include <ArduinoJson.h>
#include <PubSubClient.h>

#include <Servo.h>

#define HallPin 5
#define M_IN1 14
#define M_IN2 12

struct config_type
{
  char ssid[32];
  char psw[64];
};
config_type config;

int ClearPin = 4;
//int ServoPin = 12;
//int HallPin = 5;
//Servo myservo;

const char* mqtt_server;
char* lockno;
char* lpassword;

int statu = -2;
//int charge = 0;

WiFiClient espClient;
//BearSSL::WiFiClientSecure espclient;
//const uint8_t fp[20] = {0x5F, 0xF1, 0x60, 0x31, 0x09, 0x04, 0x3E, 0xF2, 0x90, 0xD2, 0xB0, 0x8A, 0x50, 0x38, 0x04, 0xE8, 0x37, 0x9F, 0xBC, 0x76};
//espclient.setFingerprint(fp);
PubSubClient client(espClient);
long lastMsg = 0;
char* Topic_char = NULL;

bool loadAuthInfo()
{
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount file system");
    return false;
  }

  File InfoFile = SPIFFS.open("/AuthInfo.json", "r");
  if (!InfoFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = InfoFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  InfoFile.readBytes(buf.get(), size);

  // StaticJsonBuffer 静态分配内存(in the stack) DynamicJsonBuffer 动态分配内存(on the heap)
//  DynamicJsonBuffer  jsonBuffer;
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

  mqtt_server = json["mqtt_server"];
  String n = json["lockno"];
  lockno = (char *)malloc(n.length() * sizeof(char));
  strcpy(lockno, n.c_str());
//  lockno = json["lockno"];
  String p = json["lpassword"];
  lpassword = (char *)malloc(p.length() * sizeof(char));
  strcpy(lpassword, p.c_str());
//  lpassword = json["lpassword"];

  return true;
}

char* buildTopic(char* item)
{
  String Topic;
  Topic.concat("/");
  Topic.concat(lockno);
  Topic.concat("/");
  Topic.concat(item);
  Serial.println(Topic);
  Topic_char = (char *)malloc(Topic.length() * sizeof(char));
  strcpy(Topic_char, Topic.c_str()); // 注意c_str与其String生命周期相同 造成指针游离
  return Topic_char;
}

// 保存参数到EEPROM
void saveConfig()
{
  uint8_t *p = (uint8_t*)(&config);
  for (int i = 0; i < sizeof(config); i++)
  {
    EEPROM.write(i, *(p + i));
  }
  EEPROM.commit();
}

// 从EEPROM加载参数
void loadConfig()
{
  uint8_t *p = (uint8_t*)(&config);
  for (int i = 0; i < sizeof(config); i++)
  {
    *(p + i) = EEPROM.read(i);
  }
}

// 清空EEPROM
void clearConfig()
{
  for (int i = 0; i < sizeof(config); i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();

  ESP.restart();
}

void smartConfig()
{
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig");
  WiFi.beginSmartConfig();
  while (1)
  {
    delay(1000);
    Serial.print(".");
    if (WiFi.smartConfigDone())
    {
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      Serial.println(WiFi.localIP());

      strcpy(config.ssid, WiFi.SSID().c_str());
      strcpy(config.psw, WiFi.psk().c_str());
      saveConfig();

      break;
    }
  }
}

void beginConfig()
{
  WiFi.mode(WIFI_STA);
  loadConfig();
  Serial.printf("SSID:%s\r\n", config.ssid);
  Serial.printf("PSW:%s\r\n", config.psw);
  Serial.println("\r\WiFi begin");
  WiFi.begin(config.ssid, config.psw);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  switch(payload[0])
  {
    case 'u':
      UNLOCK();
      break;
    case 'e':
      EXPORT();
      break;
    case 'o':
      OTA();
      break;
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(lockno,lockno,lpassword,buildTopic("statu"),0,1,"-2")) 
    {
      Serial.println("connected");
      client.subscribe(buildTopic("call"));
   // client.subscribe(buildTopic("ping"));
   // EXPORT();
      readval();
    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      if (client.state() == -2)
      {
        WiFi.reconnect();
        Serial.println(WiFi.localIP());
      }
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void UNLOCK()
{
  Serial.println("UNLOCK");
//  myservo.write(70);
//  delay(500);
//  myservo.write(0);
  digitalWrite(14,LOW);
  digitalWrite(12,HIGH);
  delay(250);
  digitalWrite(14,LOW);
  digitalWrite(12,LOW);
}

void EXPORT()
{
  Serial.println("EXPORT");
  char* sc = (char *)malloc(1 * sizeof(char));
  //char* cc = (char *)malloc(3 * sizeof(char));
  sprintf(sc, "%d", statu);
  //sprintf(cc, "%d", charge);;
  client.publish(buildTopic("statu"), sc, true);
  //client.publish(buildTopic("charge"), cc, true);
}

void OTA()
{
//  t_httpUpdate_return ret = ESPhttpUpdate.update("http://sakura.xeonphi.cn/lock/firmware");
//  switch(ret) {
//    case HTTP_UPDATE_FAILED:
//        Serial.println("[update] Update failed.");
//        break;
//    case HTTP_UPDATE_NO_UPDATES:
//        Serial.println("[update] Update no Update.");
//        break;
//    case HTTP_UPDATE_OK:
//        Serial.println("[update] Update ok."); // may not called we reboot the ESP
//        break;
//  }
}

void readval()
{
  while(1){
    int Hall = 0;
    for (int i=0;i<5;i++) { 
      Hall += digitalRead(HallPin);
      delay(10);
    }
    if (Hall == 5) {
    // Serial.println("1*5,UNLOCK");
      if (statu != 0) {
        statu = 0;
        EXPORT();
      }
      return ;
    }
    if (Hall == 0) {
    // Serial.println("0*5,LOCK");
      if (statu != 1) {
        if (statu == 0) {
          digitalWrite(14,HIGH);
          digitalWrite(12,LOW);
          delay(250);
          digitalWrite(14,LOW);
          digitalWrite(12,LOW);
        }
        statu = 1;
        EXPORT();
      }
      return ;
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Start module");
  pinMode(ClearPin, INPUT_PULLUP);
  pinMode(HallPin, INPUT);
  pinMode(M_IN1, OUTPUT);
  pinMode(M_IN2, OUTPUT);
  
  EEPROM.begin(1024);
  attachInterrupt(digitalPinToInterrupt(ClearPin), clearConfig , FALLING);
  if (EEPROM.read(0) == 0)
    smartConfig();
  else
    beginConfig();
  Serial.println(WiFi.localIP());

  if (!loadAuthInfo()) {
    Serial.println("Failed to load config");
    clearConfig();
  } else {
    Serial.println("Config loaded");
  }

//  myservo.attach(ServoPin);
//  myservo.write(0);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    readval();
  }
}
