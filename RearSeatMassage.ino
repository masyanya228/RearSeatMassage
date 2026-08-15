#include "I2CSlave.h"

bool isDebug=true;
bool isTest=false;
int testTimer=0;
int LastCheck=0;

I2CSlave slave;

#define PIN_L_SWITCH 9
#define PIN_R_SWITCH 6

#define PIN_L_IND_1 A0
#define PIN_L_IND_2 A1
#define PIN_R_IND_1 A2
#define PIN_R_IND_2 A3

byte L_Mode=0;
byte R_Mode=0;

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_L_SWITCH, OUTPUT);
  pinMode(PIN_R_SWITCH, OUTPUT);
  pinMode(PIN_L_IND_1, INPUT);
  pinMode(PIN_L_IND_2, INPUT);
  pinMode(PIN_R_IND_1, INPUT);
  pinMode(PIN_R_IND_2, INPUT);
  
  slave.onCommand(REG_PING, cmdPing);
  slave.onCommand(REG_L_MODE, cmdMode);
  slave.onCommand(REG_R_MODE, cmdMode);
  slave.onCommand(REG_L_GetStatus, cmdGetStatus);
  slave.onCommand(REG_R_GetStatus, cmdGetStatus);
  slave.begin();
}

void loop() {
  slave.process();

  int now = millis();
  if(now-LastCheck>1000*5)
  {
    L_Mode=ReadIndicator(0);
    R_Mode=ReadIndicator(1);
    LastCheck=now;
  }

  if(isTest && millis()-testTimer>2000)
  {
    testTimer=millis();
    ClickHardware(0);
    ClickHardware(1);
  }

  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();

    if (command == "mode0") {
      ClickHardware(0);
      Serial.println(L_Mode);
    } else if (command == "mode1") {
      ClickHardware(1);
      Serial.println(R_Mode);
    } else if (command == "test") {
      isTest = !isTest;
      Serial.println(isTest ? "Тест включён" : "Тест выключен");
    } else {
      Serial.println("Команды: mode0 | mode1 | test");
    }
  }
  delay(5);
}

//0-left; 1-right
void ClickHardware(byte seatNum){
  if(seatNum==0)
  {
    logS("Switch #0");
    L_Mode++;
    if(L_Mode>3) L_Mode=0;
    digitalWrite(PIN_L_SWITCH, HIGH);
    delay(50);
    digitalWrite(PIN_L_SWITCH, LOW);
  }
  else if(seatNum==1)
  {
    logS("Switch #1");
    R_Mode++;
    if(R_Mode>3) R_Mode=0;
    digitalWrite(PIN_R_SWITCH, HIGH);
    delay(50);
    digitalWrite(PIN_R_SWITCH, LOW);
  }
}

byte GetIndicator(byte seatNum){
  if(seatNum==0)
    return L_Mode;
  else if(seatNum==1)
    return R_Mode;
  else
    return 0;
}

byte ReadIndicator(byte seatNum){
  if(seatNum==0)
  {
    int a1=analogRead(PIN_L_IND_1);
    int a2=analogRead(PIN_L_IND_2);
    //logI("L_IND_1", a1);
    //logI("L_IND_2", a2);
    bool ind1=a1>1024/12;
    bool ind2=a2>1024/12;
    byte mode=Mode(ind1, ind2);
    if(mode!=L_Mode)
    {
      Serial.println("Режим левого массажа изменился");
    }
    L_Mode=mode;
    
    logI("Seat #0", mode);
    return mode;
  }
  else if(seatNum==1)
  {
    int a1=analogRead(PIN_R_IND_1);
    int a2=analogRead(PIN_R_IND_2);
    //logI("R_IND_1", a1);
    //logI("R_IND_2", a2);
    bool ind1=a1>1024/12;
    bool ind2=a2>1024/12;
    byte mode=Mode(ind1, ind2);
    if(mode!=R_Mode)
    {
      Serial.println("Режим левого массажа изменился");
    }
    R_Mode=mode;
    
    logI("Seat #1", mode);
    return mode;
  }
}

byte Mode(bool i1, bool i2){
  if(i1&!i2)
    return 2;
  else if(i2 & !i1)
    return 1;
  else if(i2 & i1)
    return 3;
  else
    return 0;
}

//I2C commands
void cmdMode(const uint8_t* buf, uint8_t len) {
  Serial.print("cmdMode ");
  if (len < 1) { slave.respondByte(0x00); return; }
  uint8_t seat = 2;
  if(buf[0]==REG_L_MODE)
    seat=0;
  if(buf[0]==REG_R_MODE)
    seat=1;
  Serial.println(seat);
  if (seat > 1) { slave.respondByte(0x00); return; }
  ClickHardware(seat);
  uint8_t ind=GetIndicator(seat);
  uint8_t resp[2] = {1, ind};
  slave.respond(resp, sizeof(resp));
}

void cmdGetStatus(const uint8_t* buf, uint8_t len) {
  Serial.print("cmdGetStatus ");
  if (len < 1) { slave.respondByte(0x00); return; }
  uint8_t seat = 2;
  if(buf[0]==REG_L_GetStatus)
    seat=0;
  if(buf[0]==REG_R_GetStatus)
    seat=1;
  Serial.println(seat);
  if (seat > 1) { slave.respondByte(0x00); return; }
  uint8_t ind=GetIndicator(seat);
  uint8_t resp[2] = {1, ind};
  slave.respond(resp, sizeof(resp));
}

void cmdPing(const uint8_t*, uint8_t) {
  slave.respondByte(0x01);
}

void logS(String str){
  if(!isDebug)
    return;
  Serial.println(str);
}

void logI(String str, int i){
  if(!isDebug)
    return;
  Serial.print(str);
  Serial.print(" : ");
  Serial.println(i);
}