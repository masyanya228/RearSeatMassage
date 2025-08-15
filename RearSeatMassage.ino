#include <Wire.h>
// адрес
#define SLAVE_ADDR 10

// команды
#define REG_L_MODE 0x01
#define REG_L_GetStatus 0x02
#define REG_R_MODE 0x03
#define REG_R_GetStatus 0x04
#define REG_GetErrorCount 0x05
#define REG_GetNextError 0x06

#define PIN_L_SWITCH 1
#define PIN_R_SWITCH 1

#define PIN_L_IND_1 1
#define PIN_L_IND_2 1
#define PIN_R_IND_1 1
#define PIN_R_IND_2 1

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
  Serial.begin(9600);
  // put your setup code here, to run once:
  Wire.begin(SLAVE_ADDR);

  Wire.onReceive(receiveCb);
  Wire.onRequest(requestCb);
}

void loop() {
}

void CatchErrors(){
  
}

void SaveError(){
  
}

int debugMode=0;
//0-left; 1-right
void ClickHardware(int seatNum){
  debugMode++;
  if(debugMode>2)
    debugMode = 0;
}

int GetIndicator(int seatNum){
  Serial.println(debugMode);
  return debugMode;
}
