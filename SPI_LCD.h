/*------------------------------------------------------------------------------
 * Simple LCD library for DSPIC33FJ32MC204
 * LM044L alpha LCD via MCP23S17 SPI IO extension
 * Samuel CIULLA - Microcontroleurs 2021
 *----------------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>
#define FCY 10000000
#include <libpic30.h>

/*------------------------------------------------------------------------------
 * SPI connections
 *----------------------------------------------------------------------------*/
#define INT_TICK	PORTCbits.RC9
#define SPI_CSK	    PORTCbits.RC7
#define SPI_SDI	    PORTCbits.RC4
#define SPI_SDO	    PORTCbits.RC5
#define SPI_CS1	    PORTCbits.RC2

/*------------------------------------------------------------------------------
 * MCP23S17 functions
 *----------------------------------------------------------------------------*/
#define MCP_IODIRA  0x00
#define MCP_IODIRB  0x01
#define MCP_GPIOA   0x12
#define MCP_GPIOB   0x13

/*------------------------------------------------------------------------------
 * LCD functions
 *----------------------------------------------------------------------------*/
#define LCD_CLEAR   0x01
#define LCD_LINE_1  0x80
#define LCD_LINE_2  0xC0
#define LCD_LINE_3  0x94
#define LCD_LINE_4  0xD4
#define LCD_CUR_OFF 0x0C
#define LCD_CUR_ON  0x0E
#define LCD_CUR_BLK 0x0F

/*------------------------------------------------------------------------------
 * Prototypes
 *----------------------------------------------------------------------------*/
void Init_Alpha_LCD(void);
void Send_MCP(char cmd, char dat);
void Send_Cmd_LCD(char cmd);
void Send_Chr_LCD(char dat);
void Send_Txt_LCD(char * txt, char lnr);
