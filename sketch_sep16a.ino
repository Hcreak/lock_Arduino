/**
    lockcode test

          Hcreak 2018.10.24

*/

#include <ESP8266WiFi.h>
//#include <ESP8266HTTPClient.h>

#include <EEPROM.h>

//#include <ArduinoJson.h>
#include <PubSubClient.h>

struct config_type
{
  char ssid[32];
  char psw[64];
};
config_type config;

int ClearPin = 14;

const char* mqtt_server = "172.20.0.145";
const char* lockno = "i9P3DpKkrye";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

char* buildTopic(char* item)
{
  char* Topic;  
  strcat(Topic, "/");
  strcat(Topic, lockno);
  strcat(Topic, "/");
  strcat(Topic, item);
  Serial.print(Topic);
  return Topic;
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

  while (1)
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

//  if ((char)payload[0] == '1') {
//    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
//    // but actually the LED is on; this is because
//    // it is acive low on the ESP-01)
//  } else {
//    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
//  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(lockno)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(buildTopic("heart"), "hello world");
      // ... and resubscribe
      client.subscribe(buildTopic("call"));
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Start module");
  pinMode(ClearPin, INPUT_PULLUP);

  EEPROM.begin(1024);
  attachInterrupt(digitalPinToInterrupt(ClearPin), clearConfig , FALLING);

  if (EEPROM.read(0) == 0)
    smartConfig();
  else
    beginConfig();
  Serial.println(WiFi.localIP());

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
  if (now - lastMsg > 5000) {
    lastMsg = now;
    char* msg = (char*)analogRead(A0);

    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(buildTopic("statu"), msg);
  }
}
