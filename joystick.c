#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

void ADC_init() {

	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);

	// ADEN: setting this bit enables analog-to-digital conversion.

	// ADSC: setting this bit starts the first conversion.

	// ADATE: setting this bit enables auto-triggering. Since we are

	//        in Free Running Mode, a new conversion will trigger whenever

	//        the previous conversion completes.

}

//Reference: http://maxembedded.com/2011/06/the-adc-of-the-avr/

void adc_init()
{
	// AREF = AVcc
	ADMUX = (1<<REFS0);
	
	// ADC Enable and prescaler of 128
	// 16000000/128 = 125000
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

uint16_t adc_read(uint8_t ch)
{
	// select the corresponding channel 0~7
	// ANDing with 7? will always keep the value
	// of ch between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing
	
	// start single convertion
	// write 1? to ADSC
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
	// ADSC becomes 0? again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}
