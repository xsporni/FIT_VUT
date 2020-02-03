/**
    @author Alex Sporni
    Login: xsporn01
    Zmeny: prevzata kostra programu FITkit3-demo, nasledne modifikovana pre ucely projektu
    Podiel: 85%
    Datum: 22.12.2019
    Email: xsporn01@stud.fit.vutbr.cz
*/


#include "MK60D10.h"

/*
 * Makra na manipulovanie registrov,
 * Prevzaté z FITkit3-demo
 */
#define GPIO_PIN_MASK 0x1Fu
#define GPIO_PIN(x) (((1)<<(x & GPIO_PIN_MASK)))
#define SPK 0x10      	// speaker 	--> PTA4


/*
 * 7 GPIO pinov, pomocou ktorych klavesnica komunikuje s FITkitom
 */
int ROW[4] = {0x400, 0x8000000, 0x200, 0x800};
int COL[3] = {0x40, 0x100, 0x80};
int pressed = 0;
unsigned char tlacitko;


/*
 * Funkcia delay(), je prevzata z FITkit3-demo
 */
void delay(long long bound) {

  long long i;
  for(i=0;i<bound;i++);
}
/*
 * Funkcia beepDot(), je prevzata z 2. cvicenia, lahko modifikovana
 */
void beepDot(void) {
    for (uint32_t q=0; q<750; q++) {
        GPIOA_PDOR |=  SPK; delay(200);
        GPIOA_PDOR &= ~SPK; delay(200);
    }
}

/*
 * Funkcia beepDash(), je prevzata z 2. cvicenia, lahko modifikovana
 */
void beepDash(void) {
    for (uint32_t q=0; q<2250; q++) {
        GPIOA_PDOR |=  SPK; delay(200);
        GPIOA_PDOR &= ~SPK; delay(200);
    }
}

/*
 * Inicializacia MCU, nastavenie zakladneho clocku, vypnutie watchdog
 * funkcia MCUInit() je prevzata z FITkit3-demo
 */
void MCUInit(void)  {
    MCG_C4 |= ( MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(0x01) );
    SIM_CLKDIV1 |= SIM_CLKDIV1_OUTDIV1(0x00);
    WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;
}
/*
 * Inicializovanie GPIO pinov
 */
void PortsInit(void)
{
	SIM->SCGC5 = SIM_SCGC5_PORTA_MASK;		//Zapnutie clock portu
    PORTA->PCR[4] = PORT_PCR_MUX(0x01);  	//Reproduktor

    //Nastavenie korespondujúcich pinov
    int pulldown = (PORT_PCR_MUX(0x01) | PORT_PCR_PE(0x01) | PORT_PCR_PS(0x00));
    PORTA->PCR[6] = pulldown;				//PTA6	--> 25 --> COL1 --> zeleny		--> 0x40
    PORTA->PCR[7] = pulldown;  				//PTA7 	--> 27 --> COL3 --> fialovy		--> 0x80
    PORTA->PCR[8] = pulldown; 				//PTA8 	--> 23 --> COL2 --> oranzovy	--> 0x100
    PORTA->PCR[9] = PORT_PCR_MUX(0x01); 	//PTA9 	--> 28 --> ROW3 --> sivy		--> 0x200
    PORTA->PCR[10] = PORT_PCR_MUX(0x01);	//PTA10	--> 22 --> ROW1 --> zlty		--> 0x400
    PORTA->PCR[11] = PORT_PCR_MUX(0x01);    //PTA11	--> 26 --> ROW4 --> modry		--> 0x800
    PORTA->PCR[27] = PORT_PCR_MUX(0x01); 	//PTA27	--> 35 --> ROW2 --> biely		--> 0x8000000
    //Nastavenie korespondujúcich PTA pinov ako output (vystup)
    PTA->PDDR = GPIO_PDDR_PDD(SPK) | ROW[0] | ROW[1] | ROW[2] | ROW[3];
}
/*
 * Inicializovanie low power timeru, funkcia prevzata z FITkit3-demo
 */
void LPTMR0Init()
{
    SIM_SCGC5 |= SIM_SCGC5_LPTIMER_MASK; // Enable clock to LPTMR
    LPTMR0_CSR &= ~LPTMR_CSR_TEN_MASK;   // Turn OFF LPTMR to perform setup
    LPTMR0_PSR = ( LPTMR_PSR_PRESCALE(0) // 0000 is div 2
                 | LPTMR_PSR_PBYP_MASK   // LPO feeds directly to LPT
                 | LPTMR_PSR_PCS(1)) ;   // use the choice of clock
//    LPTMR0_CMR = count;                  // Set compare value
    LPTMR0_CSR =(  LPTMR_CSR_TCF_MASK    // Clear any pending interrupt (now)
                 | LPTMR_CSR_TIE_MASK    // LPT interrupt enabled
                );
    NVIC_EnableIRQ(LPTMR0_IRQn);         // enable interrupts from LPTMR0
}
/*
 * Hlavna funkcia, ktora vola pomocne
 */
int main(void)
{
	MCUInit();
	//LPTMR0Init();
    PortsInit();


    while (1)
    {
    	for (int rowIndex = 0; rowIndex < 4; rowIndex++)
    	{
    		PTA->PDOR = 0;
    		PTA->PDOR |= ROW[rowIndex];
    		for (int columnIndex = 0; columnIndex < 3; columnIndex++)
    		{
    			if (rowIndex == 0)
    			{	//		1		//
    				if (GPIOA_PDIR & COL[0])
    				{
    					;
    				}
    				//		A		//
    				else if (GPIOA_PDIR & COL[1])
    				{
    					//LPTMR0_CSR |= LPTMR_CSR_TEN_MASK;    // Turn ON LPTMR0 and start counting
//    					tlacitko = 'A';
//    					pressed++;
    					//LPTMR0->CSR = 0;
    					beepDot();
    					delay(1000000);
    					beepDash();
    				}
    				//		D		//
    				else if (GPIOA_PDIR & COL[2])
					{
						beepDash();
						delay(1000000);
						beepDot();
						delay(1000000);
						beepDot();
					}
    			}
    			else if (rowIndex == 1)
    			{
    				//		G		//
					if (GPIOA_PDIR & COL[0])
					{
						beepDash();
						delay(1000000);
						beepDash();
						delay(1000000);
						beepDot();
					}
					//		J		//
					else if (GPIOA_PDIR & COL[1])
					{
						beepDot();
						delay(1000000);
						beepDash();
						delay(1000000);
						beepDash();
						delay(1000000);
						beepDash();
						delay(1000000);
					}
					//		M		//
					else if (GPIOA_PDIR & COL[2])
					{
						beepDash();
						delay(1000000);
						beepDash();
					}
    			}
    			else if (rowIndex == 2)
    			{
    				//		P		//
    				if (GPIOA_PDIR & COL[0])
    				{
    					beepDot();
    					delay(1000000);
    					beepDash();
    					delay(1000000);
    					beepDash();
    					delay(1000000);
    					beepDot();
    					delay(1000000);
    				}
    				//		T		//
    				else if (GPIOA_PDIR & COL[1])
    				{
    					beepDash();
    					delay(1000000);
    				}
    				//		W		//
    				else if (GPIOA_PDIR & COL[2])
					{
    					beepDot();
    					delay(1000000);
						beepDash();
						delay(1000000);
						beepDash();
						delay(1000000);
					}
    			}
    			else if (rowIndex == 3)
    			{
    				//		*		//
    				if (GPIOA_PDIR & COL[0])
					{
						;
					}
    				//		OPER	//
    				else if (GPIOA_PDIR & COL[1])
    				{
    					;
    				}
    				//		#		//
    				else if (GPIOA_PDIR & COL[2])
					{
						;
					}
    			}
    		}
    	}
    }
    return 0;
}

