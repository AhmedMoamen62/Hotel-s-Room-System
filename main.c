#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "LCD/lcd.h"
#include "Keypad/keypad.h"
#include "tm4c123gh6pm.h"
#include "GPIO_Drivers/GPIO.h"
#include "GPIO_Drivers/Data_Type.h"
#include "UART/uart.h" 
#define PASSWORD_SIZE 4

/*************************** FUNCTION DECLRATIONS *************************/ 
//void test_LCD();
void UART_Config(); 
//void test_LCD();
void password_approved();
void Login();
void getPassword(uint8_t * pw_arr);
bool Login_Validation(char * pw_arr , char* pw_true);
void gpio_config();
void setupHW();
void printmsg(char *format,...);
void roomFree_Handling();
uint8_t getRoomNumber(); 
void changeRoomStatus();
void changeRoomPW();
void roomOccupied_Handling(); 
void roomCleaning_Handling(); 
void openDoor();
void closeDoor(); 
//void EEPROM_FillPassword(char* password);
//void uint32_tToChar(char*,uint8_t,uint32_t);
void buzzer();
//void LCD_correct();
//uint32_t Pow(uint32_t,uint32_t);
//int32_t charToint(char * arr);
 
/************************** Definitions *******************************/
#define ROOM_FREE 		0
#define ROOM_OCCUPIED	1
#define ROOM_CLEANING	2

#define DOOR_CLOSED		0
#define DOOR_OPEN			1
/************************** HANDLE STRUCTS ********************************/ 
UART_HandleTypedef huart ; 
GPIO_HandlingPin portApin2;
GPIO_HandlingPin portFpin3;

/************************** GLOBAL VARIABLES *******************************/
char pw_true[PASSWORD_SIZE];
struct 
{
	uint8_t number ; 
	uint8_t room_state ;
	uint8_t door_state;
	char pw[4]; 
}room;
/************************** MAIN FUNCTION ************************************/

int main()
{  
	gpio_config();
	UART_Config();
	UART4_init(&huart); 
	keypad_Init();
	LCD_Init();
	uint16_t flag=1;
	char pw_arr[4];
	LCD_Clear();
	printmsg("\t\tWelcome to The Hotel\r\n"); 

	// filling the Global struct
	setupHW(); 

	while(1)
	{
			if(room.room_state==ROOM_OCCUPIED)
			{
				while(1)
				{
					getPassword(pw_arr);
					LCD_Clear();
					if(Login_Validation(pw_arr,room.pw))
					{
						LCD_Write_String("SUCCESS");
						delay_m(1000);
						break  ; 
					}
					else
					{
						LCD_Write_String("FALIED");
						delay_m(1000); 
						LCD_Clear(); 
					}
				}
					roomOccupied_Handling();
			}
				 
			while(room.number!=getRoomNumber())
			{
				printmsg("\r\nINVALID Room Number\r\n");
			}
		switch (room.room_state)
		{
			case ROOM_FREE 		 : roomFree_Handling(); 
											break ; 
			case ROOM_OCCUPIED : roomOccupied_Handling(); 
											break ;
			case ROOM_CLEANING : roomCleaning_Handling(); 
											break ; 	
			default : roomFree_Handling(); 
										
		}
		
				
	}
}



/*************************** FUNCTION DEFINITIONS *************************/ 

bool Login_Validation(char * pw_arr , char* pw_true)
{
	
	for ( uint8_t j=0; j<PASSWORD_SIZE; j++){
				if(pw_arr[j] != pw_true[j]){
					GPIO_PORTD_APB_DATA_PIN3 = 0x08;
					return 0;
				}
		}
	return 1;
}

void getPassword(uint8_t * pw_arr)
{	
	uint16_t i=0;
	char space_counter = 0;
		LCD_Clear(); 
		LCD_Write_String("Enter Password");
		LCD_Set_Cursor_Position(1,0); 
	while(1)
	{		
			char x=KeyPad_getPressedKey();
			//printmsg("\r\nthe pressed key is %c and i=%d\r\n",x,i);
			if (x == '/' && i == PASSWORD_SIZE )
			{
				if(space_counter < PASSWORD_SIZE )
				{
					LCD_Clear();
					LCD_Write_String("Invalid Password");
					LCD_Set_Cursor_Position(1,6);					
					i=0;
					space_counter = 0;
				}
				else
				{
					return;
				}
					
			}
			
			else if (x =='.'&& i > 0 )
			{
				LCD_Shift_Cursor_Left();
				i--;
			}
			else if ( x== '=' &&  i < PASSWORD_SIZE )
			{
				LCD_Shift_Cursor_Right();
				i ++;
			}
			else if (x=='/' && i > 0 &&  i <= PASSWORD_SIZE)
			{
				LCD_Back(); 
				--i ;
				space_counter --;
			}

			else if( ( x >= '0' && x <= '9' ) && i < PASSWORD_SIZE  ) 
			{
				LCD_Write_Char('*'); 
				pw_arr[i]=x;
				i++; 
				space_counter++;
			}

			delay_m(1000);
	}
}

void Login()
{
	
		while(1)
		{
					uint8_t password1[PASSWORD_SIZE] , password2[PASSWORD_SIZE];
					// get the password first time ... clear, write in the first line , set the cursor to the password position , get the password
					LCD_Clear();
					LCD_Set_Cursor_Position(1,6);
					getPassword(password1);
				
					// re enter the password
					LCD_Clear();
					LCD_Write_String("ReEnter Password");
					LCD_Set_Cursor_Position(1,6);
					getPassword(password2);

					if( Login_Validation(password1,password2) )
					{
								uint8_t count=4 ; 
								for(uint8_t i=0 ; i<count ; i++)
								{
									room.pw[i]=password2[i];
								}
								LCD_Clear();
								LCD_Write_String("Password Changed!");
								LCD_Blink();
								LCD_Clear();
								return;
					}
					else
					{
								LCD_Clear();
								LCD_Write_String("Password is not Matched");
								buzzer();
								LCD_Blink();
								LCD_Clear();
								return ; 
					}
									
			}

		
}


void UART_Config()
{

	huart.BaudRate = 9600 ;
	huart.FIFO = FIFO_ENABLE ; 
	huart.HighSpeed = HIGH_SPEED_DISABLE ; 
	huart.ParityEnable= PARITY_DISABLE ; 
	huart.StopBits = UART_STOPBITS_1; 
	huart.WordLength= UART_WORDLENGTH_8B ; 
	huart.TransmitEnable= TRANSMIT_ENABLE ;
	huart.ReceiveEnable= RECEIVE_ENABLE;
}



void buzzer()
{
	GPIO_PORTF_APB_DATA_PIN3 = 0x08;
	delay_m(400);
	GPIO_PORTF_APB_DATA_PIN3 = 0x00;
	delay_m(500);
	GPIO_PORTF_APB_DATA_PIN3 = 0x08;
  delay_m(700);
	GPIO_PORTF_APB_DATA_PIN3 = 0x00;
	delay_m(500);
	GPIO_PORTF_APB_DATA_PIN3 = 0x08;
  delay_m(700);
	GPIO_PORTF_APB_DATA_PIN3 = 0x00;
}


void gpio_config()
{

  portApin2.PortBase = GPIO_PORTA_APB_BASE;
	portApin2.AlternateFunctionSelect = 0;
	portApin2.AnalogModeSelect = 0;
	portApin2.Commit = 1;
	portApin2.DigitalEnable = 1;
	portApin2.Direction = Output;
	portApin2.Lock = 1;
	portApin2.PinNumber = 2;
	portApin2.PullDownSelect = 0;
	portApin2.PullUpSelect = 0;
	portApin2.PortControl = 0;
	
	GPIO_InitialPin(&portApin2);
	

  portFpin3.PortBase = GPIO_PORTF_APB_BASE;
	portFpin3.AlternateFunctionSelect = 0;
	portFpin3.AnalogModeSelect = 0;
	portFpin3.Commit = 1;
	portFpin3.DigitalEnable = 1;
	portFpin3.Direction = Output;
	portFpin3.PinNumber = 3;
	portFpin3.PullDownSelect = 0;
	portFpin3.PullUpSelect = 0;
	portFpin3.PortControl = 0;
	
	GPIO_InitialPin(&portFpin3);
}

void setupHW()
{
	UART4_trasnmitString("\r\nplease Enter Room Number\r\n"); 
	room.number =(uint8_t)UART4_receiveChar()-48;
	
	UART4_trasnmitString("\r\nplease Enter dafualt password num by num\r\n"); 
	uint8_t count=4 ; 
	for(uint8_t i=0 ; i<count ; i++)
	{
		// making -48 to convert the coming char numbers to int numbers
		room.pw[i]=UART4_receiveChar();
	}
	changeRoomStatus();
}
void printmsg(char *format,...)
{

	char str[80];

	/* Extract the arguments list using VA APIs */
	va_list args;
	va_start(args,format);
	vsprintf(str,format,args);
	UART4_trasnmitString(str);
	va_end(args);

}

void roomFree_Handling()
{
	printmsg("\r\n\tROOM_FREE\r\n1-Change Room status\r\n2-Change password\r\n");
	uint8_t user_command = UART4_receiveChar(); 
	
	switch(user_command)
	{
		case '1': changeRoomStatus();
							break ; 
		
		default : changeRoomPW();
							break ; 		
	}
}

uint8_t getRoomNumber()
{
	printmsg("\r\nPlease Enter Room Number\r\n");
	return UART4_receiveChar()-48; 
}

void changeRoomPW()
{
	while(1)
	{
			printmsg("\r\nPlease Enter the old Password\r\n"); 
			char temp_PW[4]; 
			uint8_t count=4 ; 
			for(uint8_t i=0 ; i<count ; i++)
			{
				// making -48 to convert the coming char numbers to int numbers
				temp_PW[i]=UART4_receiveChar();
			}
			if(Login_Validation(temp_PW,room.pw))
			{
				break ; 
			}
	}
	
	// correct old PW
			printmsg("\r\nPlease Enter the New Password\r\n"); 
			uint8_t count=4 ; 
			for(uint8_t i=0 ; i<count ; i++)
			{
				room.pw[i]=UART4_receiveChar();
			}
			printmsg("\r\nSuccessful Operation Passowrd is Changed ! \r\n"); 
}

void changeRoomStatus()
{
	UART4_trasnmitString("\r\nplease Enter Room Status\r\n"); 
	uint8_t temp_status =UART4_receiveChar()-48;
	
	switch(temp_status)
	{
		case ROOM_FREE 		: 	room.room_state=ROOM_FREE;
													room.door_state=DOOR_CLOSED ; 
													printmsg("\r\nRoom_number: %d \r\nRoom_status: Free \r\nDoor_status: Closed\r\n",room.number);
										break ; 
		case ROOM_OCCUPIED  : room.room_state=ROOM_OCCUPIED;
													room.door_state=DOOR_CLOSED ; 
													printmsg("\r\nRoom_number: %d \r\n",room.number);
													printmsg("Room_status: Occupied \r\nDoor_status: Closed\r\n");
										break ;
		case ROOM_CLEANING  :	room.room_state=ROOM_CLEANING;
													room.door_state=DOOR_OPEN ;
													printmsg("\r\nRoom_number: %d \r\nRoom_status: Cleaning \r\nDoor_status: Opened\r\n",room.number);
										break ;
		default: 							room.room_state=ROOM_OCCUPIED;
													room.door_state=DOOR_CLOSED ; 
													printmsg("i'm in default\r\n"); 
	}
	
}
void roomCleaning_Handling()
{
	printmsg("\r\n\tROOM_Cleaning\r\nChange Room status\r\n");
	changeRoomStatus();
}
	
void roomOccupied_Handling()
{
	while(room.room_state==ROOM_OCCUPIED)
	{
		LCD_Clear();
		LCD_Write_String_Position(0,0,"1open 2close 3PW");
		LCD_Write_String_Position(1,0,"4clean");
		LCD_Write_String_Position(1,7,"5checkout");
		//printmsg("\r\n\tROOM_Occupied\r\n1-Open Door\r\n2-Close Door\r\n3-Change password\r\n");
	//	printmsg("4-Request Clean\r\n5-check out\r\n");
		char user_command = KeyPad_getPressedKey(); 
		LCD_Clear();
		//printmsg("\r\nuser command is : %c\r\n",user_command);

		switch(user_command)
		{
			case '+' :
								openDoor();
								LCD_Clear();
								LCD_Write_String("Door is opened");
								delay_m(2000); 
								LCD_Clear(); 
										break ; 
			case '-' : 
								closeDoor(); 
								LCD_Clear();
								LCD_Write_String_Position(0,0,"Door is closed");
								delay_m(2000); 
								LCD_Clear();
								break ;
			case '*' : Login();
								break ; 
			case '/' :printmsg("\r\nRequest Cleaning Room Number: %d \r\n",room.number) ;
								room.room_state=ROOM_CLEANING;
								room.door_state=DOOR_OPEN ;
								printmsg("\r\nRoom_number: %d \r\nRoom_status: Cleaning \r\nDoor_status: Opened\r\n",room.number);
								LCD_Clear();
								LCD_Write_String_Position(0,0,"Request Clean");
								LCD_Write_String_Position(1,0,"is submitted");
								LCD_Blink(); 
								LCD_Clear();
								break ; 	
			case '=' : printmsg("\r\ncheck out Room Number: %d \r\n",room.number) ;
								room.room_state=ROOM_FREE;
								room.door_state=DOOR_CLOSED ;
								printmsg("\r\nRoom_number: %d \r\nRoom_status: Free \r\nDoor_status: Closed\r\n",room.number);
								LCD_Clear();
								LCD_Write_String_Position(0,0,"checkout");
								LCD_Write_String_Position(1,0,"is submitted");
								LCD_Blink(); 
								LCD_Clear();
								break ; 
			default: 
								LCD_Clear();
								LCD_Write_String_Position(0,0,"Invalid Input");
								LCD_Clear();
		}
	}
	
}

void openDoor()
{
	GPIO_PORTF_APB_DATA_PIN3 = 0x08;
}
void closeDoor()
{
	GPIO_PORTF_APB_DATA_PIN3 = 0x00;
}