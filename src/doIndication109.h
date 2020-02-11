#ifndef __DOINDICATION_H
#define __DOINDICATION_H

#include <Arduino.h>
uint16_t SymbolArray[11]={1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 0};

#define AnodeGroup1ON 0x100000
#define AnodeGroup2ON 0x200000
#define AnodeGroup3ON 0x400000

#define TubeOn 0xFFFF
#define TubeOff 0xFC00

#define UpperDotsMask 0x2000000
#define LowerDotsMask 0x1000000

#define noBlink  0b00000000
#define blinkAN1 0b00000011
#define blinkAN2 0b00001100
#define blinkAN3 0b00110000
#define blinkAll 0b00111111

#define upperDots 0b10000000
#define lowerDots 0b01000000
#define allDots   0b11000000
#define noDots    0b00000000


//bit mask for blinkin digits (1 - blink, 0 - constant light)
int updateBlink(byte pos, byte blinkMask, byte dotPattern) {
  static unsigned long blinkTime;
  static bool blinkState=false;
  
  int lowBit = (blinkMask >> pos) & 0b00000001;
  word mask = TubeOn;
  static int tmp;
  if (millis() - blinkTime > 300) {
    blinkTime = millis();
    blinkState = !blinkState;
    blinkState ? tmp=0 : tmp = blinkMask;
  }
  
  LD = ((dotPattern&~tmp)>>6) & 1 ;
  UD = ((dotPattern&~tmp)>>7) & 1 ;
      
  if (blinkState && lowBit) 
    mask = TubeOff;
  return mask;
}


void updateNixie(byte groupBlink, byte dotPattern, bool dotBlink = true){
  static uint32_t dotBlinkTime = updateTime;
  static byte AnodesGroup = 1;
  static bool blinkDot;
  byte dots;
  uint32_t AnodesGroupMask;
  uint32_t var32=0x0000;
  uint32_t tmpVar; 

  switch (AnodesGroup) {
    case 1:
      AnodesGroupMask=AnodeGroup1ON; 
      break;
    case 2:
      AnodesGroupMask=AnodeGroup2ON; 
      break;
    case 3:
      AnodesGroupMask=AnodeGroup3ON; 
      break;
  }

  var32 |= AnodesGroupMask;

  byte curTube=AnodesGroup*2-2;
  char tens = strToDisplay[curTube];
  char unit = strToDisplay[curTube +1];
  if(isdigit(tens))
    var32|=SymbolArray[tens - '0'];
  else
    var32|=SymbolArray[10];    // Blank ' '

  if(isdigit(unit))
    tmpVar=SymbolArray[unit - '0'];
  else
    tmpVar=SymbolArray[10];   // Blank ' ' 

  if(millis() - dotBlinkTime > 1000){
    dotBlinkTime = millis();
    blinkDot = !blinkDot;
  }

  blinkDot ? dots = dotPattern : dots = noDots;
    
  var32 &= updateBlink(curTube, groupBlink, dots);  
  tmpVar &= updateBlink(curTube+1, groupBlink, dots); 

  var32 |= tmpVar<<10;
  
  LD ? var32|=LowerDotsMask : var32&=~LowerDotsMask;
  UD ? var32|=UpperDotsMask : var32&=~UpperDotsMask; 

  digitalWrite(LE, LOW);    // allow data input (Transparent mode)

  SPI.transfer(var32>>24); //[x][x][x][x][x][x][L1][L0]                - L0, L1 - dots
  SPI.transfer(var32>>16); //[x][A2][A1][A0][RC9][RC8][RC7][RC6]       - A0-A2 - anodes
  SPI.transfer(var32>>8);  //[RC5][RC4][RC3][RC2][RC1][RC0][LC9][LC8]  - RC9-RC0 - Right tubes cathodes
  SPI.transfer(var32);     //[LC7][LC6][LC5][LC4][LC3][LC2][LC1][LC0]  - LC9-LC0 - Left tubes cathodes

  //Serial.println(var32);
  digitalWrite(LE, HIGH);     // latching data 
  AnodesGroup = ((AnodesGroup + 1) % 3) + 1;
  
}

#endif