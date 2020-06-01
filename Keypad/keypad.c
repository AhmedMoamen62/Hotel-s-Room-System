#include "keypad.h"
unsigned int ncols = 0;
unsigned int nrows = 0;                                                                    
uint8_t symbol[N_row][N_col] = {{ '1', '2',  '3', '+'},      
																{ '4', '5',  '6', '-'},      
																{ '7', '8',  '9', '*'},      
                                { '.', '0',  '=', '/'}}; 

void keypad_Init(void)
{
  SYSCTL_RCGCGPIO_R |= 0x18;            //enable clc for port E & D  
  while ((SYSCTL_RCGCGPIO_R&0x18)==0);  //wait for clock to be enabled
  //GPIO_PORTB_CR_R  |= 0xF0;           //allow changes to all the bits in port D
 // GPIO_PORTE_CR_R  |= 0x1E;           //allow changes to all the bits in port E
  GPIO_PORTD_DIR_R |= 0x0F;             //set 1st 4 bits in PORT D directions cols are o/ps
  GPIO_PORTE_DIR_R &= ~(0x1E);          //set PE1-->PE4 in PORT E directions raws are i/ps
  GPIO_PORTE_PDR_R |= 0x1E;             //pull down resistor on Raws
  GPIO_PORTD_DEN_R |= 0x0F;             //digital enable pins in port D
  GPIO_PORTE_DEN_R |= 0x1E;             //digital enable pins in port E
}

/**
*    This function returns the value of a key have been pressed at the moment the function had been called. 
*    It simply makes a 2 loops one to find the column which contains the button 
*    had been pressd and another loop to determine the row of that button. 
*    Having these data the we could determine a unique value for each button.
*/
uint8_t KeyPad_getPressedKey(void)
{
	uint32_t i,j;
  while(1)
  {
    for(i = 0; i < N_col; i++)                        //to determine the column
    {
      GPIO_PORTD_DATA_R = (1 << i);
      for(j = 0; j < N_row; j++)                     //to determing the row
      {
        if((GPIO_PORTE_DATA_R & 0x1E) & (1U << (j+1)) ) //
          return symbol[j][i];
      }
    }
  }
}