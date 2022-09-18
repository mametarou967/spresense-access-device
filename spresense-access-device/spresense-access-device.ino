// servo include
#include <Servo.h>
static Servo s_servo; /**< Servo object */

// servo function
void servoInit()
{
  s_servo.attach(PIN_D06);
  servoClose();
  delay(5000);
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
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  servoInit();
}

void loop() {
  servoOpen();
  delay(5000);
  servoClose();
  delay(5000);
  ledLightUp();
  delay(5000);
}
