#include "Excelsior_Ambassador.h"

Excelsior_Ambassador A;
void setup() {
  Wire.begin(4);                //Ambassador assigns itself the address 4
  Wire.onReceive(receiveData);  //function that gets called, if data is received
  Wire.onRequest(sendData);     //function that gets called, if data is requested by the Excelsior
}


void loop() {
  for (int i = 4; i >= 0; i--) {
    A.drawBattery(i);
    delay(2000);
  }
}

void receiveData(int n){  //number of recieved bytes (this function canno't easily be a memberfunction of the Ambassador
  int index = 0;
  while(Wire.available()){
    byte b = Wire.read();
    A.receivedData[index] = b;
    index++;
  }
  A.decodeData(index);
  Serial.println("Data transmitted successfully");

  /*
  while(1 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  int x = Wire.read();    // receive byte as an integer
  Serial.println(x);         // print the integer
  */
  
}

void sendData(){
  Wire.write("Hello, this is the Ambassador :)");
}
