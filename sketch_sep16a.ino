#include <Mouse.h>

/**
    lockcode test
      
          Hcreak 2018.10.24
          
 */

 
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <EEPROM.h>

#include <ArduinoJson.h>

struct config_type
{
  char ssid[32];
  char psw[64];
};
 
config_type config;

int ClearPin = 14;
const char* host="172.17.11.115";


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
  for (int i=0; i< sizeof(config); i++)
  {
    EEPROM.write(i,0); 
  }
  EEPROM.commit();
  
  while(1)
    Serial.println("Fuck WatchDog");
      
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
  loadConfig();
  Serial.printf("SSID:%s\r\n", config.ssid);
  Serial.printf("PSW:%s\r\n", config.psw);
  Serial.println("\r\WiFi begin");
  WiFi.begin(config.ssid,config.psw);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void createJson()
{
  // StaticJsonBuffer 在栈区分配内存   它也可以被 DynamicJsonBuffer（内存在堆区分配） 代替
  DynamicJsonBuffer  jsonBuffer;
 
  //创建根，也就是顶节点
  JsonObject& root = jsonBuffer.createObject();
 
  root["lockno"] = "i9P3DpKkrye";
  root["statu"] = 0;
  root["charge"] = 97;
  
  root.printTo(Serial);//单行打印
  Serial.println();
  root.prettyPrintTo(Serial);//多行打印
  Serial.println();
  
  HTTPClient http;
  http.begin(host,5000,"/");
  String output;
  root.printTo(output);
  int httpCode = http.POST(output);
  if (httpCode == HTTP_CODE_OK)
  {
     String payload = http.getString();
     Serial.println(payload);
     JsonObject& req = jsonBuffer.parseObject(payload);
     if (req.success()) 
     {
        int no = req["no"];
        const char* statu = req["statu"];
        Serial.println(no);
        Serial.println(statu);
     }
     else 
     {
        Serial.println("parseObject() failed");
     }
     
  } 
  else 
  {
     String payload = http.getString();
     Serial.print("context:");
     Serial.println(payload);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Start module");
  pinMode(ClearPin,INPUT);
  
  EEPROM.begin(1024); 
  attachInterrupt(digitalPinToInterrupt(ClearPin), clearConfig , RISING);
     
  if(EEPROM.read(0)==0)
    smartConfig();
  else
    beginConfig();  
}


void loop()
{
  delay(5000);
  Serial.println(WiFi.localIP());
  
  createJson();
  
}
