/**********************************************************************************************************************
File: anttt.c                                                                

Description:
Implements TIC-TAC-TOE using data input from ANT or BLE.



**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_xxAnttt"
***********************************************************************************************************************/
/* New variables */
u32 G_u32AntttFlags;                                     /* Global state flags */
fnCode_type ANTTT_SM;

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */
extern volatile u32 G_u32BPEngenuicsFlags;             /* From bleperipheral_engenuics.c  */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Anttt_<type>" and be declared as static.
***********************************************************************************************************************/
//static u32 Anttt_u32Timeout;                             /* Timeout counter used across states */
//static u32 Anttt_u32CyclePeriod;                         /* Current base time for Anttt modulation */
static u8 Anttt_u8RxData[ANTTT_COMMAND_SIZE];
static u16 Anttt_u16HomeState;
static u16 Anttt_u16AwayState;
static bool bPendingResponse;

u16 au16WinningCombos[] = 
{
  0x0007,        // 0b000000111
  0x0038,        // 0b000111000
  0x01C0,        // 0b111000000
  0x0049,        // 0b001001001
  0x0092,        // 0b010010010
  0x0124,        // 0b100100100
  0x0111,        // 0b100010001
  0x0054         // 0b001010100
};



/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/
void AntttIncomingMessage(u8* pu8Data_, u8 u8Length_)
{
  // Check length of the Command Size.
  if (u8Length_ != ANTTT_COMMAND_SIZE)
  {
    return;
  }
  
  memcpy(&Anttt_u8RxData, pu8Data_, u8Length_);
}


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: AntttInitialize

Description:
Initializes the State Machine and its variables.

Requires:

Promises:
*/
void AntttInitialize(void)
{
  Anttt_reset_rx_buffer();
  Anttt_u16HomeState = Anttt_u16AwayState = 0;
  ANTTT_SM = &AntttSM_Idle;
  bPendingResponse = false;
  
  // Set up initial LEDs.
  LedOn(STATUS_RED);
  LedOff(STATUS_YLW);
  LedOff(STATUS_GRN);
  
  for (u8 u8Led = 0; u8Led < (TOTAL_BUTTONS * 2); u8Led++)
  {
    LedOff((LedNumberType) u8Led);
  }
  
  nrf_gpio_pin_clear(16);
  
} /* end AntttInitialize() */


/*--------------------------------------------------------------------------------------------------------------------
Function: AntttHandleIncomingMessage(u8* pu8Data_, u8 u8Length_)

Description:
Copies incoming data to teh Anttt Rx buffer.

Requires:
- pu8Data_ points to the data string
- u8Length_ is the size of the data

Promises:
- Anttt_u8RxData is filled with pu8Data_ if length is correct;
  otherwise does nothing.
*/
void AntttHandleIncomingMessage(u8* pu8Data_, u8 u8Length_)
{
  // Check the appropriate length.
  if (u8Length_ == ANTTT_COMMAND_SIZE)
  {
    memcpy(Anttt_u8RxData, pu8Data_, u8Length_);
  }
}


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: static bool Anttt_is_game_over(void)

Description:
Check to see how a game has ended

Requires:
- Anttt_u16HomeState holds the game data for the "home" team
- Anttt_u16AwayState holds the game data for the "away" team

Promises:
- Anttt_u8RxData is filled with pu8Data_ if length is correct;
  otherwise does nothing.
*/
static bool Anttt_is_game_over(void)
{
  // Check all 8 winning combinations.
  for (u8 i = 0; i < 8; i++)
  {
    if ((Anttt_u16HomeState & au16WinningCombos[i]) == au16WinningCombos[i])
    {
      Anttt_u16HomeState = au16WinningCombos[i];
      Anttt_u16HomeState |= _U16_ANTTT_WIN_FLAG;       // Set this flag to indicate home won.
      
      return true;
    }
    else if ((Anttt_u16AwayState & au16WinningCombos[i]) == au16WinningCombos[i])
    {
      Anttt_u16AwayState = au16WinningCombos[i];
      Anttt_u16AwayState |= _U16_ANTTT_WIN_FLAG;       // Set this flag to indicate away winning won.
      
      return true;
    }
  }
  
  // Check if draw.
  if ((Anttt_u16HomeState | Anttt_u16AwayState) == U16_ANTTT_DRAW)
  {
    return true;
  }
  
  return false;
}


/*--------------------------------------------------------------------------------------------------------------------
Function: static void Anttt_reset_rx_buffer(void)

Description:
Clear the Rx buffer

Requires:
- 

Promises:
- Anttt_u8RxData is filled with 0xFF
*/
static void Anttt_reset_rx_buffer(void)
{
  u8 u8Status;
  
  SystemEnterCriticalSection(&u8Status);
  memset(Anttt_u8RxData, 0xFF, ANTTT_COMMAND_SIZE);   
  SystemExitCriticalSection(u8Status);
}


/*--------------------------------------------------------------------------------------------------------------------*/
/* State Machine definitions                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
State: AntttSM_Idle
*/
static void AntttSM_Idle(void)
{
  // Check if module is connected to client.
  if (G_u32BPEngenuicsFlags == _BPENGENUICS_CONNECTED)
  {
    // Set LEDs and proceed to wait state.
    LedOn(STATUS_GRN);   // Connected to Client.
    
    // Reset any pending button states incase the user was pressing while waiting.
    ButtonInitialize();
    
    ANTTT_SM = &AntttSM_Wait;
  }
  
#if 0
  /* TEST CODE TO TEST BUTTONS */
  for (u8 i = 0; i < TOTAL_BUTTONS; i++)
  {
   if (WasButtonPressed(i))
   {
     LedToggle(0xFF);
     ButtonAcknowledge(i);
     return;
   }
  }
#endif
  
} 

/*--------------------------------------------------------------------------------------------------------------------
State: AntttSM_Wait
Wait for the other player to make a move.
*/
static void AntttSM_Wait(void)
{
  u8 au8Temp[ANTTT_COMMAND_SIZE];
  u8 u8Position;
  
  // Check if module has established connection with client.
  if (G_u32BPEngenuicsFlags & _BPENGENUICS_CONNECTED)
  {
    // Wait for Client to make a move.
    if (Anttt_u8RxData[ANTTT_COMMAND_ID_OFFSET] == ANTTT_COMMAND_ID_MOVE)
    {
      u8Position = Anttt_u8RxData[ANTTT_COMMAND_POSITION_OFFSET];
      
      // Check if position is already chosen or incorrect position sent.
      if ( (Anttt_u16HomeState & (1 << u8Position)) || 
           (Anttt_u16AwayState & (1 << u8Position)) ||
           (u8Position >= TOTAL_BUTTONS) )
      {
        return;
      }
      
      // New Position.
      LedOn((LedNumberType) (u8Position + U8_AWAY_POSITION_OFFSET));  
      Anttt_u16AwayState |= 1 << (u8Position);
      
      // Send response.
      au8Temp[ANTTT_COMMAND_ID_OFFSET] = ANTTT_COMMAND_ID_MOVE_RESP;
      BPEngenuicsSendData(au8Temp, ANTTT_COMMAND_SIZE);
      
      // Check if game is over.
      if (Anttt_is_game_over())
      {
        // Set up initial LEDs.
        LedOff(STATUS_RED);
        LedOff(STATUS_YLW);
        LedOff(STATUS_GRN);
  
        for (u8 i = 0; i < (TOTAL_BUTTONS * 2); i++)
        {
          LedOff((LedNumberType) i);
        }
        
        ANTTT_SM = &AntttSM_Gameover;
        return;
      }
      
      Anttt_reset_rx_buffer();
      
      // Update State.
      ANTTT_SM = &AntttSM_Active;
      LedOn(STATUS_YLW);
    }
  } /* end if (G_u32BPEngenuicsFlags & _BPENGENUICS_CONNECTED) */
  else
  {
    // Disconnected from client.
    AntttInitialize();
  }
  
  // User may be pressing buttons in this state, Ack the button presses to 
  // disable queuing of the presses as valid game presses.
  for (u32 u32Button = 0; u32Button < TOTAL_BUTTONS; u32Button++)
  {
    if (WasButtonPressed(u32Button) & !bPendingResponse)
    {
      ButtonAcknowledge(u32Button);  
    }
  }
}

/*--------------------------------------------------------------------------------------------------------------------
State: AntttSM_Active
Local player makes a move
*/
static void AntttSM_Active(void)
{
  // Check if module has established connection with client.
  if (G_u32BPEngenuicsFlags & _BPENGENUICS_SERVICE_ENABLED)
  {
    // Make a move.
    // Check if a button was pressed, then update UI and send message.  
    for (u8 u8Button = 0; u8Button < TOTAL_BUTTONS; u8Button++)
    {
      if (WasButtonPressed(u8Button) & !bPendingResponse)
      {
        u8 au8Temp[ANTTT_COMMAND_SIZE];
        
        // Check if a valid press. Check if position already chosen via client
        // or itself or incorrect button index reported
        if ( (Anttt_u16HomeState & (1 << u8Button)) || 
             (Anttt_u16AwayState & (1 << u8Button)) ||
             (u8Button >= TOTAL_BUTTONS))
        {
          continue;
        }
        
        // Button index directly corresponds to Home LED Index.
        // First Update UI.
        LedOn((LedNumberType) u8Button);
        Anttt_u16HomeState |= 1 << (u8Button);
        
        // Send message to client.
        au8Temp[ANTTT_COMMAND_ID_OFFSET] = ANTTT_COMMAND_ID_MOVE;
        au8Temp[ANTTT_COMMAND_POSITION_OFFSET] = u8Button;
        au8Temp[ANTTT_COMMAND_SOURCE_OFFSET] = 0;
        BPEngenuicsSendData(au8Temp, ANTTT_COMMAND_SIZE);
        
        ButtonAcknowledge(u8Button);   
        bPendingResponse = true;
        return;
      }
      else if (bPendingResponse)
      {
        // User may be pressing buttons in this state, Ack the button presses to 
        // disable queuing of the presses as valid game presses.
        ButtonAcknowledge(u8Button);    
      }
    }
    
    // Check if response received.
    if (Anttt_u8RxData[ANTTT_COMMAND_ID_OFFSET] == ANTTT_COMMAND_ID_MOVE_RESP)
    {
      Anttt_reset_rx_buffer();
      bPendingResponse = false;
      
      // Check if game is over.
      if (Anttt_is_game_over())
      {
        // Set up initial LEDs.
        LedOff(STATUS_RED);
        LedOff(STATUS_YLW);
        LedOff(STATUS_GRN);
  
        for (u8 i = 0; i < (TOTAL_BUTTONS * 2); i++)
        {
          LedOff((LedNumberType) i);
        }
        
        ANTTT_SM = &AntttSM_Gameover;
        return;
      }
      
      // Update State.
      ANTTT_SM = &AntttSM_Wait;
      LedOff(STATUS_YLW);
    }
  }
  else
  {
    // Disconnected from client.
    AntttInitialize();
  }
}


/*--------------------------------------------------------------------------------------------------------------------
State: AntttSM_Gameover
Game ending sequence
*/
static void AntttSM_Gameover(void)
{   
  // Play Winning Sequence. 
  if ((G_u32SystemTime1ms % 500) == 0)
  {
    nrf_gpio_pin_toggle(16);
    
    // Toggle LED sequences.
    LedToggle(STATUS_GRN);
    LedToggle(STATUS_RED);
    LedToggle(STATUS_YLW);
    
    // Blink winning sequence.
    if (Anttt_u16HomeState & _U16_ANTTT_WIN_FLAG)
    {
      // Home won.
      u8 au8Three[3];
      u8 u8Index = 0;
      u16 u16Temp = Anttt_u16HomeState;  
      
      // Convert LED bitmask to LED value.
      for (u8 i = 0; i < 9; i++)
      {
        if (u16Temp & 0x01)
        {
          au8Three[u8Index++] = i;
        }
        
        u16Temp = u16Temp >> 1;
      }
      
      LedToggle((LedNumberType)au8Three[0]);
      LedToggle((LedNumberType)au8Three[1]);
      LedToggle((LedNumberType)au8Three[2]);
    }
    else if (Anttt_u16AwayState & _U16_ANTTT_WIN_FLAG)
    {
      // Away won.
      u8 au8Three[3];
      u8 u8Index = 0;
      u16 u16Temp = Anttt_u16AwayState;  
      
      // Convert LED bitmask to LED value.
      for (u8 i = 0; i < 9; i++)
      {
        if (u16Temp & 0x01)
        {
          au8Three[u8Index++] = i;
        }
        
        u16Temp = u16Temp >> 1;
      }
      
      LedToggle((LedNumberType)(au8Three[0] + 9));
      LedToggle((LedNumberType)(au8Three[1] + 9));
      LedToggle((LedNumberType)(au8Three[2] + 9));
    }
    else
    {
      // Play Draw Sequence.
      LedToggle((LedNumberType)(0));
      LedToggle((LedNumberType)(1));
      LedToggle((LedNumberType)(2));
      LedToggle((LedNumberType)(3));
      LedToggle((LedNumberType)(5));
      LedToggle((LedNumberType)(6));
      LedToggle((LedNumberType)(7));
      LedToggle((LedNumberType)(8));
      LedToggle((LedNumberType)(0 + U8_AWAY_POSITION_OFFSET));
      LedToggle((LedNumberType)(1 + U8_AWAY_POSITION_OFFSET));
      LedToggle((LedNumberType)(2 + U8_AWAY_POSITION_OFFSET));
      LedToggle((LedNumberType)(3 + U8_AWAY_POSITION_OFFSET));
      LedToggle((LedNumberType)(5 + U8_AWAY_POSITION_OFFSET));
      LedToggle((LedNumberType)(6 + U8_AWAY_POSITION_OFFSET));
      LedToggle((LedNumberType)(7 + U8_AWAY_POSITION_OFFSET));
      LedToggle((LedNumberType)(8 + U8_AWAY_POSITION_OFFSET));
    }
  }
  
  // Check if any button was pressed and return to initialized state.
  for (u8 u8Button = 0; u8Button < TOTAL_BUTTONS; u8Button++)
  {
    if (WasButtonPressed(u8Button))
    {
      AntttInitialize();
      ButtonAcknowledge(u8Button);
      return;
    }
  }
}



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
