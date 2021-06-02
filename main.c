#define F_CPU 16000000UL
#define USART_BAUDRATE 9600 
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL)))-1) 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>


#define Trigger_pin PC5	// Trigger pin 
#define Echo_pin PB0 // Echo pin
#define LED PC1

/* LEDs*/
#define RED_LED PD2
#define GREEN_LED PD4
#define BLUE_LED PD7

/* WHEELS */
#define LWB PD6, OCR0A //LEFT WHEEL BACKWARD
#define LWF PD5, OCR0B //LEFT WHEEL FORWARD
#define RWB PB3, OCR2A //RIGHT WHEEL BACKWARD
#define RWF PD3, OCR2B //RIGHT WHEEL FORWARD

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/*
OCR0A - PD6 -	LWB
OCR0B - PD5 -	LWF
OCR2A - PB3 -	RWB
OCR2B - PD3 - 	RWF
*/
void init(void);
int ADC_read(void);

/*methords for communication*/
void uart_init(void);
void send_uart(unsigned char val);
unsigned char recieve_uart(void);



//methords and variables for obstacl detection
void ultrasonic_init(void);
void detect_object(void);

int TimerOverflow = 0;
long count;
double distance;
unsigned char R;
unsigned int a,b; 

	/*methords for car movement*/
void drive_init(void);
void forward(a,b);
void forward2(a,b);
void backward(a,b);
void stop(a);

/*Color Detect*/
void detect_red(void);
void detect_green(void);
void detect_blue(void);
void extracr_number(int val);

void total_R(unsigned int val);
void total_G(unsigned int val);
void total_B(unsigned int val);

void print_R(int val);
void print_G(int val);
void print_B(int val);

unsigned int delay = 40;

void color(void);

	//store total of data
unsigned int sum_R = 0;
unsigned int sum_G = 0;
unsigned int sum_B = 0;

	//store average values
unsigned int avg_R = 0;
unsigned int avg_G = 0;
unsigned int avg_B = 0;

	//store analog vaues
unsigned int val_R;
unsigned int val_G;
unsigned int val_B;

	//to store extracted value (num1,2,3,4)
unsigned char num1 = 0;
unsigned char num2 = 0;
unsigned char num3 = 0;
unsigned char num4 = 0;


int main(void) {
	int i = 0;
	int j = 1;
	
	DDRD |= (1<<PD2)|(1<<PD4)|(1<<PD7);
	

	init();
	uart_init();
	ultrasonic_init();
	drive_init();
	
		
	forward(85,0);
	
	right(115);
	
	forward(85,0);
	
	left(90);
	
	forward(85,0);
	
	left(90);
	
	forward2(85,0);
	
	left(90);
	
	forward(85,0);
	
	left(90);
	
	forward(85,0);
	
	right(115);
	
	forward(85,0);
	
		/*
		right(135);
		
		forward(140,0);
			for(int i=0; i<=1; i++){
				detect_object();
				send_uart('\n');
			}
			//color();
		//_delay_ms(2000);
		stop(0);
		
		left(120);
	
		
		forward(135,0);
			for(int i=0; i<=1; i++){
				detect_object();
				send_uart('\n');
			}
			color();
		_delay_ms(440);
		stop(0);
		
		left(120);
		
		forward(150,0);
			for(int i=0; i<=3; i++){
				detect_object();
				send_uart('\n');
			}
		_delay_ms(440);
		stop(0);
		
		
		left(125);
		
		forward(160,0);
			for(int i=0; i<1; i++){
				detect_object();
				send_uart('\n');
			}
			color();
		_delay_ms(740);
		stop(0);
		
		left(120);
		
		forward(130,0);
			for(int i=0; i<=1; i++){
				detect_object();
				send_uart('\n');
			}
			color();
		_delay_ms(440);
		stop(0);
		
		right(130);
		
		forward(130,0);
			for(int i=0; i<=1; i++){
				detect_object();
				send_uart('\n');
			}
			color();
		_delay_ms(440);
		
		stop(0);
		
		*/


}

void init(void) {

	DDRC = DDRC | (1<<PC1);	// testing LED
	DDRC = DDRC | (1<< RED_LED) | (1 << GREEN_LED) | (1 << BLUE_LED);	

	ADCSRA = ADCSRA | (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (ADPS0); 	//enable adc and set prescaller to 128
	ADMUX = 0x40;	// set PC0  as analog input
	
}

int ADC_read(void) {
	
	int reading;
	ADCSRA = ADCSRA | (1 << ADSC);	//start convertion
	while(ADCSRA & (1 << ADIF));
	_delay_us(10);
	reading = ADC;
	
	return reading;

}

///Initialization of USART.\\\	

void uart_init(void) {

	UCSR0B |= (1 << TXEN0) | (1 << RXEN0); 	// Transmission & Reception Enable (TXEN=1, RXEN=1)
	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);	
	UBRR0H = (BAUD_PRESCALE >> 8);			
	UBRR0L = (BAUD_PRESCALE);
	
}
/*
	* send_uart send characters from microcontroller
	* val : value send by USART
*/

void send_uart(unsigned char val){

	while (UCSR0A & (1 << UDRE0) == 0); //wait until transmition is finished
	UDR0 = val; 	//Transmit the charater
	_delay_ms(1);
}


///recieve_uart recieve characters to the microcontroller\\\
	
unsigned char recieve_uart(void){

	while (!(UCSR0A & (1 << RXC0)));//wait until receive to finish		
	return(UDR0);//return recieved value.							
	
}


	/* obstacal detection*/

ISR(TIMER1_OVF_vect)
{
	TimerOverflow++;	/* Increment Timer Overflow count */
}


void ultrasonic_init(void) {
	
	DDRC = DDRC | (1<<Trigger_pin) | (1 << LED);	
	
	sei();			// Enable global interrupt
	TIMSK1 = TIMSK1 | (1 << TOIE1);	//Enable Timer1 overflow interrupts 
	TCCR1A = 0;	

}


void detect_object(void) {

	PORTC |= (1 << Trigger_pin);	//trigger the pin
	_delay_us(10);
	PORTC &= (~(1 << Trigger_pin));

	TCNT1 = 0;	
	TCCR1B = 0x41;	// Capture on rising edge, No prescaler
	TIFR1 = TIFR1 | (1<<ICF1) | (1<<TOV1);		//Clear Input Capture flag, timer overflow

	while ((TIFR1 & (1 << ICF1)) == 0); // Wait for rising edge 
	TCNT1 = 0;
	TCCR1B = 0x01;	// Capture on falling edge, No prescaler
	TIFR1 = TIFR1 | (1<<ICF1) | (1<<TOV1);
	TimerOverflow = 0;

	while ((TIFR1 & (1 << ICF1)) == 0);	//Wait for falling edge
	count = ICR1 + (65535 * TimerOverflow);	//Take count
	distance = (double)count / 850;	//calculating the distance


	_delay_ms(1);
	
	if(distance < 50 ) {
	
		PORTC = PORTC | (1 << LED);
		send_uart(49);	//1
		
	} else {
		
		PORTC = PORTC & ~(1 << LED);	
		send_uart(48);	//0
			
	}
	_delay_ms(1);

}

		/*drive car*/

void drive_init(void) {

	TCCR0A = (1<<COM0A1)|(1<<COM0B1)|(1<<WGM01)|(1<<WGM00);
	TCCR0B = (1<<CS00);
	DDRD = DDRD | (1 << PD3) | (1 << PD5) | (1 << PD6);

	TCCR2A = (1<<COM2A1)|(1<<COM2B1)|(1<<WGM21)|(1<<WGM20);
	TCCR2B = (1<<CS20);
	DDRB = DDRB | (1 << PB3);	

}

void forward(a,b) {

	LWF = a;  	//Left Wheel Forward , OCR0B
	RWF = a;	//Right Wheel Forward , OCR2B 
	LWB = 0;	//Left wheel backward , OCR0A 
	RWB = 0;	//Right wheel backward , OCR2A 
	
	for(int i=0; i<=30; i++){
		detect_object();
		color();
		send_uart('\n');
	}	
	
}
void forward2(a,b) {

	LWF = a;  	//Left Wheel Forward , OCR0B
	RWF = a;	//Right Wheel Forward , OCR2B 
	LWB = 0;	//Left wheel backward , OCR0A 
	RWB = 0;	//Right wheel backward , OCR2A 
	
	for(int i=0; i<=60; i++){
		detect_object();
		color();
		send_uart('\n');
	}	
	
}
void backward(a,b) {

	LWB = a;	//Left wheel backward , OCR0A
	RWB = a;	//Right wheel backward , OCR2A 
	LWF = 0;	//Left Wheel Forward , OCR0B 
	RWF = 0;	//Right Wheel Forward , OCR2B 
	

}
void left(a){

	LWB = a; //Left wheel backward
	RWB = 0; //Right wheel backward
	LWF = 0; //Left Wheel Forward
	RWF = a; //Right Wheel Forward
	
	_delay_ms(2000);
		
	LWB = 0; //Right wheel backward
	RWF = 0; //Left Wheel Forward
	//_delay_ms(1000);
		
}

void right(a){

	LWB = 0;	//Left wheel backward
	RWB = a;	//Right wheel backward
	LWF = a;	//Left Wheel Forward
	RWF = 0;	//Right Wheel Forward
	
	_delay_ms(1500);
		
	RWB = 0;	//Left wheel backward
	LWF = 0;	//Right Wheel Forward	
	//_delay_ms(1000);

}

void stop(a) {
	
	LWF = a;
	LWB = a;
	RWF = a;
	RWB = a;
	_delay_ms(1000);

}
void color(void){
	
		sum_R = 0;
		sum_G = 0;
		sum_B = 0;
		
		val_R = 0;
		val_G = 0;
		val_B = 0;
		
		for(int j=1; j<=5; j++) {	//number of repers

			detect_red();
			total_R(val_R);
			
			detect_green();
			total_G(val_G);
			
			detect_blue();
			total_B(val_B);
		}
		avg_R = sum_R/5;
		avg_G = sum_G/5;
		avg_B = sum_B/5;		
						/* print analog values
		print_R(avg_R);
		print_G(avg_G);			
		print_B(avg_B);	*/
		
		
						/*indentifing the color*/
		if(((avg_R >=262) && (avg_R <=462)) && ((avg_G >=520) && (avg_G <= 720)) && ((avg_B >=261) && (avg_B <=461))) {
				//send_uart('r');	//red
				PORTD |= (1<<RED_LED);
				PORTD &= ~((1<<GREEN_LED)|(1<<BLUE_LED));
		}
		
		else if(((avg_R >=397) && (avg_R <=597)) && ((avg_G >=386) && (avg_G <= 586)) && ((avg_B >=484) && (avg_B <=684))) {
				//send_uart('g');	//green
				PORTD |= (1<<GREEN_LED);
				PORTD &= ~((1<<RED_LED)|(1<<BLUE_LED));

		}	
		
		else if(((avg_R >=388) && (avg_R <=588)) && ((avg_G >=206) && (avg_G <= 506)) && ((avg_B >=327) && (avg_B <=483))) {
				//send_uart('b');	//blue
				PORTD |= (1<<BLUE_LED);
				PORTD &= ~((1<<GREEN_LED)|(1<<RED_LED));

		}
		
		//delect_object(); // detecting object*/
	
}
void detect_red(void) {

	PORTC = PORTC | (1<< RED_LED);
	_delay_ms(5);
	val_R = (ADC_read()-58)*2.4070;		//mapping
	PORTC = PORTC & ~(1<< RED_LED);
	
}

void total_R(unsigned int val) {

	sum_R = sum_R + val;

}

void print_R(int val) {

	send_uart(82);
	_delay_ms(1);	
	send_uart(61);
	_delay_ms(1);	
	
	extracr_number(val);
}

void detect_green(void) {
	
	PORTC = PORTC | (1<< GREEN_LED);
	_delay_ms(5);
	val_G = (ADC_read()-39)*3.2579;	//mapping
	PORTC = PORTC & ~(1<< GREEN_LED);

}

void total_G(unsigned int val) {

	sum_G = sum_G + val;

}

void print_G(int val) {

	send_uart(71);
	_delay_ms(1);	
	send_uart(61);
	_delay_ms(1);	
	
	extracr_number(val);
}


void detect_blue(void) {
	
	PORTC = PORTC | (1<< BLUE_LED);
	_delay_ms(5);
	val_B = (ADC_read()-71)*2.20;	//mapping
	PORTC = PORTC & ~(1<< BLUE_LED);

}

void total_B(unsigned int val) {

	sum_B = sum_B + val;

}

void print_B(int val) {

	send_uart(66);
	_delay_ms(1);	
	send_uart(61);
	_delay_ms(1);	
	
	extracr_number(val);
	
}



void extracr_number(int val) {

	num1 = 48+(val/1000);	//extract first digit
	send_uart(num1);
	_delay_ms(1);
		
	num2 = 48+((val/100)%10);	//extract second digit
	send_uart(num2);
	_delay_ms(1);			
		
	num3 = 48+((val%100)/10);	//extract thrid digit
	send_uart(num3);	
	_delay_ms(1);	
		
	num4 = 48+(val%10);	//extract fourth digit
	send_uart(num4);
	_delay_ms(1);

	send_uart(10);	//new line
	_delay_ms(1);

}