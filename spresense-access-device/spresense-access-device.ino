// ■include
#include <ArduinoQueue.h>
// felica
#include "RCS620S.h"
#include <SoftwareSerial.h>
// mqtt
#define TINY_GSM_MODEM_SIM7080
#define TINY_GSM_RX_BUFFER 650  // なくても動いたけど、あったほうが安定する気がする
#define TINY_GSM_YIELD() { delay(2); } // なくても動いたけど、あったほうが安定する気がする
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include "mqtt-config.h"
// servo include
#include <Servo.h>
// disp
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "qrcode.h"

// ■constant
#define QUEUE_SIZE_ITEMS 10
ArduinoQueue<int> authQueue(QUEUE_SIZE_ITEMS);
// felica
SoftwareSerial mySerial(22, 23);    // RX,TXの割り当て
RCS620S rcs620s(mySerial);
#define COMMAND_TIMEOUT  400
#define PUSH_TIMEOUT     2100
#define POLLING_INTERVAL 500
// mqtt
const char apn[]      = "povo.jp";
const char* broker = MY_BROKER;
const char* topicTest       = "test";
const char* topicTest2       = "test2";
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200
TinyGsm        modem(Serial2);
TinyGsmClient  client(modem);
PubSubClient  mqtt(client);
uint32_t lastReconnectAttempt = 0;
// servo
static Servo s_servo; /**< Servo object */
// disp
#define TFT_DC 9
#define TFT_CS -1
#define TFT_RST 8
#define X_BASE_PIXLE 60
#define Y_BASE_PIXLE 90
#define SCALE_SIZE 4 // x3
Adafruit_ILI9341 tft = Adafruit_ILI9341(&SPI, TFT_DC, TFT_CS, TFT_RST);

// ■function
// felica
void felicaInit()
{
  int ret = 0;
  mySerial.begin(115200);
  delay(1000);

  while(ret != 1){
    ret = rcs620s.initDevice();
    Serial.print("RCS620S Init = ");
    Serial.println(ret);
    delay(1000);
  }
  
  Serial.print("felica Init success\n");
}

void felicaRead()
{
  bool result = false;
  rcs620s.timeout = COMMAND_TIMEOUT;
  int ret = rcs620s.polling();
  Serial.print("RCS620S polling = ");
  Serial.println(ret);
  if (ret) {
    
    Serial.print("idm = ");
    for (int i = 0; i < 8; i++) {
      Serial.print(rcs620s.idm[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    Serial.print("pmm = ");
    for (int i = 0; i < 8; i++) {
      Serial.print(rcs620s.pmm[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    rcs620s.readWithEncryption(
      rcs620s.pmm,
      0x000B,
      1 /* block id = 1 の末尾に番号が入っている*/);

    // 認証Queueにenqueue
    authQueue.enqueue(1);

    while(1){
      if(rcs620s.polling() == 0){
        break;
      }
      delay(1);
    }
  }

  rcs620s.rfOff();

  return result;
}

// mqtt
void mqttCallback(char* topic, byte* payload, unsigned int len) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("]: ");
  Serial.print("bin:");
  for(int i = 0;i < len;i++){
    Serial.print(payload[i]);
  }
  Serial.println();

  char payloadBuf[256] = {0};
  strncpy(payloadBuf,(const char *)payload,len);
  
  String str = String(payloadBuf);
  Serial.print("str:");
  Serial.println(str);

  Serial.print("Message send to ");
  Serial.println(topicTest2);
  Serial.println(str);

  char buf[256];
  str.toCharArray(buf, 256);
  mqtt.publish(topicTest2, buf);

  // 認証Queueにenqueue
  authQueue.enqueue(1);
}

boolean mqttConnect() {
  Serial.print("Connecting to ");
  Serial.println(broker);

  // Connect to MQTT Broker
  boolean status = mqtt.connect("GsmClientTest");

  // Or, if you want to authenticate MQTT:
  // boolean status = mqtt.connect("GsmClientName", "mqtt_user", "mqtt_pass");

  if (status == false) {
    Serial.println(" fail");
    return false;
  }
  Serial.println(" success");
  mqtt.subscribe(topicTest);
  return mqtt.connected();
}

void mqttInit()
{
  Serial2.begin(9600, SERIAL_8N1);
  

  Serial.println("Wait...");  // Print text on the screen (string) 在屏幕上打印文本(字符串)
  // Set GSM module baud rate

  // モデムのリスタート
  Serial.println("Initializing modem...");  // Print text on the screen (string) 在屏幕上打印文本(字符串)
  modem.restart();

  // モデムの情報の取得
  String modemInfo = modem.getModemInfo();
  Serial.print("Modem Info: ");
  Serial.println(modemInfo);


  // GPRS connection parameters are usually set after network registration
  Serial.print(F("Connecting to "));
  Serial.print(apn);
  if (!modem.gprsConnect(apn, "", "")) {
    Serial.println("-> fail");
    delay(10000);
    return;
  }
  Serial.println("-> success");

  if (modem.isGprsConnected()) { Serial.println("GPRS connected"); }

  mqtt.setServer(broker, 1883);
  mqtt.setCallback(mqttCallback);

  mqtt.publish(topicTest2, "Hello Server! I'm spresense");  
}

void mqttLoop()
{
  // Make sure we're still registered on the network
  if (!modem.isNetworkConnected()) {
      Serial.println("Network disconnected");
      
      if (!modem.waitForNetwork(180000L, true)) {
        Serial.println(" fail");
        delay(10000);
        return;
      }
      
      if (modem.isNetworkConnected()) {
        Serial.println("Network re-connected");
      }
      
      // and make sure GPRS/EPS is still connected
      if (!modem.isGprsConnected()) {
        Serial.println("GPRS disconnected!");
        Serial.print(F("Connecting to "));
        Serial.print(apn);
        if (!modem.gprsConnect(apn, "", "")) {
          Serial.println(" fail");
          delay(10000);
          return;
        }
        if (modem.isGprsConnected()) { Serial.println("GPRS reconnected"); }
      }
  }
      
  if (!mqtt.connected()) {
    Serial.println("=== MQTT NOT CONNECTED ===");
    // Reconnect every 10 seconds
    uint32_t t = millis();
    if (t - lastReconnectAttempt > 10000L) {
      lastReconnectAttempt = t;
      if (mqttConnect()) { lastReconnectAttempt = 0; }
    }
    delay(100);
    return;
  }
  
  mqtt.loop();
}

// servo 
void servoInit()
{
  s_servo.attach(PIN_D06);
  servoClose();
  delay(5000);
  
  Serial.print("servo Init success\n");
}

void servoOpen()
{
  s_servo.write(0);
}

void servoClose()
{
  s_servo.write(90);
}

void dispInit()
{
  Serial.println("ILI9341 Test!"); 
  tft.begin(40000000);

  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 
  
  // Create the QR code
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, DISP_URL);
  
  // Serial.print(F("Text                     "));
  // Serial.println(testText());
  tft.fillScreen(ILI9341_WHITE);
  // tft.setCursor(50, 50);
  for (unsigned int y = 0; y < qrcode.size; y++) {
    for (unsigned int x = 0; x < qrcode.size; x++) {
        if (qrcode_getModule(&qrcode, x, y)){
          tft.writeFillRect(
            X_BASE_PIXLE +  (x * SCALE_SIZE),
            Y_BASE_PIXLE +  (y * SCALE_SIZE),
            SCALE_SIZE,
            SCALE_SIZE,
            ILI9341_BLACK);
        }
    }
    // Serial.print("\n");
  }

  tft.setCursor(40, 240);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(3);
  tft.println("Spresense");
}


void ledLightUp()
{
  digitalWrite(LED0, HIGH);
  delay(100);
  digitalWrite(LED1, HIGH);
  delay(100);
  digitalWrite(LED0, LOW);
  digitalWrite(LED2, HIGH);
  delay(100);
  digitalWrite(LED1, LOW);
  digitalWrite(LED3, HIGH);
  delay(100);
  digitalWrite(LED2, LOW);
  delay(100);
  digitalWrite(LED3, LOW);
}

void setup() {
  // ログ用シリアル
  Serial.begin(115200);

  // LED
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  felicaInit();
  mqttInit();
  servoInit();
  dispInit();
  ledLightUp();
}

void loop() {
  felicaRead();
  mqttLoop();

  if (!authQueue.isEmpty())
  {
    int value = authQueue.dequeue();
    servoOpen();
    delay(5000);
    servoClose();
  }
  
  delay(500);
}
