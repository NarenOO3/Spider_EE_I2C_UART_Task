#include <avr/io.h>
#include<stdlib.h>
#include <unistd.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define SCL_CLOCK 100000 // 100Khz


void i2c_init(){
  TWSR = 0; //no prescalar
  TWBR =((F_CPU/SCL_CLOCK)-16)/2; //>10 stable operation
};

void i2c_start(void){
  TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
  while((TWCR & (1<<TWINT)) == 0);// useful when workning multiple devices
}

void i2c_write(unsigned char address){
  TWDR = address; // TWDR has the last received byte
  TWCR = (1<<TWEN) | (1<<TWINT);
  while((TWCR & (1<<TWINT)) == 0);// useful when workning multiple devices
}

int i2c_read(){
  TWCR = (1<< TWINT) | (1<<TWEN);
  while ((TWCR & (1 <<TWINT)) == 0) ;
  return TWDR;
}

void i2c_stop(){
  TWCR =(1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
  while (TWCR & (1 <<TWSTO)); 
}

void UART_init(uint16_t ubrr){
    // set baudrate in UBRR
    UBRR0L = (uint8_t)(ubrr & 0xFF);
    UBRR0H = (uint8_t)(ubrr >> 8);

    // enable the transmitter and receiver
    UCSR0C = 0x06;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
}

void UART_putc(unsigned char data){
    // wait for transmit buffer to be empty
    while(!(UCSR0A & (1 << UDRE0)));

    // load data into transmit register
    UDR0 = data;
}
void UART_puts(char* s){
    // transmit character until NULL is reached
    while(*s > 0) UART_putc(*s++);
}

void UART_putU8(uint8_t val)
{
    uint8_t dig1 = '0', dig2 = '0';

    // count value in 100s place
    while(val >= 100)
    {
        val -= 100;
        dig1++;
    }

    // count value in 10s place
    while(val >= 10)
    {
        val -= 10;
        dig2++;
    }

    // print first digit (or ignore leading zeros)
    if(dig1 != '0') UART_putc(dig1);

    // print second digit (or ignore leading zeros)
    if((dig1 != '0') || (dig2 != '0')) UART_putc(dig2);

    // print final digit
    UART_putc(val + '0');
}

void readTemp(unsigned char temp){
  UART_puts("Temp(in C): ");
  UART_putU8(temp);
  UART_putc("\n");

  _delay_ms(100);
}

int main(void){
  i2c_init();
  UART_init(103); // 103 ubrr value to get 9600 bits per second
  int tempTC74;
  DDRB |= (1<<PB5);
  while(1){
  
    //to create infinite loop  
    i2c_start(); // START condition
    i2c_write(0b10011010); //transmit SLA + W(0)
    i2c_write(0b00000000); // To access Read Temp
    i2c_start();
    i2c_write(0b10011011); //transmit SLA + R(1)
   tempTC74 = i2c_read();
   i2c_stop(); // STOP condition;
    if(tempTC74 >= 50){
      PORTB |= (1<<PB5);
       readTemp(tempTC74);
    
    }
  else{
      readTemp(tempTC74);
      PORTB &= ~(1<<PB5);
  }
   }
  return 0;
}
