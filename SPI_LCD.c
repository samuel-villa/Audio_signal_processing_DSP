/*------------------------------------------------------------------------------
 * Simple LCD library for DSPIC33FJ32MC204
 * LM044L alpha LCD via MCP23S17 SPI IO extension
 * Samuel CIULLA - Microcontroleurs 2021
 *----------------------------------------------------------------------------*/
#include <xc.h>
#include "SPI_LCD.h"

/*------------------------------------------------------------------------------
 * Initialise SPI, MCP and LCD
 *----------------------------------------------------------------------------*/
void Init_Alpha_LCD(void) {
    
    TRISCbits.TRISC0 = 0;           // Configure SPI pins
    TRISCbits.TRISC2 = 0;
    TRISCbits.TRISC3 = 0;
    TRISCbits.TRISC4 = 1;
    TRISCbits.TRISC5 = 0;
    TRISCbits.TRISC6 = 0;
    TRISCbits.TRISC7 = 0;
    
    SPI_CS1 = 1;
    
    Send_MCP(MCP_IODIRA, 0x00);     // Configure MCP via SPI
    Send_MCP(MCP_GPIOA, 0x00);
    Send_MCP(MCP_IODIRB, 0x00);
    Send_MCP(MCP_GPIOB, 0x00);
    
    Send_Cmd_LCD(0x33);             // Configure LCD via MCP
    __delay_ms(10);
    Send_Cmd_LCD(0x33);
    __delay_ms(10);
    Send_Cmd_LCD(0x38);
    __delay_ms(10);
    Send_Cmd_LCD(0x0C);
    Send_Cmd_LCD(0x06);
    
    Send_Cmd_LCD(LCD_CLEAR);
    Send_Cmd_LCD(LCD_LINE_1);
    
    return;
}

/*------------------------------------------------------------------------------
 * Send command and related data to MCP via SPI_CS1
 *----------------------------------------------------------------------------*/
void Send_MCP(char cmd, char dat) {
    
    SPI_CS1 = 0;
    
    IFS0bits.SPI1IF = 0;
    SPI1BUF = 0x40;
    while(!IFS0bits.SPI1IF);
    
    IFS0bits.SPI1IF = 0;
    SPI1BUF = cmd;
    while(!IFS0bits.SPI1IF);
    
    IFS0bits.SPI1IF = 0;
    SPI1BUF = dat;
    while(!IFS0bits.SPI1IF);
    
    SPI_CS1 = 1;
    
    return;
}

/*------------------------------------------------------------------------------
 * Send a command to LCD
 *----------------------------------------------------------------------------*/
void Send_Cmd_LCD(char cmd) {
    
    Send_MCP(MCP_GPIOA, 0x00);  // RS to 0
    Send_MCP(MCP_GPIOB, cmd);   // Send cmd
    Send_MCP(MCP_GPIOA, 0x80);  // EN to 1
    Send_MCP(MCP_GPIOA, 0x00);  // EN to 0
    __delay_ms(4);

    return;
}

/*------------------------------------------------------------------------------
 * Send a character to LCD
 *----------------------------------------------------------------------------*/
void Send_Chr_LCD(char dat) {
    
    Send_MCP(MCP_GPIOA, 0x40);  // RS to 1
    Send_MCP(MCP_GPIOB, dat);   // Send data
    Send_MCP(MCP_GPIOA, 0xC0);  // EN to 1
    Send_MCP(MCP_GPIOA, 0x40);  // EN to 0
    __delay_us(10);
    
    return;
}

/*------------------------------------------------------------------------------
 * Display a string on line 1,2,3,4, at cursor otherwise
 *----------------------------------------------------------------------------*/
void Send_Txt_LCD(char * txt, char lnr) {
    
    switch (lnr) {
        
        case 0: break;
        case 1: Send_Cmd_LCD(LCD_LINE_1); break;
        case 2: Send_Cmd_LCD(LCD_LINE_2); break;
        case 3: Send_Cmd_LCD(LCD_LINE_3); break;
        case 4: Send_Cmd_LCD(LCD_LINE_4); break;
        default: break;
    }
    
    while(*txt) {
        
        Send_Chr_LCD(*txt);
        txt++;
    }
        
    return;
}
