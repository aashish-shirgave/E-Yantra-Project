 /*
* Team Id: eYRC#285
* Author List: Yadnyi Deshpande , Sanket Patil, Aashish Shirgave, Yash Agrawal
*Filename: linefollow .c
* Theme: eYRC Thirsty Crow
* Functions: uart0_init, get_direction, get_destination, get_input, timer5_init, uart_rx, uart_tx, init_devices,_direction
			motion_pin_config,forward, back, right, left, soft_left, soft_right, stop, pickup, 
*Global Variables: prev_x, prev_y, curr_x, curr_y, next_x, next_y, dest_x, dest_y, direction[50],
					 num_direction,ori_bot, final_dest_x, final_dest_y,flag_magnet,led[3],position[10],
					 orientation[10],start, num_positions,positions_done, xbee_input[20]
 */ 

/********************************************************************************/
#define F_CPU 14745600
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


#include "init.h"  // initialization of ports of atmega2560
#define LEFT 0
#define RIGHT 1
#define REVERSE 2


unsigned int threshold = 45;
static int prev_x, prev_y, curr_x, curr_y, next_x, next_y, dest_x, dest_y, direction[50], num_direction = 0,ori_bot = 1, final_dest_x, final_dest_y,flag_magnet = 0;
static int led[3],position[10],orientation[10],start, num_positions,positions_done = 0;
char xbee_input[20];

//cell wise graph coordinates mapping
int arr[19][12] = {
					{4,	9,	7,	9,	5,	10,	6,	8,	6,	0,	5,	8},
					{2,	8,	5,	8,	3,	9,	4,	7,	4,	9,	3,	7},
					{4, 7,	7,	7,	5,	8,	6,	6,	6,	8,	5,	6},
					{6,	8,	9,	8,	7,	9,	8,	7,	8,	9,	7,	7},
					{0,	7,	3,	7,	1,	8,	2,	6,	2,	8,	1,	6},
					{2,	6,	5,	6,	3,	7,	4,	5,	4,	7,	3,	5},
					{4,	5,	7,	5,	5,	6,	6,	4,	6,	6,	5,	4},
					{6,	6,	9,	6,	7,	7,	8,	5,	7,	5,	8,	7},
					{8,	7,	11,	7,	9,	8,	10,	6,	9,	6,	10,	8},
					{0,	5,	3,	5,	1,	6,	2,	4,	2,	6,	1,	4},
					{2,	4,	5,	4,	3,	5,	4,	3,	3,	3,	4,	5},
					{4,	3,	7,	3,	5,	4,	6,	2,	6,	4,	5,	2},
					{6,	4,	9,	4,	7,	5,	8,	3,	7,	3,	8,	5},
					{8,	5,	11,	5,	9,	6,	10,	4,	9,	4,	10,	6},
					{0,	3,	3,	3,	1,	4,	2,	2,	1,	2,	2,	4},
					{2,	2,	5,	2,	3,	3,	4,	1,	4,	3,	3,	1},
					{4,	1,	7,	1,	5,	2,	6,	0,	5,	0,	6,	2},
					{6,	2,	9,	2,	7,	3,	8,	1,	7,	1,	8,	3},
					{8,	3,	11,	3,	9,	4,	10,	2,	9,	2,	10,	4},
					};


/************************************************************************************************/
#define RX  (1<<4)
#define TX  (1<<3)
#define TE  (1<<5)
#define RE  (1<<7)

volatile unsigned char data;

void uart0_init()
{
	UCSR0B = 0x00;							//disable while setting baud rate
	UCSR0A = 0x00;
	UCSR0C = 0x06;
	UBRR0L = 0x5F; 							//9600BPS at 14745600Hz
	UBRR0H = 0x00;
	UCSR0B = 0x98;
	UCSR0C = 3<<1;							//setting 8-bit character and 1 stop bit
	UCSR0B = RX | TX;
}


void uart_tx(char data)
{
	while(!(UCSR0A & TE));						//waiting to transmit
	UDR0 = data;
}

ISR(USART0_RX_vect)
{
	data = UDR0;
}



char uart_rx()
{
	while(!(UCSR0A & RE));						//waiting to receive
	return UDR0;
}




/***********************************************************************************/

/*
*Function Name: _direction
*Input: none
*Output: none
*Logic: direction function the next position to go for destination and then saves the turn to direction variable
*Example Call: _direction()
*/

void _direction()
{
	if(curr_x & 1){//current row is odd
		if(dest_x < curr_x){//up
			
			if(dest_y < curr_y){//left
				//x-- y--
				next_x = curr_x - 1 ;
				next_y = curr_y - 1;
			}
			else if(dest_y > curr_y){//right
				//x-- y++
				next_x = curr_x - 1;
				next_y = curr_y + 1;
			}
			else{
				//same column
				//we will consider it as a TODO task 
				if(curr_y > 5){
					//x--, y--
					next_x = curr_x - 1;
					next_y = curr_y - 1;
				}
				else{
					//x-- y++
					next_x = curr_x - 1;
					next_y = curr_y + 1;
				}
			}
		}
		
		
		else if(dest_x > curr_x){//down
			//x++ y same
			next_x = curr_x + 1;
			next_y = curr_y ;

		}
		else{
			//same row number
			//here row is ODD so decease x
			if(dest_y < curr_y){//left
				//x-- y--
				next_x = curr_x - 1;
				next_y = curr_y - 1;


			}
			if(dest_y > curr_y){//right
				//x-- y++
				next_x = curr_x - 1;
				next_y = curr_y + 1;
			}
		}

	}
	else{//row is even
		if(dest_x > curr_x){//down
			if(dest_y < curr_y){//left
				//x++ y--
				next_x = curr_x + 1;
				next_y = curr_y - 1;
			}
			else if(dest_y > curr_y){//right
				//x++ y++
				next_x = curr_x + 1;
				next_y = curr_y + 1;
			}
			else{
				//same column number
				//we will consider it as a TODO task 
				if(curr_y > 5){
					//x++, y--
					next_x = curr_x + 1;
					next_y = curr_y - 1;
				}
				else{
					//x++ y++
					next_x = curr_x + 1;
					next_y = curr_y + 1;
				}
			}

		}
		else if(dest_x < curr_x){//up
			//x-- y same
			next_x = curr_x - 1;
			next_y = curr_y ;
		}
		else{
			//same row number
			//row is EVEN so x increase
			if(dest_y < curr_y){//left
				//x++ y--
				next_x = curr_x + 1;
				next_y = curr_y - 1;

			}
			if(dest_y > curr_y){//right
				//x++ y++
				next_x = curr_x + 1;
				next_y = curr_y + 1;
			}
		}

	}
	/*************************************************************/
	//if previous node current node and next node is known and we have to find the turn required
	if(prev_y == next_y && prev_x == next_x){
		//printf("Reverse\n");
		direction[num_direction++] = REVERSE;
	}
	else if(prev_x == next_x){
		if(curr_x < prev_x){
			if(prev_y > next_y){
				direction[num_direction++] = LEFT;
				//printf("left\n");
			}
			else{
				//printf("right\n");
				direction[num_direction++] = RIGHT;
			}
		}
		else{
			if(prev_y < next_y){
				direction[num_direction++] = LEFT;
				//printf("left\n");
			}
			else{
				//printf("right\n");
				direction[num_direction++] = RIGHT;
			}

		}

	}
	else if(prev_y == curr_y){
		if(prev_x < curr_x){
			if(next_y > curr_y){
				//printf("left\n");
				direction[num_direction++] = LEFT;
			}
			else{
				//printf("right\n");
				direction[num_direction++] = RIGHT;
			}
		}
		else{
			if(next_y < curr_y){
				//printf("left\n");
				direction[num_direction++] = LEFT;
			}
			else{
				//printf("right\n");
				direction[num_direction++] = RIGHT;
			}


		}
	}
	else if(curr_y == next_y){
		if(curr_x > next_x){
			if(curr_y < prev_y){
				direction[num_direction++] = RIGHT;
				//printf("right\n");
			}
			else{
				//printf("left\n");
				direction[num_direction++] = LEFT;
			}
		}
		else{
			if(curr_y > prev_y){
				//printf("right\n");
				direction[num_direction++] = RIGHT;
			}
			else{
				//printf("left\n");
				direction[num_direction++] = LEFT;
			}

		}

	}
	else{
		stop(); // extreme case when bot is lost
		while(1);
	}
}


/********************************************************************************************************/




/*
*Function Name: get_direction
*Input: none
*Output: none
*Logic: this function calls direction function until destination
*Example Call: get_direction();
*/


void get_direction(){
	num_direction = 0;
	//this will get direction to the previous position of the final position
	while(next_x != dest_x || next_y != dest_y){
		_direction();
		prev_x = curr_x;
		prev_y = curr_y;
		curr_x = next_x;
		curr_y = next_y;

	}
	dest_x = final_dest_x;
	dest_y = final_dest_y;
	//this one turn will get form previous position to final position
	while(next_x != dest_x || next_y != dest_y){
		_direction();
		prev_x = curr_x;
		prev_y = curr_y;
		curr_x = next_x;
		curr_y = next_y;

	}		
}

/***********************************************************************************/
/*
*Function Name: motion_pin_config
*Input: none
*Output: none
*Logic:
*Example Call: motion_pin_config()
*/
//Function to configure ports to enable robot's motion
void motion_pin_config (void) 
{
 DDRA = DDRA | 0x0F;
 PORTA = PORTA & 0xF0;
 DDRL = DDRL | 0x18;   //Setting PL3 and PL4 pins as output for PWM generation
 PORTL = PORTL | 0x18; //PL3 and PL4 pins are for velocity control using PWM.
}

//Function to initialize ports
void init_ports()
{
 motion_pin_config();
}

// Timer 5 initialized in PWM mode for velocity control
// Prescale:256
// PWM 8bit fast, TOP=0x00FF
// Timer Frequency:225.000Hz
void timer5_init()
{
	TCCR5B = 0x00;	
	TCNT5H = 0xFF;	
	TCNT5L = 0x01;	
	OCR5AH = 0x00;	
	OCR5AL = 0xFF;	
	OCR5BH = 0x00;	
	OCR5BL = 0xFF;	
	OCR5CH = 0x00;	
	OCR5CL = 0xFF;	
	TCCR5A = 0xA9;	
	
	TCCR5B = 0x0B;	
}

/*
*Function Name: velocity
*Input: left_motor_value , right motor_value
*Output: none
*Logic: Function for robot velocity control
*Example Call : velocity(150, 150);
*/

void velocity (unsigned char left_motor, unsigned char right_motor)
{
	OCR5AL = (unsigned char)left_motor;
	OCR5BL = (unsigned char)right_motor;
}

//Function used for setting motor's direction
void motion_set (unsigned char Direction)
{
 unsigned char PortARestore = 0;

 Direction &= 0x0F; 			
 PortARestore = PORTA; 			
 PortARestore &= 0xF0; 			
 PortARestore |= Direction; 	
 PORTA = PortARestore; 			
}

/*
*Function Name: line_sensor
*Input: none
*Output: none
*Logic: read values from line sensor and set appropriate values in array
*Example Call: line_sensor()
*/

void line_sensor(){
	if(read_adc(1) > threshold){
		led[0] = 1;
		//PORTB = 0xFF;
	}
	else{
		led[0] = 0;
		//PORTB = 0x00;
	}
	if(read_adc(2) > threshold){
		led[1] = 1;
		//PORTB = 0xFF;
	}
	else{
		led[1] = 0;
		//PORTB = 0x00;
	}
	if(read_adc(3) > threshold){
		led[2] = 1;
		//PORTB = 0xFF;
	}
	else{
		led[2] = 0;
		//PORTB = 0x00;
	}
}

/*
*Function Name: forward
*Input: none
*Output: none
*Logic: to move the bot in Forward direction
*Example Call: forward() 
*/
void forward (void) //both wheels forward
{
  motion_set(0x06);
}
/*
*Function Name: back
*Input: none
*Output: none
*Logic: to move bot in backward direction
*Example Call: back()
*/
void back (void) //both wheels backward
{
  motion_set(0x09);
}
/*
*Function Name: left
*Input: none
*Output: none
*Logic: to move the bot in left direction
*Example Call: left()
*/
void left (void) //Left wheel backward, Right wheel forward
{
	motion_set(0x0A);
}
/*
*Function Name: right
*Input: none
*Output: none
*Logic: to move the bot in right direction
*Example Call: right()
*/
void right (void) //Left wheel forward, Right wheel backward
{
	motion_set(0x05);
  
}
/*
*Function Name: soft_left
*Input: none
*Output: none
*Logic: to move the bot in left direction with one wheel moving
*Example Call: soft_left()
*/
void soft_left (void) //Left wheel stationary, Right wheel forward
{
	motion_set(0x04);
}
/*
*Function Name: soft_right()
*Input: none
*Output: none
*Logic: To move the bot in right direction with one wheel moving
*Example Call:
*/
void soft_right (void) //Left wheel forward, Right wheel is stationary
{
 motion_set(0x02);
}


void stop (void)//both wheel stationary
{
  motion_set(0x00);
}
/**************************************************************************************************/

void init_devices (void) //use this function to initialize all devices
{
 cli(); //disable all interrupts
 init_ports();
 timer5_init();
 sei(); //re-enable interrupts
}

/*
*Function Name: new_left
*Input: none
*Output: none
*Logic: left turn of robot
*Example Call: new_left()
*/

void new_left(){
	forward();
	velocity(122, 122);
	_delay_ms(200);
	velocity(110, 110);
	
	left();
	line_sensor();
	//turn the bot until it reaches next line
	while(led[0] != 2){
		line_sensor();
	}	
}

/*
*Function Name: new_right
*Input: none
*Output: none
*Logic: right turn of robot
*Example Call: new_right()
*/

void new_right(){
	forward();
	velocity(122, 122);
	_delay_ms(200);
	velocity(110, 110);
	
	right();
	line_sensor();
	//turn the bot until it reaches next line
	while(led[2] != 0){
		line_sensor();
	}	
}

/*Function Name: reverse
*Input: none
*Output: none
*Logic: To take a reverse turn. The bot should pass through 2 lines on the arena to get to the final line 
*Example Call: reverse()
*/
void reverse(){
	forward();
	velocity(122, 122);
	_delay_ms(200);
	velocity(110, 110);
	
	right();
	line_sensor();
	while(led[1] != 1){
		line_sensor();
	}
	while(led[1] == 1){
		line_sensor();
	}
	while(led[1] != 1){
		line_sensor();
	}
	
}

/*
*Function Name: buzzer_of
*Input: none
*Output: none
*Logic: Turns OFF the buzzer
*Example Call: buzzer_of()
*/
void buzzer_of(){
	PORTB = 0x00;
}
/*Function Name: buzzer_on()
*Input: none
*Output: none
*Logic:Turns ON the buzzer
*Example Call:
*/
void buzzer_on(){
	PORTB = 0xFF;
}
/*
*Function Name: magnet_of
*Input: none
*Output: none
*Logic: turns OFF the magnet
*Example Call: magnet_of()
*/
void magnet_of(){
	PORTK = 0x00;
}
/*
*Function Name: magnet_on
*Input: none
*Output: none
*Logic: Turns ON the magnet
*Example Call:
*/
void magnet_on(){
	PORTK = 0xFF;
}
/*
*Function Name: pickup
*Input: none
*Output: none
*Logic: This function is called when the bot reaches the node and it have to pickup or drop the magnetic pebbles
*Example Call:pickup()
*/

void pickup(){
	//forward the bot for some distance
	forward();
	velocity(122, 122);
	_delay_ms(350);
	//stop for the bot to complete action
	stop();
	_delay_ms(2000);
	back();
	velocity(133, 120);
	line_sensor();
	if(flag_magnet == 0){
		magnet_on();
		flag_magnet = 1;
	}
	else{
		magnet_of();
		flag_magnet = 0;
	}
	//bot will come back until it reaches the line
	while(1){
		if(led[0] == 0 && led[1]== 1 && led[2] == 0){
			break;
		}
		line_sensor();
	}
	
	back();
	velocity(133, 120);
	_delay_ms(50);
	
}




/********************************************************************/

/*
*Function Name: get_direction 
*Input: none
*Output: none
*Logic: This function saves the destination of the bot in dest_x, dest_y (which is the previous position of the final destination of the bot)
		and the final_dest_x, final_dest_y (which is final destination where the Ar_Uco Object is placed) 
*Example Call: get_direction()
*/

/*********************************************************************************************************/

get_destination(){
	int i,k,o,n;
	int destination_positions[4];
	o = orientation[positions_done] - 1;
	n = 0;
	if(positions_done == num_positions){
		//end of all positions
		stop();
		uart_tx('@');
		while(1){
			buzzer_on();
			_delay_ms(5000);
			buzzer_of();
			while(1);
		}
	}
	/* To read The positions from array*/
	for(k = o * 4; k < (o * 4 + 4); k++){

		destination_positions[n++] = arr[position[positions_done]][k];
		
	}
	positions_done++;
	/*to find distance from current position to 2 destination positions by distance formula and the dessitnation will be the smaller*/
	int sq1, sq2;
	sq1 = ((destination_positions[0] - curr_x) * (destination_positions[0] - curr_x)) + ((destination_positions[1] - curr_y) * (destination_positions[1] - curr_y));
	sq2 = ((destination_positions[2] - curr_x) * (destination_positions[2] - curr_x)) + ((destination_positions[3] - curr_y) * (destination_positions[3] - curr_y));
	
	if(sq1 > sq2){
		dest_x = destination_positions[2];
		dest_y = destination_positions[3];
	}
	else{
		dest_x = destination_positions[0];
		dest_y = destination_positions[1];
	}
	/*Now final_dest is the position in arena where the Ar_Objects are present and 
	we will find a previous position before the last position so that bot will go straight under the ar_Objects */
		
	final_dest_x = dest_x;
	final_dest_y = dest_y;
	// o is orientation of the Ar_object (0,1,2) i.e 0 for 1-1 , 1 for 2-2 , 2 for 3-3
	if(o == 0) {
		if(dest_x % 2 == 0){
			dest_x = dest_x - 1;
		}
		else{
			dest_x = dest_x + 1;
		}
	}
	else if(o == 1){
		if(dest_x % 2 == 0){
			dest_x = dest_x + 1;
			dest_y = dest_y - 1;
		}
		else{
			dest_x = dest_x - 1;
			dest_y = dest_y + 1;
		}
	}
	else{
		if(dest_x % 2 == 0){
			dest_x = dest_x + 1;
			dest_y = dest_y + 1;
		}
		else{
			dest_x = dest_x - 1;
			dest_y = dest_y - 1;
		}
	}	
}
/* 
*Function Name: get_input
*Input: none
*Output: none
*Logic: this function takes input from Xbee module and saves in variable Xbee_input and converts that
	in positions and orientation arrays.
	Xbee input format
	1) start position
	2)position of the first pebble 
	3)orientation of first pebble
	4)position of water pitcher
	5)orientation of water pitcher
	.
	.
	.
	such that
*Example Call: get_input();
*/ 
/***********************************************************************************************************/
void get_input(){ 
	int i,k,o;
	char ch = 0;
	
	uart0_init();
	i = 0;
	//getting input from xbee
	while(ch != '*'){
		ch = uart_rx();
		xbee_input[i++] = ch;
		uart_tx(xbee_input[i - 1]);
						
	}
	start = xbee_input[0] - '0';
	i = 1;
	num_positions = 0;
	ch = xbee_input[i];
	//the positions and the respective orientations are saved in arrays
	while(ch != '*'){
		position[num_positions] = xbee_input[i++] - 'a';
		orientation[num_positions] = xbee_input[i++] - '0';
		num_positions++;
		ch = xbee_input[i];
	}	
}
int main()
{
	  init();
	  init_devices();
	  uart0_init();
	  int n = 0, o;
	  //Port initialization
	  DDRC = 0xFF;
	  DDRJ = 0xFF;
	  DDRB = 0xFF;
	  DDRK = 0xFF;
	  PORTC = 0xFF;
	  PORTJ = 0x00;	  
	  int k;
	  //check variable is to check the bot is started so that the pebble positions can be sent by the python script
	  char check;
	  while(1){
		  check = uart_rx();
		  uart_tx(check);
		 
		  if(check == '*'){
			  break;
		  }
		  
	  }
	  
	  led[0] = led[1] = led[2] = 0;
	  get_input();
	  //initialize previous node and current node according to start position
	  if(start == 1){
		  //these are arena positions in grid 
		  prev_x = -1;
		  prev_y = 5;
		  curr_x = 0;
		  curr_y = 5;
	  }
	  else{
		  prev_x = 12;
		  prev_y = 5;
		  curr_x = 11;
		  curr_y = 5;		  
	  }
	  n = 0;
	  
	  //'destination_positions' is to save the four positions of next cell
	  int destination_positions[4];
	  positions_done = 0;

	  buzzer_of();			
	  get_destination();
	  get_direction();
	  stop();
	  _delay_ms(1000);
	  n = 0;
	  while(1)
	  {
		  
			line_sensor();  
		  
		  //forward
		  if(led[0] == 0 && led[1] == 0 && led[2] == 0){
			  stop();
		  }
		  
		  else if(led[0] == 0 && led[1] == 1 && led[2] == 0){
			  //velocity(122, 117);
			  velocity(165, 165);
			  forward();
		  }
		  else if(led[0] == 1 && led[1] == 0 && led[2] == 0){
			  //velocity(120,130);
			  velocity(165, 160);
			  soft_right();
		  }
		  else if(led[0] == 0 && led[1] == 0 && led[2] == 1){
			  //velocity(130, 120);
			  velocity(165, 160);
			  soft_left();
		  }
		  else if(led[0] == 0 && led[1] == 1 && led[2] == 1){
			  velocity(130,120);
			  //velocity(165, 160);
			  soft_left();
		  }
		  else if(led[0] == 1 && led[1] == 1 && led[2] == 0){
			  velocity(120,130);
			  //velocity(165, 160);
			  soft_right();
		  }
		   else if(led[0] == 1 && led[1] == 1 && led[2] == 1){
				//node detected 
				if(n == num_direction){
					//reached the destination
					uart_tx('#');
					if(flag_magnet == 0){
						magnet_on();						
					}
					else{
						//magnet_on();
					}
					pickup();
					forward();
					n = -1;
				}
				else if(n == -1){
					//when we need destination after picking up the bot
					get_destination();
					get_direction();
					n = 0;
				}
				else
				{
					if(direction[n] == LEFT){
						new_left();
						uart_tx('L');
					}
					else if(direction[n] == RIGHT){
						new_right();
						uart_tx('R');
					}
					else{
						uart_tx('E');
						reverse();					
					}
					n++;	  
				}
			}  
	  }	  
}
