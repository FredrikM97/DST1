#ifndef display_h
#define display_h

#include "at91sam3x8.h"
#include "project.h"

void Delay(int Value);
void setBus(int select);

unsigned char Read_Status_Display();
void Write_Command_2_Display(unsigned char Command);
void Write_Data_2_Display(unsigned char Data);
void Auto_Write(unsigned char Data);
void Init_Display();
void Clear_Display(unsigned char text[]);
int Write_Display_At(char x,char y,unsigned char text[]);
void Set_ADP(char lower,char upper);
int Write_Text(unsigned char text[]);
void Clear_Display_At(char x,char y,int length);
unsigned char Read_Data_From_Display();
unsigned char Auto_Read();
void Write_Data(unsigned char data[]);
void exportData(float param);
float inportData(short ADP);
void clearGraphics();
void clearRAM(unsigned short bytes);
void Write_Byte(unsigned char param);
void drawImage(short start,unsigned char* image);
void drawVerticalLine(short start, char pixel, char length);
void drawHorisontalLine(short start, char length);
void Write_Pixel(unsigned short ADP, char pixel);

#endif