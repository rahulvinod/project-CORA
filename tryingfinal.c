/*
 * tryingfinal.c
 *
 * Created: 14-03-2018 11:27:35 AM
 *  Author: DELL
 */ 

#include <stdlib.h>
#include <string.h>
#include <math.h> 
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/io.h>
#define F_CPU 16000000UL
//#include <avr/io.h>
//#include <util/delay.h>
#include "uart.h"
#define ADC_REFRES 1024
//variables  for calculation
//maybe modified if required
#define MQ135_PULLDOWNRES 10000 //10k resistor
#define MQ135_DEFAULTPPM 392 //default ppm of CO2 for calibration
#define MQ135_DEFAULTRO 41763 //default Ro for MQ135_DEFAULTPPM ppm of CO2
#define MQ135_SCALINGFACTOR 116.6020682 //CO2 gas value
#define MQ135_EXPONENT -2.769034857 //CO2 gas value
#define MQ135_MAXRSRO 2.428 //for CO2
#define MQ135_MINRSRO 0.358 //for CO2


/*
 * get the calibrated ro based upon read resistance, and a know ppm
 */


void adc()
{
	DDRC = 0; // input direction for sensor
	//DDRB = 0xff; //output direction for display
	ADCSRA = 0x87; //to enable adc & select ck/128
	ADMUX = 0xc0; // 2.56V Vref
}



long mq135_getro
(long resvalue, double ppm) {
	return (long)(resvalue * exp( log(MQ135_SCALINGFACTOR/ppm) / MQ135_EXPONENT ));
}

/*
 * get the ppm concentration
 */
double mq135_getppm(long resvalue, long ro) {
	double ret = 0;
	double validinterval = 0;
	validinterval = resvalue/(double)ro;
	if(validinterval<MQ135_MAXRSRO && validinterval>MQ135_MINRSRO) {
		ret = (double)MQ135_SCALINGFACTOR * pow( ((double)resvalue/ro), MQ135_EXPONENT);
	}
	return ret;
	
}

long adc_getresistence(uint16_t adcread, uint16_t adcbalanceresistor)
{
	if(adcread == 0)
	return 0;
	else
	return (long)((long)(ADC_REFRES*(long)adcbalanceresistor)/adcread-(long)adcbalanceresistor);
}

int main(void) {
	// initialize d, adc, mq135_ro
	double d = 0;	
	uint16_t output = 0;
	long mq135_ro = 0;
	
	//initialize strings which store values
	//these strings are sent to server
	char str_d[15];
	char str_adc[15];
	char str_res[15];
	char str_mq135_ro[15];
	char api[44]="GET /update?api_key=QSY81FZ502WI8ZTH&field1=";
	char rn[4]="\r\n";
	
	DDRB=0b11111111;
	
	//initialize adc  ***
	//initialize uart ***
	//adc(); //initialize adc
	
	//for (;;) 
//{
		adc(); //initialize adc
		ADCSRA|=(1<<ADSC); // start conversion
		while(!(ADCSRA&(1<<ADIF))); //wait for conversion to finish
		output=ADC; //analog value on variable adc
		
		//get res
		long res = adc_getresistence(output, MQ135_PULLDOWNRES);
		//get ro
		mq135_ro = mq135_getro(res, MQ135_DEFAULTPPM);
		//convert to ppm (using default ro)
		d = mq135_getppm(res, MQ135_DEFAULTRO);
		
		/*if(output>333)
		{
			while(1)
			{
				PORTB=0b11111111;
				_delay_ms(500);
				PORTB=0b00000000;
				_delay_ms(2500);
			}
		}*/
		
	//convert integer to string
		itoa(output, str_adc, 10);
		ltoa(res, str_res, 10);
		ltoa(mq135_ro, str_mq135_ro, 10);
		//convert double to string
		dtostrf(d, 3, 5, str_d);
		

	DDRB=0b11111111;
	u_init();
	usart_tx("AT\r\n");
	PORTB=0b10101010;
	_delay_ms(5000);
	PORTB=0b01010101;
	_delay_ms(5000);
	PORTB=0b00000000;
	usart_tx("ATE0\r\n");
	PORTB=0b10101010;
	_delay_ms(5000);
	PORTB=0b01010101;
	_delay_ms(5000);
	PORTB=0b00000000;
	PORTB=0b01010101;	
	usart_tx("AT+CWJAP=\"Crescent 2\",\"00115577\"\r\n");
	PORTB=0b10101010;
	_delay_ms(50000);
	PORTB=0b01010101;
	_delay_ms(5000);
	PORTB=0b00000000;
	usart_tx("AT+CIPMUX=1\r\n");
	PORTB=0b10101010;
	_delay_ms(5000);
	PORTB=0b01010101;
	_delay_ms(10000);
	PORTB=0b00000000;
	usart_tx("AT+CIPSERVER=1,80\r\n");
	PORTB=0b10101010;
	_delay_ms(20000);
	PORTB=0b01010101;
	_delay_ms(5000);
	PORTB=0b00000000;
	usart_tx("AT+CIPSTART=4,\"TCP\",\"184.106.153.149\",80\r\n");
	PORTB=0b10101010;
	_delay_ms(50000);
	PORTB=0b01010101;
	_delay_ms(5000);
	PORTB=0b00000000;
	usart_tx("AT+CIPSEND=4,64\r\n");
	PORTB=0b10101010;
	_delay_ms(2000);
	PORTB=0b01010101;
	_delay_ms(5000);
	PORTB=0b00000000;
	//strcpy(strcat(api,strcat(str_adc,rn)));
	usart_tx("GET /update?api_key=QSY81FZ502WI8ZTH&field1=");
	usart_tx(str_adc);
	usart_tx("\r\n");
	PORTB=0b10101010;
	_delay_ms(5000);
	PORTB=0b01010101;
	_delay_ms(5000);
	PORTB=0b00000000;
	usart_tx("AT+CIPCLOSE=4\r\n");
	PORTB=0b01010101;
//}
}	




