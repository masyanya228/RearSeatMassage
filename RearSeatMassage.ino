#include <Wire.h>

bool isDebug=false;
bool isTest=true;

// адрес
#define SLAVE_ADDR 10
// команды
#define REG_L_MODE 0x01
#define REG_L_GetStatus 0x02
#define REG_R_MODE 0x03
#define REG_R_GetStatus 0x04
#define REG_GetErrorCount 0x05
#define REG_GetNextError 0x06

#define PIN_L_SWITCH A6
#define PIN_R_SWITCH A7

#define PIN_L_IND_1 5
#define PIN_L_IND_2 6
#define PIN_R_IND_1 7
#define PIN_R_IND_2 8

int L_Mode=0;
int R_Mode=0;
int LastCheck=0;

// последняя выбранная команда
// в обработчике приёма
uint8_t cmd = 0;
// счётчик сообщений
uint8_t counter = 0;

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
    }
}

void setup() {
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
  int now = millis();
  if(now-LastCheck>1000*5)
  {
    L_Mode=GetIndicator(0);
    R_Mode=GetIndicator(1);
  }

  if(isTest){
    delay(2000);
    ClickHardware(0);
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
    logS("Switch #"+seatNum);
    digitalWrite(PIN_L_SWITCH, HIGH);
    delay(50);
    digitalWrite(PIN_L_SWITCH, LOW);
  }
  else if(seatNum==1)
  {
    logS("Switch #"+seatNum);
    digitalWrite(PIN_R_SWITCH, HIGH);
    delay(50);
    digitalWrite(PIN_R_SWITCH, LOW);
  }
}

int GetIndicator(int seatNum){
  if(seatNum==0)
  {
    bool L1=digitalRead(PIN_L_IND_1)==HIGH;
    bool L2=digitalRead(PIN_L_IND_2)==HIGH;
    logI("Seat #"+seatNum, Mode(L1, L2));
    return Mode(L1, L2);
  }
  else if(seatNum==1)
  {
    bool R1=digitalRead(PIN_R_IND_1)==HIGH;
    bool R2=digitalRead(PIN_R_IND_2)==HIGH;
    logI("Seat #"+seatNum, Mode(R1, R2));
    return Mode(R1, R2);
  }
}

int Mode(bool i1, bool i2){
  if(i1&!i2)
    return 1;
  else if(i2 & !i1)
    return 2;
  else if(i2 & i1)
    return 3;
  else
    return 0;
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
