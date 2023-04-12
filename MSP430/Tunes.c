#include <msp430.h>

//unsigned int DAC_data=0;
unsigned  long int  counter;
unsigned  long int wave33[128]={0x800,0x864,0x8c9,0x92d,0x990,0x9f2,0xa53,0xab2,
                               0xb10,0xb6c,0xbc5,0xc1d,0xc72,0xcc4,0xd13,0xd5f,
                               0xda8,0xded,0xe2f,0xe6d,0xea7,0xedd,0xf0e,0xf3b,
                               0xf64,0xf88,0xfa8,0xfc3,0xfd9,0xfea,0xff6,0xffe,
                               0x1000,0xffe,0xff6,0xfea,0xfd9,0xfc3,0xfa8,0xf88,
                               0xf64,0xf3b,0xf0e,0xedd,0xea7,0xe6d,0xe2f,0xded,
                               0xda8,0xd5f,0xd13,0xcc4,0xc72,0xc1d,0xbc5,0xb6c,
                               0xb10,0xab2,0xa53,0x9f2,0x990,0x92d,0x8c9,0x864,
                               0x800,0x79c,0x737,0x6d3,0x670,0x60e,0x5ad,0x54e,
                               0x4f0,0x494,0x43b,0x3e3,0x38e,0x33c,0x2ed,0x2a1,
                               0x258,0x213,0x1d1,0x193,0x159,0x123,0xf2,0xc5,
                               0x9c,0x78,0x58,0x3d,0x27,0x16,0xa,0x2,
                               0x0,0x2,0xa,0x16,0x27,0x3d,0x58,0x78,
                               0x9c,0xc5,0xf2,0x123,0x159,0x193,0x1d1,0x213,
                               0x258,0x2a1,0x2ed,0x33c,0x38e,0x3e3,0x43b,0x494,
                               0x4f0,0x54e,0x5ad,0x60e,0x670,0x6d3,0x737,0x79c,
};

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

unsigned long int wave131[32] = {          // Wave array, preset to values of sine
    128, 140, 152, 164, 173, 181, 187, 191,
    192, 191, 187, 181, 173, 164, 152, 140,
    128, 116, 104, 92, 83, 75, 69, 65,
    64, 65, 69, 75, 83, 92, 104, 116 };


int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watch dog timer

  P1SEL0 |= BIT1;                           // Select P1.1 as OA0O function
  P1SEL1 |= BIT1;                           // OA is used as buffer for DAC

  PM5CTL0 &= ~LOCKLPM5;                     // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

  // Configure reference module
  PMMCTL0_H = PMMPW_H;                      // Unlock the PMM registers
  PMMCTL2 = INTREFEN | REFVSEL_2;           // Enable internal 2.5V reference
  while(!(PMMCTL2 & REFGENRDY));            // Poll till internal reference settles

  SAC0DAC = DACSREF_1 + DACLSEL_2 + DACIE ;  // Select int Vref as DAC reference
 // SAC0DAT = wave33[counter];                  // Initial DAC data
  SAC0DAT = waveTest[counter];                  // Initial DAC data
  SAC0DAC |= DACEN;                         // Enable DAC

  SAC0OA = NMUXEN + PMUXEN + PSEL_1 + NSEL_1;//Select positive and negative pin input
  SAC0OA |= OAPM;                            // Select low speed and low power mode
  SAC0PGA = MSEL_1;                          // Set OA as buffer mode
  SAC0OA |= SACEN + OAEN;                    // Enable SAC and OA

  // Use TB2.1 as DAC hardware trigger
  TB2CCR0 = 1;                           // PWM Period/2
  TB2CCTL1 = OUTMOD_6;                       // TBCCR1 toggle/set
  TB2CCR1 = 1;                              // TBCCR1 PWM duty cycle
  TB2CTL = TBSSEL__SMCLK | MC_1 | TBCLR;     // SMCLK, up mode, clear TBR

  __bis_SR_register(LPM3_bits + GIE);        // Enter LPM3, Enable Interrupt
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
        //DAC_data++;
        //DAC_data &= 0xFFF;
        //SAC0DAT = DAC_data;   // DAC12 output positive ramp
//        SAC0DAT=wave33[counter];
//        wave33[counter]&=0xFFF;
        SAC0DAT=waveTest[counter];
        waveTest[counter]&=0xFFF;
        counter += 1;
        if ( counter == 128)         // If counter is at the end of the array
        {
            counter = 0;            // Reset counter
        }


        break;
    default: break;
  }
}
