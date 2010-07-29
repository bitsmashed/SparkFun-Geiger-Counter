/*
    10-5-09
    Aaron Weiss
    
	Geiger Counter Random Number Generator
	
	Internal 8MHz: lfuse = 0xE2, hfuse = 0xDF
    
*/
#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#define FOSC 8000000
#define BAUD 9600
#define MYUBRR 51 

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

#define STATUS_LED 5

//uint16_t global_clock;
volatile uint16_t i=0;
volatile long int j=0;
volatile long int jlast;
volatile uint8_t bit;
uint8_t a, b, c, d, e, f, g, h;
volatile uint8_t byte[8];

///============Initialize Prototypes=====================///////////////////////
void ioinit(void);      // initializes IO
static int uart_putchar(char c, FILE *stream);
uint8_t uart_getchar(void);
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
void delay_ms(uint16_t x); // general purpose delay
/////===================================================////////////////////////

ISR (SIG_INTERRUPT0) 
{	
	
	cli();
	cbi(PORTC, STATUS_LED);
	delay_ms(10);
	i++;

	if(jlast < j) bit = 0;
	if(jlast > j) bit = 1;
	
	printf("%d", bit);
	
	jlast = j;
	j = 0;
	sei();
	
}

//=========MAIN================/////////////////////////////////////////////////
int main(void)
{
	ioinit(); //Setup IO pins and defaults
	
	delay_ms(1200); //wait to settle
	sei();
	
	while(1)
	{	
		sbi(PORTC, STATUS_LED);
		delay_ms(30);
		j++;
	}
	
	cli();
	
	//for(int z = 0; z < 8; z++)
	//{	
	//	printf("%d", byte[z]);
	//}
	//printf("    \n");
}

////////////////////////////////////////////////////////////////////////////////
///==============Initializations=======================================/////////
////////////////////////////////////////////////////////////////////////////////
void ioinit (void)
{
    //1 = output, 0 = input
    DDRB = 0b11101111; //PB4 = MISO 
    DDRC = 0b00110001; //Output on PORTC0, PORTC4 (SDA), PORTC5 (SCL), all others are inputs
    DDRD = 0b11110010; //PORTD (RX on PD0), input on PD2
	//PORTD = 0b00000100; //enable pullup on PD2

    //USART Baud rate: 9600
    UBRR0H = MYUBRR >> 8;
    UBRR0L = MYUBRR;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);    
	
	EICRA=(1<<ISC01); // falling edge 
	EIMSK = (1<<INT0);
	
	stdout = &mystdout; //Required for printf init
	
	printf("\r           \r");
	
	sei();
}

static int uart_putchar(char c, FILE *stream)
{
    if (c == '\n') uart_putchar('\r', stream);
  
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    
    return 0;
}

uint8_t uart_getchar(void)
{
    while( !(UCSR0A & (1<<RXC0)) );
    return(UDR0);
}

//General short delays
void delay_ms(uint16_t x)
{
  uint8_t y, z;
  for ( ; x > 0 ; x--){
    for ( y = 0 ; y < 40 ; y++){
      for ( z = 0 ; z < 40 ; z++){
        asm volatile ("nop");
      }
    }
  }
}
