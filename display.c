#include "display.h"

/*
PC14-WR	Write Control (active low)
PC15-RD	Read Control (active low)
PC16-CE	Chip Enable (active low)
PC17-C/D	Command/Data (Data=0)
*/

void Init_Display(){
	*AT91C_PIOD_CODR=1; // Clear Reset display
	Delay(10);
	*AT91C_PIOD_SODR=1; // End Reset
	Write_Data_2_Display(0x00);
	Write_Data_2_Display(0x00);
	Write_Command_2_Display(0x40);//Set text home address
	union Var var;
	var.Int = graphicOffset;
	Write_Data_2_Display(var.Char[0]);
	Write_Data_2_Display(var.Char[1]);
	Write_Command_2_Display(0x42); //Set graphic home address
	Write_Data_2_Display(0x1e);
	Write_Data_2_Display(0x00);
	Write_Command_2_Display(0x41); // Set text area
	Write_Data_2_Display(0x1e);
	Write_Data_2_Display(0x00);
	Write_Command_2_Display(0x43); // Set graphic area
	Write_Command_2_Display(0x80); // OR Mode with Internal Character Generator
	Set_ADP(0,0);
	clearRAM(1<<13); // clear all physical RAM
	Write_Command_2_Display(0x9C); // Text & graphic on
}

unsigned char Read_Status_Display(){
	unsigned char Temp;
	*AT91C_PIOC_ODR = 0xFF<<2; // Enable inputs
	setBus(1); // Set OE and dir
	*AT91C_PIOC_SODR = 1<<17; //Set C/D
	*AT91C_PIOC_CODR = 3<<15; // Clear read display & chip select display
	*AT91C_PIOC_CODR = 1<<1; // Shits broken yo (ersättnings pin)
	Delay(2);
	Temp=(*AT91C_PIOC_PDSR&0xFF<<2)>>2; //Read data bus and save it in temp
	*AT91C_PIOC_SODR = 3<<15; //Set chip select display & read display
	*AT91C_PIOC_SODR = 1<<1; // Shits broken yo (ersättnings pin)
	setBus(0); // Disable output & Set dir as output (74chip)
	return Temp;
}

void Write_Command_2_Display(unsigned char Command){
	while((Read_Status_Display()&3)!=3)asm("NOP"); //wait for an OK
	*AT91C_PIOC_CODR = 0xFF<<2; // Clear databus
	*AT91C_PIOC_SODR = Command<<2; // Set Command to databus
	setBus(3); //Set dir as output & Enable output (74chip)
	*AT91C_PIOC_OER = 0xFF<<2; // Set databus as output
	*AT91C_PIOC_SODR = 1<<17; // Set C/D signal High (1 = Command)
	*AT91C_PIOC_CODR = 1<<16|1<<14; // Clear chip select display & write display
	Delay(2);
	*AT91C_PIOC_SODR = 1<<16|1<<14; // Set chip enable display & write display
	*AT91C_PIOC_ODR = 0xFF<<2; // Make databus as input
	setBus(4); // Disable output (74chip)
}

void Write_Data_2_Display(unsigned char Data){
	while((Read_Status_Display()&3)!=3)asm("NOP"); //wait for an OK
	*AT91C_PIOC_CODR = 0xFF<<2; // Clear databus
	*AT91C_PIOC_SODR = Data<<2; // Set Data to databus
	setBus(3); //Set dir as output & Enable output (74chip)
	*AT91C_PIOC_OER = 0xFF<<2; // Set databus as output
	*AT91C_PIOC_CODR = 1<<17; // Set C/D signal Low (0 = Data)
	*AT91C_PIOC_CODR = 1<<16|1<<14; // Clear chip select display & write display
	Delay(2);
	*AT91C_PIOC_SODR = 1<<16|1<<14; // Set chip enable display & write display
	*AT91C_PIOC_ODR = 0xFF<<2; // Make databus as input
	setBus(4); // Disable output (74chip)
}

void Auto_Write(unsigned char Data){
	while((Read_Status_Display()&8)!=8)asm("NOP"); //wait for an OK
	*AT91C_PIOC_CODR = 0xFF<<2; // Clear databus
	*AT91C_PIOC_SODR = Data<<2; // Set Data to databus
	setBus(3); //Set dir as output & Enable output (74chip)
	*AT91C_PIOC_OER = 0xFF<<2; // Set databus as output
	*AT91C_PIOC_CODR = 1<<17; // Set C/D signal Low (0 = Data)
	*AT91C_PIOC_CODR = 1<<16|1<<14; // Clear chip select display & write display
	Delay(2);
	*AT91C_PIOC_SODR = 1<<16|1<<14; // Set chip enable display & write display
	*AT91C_PIOC_ODR = 0xFF<<2; // Make databus as input
	setBus(4); // Disable output (74chip)
}

unsigned char Read_Data_From_Display(){ // glitchy ADP, don't use
	Write_Command_2_Display(0xC1); // Request data 
	setBus(1); // Set OE and dir
	*AT91C_PIOC_ODR = 0xFF<<2; // Set databus to input
	*AT91C_PIOC_CODR = 7<<15; // Enable read display, chip enable & C/D
	*AT91C_PIOC_CODR = 1<<1; // Shits broken yo (ersättnings pin)
	Delay(2);
	unsigned char data = (*AT91C_PIOC_PDSR&0xFF<<2)>>2; //Read data bus and save it in temp
	*AT91C_PIOC_SODR = 3<<15; //Set chip select display & read display
	*AT91C_PIOC_SODR = 1<<1; // Shits broken yo (ersättnings pin)
	setBus(4); // Disable output (74chip)
	return data;
}

unsigned char Auto_Read(){
	while((Read_Status_Display()&4)!=4)asm("NOP"); // wait for an OK
	setBus(1); // Set OE and dir
	*AT91C_PIOC_ODR = 0xFF<<2; // Set databus to input
	*AT91C_PIOC_CODR = 7<<15; // Enable read display, chip enable & C/D
	*AT91C_PIOC_CODR = 1<<1; // Shits broken yo (ersättnings pin)
	Delay(2);
	unsigned char data = (*AT91C_PIOC_PDSR&0xFF<<2)>>2; //Read data bus and save it in temp
	*AT91C_PIOC_SODR = 3<<15; //Set chip select display & read display
	*AT91C_PIOC_SODR = 1<<1; // Shits broken yo (ersättnings pin)
	setBus(4); // Disable output (74chip)
	return data;
}

int Write_Text(unsigned char text[]){
	int blanks = 0;
	int i = 0;
	Write_Command_2_Display(0xB0); // Auto Wtrite
	while(text[i]!='\0'){
		if(text[i]=='\n'){
			for(int temp=30-(i+blanks)%30;temp>0;temp--){
				Auto_Write(0);
				blanks++;
			}
			blanks--;	// since \n counts as a char
		}else{
			Auto_Write(text[i]-0x20);
		}
		i++;
	}
	Write_Command_2_Display(0xB2);
	return i+blanks;
}

void Write_Data(unsigned char data[]){
	Write_Command_2_Display(0xB0);
	for(int i = 3;i >= 0;i--)
		Auto_Write(data[i]);
	Write_Command_2_Display(0xB2);
}

void Write_Byte(unsigned char param){
	Write_Data_2_Display(param);
	Write_Command_2_Display(0xC0);
}

void Clear_Display(unsigned char text[]){
	Set_ADP(0x0,0x0);	
	clearRAM(480-Write_Text(text));
}

int Write_Display_At(char x,char y,unsigned char text[]){
	Set_ADP(x,y);
	return Write_Text(text);
}

void Clear_Display_At(char x,char y,int length){
	Set_ADP(x,y);
	clearRAM(length);
}

void Set_ADP(char lower,char upper){
	Write_Data_2_Display(lower);
	Write_Data_2_Display(upper);
	Write_Command_2_Display(0x24);
}

void exportData(float param){
	union Var var;
	var.Float = param;
	Write_Data(var.Char);
}

float inportData(short ADP){
	union Var var;
	var.Int = ADP;
	Set_ADP(var.Char[0],var.Char[1]);
	Write_Command_2_Display(0xB1); // Auto Read
	for(int i = 3; i>=0 ;i--)
		var.Char[i] = Auto_Read();
	Write_Command_2_Display(0xB2);
	return var.Float;
}

void clearGraphics(){
	union Var var;
	var.Int = graphicOffset;
	Set_ADP(var.Char[0],var.Char[1]);
	clearRAM(3840);
}

void clearRAM(unsigned short bytes){
	Write_Command_2_Display(0xB0);
	for(int i = bytes; i !=0 ;i--)
		Write_Data_2_Display(0);
	Write_Command_2_Display(0xB2);
}

void drawImage(short start,unsigned char* image){
	// draws image data
	// first 2 chars of the data must be x&y size
	union Var var;
	unsigned char* pointer = image;
	char xSize = *(pointer++);
	char ySize = *(pointer++);
	for(char y = 0; y < ySize; y++){
		var.Int = start + y*(31-xSize);
		Set_ADP(var.Char[0],var.Char[1]);
		for(char x = xSize; x!=0; x--)
			Write_Byte(*(pointer++));
	}
}

void drawVerticalLine(short start, char pixel, char length){
	union Var var;
	for(int y = length; y!=0; y--){
		var.Int = start + (y-1)*30;
		Set_ADP(var.Char[0],var.Char[1]);
		Write_Command_2_Display(0xF8 + pixel);
	}
}

void drawHorisontalLine(short start, char length){
	union Var var;
	var.Int = start;
	Set_ADP(var.Char[0],var.Char[1]);
	for(int x = length; x!=0; x--)
		Write_Byte(0xFF);
}

void Write_Pixel(unsigned short ADP, char pixel){
	union Var var;
	var.Int=ADP;
	Set_ADP(var.Char[0],var.Char[1]);
	Write_Command_2_Display(0xF8 + pixel); //0xF8 set bit	
}
