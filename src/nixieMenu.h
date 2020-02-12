#ifndef __NIXIE_MENU_H
#define __NIXIE_MENU_H

typedef struct{
    uint8_t idPage;
    uint8_t nextPage;  
    uint8_t childPage;      
    int minVal ;
    int maxVal ;
    uint8_t blinkGroup ;
    uint8_t dotPattern ;
} Pages;


#define H24 0
#define H12 1
#define EU_DATE 0
#define US_DATE 1
#define CELSIUS 0
#define FAHRENHEIT 1

#define P_NULL   99
#define P_TIME   10    
#define P_DATE   20    
#define P_ALARM  30    
#define P_TEMP   40
#define P_RGB    50

#define PE_HTIME 11
#define PE_HOUR  12
#define PE_MIN   13
#define PE_SEC   14

Pages pEditHTime = {PE_HTIME, PE_HOUR, P_NULL, H24, H12, blinkAll, allDots};
Pages pEditHour =  {PE_HOUR, PE_MIN, P_NULL, 0, 24, blinkAN1, allDots };
Pages pEditMin =   {PE_MIN, PE_SEC, P_NULL, 0, 59, blinkAN2, allDots};
Pages PEditSec =   {PE_SEC, P_TIME, P_NULL, 0, 59, blinkAN3, allDots};

#define PE_HDATE 21
#define PE_DAY   22
#define PE_MON   23
#define PE_YEAR  24

Pages pEditHDate = {PE_HTIME, PE_DAY, P_NULL, EU_DATE, US_DATE, blinkAll, lowerDots};
Pages pEditDay =   {PE_DAY, PE_MON, P_NULL, 0, 31, blinkAN1, lowerDots};
Pages pEditMonth = {PE_MON, PE_YEAR, P_NULL, 0, 12, blinkAN2, lowerDots};
Pages pEditYear =  {PE_YEAR, P_DATE, P_NULL, 0, 99, blinkAN3, lowerDots};

#define PE_A_ENABLE 31
#define PE_A_HOUR 32
#define PE_A_MIN  33
#define PE_A_SEC  34
Pages pEditAlarmOn =   {PE_A_ENABLE, PE_A_HOUR, P_NULL, 0, 1, blinkAll, upperDots};
Pages pEditAlarmHour = {PE_A_HOUR, PE_A_MIN, P_NULL, 0, 24, blinkAN1, upperDots};
Pages pEditAlarmMin =  {PE_A_MIN, PE_A_SEC, P_NULL, 0, 59, blinkAN2, upperDots};
Pages pEditAlarmSec =  {PE_A_SEC, P_ALARM, P_NULL, 0, 59, blinkAN3, upperDots};


#define PE_CELSIUS   41
#define PE_T_OFFSET  42
Pages pEditCelsius = {PE_CELSIUS, PE_T_OFFSET, P_NULL, CELSIUS, FAHRENHEIT, blinkAll, upperDots};
Pages pEditTOffset = {PE_T_OFFSET, P_TEMP, P_NULL, -10, 10, blinkAll, lowerDots};

#define PE_RGB_POWER 51
#define PE_RGB_SPEED  52
Pages pEditRGBPower = {PE_RGB_POWER, PE_RGB_SPEED, P_NULL, 0, 60, blinkAll, upperDots};
Pages pEditRGBSpeed= {PE_RGB_SPEED, P_RGB, P_NULL, 0, 100, blinkAll, lowerDots};

Pages pTime =  {P_TIME, P_DATE, PE_HTIME, -1, -1, noBlink, allDots};
Pages pDate =  {P_DATE, P_ALARM, PE_HDATE, -1, -1, noBlink, lowerDots};
Pages pAlarm = {P_ALARM, P_TEMP, PE_A_ENABLE, -1, -1, noBlink, upperDots};
Pages pTemp =  {P_TEMP, P_RGB, PE_CELSIUS, -1, -1, noBlink, noDots};
Pages pRGB =  {P_RGB, P_TIME, PE_RGB_POWER,  -1, -1, noBlink, noDots};

Pages pActive = pTime;
uint8_t currentPage = pActive.idPage;
uint8_t oldPage = 99;





// Displayed text on nixie will be setted according to the actual page
void setStringTime() {
  if(pActive.idPage == PE_HTIME){
    if(h_time)
      sprintf(strToDisplay, "    12" );
    else
      sprintf(strToDisplay, "    24" );
    return;
  }

  if (h_time) 
    sprintf(strToDisplay, "%02u%02u%02u", hours % 12, minutes, seconds);    
  else 
    sprintf(strToDisplay, "%02u%02u%02u", hours , minutes, seconds);
     
}

void setStringDate() {
  if(pActive.idPage == PE_HTIME){
    if(h_date)
      sprintf(strToDisplay, "123199" );
    else
      sprintf(strToDisplay, "311299" );
    return;
  }

  if (h_date)     
    sprintf(strToDisplay, "%02u%02u%02u", month, day, year % 1000);
  else 
    sprintf(strToDisplay, "%02u%02u%02u", day , month, year % 1000);    
}

void setStringAlarm() {
  if(pActive.idPage == PE_A_ENABLE){
    sprintf(strToDisplay, "%01d     ", alarmOn );
    return;
  }
  sprintf(strToDisplay, "%02u%02u%02u", alarmHour , alarmMin, alarmSec);
}

void setStringRGB() {
  if(pActive.idPage == PE_RGB_POWER){
    //int RGBPower = LED.getPower() * 100;
    sprintf(strToDisplay, "   %03d", RGBPower );
    return;
  }
  if(pActive.idPage == PE_RGB_SPEED){
    //int FAdeSpeed = LED.getSpeed() / 1000;
    sprintf(strToDisplay, "    %02d", RGBFadeSpeed );
    return;
  }
  int curRGBPower = LED.getPower() * 100;
  sprintf(strToDisplay, "   %03d", curRGBPower );
}

void setStringTemperature(){
  if(pActive.idPage == PE_T_OFFSET){
    sprintf(strToDisplay, "    %02d", tempOffset );
    return;
  }

  if(pActive.idPage == PE_CELSIUS){
    sprintf(strToDisplay, "%01d     ", tempType );
    return;
  }

  float fdegree = RTC.getTemperature();
  if(tempType){
    fdegree = (fdegree * 1.8 + 32.0) ;
    sprintf(strToDisplay, " %03d%02d", (int)fdegree, (int)(fdegree * 100)%100 );
  }
  else
    sprintf(strToDisplay, "  %02d%02d", (int)fdegree, (int)(fdegree * 100)%100 );
}


#endif
