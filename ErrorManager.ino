void InitEEPROM(){
  EEPROM.get(0, errors);
  if(errors[0].code==0){
    ResetEEPROM();
  }
}

void ResetEEPROM(){
  if(isDebug) Serial.println("EEPROM Init");
  SetupErrors();
  EEPROM.put(0, errors); 
}

void SaveError(uint16_t code){
  logI("encountered", code);
  int i = IndexOfError(code);
  errors[i].times++;
  EEPROM.update(sizeErr*i+2+4, errors[i].times);
  if(errors[i].tfs==0){
    errors[i].tfs=millis();
    EEPROM.update(sizeErr*i+2, errors[i].tfs);
  }
}

int IndexOfError(uint16_t code){
  int len=sizeof(errors) / sizeErr;
  for(int i=0; i<len; i++)
  {
    if(errors[i].code==code){
      return i;
    }
  }
  return -1;
}

void ResetError(uint16_t code){
  int i = IndexOfError(code);
  errors[i].tfs=0;
  errors[i].times=0;
  EEPROM.update(sizeErr*i+2+4, 0);
  EEPROM.update(sizeErr*i+2, 0);
  logI("reseted", code);
}

void LogError(uint16_t code){
  if(!isDebug) return;
  
  int i = IndexOfError(code);
  Serial.print(errors[i].code);
  Serial.print(" ");
  Serial.print(errors[i].tfs);
  Serial.print(" ");
  Serial.println(errors[i].times);
}

void SendHealth(){
  int len=sizeof(errors) / sizeErr;
  uint8_t errorCount=0;
  for(int i=0; i<len; i++){
    if(errors[i].times>0)
    {
      errorCount++;
    }
  }
  I2C_writeAnything(errorCount);
  I2C_writeAnything(millis());
}

void SendError(int i){
  int len=sizeof(errors) / sizeErr;
  if(i>=len){
    Wire.write(0);
    return;
  }
  Wire.write(1);
  I2C_writeAnything(errors[i].code);
  I2C_writeAnything(errors[i].tfs);
  I2C_writeAnything(errors[i].times);
}
