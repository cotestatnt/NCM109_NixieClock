
#ifndef RGB_LED_h
#define RGB_LED_h

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#include <wiring.h>
#endif

//colours
#define Black 0
#define White 1
#define Red 2
#define Green 3
#define Blue 4
#define Yellow 5
#define Purple 6
#define Aqua 7

//functions
#define Fade 1

//PERCENT FUNCTION
float linear(float x);
float quadratic(float x);
float cos(float x);
float waveup(float x);
float wavefaster(float x);

class RGB_LED
{
    public:
        // SETUP 
        RGB_LED(uint8_t pinR,uint8_t pinG,uint8_t pinB);
        RGB_LED(bool inverted, uint8_t pinR, uint8_t pinG, uint8_t pinB);
        void setCallback(float (*CallBack)(float x));

        // SET 
        void set(uint8_t Rvalue,uint8_t Gvalue,uint8_t Bvalue);
        void setSpeed(unsigned long speedValue);
        void setPercentFade(float FadeValue);
        void setPower(float newPower);
        void fadeTo(uint8_t Rvalue,uint8_t Gvalue,uint8_t Bvalue,unsigned long speedValue);
        void setFunction(byte functionValue);

        void setColour(byte colour);
        void fadeToColour(byte colour, unsigned long speedValue);
        void setFunctionCount(int FunctionCount);
        
        //GET
        unsigned long getSpeed();
        float getPercentFade();
        byte getFunction();
        uint8_t getCurrentRValue();
        uint8_t getCurrentGValue();
        uint8_t getCurrentBValue();
        int getFunctionCount();
        float getPower();

        // CHECKING 
        boolean hasFinished();
        
        // COMMANDS 
        void run();
        void runOff();
        void stop();
        void delay(unsigned long delayValue);
        float power = 1.0;
        
        
    private:
        uint8_t R_Pin;
        uint8_t G_Pin;
        uint8_t B_Pin;
        uint8_t R_Last_value;
        uint8_t G_Last_value;
        uint8_t B_Last_value;
        uint8_t R_Current_value;
        uint8_t G_Current_value;
        uint8_t B_Current_value;
        
        uint8_t R_Future_value;
        uint8_t G_Future_value;
        uint8_t B_Future_value;

        unsigned long Speed = 2000;
        unsigned long starting_time;
        int function=0;
        int count;
        bool invertedPins=false;
        
        
        float (*FadeFunctionCallBack) (float x);

        void writeOutput();
        void FunctionsFinished();
        void FadeFunction();
};

#endif 
