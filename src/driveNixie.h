#ifndef __DOINDICATION_H
#define __DOINDICATION_H

uint16_t DigitArray[21]= {
/* 0 */  0b0000000000000001,   // Digit 0
/* 1 */  0b0000000000000010,   // Digit 1
/* 2 */  0b0000000000000100,   // Digit 2
/* 3 */  0b0000000000001000,   // Digit 3
/* 4 */  0b0000000000010000,   // Digit 4
/* 5 */  0b0000000000100000,   // Digit 5
/* 6 */  0b0000000001000000,   // Digit 6
/* 7 */  0b0000000010000000,   // Digit 7
/* 8 */  0b0000000100000000,   // Digit 8
/* 9 */  0b0000001000000000,   // Digit 9
/* 10 */ 0b0000000000000000    // Blank
};

#define BLANK 20

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


uint16_t getOutsForDigit(uint8_t digit){
  uint16_t res = DigitArray[BLANK];
  if( (digit >= 0 && digit <= 21) && digit != 10 ){
    res = DigitArray[digit];
    return res;
  }   
  return res;
}

//bit mask for blinkin digits (1 - blink, 0 - constant light)
int updateBlink(byte pos, byte blinkMask, byte dotPattern) {
  static unsigned long blinkTime;
  static bool blinkState=false;
  
  int lowBit = (blinkMask >> pos) & 0b00000001;
  word mask = TubeOn;
  static int tmp;
  if (millis() - blinkTime > 200) {
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
  static bool blinkSwap;
  byte dots;
  uint32_t AnodesGroupMask;
  uint32_t var32=0x0000;
  uint32_t tmpVar=0x0000; 

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
  char tens = strToDisplay[curTube] ;
  char unit = strToDisplay[curTube +1] ;
  
  if(isDigit(tens))
    var32|=DigitArray[tens - '0'];
  else
    var32|=DigitArray[BLANK]; 

  if(isDigit(unit))
    tmpVar=DigitArray[unit - '0'];
  else
    tmpVar=DigitArray[BLANK]; 
  
    


  if(dotBlink){
    if(millis() - dotBlinkTime > 1000){
      dotBlinkTime = millis();
      blinkSwap = !blinkSwap;
    }
  }
  else blinkSwap = true;
  
  blinkSwap ? dots = dotPattern : dots = noDots;    
  var32 &= updateBlink(curTube, groupBlink, dots);  
  tmpVar &= updateBlink(curTube+1, groupBlink, dots); 

  var32 |= tmpVar<<10;  
  LD ? var32|=LowerDotsMask : var32&=~LowerDotsMask;
  UD ? var32|=UpperDotsMask : var32&=~UpperDotsMask; 

  digitalWrite(LE, LOW);   // allow data input (Transparent mode)
  SPI.transfer(var32>>24); // [x][x][x][x][x][x][L1][L0]                - L0, L1 - dots
  SPI.transfer(var32>>16); // [x][A2][A1][A0][RC9][RC8][RC7][RC6]       - A0-A2 - anodes
  SPI.transfer(var32>>8);  // [RC5][RC4][RC3][RC2][RC1][RC0][LC9][LC8]  - RC9-RC0 - Right tubes cathodes
  SPI.transfer(var32);     // [LC7][LC6][LC5][LC4][LC3][LC2][LC1][LC0]  - LC9-LC0 - Left tubes cathodes
  digitalWrite(LE, HIGH);  // latching data 
  AnodesGroup = ((AnodesGroup + 1) % 3) + 1;  
}

#endif