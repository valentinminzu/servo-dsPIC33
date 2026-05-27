#include "p33Fxxxx.h"
double valoare = 0;
float puls = 1.0;
int directie = 1;
int front = 1;

_FOSCSEL(FNOSC_FRC);
_FOSC(FCKSM_CSECMD & OSCIOFNC_OFF);

// Subpunctul 1 (achizitii AD pe canal AN4(RB2) la 0,1 s)
void initAdc1(void)
{
 AD1CON1bits.AD12B = 1; // conversie AD pe 12 biti
 AD1CON1bits.SSRC = 2; // timerul 3 starteazã conversia
 AD1CON1bits.ASAM = 1; // începe esantionarea dupã terminarea unei conversii
 AD1CON2bits.CSCNA = 1; // scaneazã intrãrile pe CH0+

 AD1CON3bits.ADRC = 0; // foloseste ceasul de magistralã
 AD1CON3bits.ADCS = 63; // Tad=Tcy*(adcs+1)=25ns*64=1.6us

 AD1CSSLbits.CSS4 = 1; // este scanatã intrarea analogicã AN4(RB2)
 AD1PCFGL=0xFFFF; // seteazã pinii portului ADC1 ca fiind digitali
 AD1PCFGLbits.PCFG4 = 0; // seteazã pinul AN4(RB2) ca intrare analogicã

 _AD1IF = 0; // reseteaza flag-ul întreruperii convertorului AD
 _AD1IE = 1; // permite întreruperea convertorului AD

 AD1CON1bits.ADON = 1; // porneste convertorul AD
} 

// rutina de tratare a întreruperii convertorului AD
void __attribute__((interrupt, no_auto_psv)) _ADC1Interrupt(void)
{
	_RB15 = ~_RB15;
	valoare = ADC1BUF0 * (3.3/4096);
	_AD1IF = 0; // Achita intreruperea convertorului AD
}
//Final subpunct 1

void init_PWM1()
{
 P1DC1 = 0;
 P1DC2 = 461;
 P1TPER = 4612;		 

 PWM1CON1bits.PMOD1 = 1; // Canalele PWM1H si PWM1L sunt independente
 PWM1CON1bits.PMOD2 = 1; // Canalele PWM2H si PWM2L sunt independente
 PWM1CON1bits.PMOD3 = 0; // Canalele PWM3H si PWM3L sunt complementare

 PWM1CON1bits.PEN1H = 1; // Pinul PWM1H setat pe iesire PWM
 PWM1CON1bits.PEN1L = 0; // Pinul PWM1L setat pe I/O general purpose
 PWM1CON1bits.PEN2H = 1; // Pinul PWM1H setat pe iesire PWM
 PWM1CON1bits.PEN2L = 0; // Pinul PWM1L setat pe iesire PWM
 PWM1CON1bits.PEN3H = 1; // Pinul PWM1H setat pe iesire PWM
 PWM1CON1bits.PEN3L = 1; // Pinul PWM1L setat pe iesire PWM
 P1TCONbits.PTCKPS = 0b10;
 P1TCONbits.PTEN = 1;
}
void Init_Timer3( void )
{
 T3CON = 0;
 T3CONbits.TCKPS = 0b11; // Selectare prescaler 1:256
 TMR3= 0;
 PR3 = 1474;
 _T3IF = 0;
 _T3IP = 1;
 _T3IE = 1;
 T3CONbits.TON = 1;
}
void __attribute__ ((interrupt, no_auto_psv)) _T3Interrupt( void )
{
 _T3IF = 0; // Resetare flag de intrerupere
}

// Subpunctul 5 (comutare stare pin RB5)
void Init_Timer1( void )
{
 T1CON = 0;
 T1CONbits.TCKPS = 0b11; // Selectare prescaler 1:256
 TMR1= 0;
 PR1 = 14740;
 _T1IF = 0;
 _T1IP = 1;
 _T1IE = 1;
 T1CONbits.TON = 1;
}
void __attribute__ ((interrupt, no_auto_psv)) _T1Interrupt( void )
{
 _RB5 = ~_RB5;
 _RB13 = _RB5;
 _T1IF = 0; // Resetare flag de intrerupere
}

// Final subpunct 5

void delay(unsigned int t) 
{
	unsigned int x;
	unsigned int i;
	for(i=0;i<650;i++)
		for(x=0;x<t;x++);
}

int ms_to_dc(float ms)
{
	return (ms / 10) * P1TPER;
}

void verificare_buton()
{
	int curent = _RB7;
	if (curent == 0)
	{
		update_puls();
		delay(615);
	}
}
void __attribute__ ((interrupt, no_auto_psv)) _INT0Interrupt(void) {
	_INT0EP = ~_INT0EP;
	_INT0IF = 0;
}
void update_puls()
{
	puls += directie * 0.2;
	
	if(puls >= 2)
	{
		puls = 2;
		directie = -1;
	}
	else
	{
		if(puls <= 1)
		{
			puls = 1;
			directie = 1;
		}
	}
	P1DC2 = ms_to_dc(puls);
}


int main( void )
{
 TRISB = 0x0000;
 _TRISB7 = 1;
 PORTB = 0xF000;
 _TRISB2 = 1;
 Init_Timer3();
 Init_Timer1();
 init_PWM1();
 initAdc1();
 _INT0IF = 0;
 _INT0IE = 1;
 _INT0EP = 1;
 while(1)
 {	 
 
	P1DC1 = 461 + (valoare*(923-461)/3.3 );
	verificare_buton();
 }
}