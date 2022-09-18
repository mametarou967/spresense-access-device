#if( SUBCORE != 1)
#error "Core selection is wrong!!"
#endif

#include <MP.h>
#include "RCS620S.h"
#include <SoftwareSerial.h>

SoftwareSerial mySerial(22, 23);    // RX,TXの割り当て

RCS620S rcs620s(mySerial);

#define COMMAND_TIMEOUT  400
#define PUSH_TIMEOUT     2100
#define POLLING_INTERVAL 500

void setup() {
  MP.begin();
  mySerial.begin(115200);
  delay(1000);

  int ret = rcs620s.initDevice();
  MPLog("RCS620S Init = %d\n",ret);
}

void loop() {
  
    rcs620s.timeout = COMMAND_TIMEOUT;
    int ret = rcs620s.polling();
    MPLog("RCS620S polling = %d\n",ret);
    if (ret) {
      
      MPLog("idm = ");
      for (int i = 0; i < 8; i++) {
        MPLog("%x",rcs620s.idm[i]);
        MPLog(" ");
      }
      MPLog("\n");
  
      MPLog("pmm = ");
      for (int i = 0; i < 8; i++) {
        MPLog("%x",rcs620s.pmm[i]);
        MPLog(" ");
      }
      MPLog("\n");

      rcs620s.readWithEncryption(
        rcs620s.pmm,
        0x000B,
        1 /* block id = 1 の末尾に番号が入っている*/);

      // main coreに意味のある情報が読み込めたことを通知する
      int8_t msgid = 0;
      uint32_t msgdata = 0;
      int subid = 0;
      MP.Send(msgid,msgdata,subid);

      while(1){
        if(rcs620s.polling() == 0){
          break;
        }
        delay(1);
      }
    }
  
    rcs620s.rfOff();
    delay(POLLING_INTERVAL);
}
