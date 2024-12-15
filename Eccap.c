#include <p18f4520.h>
#include <delays.h>
#include <stdlib.h>
void Init_LCD(void);
void display_data(void);
void W_ctr_8bit(char); // 8-bit Control word for LCD
void W_data_8bit(char); // 8-bit Text Data for LCD
void beep(void);
void beep1(void);
void prog(void);
void AtoD(void);
void manipulation(void);
void calender(void);
void Set_date_time(void);
#define touch	PORTEbits.RE0
#define buzzer PORTCbits.RC2
#define YELLOW PORTCbits.RC1
#define RED PORTBbits.RB5
#define S3 PORTBbits.RB3
#define S2 PORTBbits.RB2
#define S7 PORTBbits.RB0
#define LCD_DATA PORTD	//PORTD for LCD
#define LCD_E PORTCbits.RC7 // E signal for LCD
#define LCD_RS PORTCbits.RC6 // RS signal for LCD
#define coolentpump PORTAbits.RA4

char TIME[]=" S3=SET TIME/DATE S2=SKIP";
char TIMEHR[]=" SET HOUR";
char TIMEMIN[]=" SET MINUTE";
char TIMESEC[]=" SET SECOND";
char TIMEMODE[]=" SET AM/PM";
char DATE[]=" SET DATE(y/m/d)";
char TIMED[]=" TIME DATE        DAY";
char DAY[]="SUNMONTUEWEDTHRFRISAT ";
char MONTH[]="JANFEBMARAPRMAYJUNJLYAUGSEPOCTNOVDEC"; 
char DISPLAY[]=" SPEED   TEMP FUEL LEVEL(";
char OVER[]=" OVER LOAD";
int a[]={0,0,3,3,6,1,4,6,2,5,0,3,5};
char TIMESTORE[12];
unsigned char i,j,s=1,r=1,select=0,mode=0,daycount=1,monthcount=1,k=0;
int yearcount=2010,rem,t=0,speed,temp,fuel,l,hrcount=0,mincount=0,seccount=0,dup1;

void Interrupt_External (void);
void Interrupt_timer (void);
#pragma code H_vector = 0x08		// External Interrupt high priority
void H_vector(void) 
{  _asm   
	goto Interrupt_External _endasm
}
#pragma code
#pragma interrupt Interrupt_External


#pragma code L_vector = 0x18		//Timer Interrupt low priority
void L_vector(void)
{_asm    
		goto Interrupt_timer    
		_endasm
}
#pragma code
#pragma interruptlow Interrupt_timer

void Interrupt_External(void) 							//External ISR
{  
	if (INTCONbits.INT0IF)  		{        	display_data();								//to display time & date
			seccount=seccount+6;
        	INTCONbits.INT0IF=0;             
        }
	
	if (INTCON3bits.INT1IF)  		{          	Init_LCD();
			for(i=13;i<DISPLAY[i]!=0;i++)
			W_data_8bit(DISPLAY[i]);
			dup1=fuel*0.0977;
   			itoa(dup1,TIMESTORE);				// C18 function  itoa is used to convert integer  
			for(i=0;i<TIMESTORE[i]!=0;i++)
			W_data_8bit(TIMESTORE[i]);
			W_data_8bit(0b00100101);
			W_data_8bit(')');
			W_ctr_8bit(0b11000000);
			dup1=16*dup1*0.01;
			for(i=0;i<dup1;i++)
			W_data_8bit(0xff);
			Delay10KTCYx(200);
			seccount=seccount+2;
        	INTCON3bits.INT1IF=0;             
        }}


void Interrupt_timer(void)							//Timer ISR
{
	
	if (INTCONbits.TMR0IF)
	  {
			AtoD();
			manipulation();
			calender();
			if(coolentpump)
			{
		PR2 = 124;			// Set PR2 = 249 for 1ms 
 		CCPR1L = 0b00111110;	// CCPR1L:CCP1CON<5:4> = 500	
 		CCP1CON = 0b00101111;	// DC1B1 & DC1B0 = 0, PWM mode
			}
		else
			CCP1CON=0;
			

 	}
	INTCONbits.TMR0IF=0;		
}


void main ()
{
	void FIRST[]="ENGIN MONITORING";
	void WELCOME[]="ENGINE STARTING";
	ADCON0 	= 0b00000001;			// Fosc/8, A/D enabled, A/D result left justified
	ADCON1	= 0b00001100;			// 3 analog channels
	ADCON2	= 0b10000100;			
	RCONbits.IPEN = 1;
	INTCONbits.GIEH = 0;				//disable all interrupts   	INTCONbits.GIEL = 0;
	INTCON2bits.TMR0IP = 0;				// low priority 
	INTCON3bits.INT1IP=1;	
	INTCONbits.INT0IF=0;				// Clear interrupt flag
	INTCON3bits.INT1IF=0;				// Clear interrupt flag
	TRISD=0b00000000;
	TRISC=0b00000000;
	T2CON = 0b00000101;	// Timer 2 On, postscaler = 1:1, prescaler = 1:4
	TRISAbits.RA5=1;
	TRISBbits.RB5=0;
	TRISBbits.RB3=1;
	TRISBbits.RB2=1;
	TRISCbits.RC1=0;
	RED=0;
	YELLOW=0;
	Init_LCD();
	for(i=0;i<FIRST[i]!=0;i++)
    W_data_8bit(FIRST[i]);
	while(touch);
	while(!touch);
	beep();
    Init_LCD();
	for(i=0;i<WELCOME[i]!=0;i++)
    W_data_8bit(WELCOME[i]);
	Delay10KTCYx(100);
	Init_LCD();
	for(i=0;i<17;i++)
    W_data_8bit(TIME[i]);
 	W_ctr_8bit(0b11000000);
	for(i=18;i<TIME[i]!=0;i++)
    W_data_8bit(TIME[i]);
	while(S3&&S2);					
	while(!S3||!S2) //check any switch pressed?
	{
	if(!S3)		
	{
		Delay10KTCYx(50);
		if(S3)
		select=1;
		beep();
	}
	if(!S2)
	{
		Delay10KTCYx(50);
		if(S2)
		select=2;
		beep();	
	}
	}

	switch(select)
{
	case 1:
	prog();
break;
case 2:
	Init_LCD();
    for(i=0;i<WELCOME[i]!=0;i++)
    W_data_8bit(WELCOME[i]);
	Delay10KTCYx(200);
break;
default:
break;
} //End of switch

	TMR0H = 0x0B; 
	TMR0L = 0xDC; 		// TRM0H:L = 0x0BDC 
    T0CON = 0b10000011; 
    INTCON = 0b11110000;
	INTCON3bits.INT1IE=1;
while(1)
{
}

} // End of main



void Init_LCD()					/* LCD display initialization */
{
W_ctr_8bit(0b00111000); 		// Function Set - 8-bit, 2 lines, 5X7
W_ctr_8bit(0b00001100); 		// Display on, cursor on
W_ctr_8bit(0b00000110); 		// Entry mode - inc addr, no shift
W_ctr_8bit(0b00000001); 		// Clear display
W_ctr_8bit(0b00000010); 		// Return cursor to home position
}

void W_ctr_8bit(char x)			/* Write control word to LCD */
{
LCD_RS = 0; 					
LCD_E = 1;						
LCD_DATA = x;
LCD_E = 0; 						
Delay10TCYx(50); 				// 1ms delay
}

void W_data_8bit(char x)		/* Write text data to LCD */
{
LCD_RS = 1; 
LCD_E = 1; 
LCD_DATA = x;
LCD_E = 0; 
Delay10TCYx(50); 				// 1ms delay
}
	

void prog()
{
while(r)
{	 
		while(s)
			{	
				Init_LCD();
				for(i=0;i<TIMEHR[i]!=0;i++)
	    		W_data_8bit(TIMEHR[i]);
				W_ctr_8bit(0b11000000);
				itoa(hrcount,TIMESTORE);
				for(i=0;i<TIMESTORE[i]!=0;i++)
				W_data_8bit(TIMESTORE[i]);
				while(S2&&S3&&S7);					
				while(!S2||!S3||!S7)	//check any switch pressed?
				{
				beep();
				if(!S3)
					{
					Delay10KTCYx(40);
					if(S3)
					{
					hrcount++;
					if(hrcount>11)
					hrcount=0;

					}
					}
				if(!S2)
					{
					Delay10KTCYx(50);
					if(S2)
					s=0;
					}
	
				if(!S7)
					{
					Delay10KTCYx(40);
					if(S7)
					{
					hrcount--;
					if(hrcount<0)
					hrcount=11;
					}
					}
					}
			}
			s=1;
		while(s)
			{	
				Init_LCD();
				for(i=0;i<TIMEMIN[i]!=0;i++)
	    		W_data_8bit(TIMEMIN[i]);
				W_ctr_8bit(0b11000000);
				itoa(mincount,TIMESTORE);
				for(i=0;i<TIMESTORE[i]!=0;i++)
				W_data_8bit(TIMESTORE[i]);	
				while(S3&&S2&&S7);					
				while(!S3||!S2||!S7)	//check any switch pressed?
				{
					beep();
				if(!S3)
					{
					Delay10KTCYx(40);
					if(S3)
					{
					mincount++;
					if(mincount>59)
					mincount=0;
					}
					}
				if(!S2)
					{
					Delay10KTCYx(40);
					if(S2)
					s=0;
					}

				if(!S7)
					{
					Delay10KTCYx(40);
					if(S7)
					{
					mincount--;
					if(mincount<0)
					mincount=59;
					}	
				}
			}
			}
			s=1;
			while(s)
			{	
				Init_LCD();
				for(i=0;i<TIMESEC[i]!=0;i++)
	    		W_data_8bit(TIMESEC[i]);
				W_ctr_8bit(0b11000000);
				itoa(seccount,TIMESTORE);
				for(i=0;i<TIMESTORE[i]!=0;i++)
				W_data_8bit(TIMESTORE[i]);	
				while(S3&&S2&&S7);					
				while(!S3||!S2||!S7)	//check any switch pressed?
				{	
					beep();
				if(!S3)
					{
					Delay10KTCYx(40);
					if(S3)
					{
					seccount++;
					if(seccount>59)
					seccount=0;
					}
					}
				if(!S2)
					{
					Delay10KTCYx(40);
					if(S2)
					s=0;
					}

				if(!S7)
					{
					Delay10KTCYx(40);
					if(S7)
					{
					seccount--;
					if(seccount<0)
					seccount=59;
					}	
				}	
			}
			}
			s=1;
			while(s)
			{	
				Init_LCD();
				for(i=0;i<TIMEMODE[i]!=0;i++)
	    		W_data_8bit(TIMEMODE[i]);
				W_ctr_8bit(0b11000001);
					if(!mode)
						{
						W_data_8bit('A');
						W_data_8bit('M');
						}
					else
						{
						W_data_8bit('P');
						W_data_8bit('M');
						}
				while(S3&&S2);					
				while(!S3||!S2)	//check any switch pressed?
				{
					beep();
				if(!S3)
					{
					Delay10KTCYx(40);
					if(S3)
					{
					mode=~mode;
					}
					}
				if(!S2)
					{
					Delay10KTCYx(40);
					if(S2)
					s=0;
					}	
				}	
			}
		s=1;
		while(s)
			{	
				Init_LCD();
				for(i=0;i<DATE[i]!=0;i++)
	    		W_data_8bit(DATE[i]);
				W_ctr_8bit(0b11000001);
				itoa((yearcount/100),TIMESTORE);
				for(i=0;i<TIMESTORE[i]!=0;i++)
				W_data_8bit(TIMESTORE[i]);	
				itoa((yearcount%100),TIMESTORE);
				if((yearcount%100)<10)
				{	
					W_data_8bit('0');	
					for(i=0;i<TIMESTORE[i]!=0;i++)
					W_data_8bit(TIMESTORE[i]);

				}
				else
				{	
				for(i=0;i<TIMESTORE[i]!=0;i++)
				W_data_8bit(TIMESTORE[i]);	
				}
				W_data_8bit(':');
				for(i=(monthcount-1)*3;i<((monthcount-1)*3)+3;i++)
				W_data_8bit(MONTH[i]);	
				W_data_8bit(':');
				itoa(daycount,TIMESTORE);
				for(i=0;i<TIMESTORE[i]!=0;i++)
				W_data_8bit(TIMESTORE[i]);	
				while(S3&&S2&&S7);					
				while(!S3||!S2||!S7)	//check any switch pressed?
				{
					beep();
				if(!S3)
					{
					Delay10KTCYx(40);
					if(S3)
					{
					if(k==0)
					yearcount++;
					else if(k==1)
					monthcount++;
					else
					daycount++;

					
					if((yearcount%4==0)&&(monthcount==2))
						if(daycount>29)
							daycount=1;
					if((yearcount%4!=0)&&(monthcount==2))
						if(daycount>28)
							daycount=1;
					if(monthcount==1||monthcount==3||monthcount==5||monthcount==7||monthcount==8||monthcount==10||monthcount==12)
						if(daycount>31)
							daycount=1;
					if(monthcount==4||monthcount==6||monthcount==9||monthcount==11)
						if(daycount>30)
							daycount=1;
					if(monthcount>12)
						monthcount=1;
					}
					}
				if(!S2)
					{
					Delay10KTCYx(40);
					if(S2)
					{
					k++;
					if(k==3)
					{
					s=0;
					r=0;
					}
					}
					}

				if(!S7)
					{
					Delay10KTCYx(40);
					if(S7)
					{
					if(k==0)
					yearcount--;
					else if(k==1)
					monthcount--;
					else
					daycount--;

					
					if((yearcount%4==0)&&(monthcount==2))
						if(daycount<1)
							daycount=29;
					if((yearcount%4!=0)&&(monthcount==2))
						if(daycount<1)
							daycount=28;
					if(monthcount==1||monthcount==3||monthcount==5||monthcount==7||monthcount==8||monthcount==10||monthcount==12)
						if(daycount<1)
							daycount=31;
					if(monthcount==4||monthcount==6||monthcount==9||monthcount==11)
						if(daycount<1)
							daycount=30;
					if(monthcount<1)
						monthcount=12;

					}	
				}	
			}
	}
}
}

void display_data()
{
		Init_LCD();
		for(i=0;i<5;i++)
    	W_data_8bit(TIMED[i]);
		W_ctr_8bit(0b11000000);
		itoa(hrcount,TIMESTORE);
		for(i=0;i<TIMESTORE[i]!=0;i++)
		W_data_8bit(TIMESTORE[i]);
		W_data_8bit(':');
		itoa(mincount,TIMESTORE);
		for(i=0;i<TIMESTORE[i]!=0;i++)
		W_data_8bit(TIMESTORE[i]);
		W_data_8bit(':');
		itoa(seccount,TIMESTORE);
		for(i=0;i<TIMESTORE[i]!=0;i++)
		W_data_8bit(TIMESTORE[i]);
		W_data_8bit(':');
		if(!mode)
	{	
		W_data_8bit('A');
		W_data_8bit('M');
	}
		else
	{	
		W_data_8bit('P');
		W_data_8bit('M');
	}
	Delay10KTCYx(200);
	Delay10KTCYx(100);
	Init_LCD();
				for(i=5;i<21;i++)
		    	W_data_8bit(TIMED[i]);
				W_ctr_8bit(0b11000000);
				itoa((yearcount/100),TIMESTORE);
				for(i=0;i<TIMESTORE[i]!=0;i++)
				W_data_8bit(TIMESTORE[i]);	
				itoa((yearcount%100),TIMESTORE);
				for(i=0;i<TIMESTORE[i]!=0;i++)
				W_data_8bit(TIMESTORE[i]);	
				W_data_8bit(':');
				for(i=(monthcount-1)*3;i<((monthcount-1)*3)+3;i++)
				W_data_8bit(MONTH[i]);
				W_data_8bit(':');
				itoa(daycount,TIMESTORE);
				for(i=0;i<TIMESTORE[i]!=0;i++)
				W_data_8bit(TIMESTORE[i]);
				W_data_8bit(' ');
				rem=((((yearcount%100)/4)+(yearcount%100)+a[monthcount]+daycount)%7)-1;
				for(i=rem*3;i<(rem*3)+3;i++)
				W_data_8bit(DAY[i]);
				Delay10KTCYx(200);
				Delay10KTCYx(100);
}
	

void beep(void)
	{
	for(i=0;i<90;i++)
	{
	buzzer=~buzzer;
	Delay10TCYx(20);
	}
	}

void AtoD(void)
{

			Init_LCD();									// Init LCD 4-bit interface, multiple line
			for(i=0;i<13;i++)
			W_data_8bit(DISPLAY[i]);
	for(j=0;j<2;j++)
			{
			for(l=0;l<2;l++)
			{
			ADCON0bits.CHS1 =j;
			ADCON0bits.CHS0 =l;
		if(!j&&!l)
			{	
			Delay10KTCYx(1);			// Acquisition time (>20us)		
			ADCON0bits.GO = 1;			// Start ADC 
		while(ADCON0bits.DONE);		// ADC completed?
       		speed=ADRESH*256+ADRESL;
  			W_ctr_8bit(0b11000000);
			dup1=0.195*speed;
   			itoa(dup1,TIMESTORE);				// C18 function  itoa is used to convert integer  
			for(i=0;i<TIMESTORE[i]!=0;i++)
			W_data_8bit(TIMESTORE[i]);
			W_data_8bit('K');
			W_data_8bit('m');
			W_data_8bit('/');
			W_data_8bit('h');


			}
		else if(!j&&l)
			{	
			Delay10KTCYx(1);			// Acquisition time (>20us)		
			ADCON0bits.GO = 1;			// Start ADC 
		while(ADCON0bits.DONE);		// ADC completed?
       		temp=ADRESH*256+ADRESL;
			W_ctr_8bit(0b11001000);
			dup1=0.097*temp;
			itoa(dup1,TIMESTORE);				// C18 function  itoa is used to convert integer  
			for(i=0;i<TIMESTORE[i]!=0;i++)
			W_data_8bit(TIMESTORE[i]);
			W_data_8bit(0b11011111);
			W_data_8bit('C');
	
			}	
		else if(j&&!l)
			{	
			Delay10KTCYx(1);			// Acquisition time (>20us)		
			ADCON0bits.GO = 1;			// Start ADC 
		while(ADCON0bits.DONE);		// ADC completed?
       		fuel=ADRESH*256+ADRESL;			}
		else
{
}
			}
			}
}

void calender(void)
{
seccount++;
		if(seccount>59)
		{
			seccount=0;
			mincount++;
		}
		if(mincount==60)
		{
			mincount=0;
			hrcount++;
		}
		if(hrcount>11)
			hrcount=0;
		if(hrcount==0&&mincount==0&&seccount==0)
			mode=~mode;
		if(mode==0&&hrcount==0&&mincount==0&&seccount==0)
			daycount++;
		
		if((yearcount%4==0)&&(monthcount==2))
						if(daycount>29)
							{
							daycount=1;
							monthcount++;
							}
		if((yearcount%4!=0)&&(monthcount==2))
						if(daycount>28)
							{
							daycount=1;
							monthcount++;
							}
		if(monthcount==1||monthcount==3||monthcount==5||monthcount==7||monthcount==8||monthcount==10||monthcount==12)
						if(daycount>31)
							{
							daycount=1;
							monthcount++;
							}
		if(monthcount==4||monthcount==6||monthcount==9||monthcount==11)
						if(daycount>30)
							{
							daycount=1;
							monthcount++;
							}
		if(monthcount>12)
			{
				yearcount++;
				monthcount=1;
			}


}

void manipulation(void)
{
if(speed>600)
{
Init_LCD();
for(i=0;i<OVER[i]!=0;i++)
W_data_8bit(OVER[i]);
RED=1;
PR2 = 249;			// Set PR2 = 249 for 1ms 
CCPR1L = 0b01111111;	// CCPR1L:CCP1CON<5:4> = 500
CCP1CON = 0b00001111;	// DC1B1 & DC1B0 = 0, PWM mode
RED=0;
}
else
CCP1CON = 0;
if(fuel<50)
{
PR2 = 249;			// Set PR2 = 249 for 1ms 
CCPR1L = 0b01111111;	// CCPR1L:CCP1CON<5:4> = 500
CCP1CON = 0b00001111;	// DC1B1 & DC1B0 = 0, PWM mode
RED=1;
beep();
Delay10KTCYx(10);
RED=0;
}
if(fuel<100&&fuel>50)
{
CCP1CON = 0;
YELLOW=0;
RED=1;
beep();
Delay10KTCYx(50);
beep();
}
else 
RED=0;


if(fuel<200&&fuel>100)
{
RED=0;
YELLOW=1;
beep();
}
else
YELLOW=0;

}


