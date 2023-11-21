#include <Arduino.h>
#include <Excelsior_Ambassador.h>

using namespace std;

//------SETUP------------------
Excelsior_Ambassador::Excelsior_Ambassador() : display(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS){  //Display Dimensions: 180,220
  Serial.begin(9600);
  Serial.println((String)"Excelsior Ambassador Version " + version);

  //Wire.begin(4);                //Ambassador assigns itself the address 4
  //Wire.onReceive(receiveEvent);  //function that gets called, if data is received

  display.begin();
  display.clear();
  display.setOrientation(2);
  display.setBackgroundColor(COLOR_BLACK);

  drawBootScreen();
}

void Excelsior_Ambassador::decodeData(int length){      //message length
  Serial.println("Recieved new Message: ");
  for(int i = 0; i < length; i++){
    Serial.print(receivedData[i], BIN);
    if(i != length - 1)
      Serial.print(", ");
  }
  Serial.println();
  //--Header--
  protocolVersion = receivedData[0];
  dataAndDisplayType = receivedData[1];

  //--ErrorMessage--[take into account, that errorMessages span multiple transmissions]
  if(protocolVersion == 1 && dataAndDisplayType == 0){
    byte lineNumber = receivedData[2];    //which line of the error Message are we on
    byte length = receivedData[3];        //how long is the given line

    if(lineNumber == 0){                  //resets errorMessage for every new Message recieved
      for(int i = 0; i < maxErrorMessageLines; i++)
        errorMessage[i] = "";                
    }

    String temp = "";
    for(int i = 0; i < length; i++){      //sets the new errorMessage line
      temp += (char)receivedData[4 + i];
    }
    errorMessage[lineNumber] = String(temp);

    for(int i = 0; i < maxErrorMessageLines; i++){
      Serial.println(errorMessage[i]);
    }
  }
  //--Normal Transmission--
  else{
    //--Motors--
    for(int i = 0; i < maxMotors; i++){
      byte mask = B10000000 >> i;
      bool sign = receivedData[6] & mask;
      motorValues[i] = (-2 * sign + 1) * receivedData[2 + i];
    }
    //--Sensors--
    for(int i = 0; i < maxSensors; i++){
      byte mask = B10000000 >> i;
      bool sign = (receivedData[15] & mask) >> (7 - i);
      sensorValues[i][0] = (-2 * sign + 1) * receivedData[7 + i];
    }
    //--Gyroscopes--
    for(int i = 0; i < maxGyroscopes; i++){
      sensorValues[maxSensors + i][0] = (receivedData[16 + i * 4] << 24) +
                                        (receivedData[17 + i * 4] << 16) +
                                        (receivedData[18 + i * 4] << 8) + 
                                        receivedData[19 + i * 4];
    }
    //--ErrorTriangle--
    errorTriangle = (receivedData[6] & B00001000) >> 3;

    //--Printing--
    Serial.println((String)"protocol version: " + (byte)protocolVersion);
    Serial.println((String)"Data and Display Type: " + (byte)dataAndDisplayType);
    for(int i = 0; i < maxMotors; i++)
      Serial.println((String)"Motor " + (char)('A' + i) + ": " + motorValues[i]);
    for(int i = 0; i < maxSensors; i++)
      Serial.println((String)"Sensor " + (i + 1) + ": " + sensorValues[i][0]);
    for(int i = 0; i < maxGyroscopes; i++)
      Serial.println((String)"Gyroscope " + (char)('X' + i) + ": " + sensorValues[maxSensors + i][0]);
    Serial.println((String)"ErrorTriangle: " + errorTriangle);
    Serial.println("-------------------------");
  }
  displayUpdate();
}

void Excelsior_Ambassador::displayUpdate(){
  switch(protocolVersion){
    case 1: protocolVersion1(); break;
    default: drawBootScreen(); break;
  }
  //drawBattery(batteryStatus);
  drawErrorTriangle();
}

void Excelsior_Ambassador::drawBootScreen(){
  display.drawBitmap(26, 60, logo, 128,64, 0xFEA0, 0x0000);
  display.setFont(Terminal6x8);
  display.drawText(48,132,(String)"Version: " + version, COLOR_YELLOW);
  delay(1000);
  display.clear();
}

void Excelsior_Ambassador::drawErrorTriangle(){
  if(errorTriangle)
    display.drawBitmap(152,188,error,error_width,error_height,COLOR_BLACK,COLOR_RED);
  else
    display.fillRectangle(152,188,152+error_width, 188+error_height,COLOR_BLACK);
}

void Excelsior_Ambassador::drawBattery(int status){
  switch (status){
    case 4:  display.drawBitmap(7,189,battery_full ,battery_width,battery_height,COLOR_BLACK,COLOR_GREEN); break;
    case 3:  display.drawBitmap(7,189,battery_half ,battery_width,battery_height,COLOR_BLACK,0xff20);      break;
    case 2:  display.drawBitmap(7,189,battery_low  ,battery_width,battery_height,COLOR_BLACK,0xfd00);      break;
    case 1:  display.drawBitmap(7,189,battery_vlow ,battery_width,battery_height,COLOR_BLACK,COLOR_RED);   break;
    default: display.drawBitmap(7,189,battery_empty,battery_width,battery_height,COLOR_BLACK,COLOR_RED);   break;
  }
}

void Excelsior_Ambassador::protocolVersion1(){
  if(dataAndDisplayType == 0){    //ErrorMessage
    errorTriangle = true;
    
    display.setFont(Terminal11x16);
    display.drawText(60, 30, "ERROR!", COLOR_RED);
    _drawRoundedRect(57, 27, 124, 47, 1, COLOR_RED);

    for(int i = 0; i < maxErrorMessageLines; i++){
//      display.drawText(20, 55 + 20*i, "               ", COLOR_YELLOW);
      display.drawText(20, 55 + 20*i, errorMessage[i], COLOR_YELLOW);
    }

  } else if(dataAndDisplayType == 1){ //StandardDisplay
    display.setFont(Terminal6x8);
    for(int i = 0; i < 4; i++){
      display.drawText(19 + i * 44,7,char('A' + i), 0x067f);
      _drawRoundedRect(17 + i * 44,4,27 + i * 44,16,1,0x067f);
      display.drawText(7 + i * 44,19,"       ", 0x067f);             //CLEARs the row
      display.drawText(7 + i * 44,19,motorValues[i], 0x067f);
      _drawRoundedRect(5 + i * 44,16,37 + i * 44,28,1,0x067f);
    }

    display.drawText(0, 37,"---------Sensors---------", 0xafe0);

    //---Only Main Sensors---
    if(true){           
      display.setFont(Terminal11x16);
      int xOffset = 80;
      for(int i = 0; i < 8; i++){
        display.drawText(i < 4? 18 : 18 + xOffset, 55 + (i % 4) * 30, 8 - i, 0xafe0);
        _drawRoundedRect(i < 4? 16 : 16 + xOffset, 52 + (i % 4) * 30, i < 4? 30 : 30 + xOffset, 71 + (i % 4) * 30,1,0xafe0);
        display.drawText(i < 4? 34 : 34 + xOffset, 55 + (i % 4) * 30, "       ", 0xafe0);
        display.drawText(i < 4? 34 : 34 + xOffset, 55 + (i % 4) * 30, sensorValues[7 - i][0], 0xafe0);
      }

    //---Subsensors---(WIP)--Currently Disabled
    }else{
      display.setFont(Terminal6x8);
      for(int i = 0; i < 8; i++){
        display.drawText(18,52 + i * 15,i + 1, 0xafe0);
        _drawRoundedRect(16,49 + i * 15,26,61 + i * 15,1,0xafe0);
        for(int j = 0; j < 4; j++){
          display.drawText(30 + j * 32, 52 + i * 15,-255, 0xafe0);
        }
      }
    }

    display.setFont(Terminal6x8);
    display.drawText(0, 173,"--------Gyroscopes-------", 0xfca0);
    for(int i = 0; i < 3; i++){
      display.drawText(38 + i * 44,190,char('X' + i), 0xfca0);
      _drawRoundedRect(36 + i * 44,187,46 + i * 44,199,1,0xfca0);
      display.drawText(24 + i * 44,202,"          ", 0xfca0);
      display.drawText(24 + i * 44,202,sensorValues[maxSensors + i][0], 0xfca0);
      _drawRoundedRect(22 + i * 44,199,60 + i * 44,211,1,0xfca0);
    }
  }
}

void Excelsior_Ambassador::_drawRoundedRect(int xs,int ys,int xe,int ye,int weight,int color){
  for(int i = 0; i < weight; i++){
    display.drawRectangle(xs+i, ys+i, xe-i, ye-i, color);
  }
  display.drawPixel(xs,ys,COLOR_BLACK);
  display.drawPixel(xs,ye,COLOR_BLACK);
  display.drawPixel(xe,ys,COLOR_BLACK);
  display.drawPixel(xe,ye,COLOR_BLACK);

  display.drawPixel(xs + weight,ys + weight,color);
  display.drawPixel(xs + weight,ye - weight,color);
  display.drawPixel(xe - weight,ys + weight,color);
  display.drawPixel(xe - weight,ye - weight,color);
}

