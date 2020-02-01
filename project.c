#include "project.h"

// Set Globals
struct flag sysState = {
	.functionState = 0,
	.counter = 0, 
	.temperatureState = 0,
	.inputInt = 0,
	.lightReady = false,
	.keyPad = false,
	.fastMode = true,
	.alarmTriggerd = false,
	.error = false,
	.toggle = false
};

char lightLength = 0;
char tempLength = 0;
short lastLight = 0;
float alarmMaxTemp = 140.0;
float alarmMinTemp = -40.0;
unsigned char temp = 0;
float tempFloat = 1;
unsigned char LogsPerTemp = 1;
unsigned char LogCounter = 0;
float tempValues = 0;
unsigned short tempPending = 0;
unsigned short storePending = 0;
unsigned short lightPending = 0;
unsigned short tempTimeStamp;
bool ticked = false;
short minLight;
char lastAngle = 0;
int bestAngle = 0;
unsigned int uptime = 0;
unsigned char imageUpArrow[6] = {1,4,0x08,0x1C,0x3E,0x7F};
unsigned char imageRightArrow[9] = {1,7,0x80,0xC0,0xE0,0xF0,0xE0,0xC0,0x80};
uint16_t tempData[10102];
uint16_t barData[10102];


void init(){
	SystemInit(); // Disables the Watchdog and setup the MCK 
	
	// Initialization of Power Management Controler
	*AT91C_PMC_PCER=7<<12|1<<27; // B&C&D & TC0
	*AT91C_PMC_PCER1=1<<4|1<<5; // PWM & ADC
	
	// Initialization InPort
	unsigned int mask = 0xF<<2;
	*AT91C_PIOC_ODR = mask;		//Output Disable Register
	*AT91C_PIOC_PPUDR = mask;	//Pull Up Disable Register
	*AT91C_PIOC_PER = mask;		//Enable Pin	
	
	// Initialization OutPort
	mask = 0xFF<<12|1<<1;
	*AT91C_PIOC_CODR=mask; 		//Clear Output Register
	*AT91C_PIOC_OER=mask;		//Output Enable Register
	*AT91C_PIOC_PPUDR=mask; 	//Pull Up Disable Register	
	*AT91C_PIOC_PER=mask;		//Enable Pin
	
	mask = 1<<25;
	*AT91C_PIOB_PER=mask;		//Enable Pin
	*AT91C_PIOB_CODR=mask; 		//Clear Output Register
	*AT91C_PIOB_PPUDR=mask; 	//Pull Up Disable Register
	*AT91C_PIOB_OER=mask;		//Output Enable Register
	
	mask = 1|1<<2;
	*AT91C_PIOD_PER=mask;		//Enable Pin
	*AT91C_PIOD_CODR=mask; 		//Clear Output Register
	*AT91C_PIOD_PPUDR=mask; 	//Pull Up Disable Register
	*AT91C_PIOD_OER=mask;		//Output Enable Register
	
	// Setup Timer Clock
	*AT91C_TC0_CMR =
		0<<0|		//Select Timer_Clock1 as TCCLK
		0<<3|		//Disable clock invert
		0<<4|		//Disable burst
		0<<6|		//counter clock is not stopped when RB loading occurs
		1<<7|		//counter clock is disabled when RB loading occurs
		0<<8|		//No external triggers
		0<<10|	//TIOB is used as an external trigger
		0<<14|	//RC Compare has no effect on the counter and its clock
		0<<15|	//Disable wave/enable capture
		2<<16|	//Load counter to A when TIOA falling in (TC0_CMR0)
		1<<18;	//Load counter to B when TIOA rising in (TC0_CMR0)
	*AT91C_TC0_CCR=5; //Enable counter and make a sw_reset in TC0_CCR0
	
	// Set interupts
	SysTick_Config(CHIP_FREQ_CPU_MAX/8000);
	*AT91C_NVIC_STICKCSR=3; // Set MCK/8 to reduce power consumption
	
	*AT91C_TC0_IER = 1<<6; // TC0 Interupt RB load	
	*AT91C_NVIC_ICPR = 1<<27;	// Interupt Clear-Pending 0
	*AT91C_NVIC_ISER = 1<<27;	// Interupt Set-Enable 0
	
	*AT91C_NVIC_ICPR1 = 1<<5;
	*AT91C_NVIC_ISER1 = 1<<5;
	
	*AT91C_NVIC_ICPR = 1<<13; // PIOC
	*AT91C_NVIC_ISER = 1<<13;
	*AT91C_PIOC_ISR; // Dummy read
	*AT91C_PIOC_IER = 0xF<<2;
	*AT91C_PIOC_AIMER = 0xF<<2;
	*AT91C_PIOC_ESR = 0xF<<2;
	*AT91C_PIOC_FELLSR = 0xF<<2;
	
	// ADC
	*AT91C_ADCC_CR=1<<0; // Reset ADC
	*AT91C_ADCC_MR |= 2<<8; // Set prescale
	*AT91C_ADCC_MR |= 1<<23; // Allow different analog settings
	*AT91C_ADCC_COR = 3<<2|3<<18; //  Offset & diff mode
	*AT91C_ADCC_CHER = 1<<2|1<<3; // Enable channel 2 & 3
	*AT91C_ADCC_CGR |= 1<<4; // Channel 2 gain = 1
	*AT91C_ADCC_SR; // Dummy read
	*AT91C_ADCC_IER = 1<<2|1<<3; 
	
	
	// PWM
	*AT91C_PWMC_MR = 1<<16|5<<24; // Set div to prescale B & prescale B to /32
	*AT91C_PWMC_ENA = 1<<1; // Enable channel 1
	*AT91C_PWMC_CH1_CMR=0xC; // Set prescale to prescale B
	*AT91C_PWMC_CH1_CDTYR = PWM_CH1_MIN_CDTY; //Write a value to PWM_CDTY (1ms)
	*AT91C_PWMC_CH1_CPRDR = 52500; //Write a value to PWM_CPRD (20ms)
	*AT91C_PIOB_PDR = 1<<17; //Let peripheral control the pin   REG_PIOB_PDR
	*AT91C_PIOB_ABMR = 1<<17; //Set peripheral B to control the pin
	
	// Display
	Init_Display(); // Setup display
	setStateAndMenu(12); // Display menu
//	imageUpArrow[6] = {1,4,0x08,0x1C,0x3E,0x7F};
}

int pollPanel(){
	int returnValue = 0;
	*AT91C_PIOD_CODR = 1<<2; //Clear OE KEY BUS (Active Low)
	*AT91C_PIOC_IDR = 0xF<<2; // Disable keypad interupt
	*AT91C_PIOC_OER = 7<<7; // Make all Column pin as output
	*AT91C_PIOC_SODR= 7<<7; // Set all Column pin as high
	for(int col=0; col<3; col++){ // Loop Column
		*AT91C_PIOC_CODR = 1<<(col+7); // Clear one column at the time
		for(int row=0; row<4; row++){ // Loop Row
			if((*AT91C_PIOC_PDSR&(1<<(row+2))) == 0) // Check if row is zero
				returnValue = row*3+col+1;
		} // End loop Row
		*AT91C_PIOC_SODR = 1<<(col+7); // Set the column again
	} // End loop Column
	*AT91C_PIOC_CODR = 7<<7;
	*AT91C_PIOC_IER = 0xF<<2; // Re-enable interupt
	//*AT91C_PIOC_ODR = 7<<7; // "Make all Column pin as input" how about no
	return returnValue;
}

void setBus(int select){ // bits: keypad OE, Disp dir (high=output), Disp OE
	
	if((select&4)==0){
		*AT91C_PIOC_IDR = 0xF<<2;
	}else{
		*AT91C_PIOC_OER = 7<<7; // Make all Column pin as output
		*AT91C_PIOC_CODR= 7<<7; // Set all Column pin as high
		*AT91C_PIOC_ISR;
		*AT91C_PIOC_IER = 0xF<<2;
	}
	
	*AT91C_PIOC_SODR=((select&3)^1)<<12;
	*AT91C_PIOC_CODR=((select&3)^2)<<12;
	*AT91C_PIOD_SODR=(select&4)^4;
	*AT91C_PIOD_CODR=select&4; // Active low
}

void Delay(int Value){ // Amount of ns to wait
	//	Value = (int)(Value*0.084); // (CHIP_FREQ_CPU_MAX*10^-9);
	for(int i=0;i<Value;i++)
		asm("NOP");
}

unsigned char* float2string(float in){
	// Allocate 3 (for "-0.") + mantissa - the smallest exponent FLT_MIN_EXP
	// assuming that FLT_RADIX is 10
	char* str=malloc((3 + FLT_MANT_DIG - FLT_MIN_EXP)*sizeof(char));
	sprintf(str,"%f",in);
	return (unsigned char*)str;
}

unsigned char* int2string(int in){
	char* str=malloc(11*sizeof(unsigned char)); // Room for 10 numbers + 1 sign
	sprintf(str,"%i",in);
	return (unsigned char*)str;
}

void tempResetStart(){ //Create a reset;
	*AT91C_PIOB_OER=1<<25;
}

void tempResetEnd(){ // End reset
	*AT91C_PIOB_ODR=1<<25; // Use pull-up resistor to go high
	Delay(100); // T setup =  10 uS
}

void tempStart(){  // Create a startpulse
	*AT91C_PIOB_OER=1<<25;
	Delay(25);	// T Start pulse > 2.5 uS
	*AT91C_PIOB_ODR=1<<25;
	*AT91C_TC0_CCR=5; // make a sw_reset in TC0_CCR0;
	*AT91C_TC0_SR; // Dummy read
	*AT91C_TC0_IER=1<<6; //Enable interrupt TC_IER_LDRBS
}

void tempGet(){
	int RA = *AT91C_TC0_RA; // extra line to specify volatile access
	float temp = (*AT91C_TC0_RB - RA)/210.0-273.15;
	if(sysState.functionState==9){ // print temp
		unsigned char* str = float2string(temp);
		Clear_Display_At(6,0,tempLength); // clear last result
		tempLength = Write_Display_At(6,0,str); // Store old length for clearing
		free(str);
	}
	if(temp<alarmMinTemp || temp>alarmMaxTemp){
		sysState.alarmTriggerd = 1;
		if(sysState.functionState == 12)
			menu(12);
	}
	if(storePending != 0){
		storePending--;
		LogCounter++;
		tempValues += temp;
		if(LogCounter >= LogsPerTemp){
			store(tempValues/LogCounter,tempData);
			LogCounter = 0;
			tempValues = 0;
		}
	}
}

void store(float input, uint16_t* array){
	union Var var;
	var.Float = input;
	uint16_t halfInput = half_from_float(var.Int);
	if((*array)%daySize == 0){  // if new day
		if(*array == (daySize*7)){ // if out of memory
			*array = 0; // start over
			sysState.error = true; // report error
		}
		memset(array+(*array)+1,0,daySize*sizeof(uint16_t)); // remove old data
		for(int i = 4; i!=0; i--) // initialize new day
			*(array + ++(*array)) = halfInput;
	} else {
		
		// update statistics
		// get
		short index = daySize * ((*array)/daySize);
		var.Int = half_to_float(*(array + ++index));
		float average = var.Float;
		uint16_t min = *(array + ++index);
		uint16_t max = *(array + ++index);
		average += (input-average)/((*array)%daySize-2);
		
		// store
		index = daySize * ((*array)/daySize);
		var.Float = average;
		*(array + ++index) = half_from_float(var.Int);
		if(min > halfInput)
			*(array + ++index) = halfInput;
		else if(max < halfInput)
			*(array + index + 2) = halfInput;
		
		*(array + ++(*array)) = halfInput;
	}
}

void lightPrint(){ // print light level
	short t = *AT91C_ADCC_LCDR&0xFFF;
	t -=1<<11;
	unsigned char* str = int2string(t);
	Clear_Display_At(36,0,lightLength); // clear last result
	lightLength = Write_Display_At(36,0,str); // Store old length for clearing
	free(str);
}

void lightStart(unsigned char Channel){ // Start light measurement
		*AT91C_ADCC_IER = 1<<Channel;
		*AT91C_ADCC_CR = 1<<1; //Start an ADC in ADC_CR
}

void menu(char input){
	switch(input){
	case 1:
		Clear_Display("Enter sample rate (0-10)");
		break;
	case 2:
		Clear_Display("Dag:   av 7\nAvg:\nMin:\nMax:\n\nMax\n\n\n\n\n\n\n\nMin\n\n   0");
//		drawVerticalLine(graphicOffset + 3, 6, 8*4);
		break;
	case 3:
		Clear_Display("Sun position: ");
		break;
	case 4:
		Clear_Display("Changing\nInput Char:\nFloat:\n\nMax:\nMin:");
		break;
	case 6:
		Clear_Display("In sovjet russia sun track you");
		break;
	case 9:
		Clear_Display("Temp:\nLjus:");
		break;
	case 12:
		Clear_Display("[1] Log temp\n[2] Present data\n[3] Detect sun\n[4] Set alarm\n[5] Fast mode\n[6] Track sun\n[7] Load pre-recorded values\n[8] Log air\n[9] Unit test");
		if(LogsPerTemp != 0){
			unsigned char* str = int2string(LogsPerTemp);
			Write_Display_At(13,0,str);
			free(str);
		}else
			Write_Display_At(13,0,"0");
		
		if(sysState.alarmTriggerd)
			Write_Display_At(30*3+13,0,". Alarm Triggerd!");
		
		if(sysState.fastMode)
			Write_Display_At(134,0,"on");
		else
			Write_Display_At(134,0,"off");
		break;
	}
}

void dispatch(unsigned char param){
	switch(sysState.functionState){
	case 1: // Set logging
		if(param == 12)
			setStateAndMenu(param);
		else{
			if(param == 11)
				LogsPerTemp = 0;
			else
				LogsPerTemp = param;
			setStateAndMenu(12);
		}	
		break;
	case 2: // display data
		if(param == 12){
			setStateAndMenu(param);
			clearGraphics();
		}
		else if(param <= 7 && param > 0){
			drawData(param-1,tempData);
		}
		break;
	case 3: // detect sun
		if(param == 12){
			setStateAndMenu(param);
			*AT91C_ADCC_MR &= ~(1<<7); // disable freerun
			*AT91C_ADCC_CHER = 1<<3; // re-enable CH3
		}
		break;
	case 4: // set alarm
		if(param == 12){
			sysState.alarmTriggerd = false;
			setStateAndMenu(param);
			sysState.inputInt = 0;
		}
		else
			setAlarm(param);
		break;
	case 6: // track light
		if(param == 12){
			setStateAndMenu(param);
			*AT91C_ADCC_MR &= ~(1<<7); // disable freerun
			*AT91C_TC0_CCR=2; // disable timer;
		}
		break;
	case 9: // unit test
		if(param == 12)
			setStateAndMenu(param);
		else
			*AT91C_PWMC_CH1_CDTYUPDR = 1650+(5050/18)*param; // set servo
		break;
	case 12: // main menu
		switch(param){
		case 1: // log temp
			setStateAndMenu(param);
			break;
		case 2:
				setStateAndMenu(param);
				drawData(0,tempData);
			break;
		case 3: // detect sun
				setStateAndMenu(param);
				minLight = SHRT_MAX;
				while(*AT91C_PWMC_CH1_CDTYR>PWM_CH1_MIN_CDTY)
				*AT91C_PWMC_CH1_CDTYUPDR = PWM_CH1_MIN_CDTY;
				*AT91C_ADCC_MR |= 1<<7; // enable freerun
				*AT91C_ADCC_CHDR = 1<<3; // Disable CH3 -> no diff mode
			break;
		case 4:
				setStateAndMenu(param);
				temp = 0;
				tempFloat = 1.0;
				printAlarm();
			break;
		case 5: // Toggle fast mode
			sysState.fastMode=~sysState.fastMode;
			menu(12);
					break;
		case 6:
				setStateAndMenu(param);
				*AT91C_ADCC_MR |= 1<<7; // enable freerun
				*AT91C_TC0_CCR=5; // reset & start timer;
			break;
		case 7:
			generateData();
			break;
		case 8:
			break;
		case 9:
				setStateAndMenu(param);
			break;
		}
		break;
	}
}

void setStateAndMenu(unsigned char param){
	sysState.functionState = param;
	menu(param);
}

void execute(){ // Time dependent stuff
	
	switch(sysState.functionState){
	case 3:
		detectLight();
		break;
	case 6: // Track light
		trackLight();
		if(sysState.counter % 100 == 0)
			asm("nop");//printAngle();
		break;
	case 9:
		if(sysState.counter % 500 == 0){
			if(!ticked){
				tempPending++;
				lightPending++;
				ticked = true;
			}
		}else
			ticked = false;
		break;
	}
	if((sysState.fastMode == true && sysState.counter >= 1000) || sysState.counter >= 60000){
		sysState.counter = 0;
		uptime++;
		if(LogsPerTemp == 0)
			tempPending++;
		else{
			tempPending += LogsPerTemp;
			storePending += LogsPerTemp;
		}
		
	}
}

void handle(){ // Handles interupt related stuff
	if(sysState.keyPad){ // If keypad is in use
		unsigned char keyPad = pollPanel();
		if(keyPad == 0){
			sysState.keyPad = false;
		}else
			dispatch(keyPad);
		sysState.keyPad = false;
	}
	
	else if(tempPending>0){ // if we want to get temperature
		
		if(((short)(sysState.counter-tempTimeStamp))>520){ //Failsafe
			// This shouldn't be needed but voltage drops can cause glitches
			sysState.temperatureState = 0;
			tempTimeStamp = sysState.counter;
		}
		
		switch(sysState.temperatureState){ // read sensor state
		case 0: // Needs reset
			tempResetStart();
			tempTimeStamp = sysState.counter;
			sysState.temperatureState = 1;
			break;
		case 1: // Wait for reset
			if(((short)(sysState.counter-tempTimeStamp))>16){
				tempResetEnd();
				tempStart();
				sysState.temperatureState = 2;
			}
			break;
		case 2: // Wait for response
			break;
		case 3: // Finish
			tempGet();
			tempPending--;
			sysState.temperatureState = 0;
			break;
		}
	}
	
	else if(lightPending>0){ // If we want to get light level
		if(sysState.lightReady){
			sysState.lightReady = false;
			lightPrint();
			lightPending--;
		}else
			lightStart(2);
	}
	
	else
		asm("WFI");
}

void trackLight(){ // Steers the servo towards light
	const short Pgain = 1; // gain control values
	const short Dgain = 50;
	
	short light = *AT91C_ADCC_LCDR&0xFFF;
	light -=1<<11;
	float delta = (light-lastLight) / (float)*AT91C_TC0_CV; // derivate
	*AT91C_TC0_CCR=5; // reset & start timer;
	int temp = (*AT91C_PWMC_CH1_CDTYR - Pgain*light + (int)(Dgain*delta));
	if(temp<PWM_CH1_MIN_CDTY) // Check so temp is within range
		temp = PWM_CH1_MIN_CDTY;
	else if(temp>PWM_CH1_MAX_CDTY)
		temp = PWM_CH1_MAX_CDTY;
	*AT91C_PWMC_CH1_CDTYUPDR = temp;
}

void detectLight(){
	if(*AT91C_PWMC_CH1_CDTYR<PWM_CH1_MAX_CDTY && (*AT91C_ADCC_OVR&1<<2) == 1<<2){
		short light = *AT91C_ADCC_LCDR&0xFFF;
		if(minLight>light){ // find smallest
			minLight = light;
			bestAngle = *AT91C_PWMC_CH1_CDTYR;
		}
		*AT91C_PWMC_CH1_CDTYUPDR = *AT91C_PWMC_CH1_CDTYR + 10;
	}
		
	if(sysState.counter % 100 == 0){ // print angle to display
		unsigned char* angle = float2string((bestAngle-PWM_CH1_MIN_CDTY)/(float)(PWM_CH1_MAX_CDTY-PWM_CH1_MIN_CDTY)*180.0);
		Clear_Display_At(14,0,lastAngle);
		lastAngle = Write_Display_At(14,0,angle);
		free(angle);
	}
}

void drawData(unsigned char day, uint16_t* data){
	union Var var;
	const char ySize = 80;
	const char columns = 30;
	clearGraphics();
	
	// Draw Graph using average of 8 minutes per pixel
	// Draw y axis
	drawImage(graphicOffset + columns*34+3, imageUpArrow);
	drawVerticalLine(graphicOffset + columns*38+3, 3, ySize);
	// draw x axis
	drawImage(graphicOffset + columns*110+27, imageRightArrow);
	drawHorisontalLine(graphicOffset + columns*113+3, 24);
	// fill with data
	short index = 2+day*daySize;
	var.Int = half_to_float(*(data+index++));
	float min = var.Float;
	var.Int = half_to_float(*(data+index++));
	float max = var.Float;
	float scale = ((float)(ySize)/(8*(max-min)));
	float min8 = min*8; // precompute for looping
	
	for(int x = 0; x<180; x++){
		uint16_t sum = *(data+index++);
		for(int i = 7; i!=0; i--)
			sum = half_add(sum, *(data+index++));
		var.Int = half_to_float(sum);
		char row = 112 - (unsigned short)(scale*(var.Float-min8));
		Write_Pixel((unsigned short)(graphicOffset + x/8 + 4 + columns*row),7-x%8);
	}
	
	// print numeric values
	unsigned char* str;
	Set_ADP(5,0);
	Write_Byte(day+0x11);
	for(int i = 1; i<4; i++){
		var.Int = half_to_float(*(data+daySize*day+i));
		str = float2string(var.Float);
		if(str==NULL){
			sysState.error = true;
			break;
		}
		Clear_Display_At(5+columns*i,0, 9);
		Write_Display_At(5+columns*i,0,str);
		free(str);
	}
	
}

void printAlarm(){
	// display current float
	unsigned char* str;
	if(sysState.toggle)
		Write_Display_At(9,0,"max");
	else
		Write_Display_At(9,0,"min");
	
	str = int2string(temp);
	Clear_Display_At(12+30,0,12);
	Write_Display_At(12+30,0,str);
	free(str);
	
	str = float2string(tempFloat);
	Clear_Display_At(7+30*2,0,12);
	Write_Display_At(7+30*2,0,str);
	free(str);
	
	str = float2string(alarmMaxTemp);
	Clear_Display_At(5+30*4,0,12);
	Write_Display_At(5+30*4,0,str);
	free(str);
	
	str = float2string(alarmMinTemp);
	Clear_Display_At(5+30*5,0,12);
	Write_Display_At(5+30*5,0,str);
	free(str);
}

void generateData(){
	for(int i = 1440; i!=0; i--)
		store(10.0*(1.0+(float)sin(i/144.0)),tempData);
}

void setAlarm(unsigned char input){
	if(input == 10){ // if *
		makeFloat(temp);
		temp = 0;
	}
	else{
		unsigned short value =  10*temp + input%11;
		if(value > 256)
			temp = 0;
		else
			temp = (unsigned char)value;
	}
	
	if(temp > 99){ // if max length
		makeFloat(temp);
		temp = 0;
	}
	
	if(sysState.inputInt == 3){ // if float is done
		if(sysState.toggle) // Store in min or max
			alarmMaxTemp = tempFloat;
		else
			alarmMinTemp = tempFloat;
		sysState.toggle ^= 1;
		makeFloat(0); // reset input
	}
	printAlarm();
}

void makeFloat(unsigned char param){
	float size;
	union Var var;
	switch(sysState.inputInt){
	case 0: // input sign
		var.Float = tempFloat;
		var.Int = (var.Int&0x7FFFFFFF) | ((param%2)<<31);
		tempFloat = var.Float;
		sysState.inputInt++;
		break;
	case 1: // input integer
		tempFloat = tempFloat*param;
		sysState.inputInt++;
		break;
	case 2: // input decimal
		size = 1;
		while(size < param)
			size = 10*size;
		if(tempFloat >= 0)
			tempFloat += param/size;
		else
			tempFloat -= param/size;
		sysState.inputInt++;
		break;
	case 3: // reseting
		sysState.inputInt = 0;
		tempFloat = 1.0;
		break;
	}
}

/******************************************************************************
*                                 HANDLERS                                    *
******************************************************************************/

void SysTick_Handler(){
	sysState.counter++;
}

void TC0_Handler(){
	*AT91C_TC0_SR; // Dummy read
	*AT91C_TC0_IDR=1<<6;
	sysState.temperatureState = 3;
}

void ADC_Handler(){
	*AT91C_ADCC_IDR = 0xFFFFFFFF;
	sysState.lightReady = true;
}

void PIOC_Handler(){
	*AT91C_PIOC_ISR;
	sysState.keyPad = true;
}
