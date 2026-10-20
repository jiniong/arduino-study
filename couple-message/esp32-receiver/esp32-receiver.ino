#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <U8g2lib.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 0x3C);

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  u8g2.clearBuffer();
  u8g2.setCursor(0, 16);

  String message = "";
  Serial.print("메세지 수신 : ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  u8g2.print(message);
  Serial.println();

  u8g2.sendBuffer();
}

void setup() {
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_unifont_t_korean1);

  Serial.begin(115200);
  Serial.println("Wifi 연결 시도...");

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("연결 성공!");
  Serial.print("IP : ");
  Serial.println(WiFi.localIP());

  delay(2000);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  while(!client.connected()) {
    Serial.println("MQTT 연결 시도...");
    if (client.connect("esp32-receiver")) {
      Serial.println("MQTT 연결 성공!");
      client.subscribe("couple/phone2device");
    } else {
      Serial.print("실패 에러 : ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    while (!client.connected()) {
      if (client.connect("esp32-receiver")) {
        client.subscribe("couple/phone2device");
      } else {
        delay(2000);
      }
    }
  }
  client.loop();
}
