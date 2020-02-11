/**********************************************************************************************
 * Arduino LED RGB Library - Version 1.3
 * by William Bailes <williambailes@gmail.com> http://tinkersprojects.com/
 *
 * This Library is licensed under a GPLv3 License
 **********************************************************************************************/

#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "RGB_LED.h"

/******************* SETUP *******************/

RGB_LED::RGB_LED(uint8_t pinR, uint8_t pinG, uint8_t pinB)
{
    R_Pin = pinR;
    G_Pin = pinG;
    B_Pin = pinB;
    pinMode(pinR, OUTPUT);
    pinMode(pinG, OUTPUT);
    pinMode(pinB, OUTPUT);

    FadeFunctionCallBack = quadratic;
}

RGB_LED::RGB_LED(bool inverted, uint8_t pinR, uint8_t pinG, uint8_t pinB)
{
    R_Pin = pinR;
    G_Pin = pinG;
    B_Pin = pinB;
    pinMode(pinR, OUTPUT);
    pinMode(pinG, OUTPUT);
    pinMode(pinB, OUTPUT);

    invertedPins = inverted;

    FadeFunctionCallBack = quadratic;
}

void RGB_LED::setCallback(float (*callback)(float x)) 
{
    FadeFunctionCallBack = callback;
}

/******************* SET *******************/

void RGB_LED::fadeTo(uint8_t Rvalue,uint8_t Gvalue,uint8_t Bvalue,unsigned long speedValue)
{
    Speed = speedValue;

    R_Last_value = R_Current_value;
    G_Last_value = G_Current_value;
    B_Last_value = B_Current_value;

    R_Future_value = Rvalue;
    G_Future_value = Gvalue;
    B_Future_value = Bvalue;

    function=Fade;
    starting_time = millis();
}

void RGB_LED::set(uint8_t Rvalue, uint8_t Gvalue, uint8_t Bvalue)
{
    R_Last_value = Rvalue;
    G_Last_value = Gvalue;
    B_Last_value = Bvalue;

    R_Current_value = Rvalue;
    G_Current_value = Gvalue;
    B_Current_value = Bvalue;

    R_Future_value = Rvalue;
    G_Future_value = Gvalue;
    B_Future_value = Bvalue;

    function=Fade;

    writeOutput();
}

void RGB_LED::setColour(byte colour)
{
    switch (colour) {
        case Black:
            RGB_LED::set(0,0,0);
            break;
        case White:
            RGB_LED::set(255,255,255);
            break;
        case Red:
            RGB_LED::set(255,0,0);
            break;
        case Green:
            RGB_LED::set(0,255,0);
            break;
        case Blue:
            RGB_LED::set(0,0,255);
            break;
        case Yellow:
            RGB_LED::set(255,255,0);
            break;
        case Purple:
            RGB_LED::set(80,0,80);
            break;
        case Aqua:
            RGB_LED::set(0,255,255);
            break;
      }
}

void RGB_LED::fadeToColour(byte colour,unsigned long speedValue)
{
    switch (colour) {
        case Black:
            RGB_LED::fadeTo(0,0,0,speedValue);
            break;
        case White:
            RGB_LED::fadeTo(255,255,255,speedValue);
            break;
        case Red:
            RGB_LED::fadeTo(255,0,0,speedValue);
            break;
        case Green:
            RGB_LED::fadeTo(0,255,0,speedValue);
            break;
        case Blue:
            RGB_LED::fadeTo(0,0,255,speedValue);
            break;
        case Yellow:
            RGB_LED::fadeTo(255,255,0,speedValue);
            break;
        case Purple:
            RGB_LED::fadeTo(80,0,80,speedValue);
            break;
        case Aqua:
            RGB_LED::fadeTo(0,255,255,speedValue);
            break;
      }
}

void RGB_LED::setSpeed(unsigned long speedValue)
{
    starting_time = millis();
    Speed = speedValue;
    R_Last_value = R_Current_value;
    G_Last_value = G_Current_value;
    B_Last_value = B_Current_value;
}

void RGB_LED::setFunction(byte functionValue)
{
    function=functionValue;
}

void RGB_LED::setPercentFade(float PercentFade)
{
    if(PercentFade>1)PercentFade=1;
    if(PercentFade<0)PercentFade=0;

    double diff = double(PercentFade)*double(Speed);
    starting_time = millis()-diff;
}

void RGB_LED::setFunctionCount(int FunctionCount)
{
    count = FunctionCount;
}

void RGB_LED::setPower(float newPower)
{
    if(newPower > 1.0) newPower = 1.0;
    if(newPower < 0.0) newPower = 0.0;    
    power = newPower;
}


/******************* GET *******************/

float RGB_LED::getPower()
{ 
    return power;
}

unsigned long RGB_LED::getSpeed()
{
    return Speed;
}

float RGB_LED::getPercentFade()
{
    unsigned long diff = millis()-starting_time;
    float PercentFade = double(diff)/double(Speed);
    if(PercentFade>1)PercentFade=1;
    return PercentFade;
}

byte RGB_LED::getFunction()
{
   return function;
}

int RGB_LED::getFunctionCount()
{
   return count;
}

uint8_t RGB_LED::getCurrentRValue()
{
   return R_Current_value;
}

uint8_t RGB_LED::getCurrentGValue()
{
   return G_Current_value;
}

uint8_t RGB_LED::getCurrentBValue()
{
   return R_Current_value;
}
        
/******************* CHECKING *******************/

boolean RGB_LED::hasFinished()
{
    return ((R_Future_value == R_Current_value) && (G_Future_value == G_Current_value) && (B_Future_value == B_Current_value));
}



/******************* COMMANDS *******************/

void RGB_LED::run()
{
    if(RGB_LED::hasFinished()) 
        RGB_LED::FunctionsFinished();
    
    unsigned long diff = millis()-starting_time;
    float PercentFade = double(diff)/double(Speed);
    if(PercentFade>1)
        PercentFade=1;
    PercentFade = FadeFunctionCallBack(PercentFade);
    R_Current_value = ((R_Future_value - R_Last_value)* PercentFade)+R_Last_value; //R_Last_value+((R_Future_value - R_Last_value)*PercentFade);
    G_Current_value = ((G_Future_value - G_Last_value)* PercentFade)+G_Last_value; //G_Last_value+((G_Future_value - G_Last_value)*PercentFade);
    B_Current_value = ((B_Future_value - B_Last_value)* PercentFade)+B_Last_value; //B_Last_value+((B_Future_value - B_Last_value)*PercentFade);
    
    writeOutput();
}


void RGB_LED::runOff()
{
    if( RGB_LED::hasFinished()) 
        RGB_LED::FunctionsFinished();
    
    unsigned long diff = millis()-starting_time;
    float PercentFade = double(diff)/double(Speed);
    if(PercentFade>1)PercentFade=1;
    PercentFade = FadeFunctionCallBack(PercentFade);
    if(PercentFade<0.5)
    {
        PercentFade = PercentFade*2;
        R_Current_value = ((0 - R_Last_value)* PercentFade)+R_Last_value; //R_Last_value+((R_Future_value - R_Last_value)*PercentFade);
        G_Current_value = ((0 - G_Last_value)* PercentFade)+G_Last_value; //G_Last_value+((G_Future_value - G_Last_value)*PercentFade);
        B_Current_value = ((0 - B_Last_value)* PercentFade)+B_Last_value; //B_Last_value+((B_Future_value - B_Last_value)*PercentFade);
    }
    else
    {
        PercentFade = PercentFade*2-1;
        R_Current_value = ((R_Future_value - 0)* PercentFade); //R_Last_value+((R_Future_value - R_Last_value)*PercentFade);
        G_Current_value = ((G_Future_value - 0)* PercentFade); //G_Last_value+((G_Future_value - G_Last_value)*PercentFade);
        B_Current_value = ((B_Future_value - 0)* PercentFade); //B_Last_value+((B_Future_value - B_Last_value)*PercentFade);
    }
    
    writeOutput();
}

void RGB_LED::stop()
{
    R_Last_value = R_Current_value;
    G_Last_value = G_Current_value;
    B_Last_value = B_Current_value;
    R_Future_value = B_Current_value;
    G_Future_value = B_Current_value;
    B_Future_value = B_Current_value;
}

void RGB_LED::delay(unsigned long delayValue)
{
    unsigned long currentLocalTime = millis();
    while(currentLocalTime+delayValue>millis())
    {
        RGB_LED::run();
    }
}



/******************* FUNCTIONS *******************/

void RGB_LED::writeOutput()
{
    if(invertedPins == true)
    {
        analogWrite(R_Pin, (255-R_Current_value) * power);
        analogWrite(G_Pin, (255-G_Current_value) * power);
        analogWrite(B_Pin, (255-B_Current_value) * power);
    }
    else
    {
        analogWrite(R_Pin, R_Current_value * power);
        analogWrite(G_Pin, G_Current_value * power);
        analogWrite(B_Pin, B_Current_value * power);
    }
}

void RGB_LED::FunctionsFinished()
{
    randomSeed(millis());
    RGB_LED::FadeFunction();
}






/******************* COLOUR FUNCTIONS *******************/

void RGB_LED::FadeFunction()
{
    switch (count%7) {
        case 0:
            RGB_LED::fadeToColour(White,Speed);
            break;
        case 1:
            RGB_LED::fadeToColour(Red,Speed);
            break;
        case 2:
            RGB_LED::fadeToColour(Yellow,Speed);
            break;
        case 3:
            RGB_LED::fadeToColour(Green,Speed);
            break;
        case 4:
            RGB_LED::fadeToColour(Purple,Speed);
            break;
        case 5:
            RGB_LED::fadeToColour(Blue,Speed);
            break;
        case 6:
            RGB_LED::fadeToColour(Aqua,Speed);
            break;
        default: 
            RGB_LED::fadeToColour(White,Speed);
            break;
      }

    count++;
}





/******************* FADE FUNCTIONS *******************/

float linear(float x)
{
    float y =  x;
    if(y <= 0)return 0;
    if(y >= 1)return 1;
    return y;
}

float quadratic(float x)
{
    float y =  x*x;
    if(y <= 0)return 0;
    if(y >= 1)return 1;
    return y;
}

float cos(float x)
{
    if(x <= 0)return 0;
    if(x >= 1)return 1;
    float y =  acos(x*2-1)/3.14;
    if(y <= 0)return 0;
    if(y >= 1)return 1;
    return y;
}

float waveup(float x)
{
    if(x <= 0)return 0;
    if(x >= 1)return 1;
    float y = x * (1+ 2 * (sin(150 * x)*(1-x)));
    if(y <= 0)return 0;
    if(y >= 1)return 1;
    return y;
}

float wavefaster(float x)
{
    if(x <= 0)return 0;
    if(x >= 1)return 1;
    float y = -0.5*( (cos(pow(100,x)-1))-1);
    if(y <= 0)return 0;
    if(y >= 1)return 1;
    return y;
}
