#include <Arduino.h>
#include <Excelsior_Ambassador.h>

using namespace std;

//------SETUP------------------
Excelsior_Ambassador::Excelsior_Ambassador() : display(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS){  //Display Dimensions: 180,220
  display.begin();
  Serial.begin(9600);
  
  display.clear();
  display.setOrientation(2);
  display.setBackgroundColor(COLOR_BLACK);
  
  
/* 
  display.setFont(Terminal11x16);
  for(int i = 0; i < 100; i++){
    display.drawText(10,10+26*i, "A -255",COLOR_YELLOW);
    drawRoundedRect(6,6+26*i,24,27+26*i,2,COLOR_BLUE);
  
    display.drawText(80,10+26*i, "B -255",COLOR_YELLOW);
    drawRoundedRect(76,6+26*i,93,27+26*i,2,COLOR_BLUE);
  }*/

  display.setFont(Terminal6x8);
  display.drawText(10,10,"-255 -255 -255 -255");
  delay(1000);
  display.setFont(Terminal6x8);
  display.drawText(10,90,"EXCELSIOR WELCOMES YOU", COLOR_YELLOW);
  display.drawText(10,100,"|1| 255, 255, 255, 255", COLOR_YELLOW);
  display.drawBitmap(26, 130, logo, 128,64, 0xFEA0, 0x0000);
  delay(5000);
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
