#ifndef project_h
#define project_h

#include "system_sam3x.h" 
#include "at91sam3x8.h" 
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "float.h"
#include "display.h"
#include <stdbool.h>
#include "limits.h"
#include "half.h"
#include "math.h"

// definitions
#define daySize 1443 //24*60+3
#define graphicOffset 640
#define PWM_CH1_MIN_CDTY 1650 // limits: 1450 - 6900
#define PWM_CH1_MAX_CDTY 6700

struct flag{
	unsigned short functionState:4;
	unsigned short counter:16;
	unsigned short temperatureState:2;
	unsigned short inputInt:2;
	unsigned short lightReady:1;
	unsigned short keyPad:1;
	unsigned short fastMode:1;
	unsigned short alarmTriggerd:1;
	unsigned short error:1;
	unsigned short toggle:1;
};

union Var{
	float Float;
	unsigned int Int;
	unsigned char Char[4];
	unsigned short Short[2];
};

extern struct flag sysState;

void init();
int pollPanel();
void setBus(int select);
void Delay(int Value);
unsigned char* float2string(float in);
unsigned char* int2string(int in);
void tempResetStart();
void tempResetEnd();
void tempStart();
void lightPrint();
void lightStart(unsigned char Channel);
void setStateAndMenu(unsigned char param);
void dispatch(unsigned char param);
void menu(char input);
void trackLight();
void tempGet();
void detectLight();
void handle();
void execute();
void store(float input, uint16_t* array);
void drawData(unsigned char day, uint16_t* data);
void generateData();
void makeFloat(unsigned char param);
void setAlarm(unsigned char input);
void printAlarm();

#endif