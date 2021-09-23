/*
 * DSP33F V1
 * 
 *      - configuration of the Oscillator (extern)
 *      - configuration of the ADC module
 *      - configuration of the SPI module and connection of an LCD
 *      - configuration of an interruption from Timer 1
 *      - signal filtering (running average filter)
 * 
 * Microcontroller: DSPIC33FJ32MC204
 * Proteus File:    DSP_V0.pdsprj
 * Author:          Samuel CIULLA
 *
 * Created on June 2021
 */ 


#include <xc.h>
#include "config.h"
#include "SPI_LCD.h"
#define FCY 10000000         //  Fin = 10 MHz
#include <libpic30.h>


/*------------------------------------------------------------------------------
 * Global variables
 *----------------------------------------------------------------------------*/
int bufX[12]={0}, bufY[12]={0}, steps=0;


/*------------------------------------------------------------------------------
 * Prototypes
 *----------------------------------------------------------------------------*/
void PLL_config(void);
void PIN_config(void);
void ADC_config(void);
void TIMER_config(void);
void PPS_config(void);
void SPI_config(void);
void LCD_display(void);
void running_average_filter(void);
void init_running_average_filter(void);



/*------------------------------------------------------------------------------
 * Interruption
 *----------------------------------------------------------------------------*/
void __attribute__((interrupt(auto_psv))) _T1Interrupt(void) {
    
    TMR1 = 0xFF00;                  // set Timer register
    
    LATCbits.LATC9 = 1;             // TICK ON
    
    AD1CON1bits.SAMP = 1;           // start sampling
    AD1CON1bits.SAMP = 0;           // finish sampling 
    while(!AD1CON1bits.DONE);       // conversion done
    
    running_average_filter();       // apply filter
    
    LATCbits.LATC9 = 0;             // TICK OFF
    
    _T1IF = 0;                      // reset T1 flag
}



/*------------------------------------------------------------------------------
 * Main program
 *----------------------------------------------------------------------------*/
int main(void) {
    
    PPS_config();                   // SPI peripheral pins configuration
    PLL_config();                   // frequency oscillator
    PIN_config();                   // I/O and analogic pins
    ADC_config();                   // ADC converter
    SPI_config();                   // SPI configuration
    TIMER_config();                 // Timer
    Init_Alpha_LCD();               // LCD initialisation
    
    init_running_average_filter();  // read parameters (running average steps)
    
    LCD_display();                  // display title and parameters

    while (1);

    return 0;
}



/*------------------------------------------------------------------------------
 * Filter the input signal using running average and output to PORTB (DAC1208)
 *----------------------------------------------------------------------------*/
void running_average_filter(void) {
    
    switch (steps) {
        case 2:
            bufX[0] = ADC1BUF0;
            bufY[0] = (bufX[0] + bufX[1]) / steps;
            bufX[1] = bufX[0];
            LATB = bufY[0];
            break;
        case 3:
            bufX[0] = ADC1BUF0;
            bufY[0] = (bufX[0] + bufX[1] + bufX[2]) / steps;
            bufX[2] = bufX[1];
            bufX[1] = bufX[0];
            LATB = bufY[0];
            break;
        case 4:
            bufX[0] = ADC1BUF0;
            bufY[0] = (bufX[0] + bufX[1] + bufX[2] + bufX[3]) / steps;
            bufX[3] = bufX[2];
            bufX[2] = bufX[1];
            bufX[1] = bufX[0];
            LATB = bufY[0];
            break;
        case 5:
            bufX[0] = ADC1BUF0;
            bufY[0] = (bufX[0] + bufX[1] + bufX[2] + bufX[3] + bufX[4]) / steps;
            bufX[4] = bufX[3];
            bufX[3] = bufX[2];
            bufX[2] = bufX[1];
            bufX[1] = bufX[0];
            LATB = bufY[0];
            break;
        default:
            LATB = 0;
            break;
    }
    
}



/*------------------------------------------------------------------------------
 * Running average filter initialisation
 * Reads parameters from last 4 bits of PORTB
 *----------------------------------------------------------------------------*/
void init_running_average_filter(void) {
    
    if      ((PORTB >> 12 & 0x000F) == 0)     steps = 2;
    else if ((PORTB >> 12 & 0x000F) == 1)     steps = 3;
    else if ((PORTB >> 12 & 0x000F) == 2)     steps = 4;
    else if ((PORTB >> 12 & 0x000F) == 3)     steps = 5;
}



/*------------------------------------------------------------------------------
 * ADC configuration
 *----------------------------------------------------------------------------*/
void ADC_config(void) {
    
    AD1PCFGL = 0xFFFE;              // all digital except AN0
    
    AD1CON1bits.AD12B = 1;          // 12 bit operation mode (4096 values)
    AD1CON1bits.FORM = 0;           // Data output format: Integer
    AD1CON1bits.ASAM = 0;           // Start when SAMP is set
    AD1CON1bits.SSRC = 0;           // Conversion start triggered manually
    AD1CON2bits.VCFG = 0;           // Voltage Vref+=AVDD Vref-=AVSS
    AD1CON2bits.CHPS = 0;           // Channel select - CH0 only (12 bit mode)
    AD1CON3bits.ADRC = 0;           // ADC clock source from system clock
    AD1CON3bits.ADCS = 2;           // TCY · (ADCS<7:0> + 1) = 3 · TCY = TAD
    AD1CHS0bits.CH0NA = 0;          // Channel 0 negative input is VREF-
    AD1CHS0bits.CH0SA = 0;          // Channel 0 positive input is AN0
    
    AD1CON1bits.ADON = 1;           // enable ADC
}



/*------------------------------------------------------------------------------
 * Pins configuration
 *----------------------------------------------------------------------------*/
void PIN_config(void) {
    
    TRISB = 0xF000;                 // PORTB 0-11 OUTPUT and 12-15 INPUT
    TRISCbits.TRISC9 = 0;           // tick interruption
}



/*------------------------------------------------------------------------------
 * PLL configuration
 * => Frequency Oscillator = (Fin x (M/(N1*N2))) / 2
 *----------------------------------------------------------------------------*/
void PLL_config(void) {
    
    PLLFBDbits.PLLDIV = 0x06;       // M=8
    CLKDIVbits.PLLPRE = 0;          // N1=2
    CLKDIVbits.PLLPOST = 0;         // N2=2
}



/*------------------------------------------------------------------------------
 * Timer configuration
 *----------------------------------------------------------------------------*/
void TIMER_config(void) {
    
    T1CON = 0x8000;         // Timer 1 ON, pre-scaler 1:1, internal clock
    _T1IF = 0;              // reset T1 flag
    _T1IP = 0x001;          // set interrupt priority (lowest)
    TMR1 = 0xFF00;          // configure Timer register
    _T1IE = 1;              // enable
}


/*------------------------------------------------------------------------------
 * SPI configuration
 *----------------------------------------------------------------------------*/
void SPI_config(void) {
    
    IFS0bits.SPI1IF = 0;            // reset SPI flag
    IEC0bits.SPI1IE = 0;            // disable SPI 
    
    SPI1CON1 = 0x0000;              // clean SPI1 control register
    SPI1CON1bits.CKE = 1;           // Serial output data on transition from 
                                    // active clock state to Idle clock state
    SPI1CON1bits.MODE16 = 0;        // mode 8 bits
    SPI1CON1bits.MSTEN = 1;         // master mode
    SPI1CON1bits.SPRE = 6;          // Secondary pre-scale 2:1
    SPI1CON1bits.PPRE = 2;          // Primary pre-scale 4:1
    
    SPI1CON2 = 0x0000;              // clean SPI1 control 2 register
    
    SPI1STAT = 0x0000;              // clean SPI status and control register
    SPI1STATbits.SPIEN = 1;         // SPI enabled
}


/*------------------------------------------------------------------------------
 * Peripheral Pin Selection
 *----------------------------------------------------------------------------*/
void PPS_config(void) {
    
    __builtin_write_OSCCONL(OSCCON & ~(1<<6));  // Unlock registers
    
    RPINR20bits.SCK1R = 20;                     // SPI input pin setup
    RPOR10bits.RP21R = 7;                       // SPI output pin setup
    RPOR11bits.RP23R = 8;                       // SPI SCK pin setup
    
    __builtin_write_OSCCONL(OSCCON | (1<<6));   // Lock registers
}



/*------------------------------------------------------------------------------
 * Set display data
 *----------------------------------------------------------------------------*/
void LCD_display(void) {
    
    char txt[21];
    
    Send_Cmd_LCD(LCD_CLEAR);
    Send_Cmd_LCD(LCD_CUR_OFF);
    Send_Txt_LCD("DSPIC33FJ32MC204", 1);
    Send_Txt_LCD("Running avg. filter", 2);
    
    switch (steps) {
        case 2 ... 5:
            sprintf(txt, "Steps: %d", steps);
            Send_Txt_LCD(txt, 4);
            break;
        default:
            Send_Txt_LCD("Signal not filtered", 4);
            break;
    }
}