#include <EEPROM.h>
#include <Wire.h>

bool isDebug=true;
bool isTest=false;

// адрес
#define SLAVE_ADDR 10
// команды
#define REG_L_MODE 0x01
#define REG_L_GetStatus 0x02
#define REG_R_MODE 0x03
#define REG_R_GetStatus 0x04
#define REG_GetErrorCount 0x05
#define REG_GetNextError 0x06

#define PIN_L_SWITCH 9
#define PIN_R_SWITCH 6

#define PIN_L_IND_1 A0
#define PIN_L_IND_2 A1
#define PIN_R_IND_1 A2
#define PIN_R_IND_2 A3

int L_Mode=0;
int R_Mode=0;
int LastCheck=0;

uint8_t cmd = 0; //последняя команда
uint8_t counter = 0; // счётчик сообщений

uint32_t lastMessage=0;
const int I2C_NoInputTimeOut = 30000; //30 секунд таймаута
bool isOnline=false;
bool isVoid=false;

struct Error{
  uint16_t code=0;
  uint32_t tfs=0;
  uint8_t times=0;
};

/* 13 - Связь не установлена
 * 14 - Связь потеряна
 * 15 - Недостоверный сигнал вентиляции слева
 * 16 - Недостоверный сигнал вентиляции справа
 */
Error errors[3];
int sizeErr;
int nextError=0;

// обработчик приёма
void receiveCb(int amount) {
  cmd = Wire.read();
  ++counter;

  switch (cmd) {
    case REG_L_MODE:
      ClickHardware(0);
      break;
    case REG_R_MODE:
      ClickHardware(1);
      break;
    case REG_L_GetStatus: break;
    case REG_R_GetStatus: break;
    case REG_GetNextError:
      nextError=Wire.read();
      break;
    case REG_GetErrorCount: break;
  }
}

// обработчик запроса
void requestCb() {
  switch (cmd) {
    case REG_L_MODE: 
    case REG_L_GetStatus:
      Wire.write(GetIndicator(0));
      break;
    case REG_R_MODE:
    case REG_R_GetStatus:
      Wire.write(GetIndicator(1));
      break;
    case REG_GetErrorCount:
      SendHealth();
      break;
    case REG_GetNextError:
      SendError(nextError);
      break;
  }
}

void setup() {
  sizeErr=sizeof(errors[0]);
  InitEEPROM();
  LastCheck=millis();
  
  pinMode(PIN_L_SWITCH, OUTPUT);
  pinMode(PIN_R_SWITCH, OUTPUT);
  pinMode(PIN_L_IND_1, INPUT);
  pinMode(PIN_L_IND_2, INPUT);
  pinMode(PIN_R_IND_1, INPUT);
  pinMode(PIN_R_IND_2, INPUT);
  
  Serial.begin(9600);
  
  Wire.begin(SLAVE_ADDR);
  Wire.onReceive(receiveCb);
  Wire.onRequest(requestCb);
}

void loop() {
  if(isTest){
    delay(4500);
    ClickHardware(0);
    delay(100);
    ReadIndicator(0);
  }
  
  int now = millis();
  if(now-LastCheck>1000*5)
  {
    L_Mode=ReadIndicator(0);
    R_Mode=ReadIndicator(1);
    LastCheck=now;
  }
  if(!isVoid && !isOnline && lastMessage==0 && now>I2C_NoInputTimeOut)
  {
    isVoid=true;
    SaveError(13);
  }
  if(!isVoid && isOnline && now-lastMessage>I2C_NoInputTimeOut)
  {
    isOnline=false;
    isVoid=true;
    SaveError(14);
  }
}

void CatchErrors(){
  
}

void SaveError(){
  
}

//0-left; 1-right
void ClickHardware(int seatNum){
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

int GetIndicator(int seatNum){
  if(seatNum==0)
    return L_Mode;
  else if(seatNum==1)
    return R_Mode;
}

int ReadIndicator(int seatNum){
  if(seatNum==0)
  {
    int a1=analogRead(PIN_L_IND_1);
    int a2=analogRead(PIN_L_IND_2);
    logI("L_IND_1", a1);
    logI("L_IND_2", a2);
    bool ind1=a1>1024/12;
    bool ind2=a2>1024/12;
    int mode=Mode(ind1, ind2);
    if(mode!=L_Mode)
    {
      SaveError(15);
    }
    
    logI("Seat #0", mode);
    return mode;
  }
  else if(seatNum==1)
  {
    int a1=analogRead(PIN_R_IND_1);
    int a2=analogRead(PIN_R_IND_2);
    logI("R_IND_1", a1);
    logI("R_IND_2", a2);
    bool ind1=a1>1024/12;
    bool ind2=a2>1024/12;
    int mode=Mode(ind1, ind2);
    if(mode!=R_Mode)
    {
      SaveError(16);
    }
    
    logI("Seat #1", mode);
    return mode;
  }
}

int Mode(bool i1, bool i2){
  if(i1&!i2)
    return 2;
  else if(i2 & !i1)
    return 1;
  else if(i2 & i1)
    return 3;
  else
    return 0;
}

void SetupErrors(){
  errors[0].code=13;
  errors[1].code=14;
  errors[2].code=15;
}

void logI(String str, int i){
  if(!isDebug)
    return;
  Serial.print(str);
  Serial.print(" : ");
  Serial.println(i);
}

void logS(String str){
  if(!isDebug)
    return;
  Serial.println(str);
}
