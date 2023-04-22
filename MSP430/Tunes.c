#include <msp430.h>
#include "GPIO_Driver.h"
#include "FR2355_UART_Driver.h"

unsigned long int  counter;

unsigned short freq;

unsigned long int waveTest[128] = {0xfa,0x106,0x113,0x11f,0x12b,0x137,0x143,0x14e,
                                   0x15a,0x165,0x170,0x17b,0x185,0x18f,0x199,0x1a2,
                                   0x1ab,0x1b3,0x1bb,0x1c3,0x1ca,0x1d0,0x1d6,0x1dc,
                                   0x1e1,0x1e5,0x1e9,0x1ed,0x1ef,0x1f1,0x1f3,0x1f4,
                                   0x1f4,0x1f4,0x1f3,0x1f1,0x1ef,0x1ed,0x1e9,0x1e5,
                                   0x1e1,0x1dc,0x1d6,0x1d0,0x1ca,0x1c3,0x1bb,0x1b3,
                                   0x1ab,0x1a2,0x199,0x18f,0x185,0x17b,0x170,0x165,
                                   0x15a,0x14e,0x143,0x137,0x12b,0x11f,0x113,0x106,
                                   0xfa,0xee,0xe1,0xd5,0xc9,0xbd,0xb1,0xa6,
                                   0x9a,0x8f,0x84,0x79,0x6f,0x65,0x5b,0x52,
                                   0x49,0x41,0x39,0x31,0x2a,0x24,0x1e,0x18,
                                   0x13,0xf,0xb,0x7,0x5,0x3,0x1,0x0,
                                   0x0,0x0,0x1,0x3,0x5,0x7,0xb,0xf,
                                   0x13,0x18,0x1e,0x24,0x2a,0x31,0x39,0x41,
                                   0x49,0x52,0x5b,0x65,0x6f,0x79,0x84,0x8f,
                                   0x9a,0xa6,0xb1,0xbd,0xc9,0xd5,0xe1,0xee,
};

// Text that prints to terminal when uart connects
char connectedText[] = {'S','u','c','c','e','s','s','f','u','l','l','y',' ','C','o','n','n','e','c','t','e','d'};
char instructionText[] = {'P','r','e','s','s',' ','k','e','y','s',' ','0','-','9',' ','t','o',' ','c','h','a','n','g','e',' ','t','h','e',' ','n','o','t','e'};

void Timer_Init();
void Reference_Init();
void GPIO_Init();
void UART_Init();

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watch dog timer

  GPIO_Init();
  UART_Init();
  uart_Init_9600();          // Does NOT initialize the GPIO Pins

  PM5CTL0 &= ~LOCKLPM5;                     // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

  Reference_Init();
  Timer_Init();
  uart_Print(connectedText);
  uart_Print(instructionText);

  __bis_SR_register(LPM3_bits + GIE);        // Enter LPM3, Enable Interrupt
  __enable_interrupt();
  __no_operation();
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = SAC0_SAC2_VECTOR
__interrupt void SAC0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(SAC0_SAC2_VECTOR))) SAC0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(SAC0IV,SACIV_4))
  {
    case SACIV_0: break;
    case SACIV_2: break;
    case SACIV_4:
        SAC0DAT=waveTest[counter];
        waveTest[counter]&=0xFFF;
        counter += 1;
        if(counter == 128)         // If counter is at the end of the array
        {
            counter = 0;            // Reset counter
        }
        break;
    default: break;
  }
}

void GPIO_Init() {
    P1SEL0 |= BIT1;                           // Select P1.1 as OA0O function
    P1SEL1 |= BIT1;                           // OA is used as buffer for DAC
}

void Timer_Init() {
    // Use TB2.1 as DAC hardware trigger
    TB2CCR0 = 5;                           // PWM Period/2
    TB2CCTL1 = OUTMOD_6;                       // TBCCR1 toggle/set
    TB2CCR1 = 1;                              // TBCCR1 PWM duty cycle
    TB2CTL = TBSSEL__SMCLK | MC_1 | TBCLR;     // SMCLK, up mode, clear TBR
//    counter = 0;
//    TB2EX0 = TBIDEX_7;
//    TB2CTL |= TBIE;
}

void Reference_Init() {
    // Configure reference module
    PMMCTL0_H = PMMPW_H;                      // Unlock the PMM registers
    PMMCTL2 = INTREFEN | REFVSEL_2;           // Enable internal 2.5V reference
    while(!(PMMCTL2 & REFGENRDY));            // Poll till internal reference settles

    SAC0DAC = DACSREF_1 + DACLSEL_2 + DACIE ;  // Select int Vref as DAC reference
    SAC0DAT = waveTest[counter];                  // Initial DAC data
    SAC0DAC |= DACEN;                         // Enable DAC

    SAC0OA = NMUXEN + PMUXEN + PSEL_1 + NSEL_1;//Select positive and negative pin input
    SAC0OA |= OAPM;                            // Select low speed and low power mode
    SAC0PGA = MSEL_1;                          // Set OA as buffer mode
    SAC0OA |= SACEN + OAEN;                    // Enable SAC and OA
}

void UART_Init() {
    P4DIR = 0xFF; P2DIR = 0xFF;
    P4REN = 0xFF; P2REN = 0xFF;
    P4OUT = 0x00; P2OUT = 0x00;
    // Configure UART pins
    P4SEL0 |= BIT2 | BIT3;                    // set 2-UART pin as second function
}

// Setup for a,b,c,d,e,f,g keys
//#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
//#pragma vector=USCI_A1_VECTOR
//__interrupt void USCI_A1_ISR(void)
//#elif defined(__GNUC__)
//void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
//#else
//#error Compiler not supported!
//#endif
//{
//  switch(__even_in_range(UCA1IV,USCI_UART_UCTXCPTIFG))
//  {
//    case USCI_NONE: break;
//    case USCI_UART_UCRXIFG:
//      while(!(UCA1IFG&UCTXIFG));
//      UCA1TXBUF = UCA1RXBUF;
//      switch(UCA1TXBUF)
//      {
//          case 0x61 :
//              freq = 141;
//              break;
//          case 0x62 :
//              freq = 127;
//              break;
//          case 0x63 :
//              freq = 239;
//              break;
//          case 0x64 :
//              freq = 208;
//              break;
//          case 0x65 :
//              freq = 189;
//              break;
//          case 0x66 :
//              freq = 179;
//              break;
//          case 0x67 :
//              freq = 159;
//              break;
//          default : freq = 500;
//      }
//      TB2CCR0 = freq;
//      __no_operation();
//      break;
//    case USCI_UART_UCTXIFG: break;
//    case USCI_UART_UCSTTIFG: break;
//    case USCI_UART_UCTXCPTIFG: break;
//    default: break;
//  }
//}

// Setup for 1,2,3,4,5,6,7,8,9,0 keys
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA1IV,USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
      while(!(UCA1IFG&UCTXIFG));
      UCA1TXBUF = UCA1RXBUF;
      switch(UCA1TXBUF)
      {
          case 0x31 :
              freq = 239;
              break;
          case 0x32 :
              freq = 208;
              break;
          case 0x33 :
              freq = 189;
              break;
          case 0x34 :
              freq = 179;
              break;
          case 0x35 :
              freq = 159;
              break;
          case 0x36 :
              freq = 141;
              break;
          case 0x37 :
              freq = 127;
              break;
          case 0x38 :
              freq = 101;
              break;
          case 0x39 :
              freq = 70;
              break;
          case 0x30 :
              freq = 15;
              break;
          default : freq = 500;
      }
      TB2CCR0 = freq;
      __no_operation();
      break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
    default: break;
  }
}
