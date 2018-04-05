#include<regx52.h>
#include<math.h>
#include <stdio.h>

#define Error  13
#define M_PI 3.1415926535
#define M_E 2.7182818284

//Function declarations

void cct_init(void);
void delay(int);
//LCD Functions
void lcdinit(void);
void writecmd(int);
void writedata(char);
char READ_SWITCHES(void);
char get_key(void);
void writeline(char*);

//Expression functions

float factorial(float x)
{
	float p = 1;
	float i;
	for(i = 1; i <= x; i+=1)
		p *= i;
	return p;
}

char* expressionToParse;

char peek()
{
	return *expressionToParse;
}

char get()
{
	return *expressionToParse++;
}

float expression();

float number()
{
	float result = (float)(get() - '0');
	float denfactor = 1.0;
	float numfactor = 1.0;
	while (peek() >= '0' && peek() <= '9' || peek() == '.')
	{
			if(peek() == '.')
			{
				numfactor = 10.0;
				denfactor = 10.0;
				get();
				continue;
			}
		result = 10*result/numfactor + (float)(get() - '0')/denfactor;
		denfactor *= numfactor;
	}
	return result;
}

float factor()
{
	float result = 0;
	if (peek() >= '0' && peek() <= '9')
		result = number();
	else if (peek() == (char)247)
	{
		get();
		result = M_PI;
	}
	else if (peek() == 'e')
	{
		get();
		result = M_E;
	}
	else if (peek() == '(')
	{
		get(); // '('
		result = expression();
		get(); // ')'
	}
	else if (peek() == '-')
	{
		get();
		result = -factor();
	}
	else if (peek() == '!')
	{
		get();
		result = factorial(factor());
	}
	 else if(peek() == 's')
	 {
		get();
		result = sin(factor());
	}
	 else if(peek() == 'c')
	 {	
		get();
		result = cos(factor());
	 }
	 else if(peek() == 't')
	 {	
		get();
		result = tan(factor());
	 }
	 else if(peek() == 'l')
	 {	
		get();
		result = log(factor());
	 }
	return result; 
}
float texp()
{
	float result = factor();
	while(peek() == '^')
	{
		get();
		result = pow(result, factor());
	}
	return result;
}

float term()
{
	float result = texp();
	while (peek() == '*' || peek() == '/')
		if (peek() == '*')
		{	
			get();
			result *= texp();
		}
		else if(peek() == '/')
		{	
			get();
			if(peek() == '/')
			{
				while(peek() == '/')
					get();
				result = (int)( result / texp());
			}
			else
				result /= texp();
		}
	return result;
}

float expression()
{
	float result = term();
	while (peek() == '+' || peek() == '-' )
		if (peek() == '+')
		{
			get();
			result += term();
		}
		else if(peek() == '-')
		{
			get();
			result -= term();
		}
		
	return result;
}


//*******************
//Pin description
/*
P2 is data bus
P3.7 is RS
P3.6 is E
P1.0 to P1.3 are keypad row outputs
P1.4 to P1.7 are keypad column inputs
*/
//********************
// Define Pins
//********************
sbit RowA = P1^0;     //RowA
sbit RowB = P1^1;     //RowB
sbit RowC = P1^2;     //RowC
sbit RowD = P1^3;     //RowD

sbit C1   = P1^4;     //Column1
sbit C2   = P1^5;     //Column2
sbit C3   = P1^6;     //Column3
sbit C4   = P1^7;     //Column4

sbit E    = P3^6;     //E pin for LCD
sbit RS   = P3^7;     //RS pin for LCD

sbit Mode = P3^4;

// ***********************************************************
// Main program
//
int main(void)
{
	char key;                     //key char for keeping record of pressed key
	char expressionstr[20];
	int k = 0;
	float result;
	char strresult[20];
	char* const blank = "                "; //16 spaces
	cct_init();                   //Make input and output pins as required
	lcdinit();                    //Initilize LCD
	writecmd(0x01);            //clear display
	expressionstr[0] = '\0';
	while(1)
	{ 
		key = get_key();
		if(key == 'C')
		{
			writecmd(0x01);
			k = 0;
			expressionstr[0] = '\0';
		}
		else if (key == '~')
		{
			k = (k-1) >= 0 ? k-1 : 0;
			expressionstr[k] = '\0';
			writecmd(0x80);
			writeline(blank);
			writecmd(0x80);
			writeline(expressionstr);
		}
		else if (key == '=')
		{
			expressionToParse = expressionstr;
			result = expression();
			// inttostr(result, strresult);
			sprintf(strresult, "%.6f", result);
			writecmd(0xC0);
			writeline(blank);
			writecmd(0xC0);
			writeline(strresult);
			strresult[0] = '\0';
			writecmd(0x80+k%16);
		}
		else
		{
			writedata(key);	
			expressionstr[k] = key;
			k = (k+1) < 16 ? k+1 : k;
			expressionstr[k] = '\0';
		}
	}

}


void cct_init(void)
{
	P0 = 0x00;   //not used
	P1 = 0xf0;   //used for generating outputs and taking inputs from Keypad
	P2 = 0x00;   //used as data port for LCD
	P3 = 0x00;   //used for RS and E and mode
	Mode = 1; 
}

void delay(int a)
{
	int i;
	for(i=0;i<a;i++);   //null statement
}

void writedata(char t)
{
	RS = 1;             // This is data
	P2 = t;             //Data transfer
	E  = 1;             // => E = 1
	delay(150);
	E  = 0;             // => E = 0
	delay(150);
}


void writecmd(int z)
{
	RS = 0;             // This is command
	P2 = z;             //Data transfer
	E  = 1;             // => E = 1
	delay(150);
	E  = 0;             // => E = 0
	delay(150);
}

void lcdinit(void)
{
	///////////// Reset process from datasheet /////////
	delay(15000);
	writecmd(0x30);
	delay(4500);
	writecmd(0x30);
	delay(300);
	writecmd(0x30);
	delay(650);
	/////////////////////////////////////////////////////
	writecmd(0x38);    //function set
	writecmd(0x0f);    //display on,cursor off,blink off
	writecmd(0x06);    //entry mode, set increment
	writecmd(0x01);    //clear display
}

char READ_SWITCHES(void)  
{ 
	RowA = 0; RowB = 1; RowC = 1; RowD = 1;   //Test Row A

	if (C1 == 0) { delay(10000); while (C1==0); return Mode ? '1' : '.'; }
	if (C2 == 0) { delay(10000); while (C2==0); return Mode ? '2' : '('; }
	if (C3 == 0) { delay(10000); while (C3==0); return Mode ? '3' : ')'; }
	if (C4 == 0) { delay(10000); while (C4==0); return Mode ? '+' : '~'; }

	RowA = 1; RowB = 0; RowC = 1; RowD = 1;   //Test Row B

	if (C1 == 0) { delay(10000); while (C1==0); return Mode ? '4' : 's'; }
	if (C2 == 0) { delay(10000); while (C2==0); return Mode ? '5' : 'c'; }
	if (C3 == 0) { delay(10000); while (C3==0); return Mode ? '6' : 't'; }
	if (C4 == 0) { delay(10000); while (C4==0); return Mode ? '-' : 'l'; }

	RowA = 1; RowB = 1; RowC = 0; RowD = 1;   //Test Row C

	if (C1 == 0) { delay(10000); while (C1==0); return Mode ? '7' : 247; }
	if (C2 == 0) { delay(10000); while (C2==0); return Mode ? '8' : 'e'; }
	if (C3 == 0) { delay(10000); while (C3==0); return Mode ? '9' : '^'; }
	if (C4 == 0) { delay(10000); while (C4==0); return Mode ? '*' : '!'; }

	RowA = 1; RowB = 1; RowC = 1; RowD = 0;   //Test Row D

	if (C1 == 0) { delay(10000); while (C1==0); return Mode ? 'C' : 'C'; }
	if (C2 == 0) { delay(10000); while (C2==0); return Mode ? '0' : '('; }
	if (C3 == 0) { delay(10000); while (C3==0); return Mode ? '=' : '='; }
	if (C4 == 0) { delay(10000); while (C4==0); return Mode ? '/' : 's'; }

	return 'n';             // Means no key has been pressed
}

char get_key(void)           //get key from user
{
	char key = 'n';              //assume no key pressed

	while(key=='n')              //wait untill a key is pressed
		key = READ_SWITCHES();   //scan the keys again and again

	return key;                  //when key pressed then return its value
}

void writeline(char* line)
{
	for (; *line; ++line)
	{
		writedata(*line);
	}
}
