#define FirmwareVersion "000170"
#define HardwareVersion "MCU109 for 4XX Series."

#define DEBUG 0
#if DEBUG
  #define Print(x) Serial.print(x)
  #define Println(x) Serial.println(x)
  #define SerialBegin(x) Serial.begin(x)
#else
  #define Print(x)
  #define Println(x)
  #define SerialBegin(x)
#endif

#define NUM_TUBES 6
#define SCROLL_TIME 200
#define VOLUME 8

#include <SPI.h>
#include <Wire.h>
#include <ClickButton.h>
#include <OneWire.h>
#include <RTClib.h>
#include <TimerFreeTone.h>
#include <RGB_LED.h>

const byte LE = 10;       //pin Latch Enabled data accepted while HI level
const byte HI_Z = 8;      //pin Z state in registers outputs (while LOW level)
const byte RED_PIN = 9;   //MCU WDM output for red LEDs 9-g
const byte GREEN_PIN = 6; //MCU WDM output for green LEDs 6-b
const byte BLUE_PIN = 3;  //MCU WDM output for blue LEDs 3-r
const byte MODE = A0;
const byte UP = A2;
const byte DOWN = A1;
const byte BUZZER = 2;
const byte UPPER_DOTS = 12; //HIGH value light a dots
const byte LOWER_DOTS = 8;  //HIGH value light a dots

RGB_LED LED(RED_PIN, GREEN_PIN, BLUE_PIN);


RTC_DS3231 RTC;

//RTC_Millis RTC;


int hours, minutes, seconds, day, month, year;
bool playing = false;
bool RTC_present;
bool UD, LD;    // DOTS control;
byte blinkGroup, dotPattern;
char strToDisplay[7] = "000000";
uint32_t updateTime, updateNixieTime;

//buttons pins declarations
ClickButton setButton(MODE, LOW, CLICKBTN_PULLUP);
ClickButton upButton(UP, LOW, CLICKBTN_PULLUP);
ClickButton downButton(DOWN, LOW, CLICKBTN_PULLUP);

// list of all editable settings 
bool ledOn = true;
bool ledLock = false;
byte RedLight;
byte GreenLight;
byte BlueLight;
int h_time = 0;
int h_date = 0;
int alarmOn = 0;
int alarmHour = 0;
int alarmMin = 0;
int alarmSec = 0;
int tempType = 0;
int tempOffset = 0;
int RGBFadeSpeed = 5;
int RGBPower = 20;

typedef struct {
  uint8_t preset = 9;
  int value = 0;
  bool ready = false;
  bool run = false;
  uint32_t startTime;
} RollDigit;

RollDigit rollDigit[NUM_TUBES];
bool editSettings, lastEdit = false;

#include "settings.h"
#include "songsTimerFree.h"
#include "driveNixie.h"
#include "nixieMenu.h"

// Define an array of pointers to functions  that will update nixie's text. 
typedef void (*StringFunctions[])();
StringFunctions setString = { setStringTime, setStringDate, setStringAlarm, setStringTemperature, setStringRGB };

// Enum selected working functions (just for using a readable instead a number)
enum showString {TIME, DATE, ALARM, TEMP, RGB}; // TIME == 0, DATE == 1 etc etc

// Set default "setString function"
uint8_t currentSetString = TIME;


/*******************************************************************************************************
  Init Programm
*******************************************************************************************************/
void setup(){
  // SPI setup
  SPI.begin(); //
  SPI.setDataMode (SPI_MODE2); // Mode 2 SPI // судя по данным осциллографа нужно использовать этот режим
  SPI.setClockDivider(SPI_CLOCK_DIV8); // SCK = 16MHz/128= 125kHz

  // Pins definition
  pinMode(LE, OUTPUT);
  digitalWrite(LE, LOW);
  pinMode(HI_Z, OUTPUT);
  digitalWrite(HI_Z, LOW);
  pinMode(MODE,  INPUT_PULLUP);
  pinMode(UP,  INPUT_PULLUP);
  pinMode(DOWN,  INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  SerialBegin(115200);
  
  // Only with RTC_millis
  //RTC.begin(DateTime(F(__DATE__), F(__TIME__))) ;
  
  if (! RTC.begin() ) {
    Println(F("Couldn't find RTC"));
    while (1);
  }
   
  RTC_present=true;
  getRTCTime();
  
  //buttons objects inits
  setButton.debounceTime   = 20;   // Debounce timer in ms
  setButton.multiclickTime = 5;  // Time limit for multi clicks
  setButton.longClickTime  = 1500; // time until "held-down clicks" register
  upButton.debounceTime   = 20;   // Debounce timer in ms
  upButton.multiclickTime = 10;  // Time limit for multi clicks
  upButton.longClickTime  = 2000; // time until "held-down clicks" register
  downButton.debounceTime   = 20;   // Debounce timer in ms
  downButton.multiclickTime = 5;  // Time limit for multi clicks
  downButton.longClickTime  = 2000; // time until "held-down clicks" register
  
  loadSettings();
  TimerFreeTone(BUZZER, 1000, 50, VOLUME);
  delay(300);
  TimerFreeTone(BUZZER, 1000, 50, VOLUME);

  LED.setFunction(Fade);
  LED.setSpeed(RGBFadeSpeed*1000);
  LED.setPercentFade(100.0);
  LED.setPower((float)RGBPower/100.0);

  blinkGroup = noBlink;
  dotPattern = allDots;
  rollDigit[0].preset = 2;
  rollDigit[1].preset = 3;
  rollDigit[2].preset = 5;
  rollDigit[3].preset = 9;
  rollDigit[4].preset = 5;
  rollDigit[5].preset = 9;

  doTest();
  // Prepare song in order to be played
  // parseMusic();
}


/***************************************************************************************************************
  MAIN Programm
***************************************************************************************************************/
void loop() {

  // Update actual date & time
  if ( millis() - updateTime > 1000 ) {
    updateTime = millis();
    if(!editSettings )
      getRTCTime();
    
    // Execute actual "setString" function selected 
    // (the one defined in "StringFunctions" with index == currentSetString)
    setString[currentSetString]();
    //Print("Display: ");    Println(strToDisplay);
  }

  // Update nixie display at 500Hz frequency
  if(micros() - updateNixieTime > 500 ){
    updateNixieTime = micros();
    updateNixie(blinkGroup, dotPattern, pActive.dotBlink);
  }

  // Drive RGB led to next coulour
  setRGBled();  
  
  // Update buttons state
  setButton.Update();
  upButton.Update();
  downButton.Update();

  // UP button short click
  if(upButton.clicks == 1){
    playing = false;
    TimerFreeTone(BUZZER, 1000, 20, VOLUME);
    if(editSettings)
      updateValue(1);
    else {
      ledLock = false;
      EEPROM.put(EE_LED_LOCK, ledLock);      
      EEPROM.get(EE_RED, RedLight);
      EEPROM.get(EE_GREEN, GreenLight);
      EEPROM.get(EE_BLUE, BlueLight);
      LED.set(RedLight, GreenLight, BlueLight);
      Print(F("RGB led unlocked "));
      Print(RedLight); Print (", "); Print(GreenLight); Print (", "); Println(BlueLight);
    }    
  }

  // DOWN button short click
  if(downButton.clicks == 1){
    playing = false;
    TimerFreeTone(BUZZER, 1000, 20, VOLUME);
    if(editSettings)
      updateValue(-1);
    else {
      ledLock = true;
      RedLight = LED.getCurrentRValue();
      GreenLight = LED.getCurrentGValue();
      BlueLight = LED.getCurrentBValue();          
      EEPROM.put(EE_LED_LOCK, ledLock);
      EEPROM.put(EE_RED, RedLight);
      EEPROM.put(EE_GREEN, GreenLight);
      EEPROM.put(EE_BLUE, BlueLight);      
      Print(F("RGB led locked: "));
      Print(RedLight); Print (", "); Print(GreenLight); Print (", "); Println(BlueLight);
      }
  }

  // UP button long click
  if(upButton.clicks == -1){
    TimerFreeTone(BUZZER, 1000, 100, VOLUME);
    ledOn = true;
    EEPROM.write(EE_LED_ON, ledOn);
    Println(F("RGB led on"));
  }

  // DOWN button long click
  if(downButton.clicks == -1){
    TimerFreeTone(BUZZER, 1000, 100, VOLUME);
    ledOn = false;
    EEPROM.write(EE_LED_ON, ledOn);
    Println(F("RGB led off"));    
  }

  // Short click -> move to next page
  if (setButton.clicks == 1) {
    playing = false;
    TimerFreeTone(BUZZER, 1000, 20, VOLUME);  
    currentPage = pActive.nextPage;    
  }

  // long click -> move to child page (if exist)
  if (setButton.clicks == -1) {
    TimerFreeTone(BUZZER, 1000, 100, VOLUME);
    if (pActive.childPage != P_NULL)
      currentPage = pActive.childPage;
  }

  // Page changed, update display
  if(currentPage != oldPage ){
    Print(F("Actual page: "));  Println(currentPage);
    oldPage = currentPage;
    changePage();
  }

  // perform Anthi Cathode Poisoning (at second 35)
  if(!playing) {  // If not music sound   
    if(seconds == 35 && currentPage == pTime.idPage){      
      for(int i=0; i<NUM_TUBES; i++)
        rollToZero(i, 9, 10);
      delay(150);
      doACP();
      currentPage = pDate.idPage;
      changePage();      
    }

    // Go back to time display
    if(seconds == 43 && currentPage == pDate.idPage) 
      scrollTo(pTime.idPage);
  }
  

  // Rising edge entering in setup mode
  if(editSettings &! lastEdit)
    lastEdit = true;
  
  // falling edge entering in setup mode
  if(lastEdit &! editSettings){
    lastEdit = false;
    // Save settings
    RTC.adjust(DateTime(year, month, day, hours, minutes, seconds));
    saveSettings();
  }

  // Alarm triggered, predispose music in order to be played
  if(alarmOn && hours == alarmHour && minutes == alarmMin && seconds == alarmSec){
    Println(F("Alarm ON"));
    parseMusic();
  }
  // Play music now
  if(playing)
    playMusic();

  // fast countdown effect every transition from 9 to 0
  checkRollDigit();
  
}




// Visual effect on nixie digits
void checkRollDigit(){
  for(int i=0; i< NUM_TUBES; i++){
    int displayedNum = strToDisplay[i] - '0';
    // Reset the actual digit
    if (displayedNum == (rollDigit[i].preset-1)  && !rollDigit[i].run ){
      rollDigit[i].ready = false;
    }
    
    bool setReady;
    switch (i){
      case 5:
        setReady = (seconds % 10 == 9);
        break;
      case 4:
        setReady = (seconds == 59);
        break;
      case 3:
        setReady = (seconds == 59) & (minutes % 10 == 9);
        break;
      case 2:
        setReady = (seconds == 59) & (minutes == 59);
        break;
      case 1:
        setReady = (hours == 23) & (seconds == 59) & (minutes == 59);
        break;
      case 0:
        setReady = (hours == 23) & (seconds == 59) & (minutes == 59);
        break;

    }
    
    
    
    // Set ready to roll when preset == actual value && last digit is ready to roll (last second before)
    if (displayedNum == rollDigit[i].preset && !rollDigit[i].ready && setReady ){
      rollDigit[i].ready = true;
      rollDigit[i].run = false;
      rollDigit[i].value = rollDigit[i].preset;
      rollDigit[i].startTime = millis();
      //Print("i: "); Print(i);  Println(" millis(): ");
      //Print(rollDigit[i].startTime ); Println(" -> ready");
    }
    // it's time to start roll back
    if(rollDigit[i].ready && !rollDigit[i].run){     
      if( millis() > rollDigit[i].startTime + 700) {
        rollDigit[i].run = true;
      }
    }
    // Execute the roll back function (non blocking)
    if(rollDigit[i].run){
      rollToZeroNB(i, 50 );
    }   
  
  }
}

// NON blocking function for executing roll to zero (normal operation)
void rollToZeroNB(uint8_t digit, uint16_t rollDelay){
  static uint32_t rollTime = millis();
  if (rollDigit[digit].value >= 0){
    updateNixie(blinkGroup, dotPattern, pActive.dotBlink);
    delayMicroseconds(500);
    if(millis() - rollTime > rollDelay){
      rollTime = millis();                                     
      strToDisplay[digit] = rollDigit[digit].value + '0';     
      rollDigit[digit].value--;    // Next    
      //Print("go back "); Println(strToDisplay);     
    } 
  } 
  else{
    rollDigit[digit].run = false;
  }
}

// blocking function for executing roll to zero (during APC)
void rollToZero(uint8_t digit, int counter, uint16_t rollDelay){
  static uint32_t rollTime = millis();
  while (counter >= 0){
    updateNixie(blinkGroup, dotPattern, pActive.dotBlink);
    delayMicroseconds(500);
    if(millis() - rollTime > rollDelay){
      rollTime = millis();                           
      strToDisplay[digit] = counter + '0'; 
      counter--;    // Next      
      //Print("Back to zero "); Println(strToDisplay);      
    }    
  }  
}

void doACP(){
  int digit=0, number=0;
  uint32_t digitTime = millis();
  uint32_t numTime = millis();

  while (digit != 2){
    if(millis() - digitTime > 20){
      digitTime = millis();
      if(millis()- numTime > 40){
        numTime = millis();
        number++;        
        strToDisplay[0] = (number %10 )+ '0';
        strToDisplay[1] = ((number + 1) %10 )+ '0';
        strToDisplay[2] = ((number + 2) %10 )+ '0';
        strToDisplay[3] = ((number + 3) %10 )+ '0';
        strToDisplay[4] = ((number + 4) %10 )+ '0';
        strToDisplay[5] = ((number + 5) %10 )+ '0';      
        if(number == 10) {
          digit++;
          number = 0;
        }
      }
    }
    delayMicroseconds(500);
    updateNixie(blinkGroup, dotPattern, pActive.dotBlink);
  }
  sprintf(strToDisplay, "      ");
  updateNixie(blinkGroup, dotPattern, pActive.dotBlink);
  delay(100);
}

// Scroll text to the left
void scrollTo( uint8_t page){
  static uint32_t scrollTime = millis();
  int pos = 0;
  while (pos <= NUM_TUBES+1){
    // Force update showed text    
    delayMicroseconds(500);
    updateNixie(blinkGroup, noDots, pActive.dotBlink);
    if(millis() - scrollTime > SCROLL_TIME){
      scrollTime = millis();                   
      //Print(pos); Print(" - "); Println(strToDisplay);   
      memmove(strToDisplay, strToDisplay + 1, NUM_TUBES);  
      pos++;    // Next
    }
  }
  currentPage = page;
  changePage();  
}




// Menu page selection and settings editing
void updateValue(int val){
  // load min/max from page structure
  int min = pActive.minVal;
  int max = pActive.maxVal;
  int *ptrData;
  switch(currentPage){    
      // edit on time pages
      case PE_HTIME :
        ptrData = &h_time;
        break;
      case PE_HOUR :
        ptrData = &hours; 
        break;
      case PE_MIN :        
        ptrData = &minutes; 
        break;
      case PE_SEC :        
        ptrData = &seconds; 
        break;
      // edit on date pages
      case PE_HDATE :
        ptrData = &h_date; 
        break;
      case PE_DAY :
        ptrData = &day; 
        break;
      case PE_MON :
        ptrData = &month;
        break;
      case PE_YEAR :
        ptrData = &year ;
        break;
      // edit on alarm pages
      case PE_A_ENABLE :
        ptrData = &alarmOn; 
        break;
      case PE_A_HOUR :
        ptrData = &alarmHour;
        break;
      case PE_A_MIN :
        ptrData = &alarmMin; 
        break;
      case PE_A_SEC :
        ptrData = &alarmSec ;
        break;
      // edit on temp pages
      case PE_CELSIUS :
        ptrData = &tempType; 
        break;
      case PE_T_OFFSET :
        ptrData = &tempOffset; 
        break;
      // edit on temp pages
      case PE_RGB_POWER :
        ptrData = &RGBPower; 
        break;
      case PE_RGB_SPEED :
        ptrData = &RGBFadeSpeed; 
        break;

    }
  // Call update() function passing selected variable as reference
  update(ptrData, val, min, max);
  // Force the update of showed text
  setString[currentSetString]();
  updateNixie(blinkGroup, dotPattern, pActive.dotBlink);    
}

void update(int *value, int x,  int min, int max){
  int res;
  res = *value ;
  res += x;
  if(res > max)
    res = min;
  if(res < min)
    res = max;
  *value = res;
}

void changePage(){
  switch(currentPage){
    // main pages
    case P_TIME :
      pActive = pTime;
      currentSetString = TIME;
      break;
    case P_DATE :
      pActive = pDate;
      currentSetString = DATE;
      break;
    case P_ALARM :
      pActive = pAlarm;
      currentSetString = ALARM;
      break;
    case P_TEMP :
      pActive = pTemp;
      currentSetString = TEMP;
      break;
    case P_RGB :
      pActive = pRGB;
      currentSetString = RGB;
      break;
    // child time pages
    case PE_HTIME :
      pActive = pEditHTime;
      currentSetString = TIME;
      break;
    case PE_HOUR :
      pActive = pEditHour;
      break;
    case PE_MIN :
      pActive = pEditMin;
      break;
    case PE_SEC :
      pActive = PEditSec;
      break;
    // child date pages
    case PE_HDATE :
      pActive = pEditHDate;
      currentSetString = DATE;
      break;
    case PE_DAY :
      pActive = pEditDay;
      break;
    case PE_MON :
      pActive = pEditMonth;
      break;
    case PE_YEAR :
      pActive = pEditYear;
      break;
    // chila alarm pages
    case PE_A_ENABLE :
      pActive = pEditAlarmOn;
      currentSetString = ALARM;
      break;
    case PE_A_HOUR :
      pActive = pEditAlarmHour;
      break;
    case PE_A_MIN :
      pActive = pEditAlarmMin;
      break;
    case PE_A_SEC :
      pActive = pEditAlarmSec;
      break;
    // child temp pages
    case PE_CELSIUS :
      pActive = pEditCelsius;
      currentSetString = TEMP;
      break;
    case PE_T_OFFSET :
      pActive = pEditTOffset;
      break;
    // child temp pages
    case PE_RGB_POWER :
      pActive = pEditRGBPower;
      currentSetString = RGB;
      break;
    case PE_RGB_SPEED :
      pActive = pEditRGBSpeed;
  }

  blinkGroup = pActive.blinkGroup;
  dotPattern = pActive.dotPattern;
  // main pages 10, 20 , 30 40, 50
  if(currentPage % 10 == 0) 
    editSettings = false;
  else {
    editSettings = true;
  
  setString[currentSetString]();
  updateNixie(blinkGroup, dotPattern, pActive.dotBlink);
  }
}




// Get actual date & time from RTC
void getRTCTime(){
  DateTime now = RTC.now();  
  seconds = now.second();
  minutes = now.minute();
  hours = now.hour();  
  day = now.day();
  month = now.month();
  year = now.year();
}

// Set RGB backligth according to the selected options
void setRGBled(){
  if (!ledOn)  {
    LED.set(0, 0, 0);
    return;
  }
  if (ledLock) {
    LED.set(RedLight, GreenLight, BlueLight);
    return;
  }
  LED.run();
}

// Perform initial test
void doTest() {
  Print(F("Firmware version: "));
  Println(F(FirmwareVersion));
  Println(F(HardwareVersion));
  Println(F("Start Test"));

  LED.set(255, 0, 0);
  delay(1000);
  LED.set(0, 255, 0);
  delay(1000);
  LED.set(0, 0, 255);
  delay(1000); 

  const char *testStringArray[] = {"000000","111111","222222","333333","444444","555555","666666","777777","888888","999999", FirmwareVersion};

  Print(F("Temp = "));
  Println(RTC.getTemperature());

  uint16_t dlay=500;

  int strIndex=-1;
  unsigned long startOfTest=millis()+1000; //disable delaying in first iteration
  bool digitsLock=false;
  while (true)
  {    
    if (digitalRead(DOWN)==0) digitsLock=true;
    if (digitalRead(UP)==0) digitsLock=false;

    if ((millis()-startOfTest)>dlay) 
     {
       startOfTest=millis();
       if (!digitsLock) strIndex=strIndex+1;
       if (strIndex==10) dlay=3000;
       if (strIndex==11) break;
       memcpy(strToDisplay, testStringArray[strIndex], 6);
       Println(strToDisplay);
      }
    updateNixie(0, 0, false);
  }
   
  Println(F("Stop Test"));
}















