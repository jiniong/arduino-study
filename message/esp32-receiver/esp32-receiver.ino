#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 0x3C);

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "broker.hivemq.com";
String messages[4] = {"", "", "", ""};
int count = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("메세지 수신 : ");
  String message = "> ";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  Serial.println();

  if (count >= 4) {
    messages[0] = messages[1];
    messages[1] = messages[2];
    messages[2] = messages[3];
  }

  if (count < 4) {
    messages[count] = message;
    count++;
  } else {
    messages[3] = message;
  }

  u8g2.clearBuffer();
  for (int i = 0; i < count; i++) {
    u8g2.setCursor(0, 16 * (i + 1));
    u8g2.println(messages[i]);
  }
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

  u8g2.clearBuffer();
  u8g2.setCursor(0, 16);
  u8g2.print("Ready!");
  u8g2.sendBuffer();
}

void loop() {
  if (!client.connected()) {
    while(!client.connected()) {
      if (client.connect("esp32-receiver")) {
        client.subscribe("couple/phone2device");
      } else {
        delay(2000);
      }
    }
  }
  client.loop();
}
