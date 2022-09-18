#ifdef SUBCORE
#error "Core selection is wrong!!"
#endif

#include <MP.h>

const int felicaReaderCore_subCore1=1; // spresense-felica-reader-SC1

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
  
  int ret = MP.begin(felicaReaderCore_subCore1);
  MPLog("run subcore = %d ret = %d\n",felicaReaderCore_subCore1,ret);
  // 確認用LED
}

void loop() {
  int8_t rcvid;
  uint32_t rcvdata;
  
  MP.RecvTimeout(MP_RECV_BLOCKING);
  int ret = MP.Recv(&rcvid,&rcvdata,felicaReaderCore_subCore1);
  MPLog("rcvid = %d,subcore = %d",rcvid,felicaReaderCore_subCore1);
  ledLightUp();
}
