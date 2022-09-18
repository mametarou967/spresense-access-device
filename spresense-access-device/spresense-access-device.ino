// ■include
// felica
#include "RCS620S.h"
#include <SoftwareSerial.h>
// servo include
#include <Servo.h>

// ■constant
// felica
SoftwareSerial mySerial(22, 23);    // RX,TXの割り当て
RCS620S rcs620s(mySerial);
#define COMMAND_TIMEOUT  400
#define PUSH_TIMEOUT     2100
#define POLLING_INTERVAL 500
// servo
static Servo s_servo; /**< Servo object */

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

bool felicaRead()
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

    result = true;

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

  // felica
  felicaInit();
  servoInit();
}

void loop() {
  bool felicaReadResult = false;
  felicaReadResult = felicaRead();
  if(felicaReadResult)
  {
    servoOpen();
    delay(5000);
    servoClose();
  }
  delay(500);
}
