/*!*********************************************************************************************************************
@file term_tac_toe.c                                                                
@brief Manages commands and terminal display for tic-tac-toe.

**** SIZE THE TERMINAL WINDOW 17 x 14 ****

This is not a game engine -- it is simply an interface to take game commands
from the nRF processor and update the terminal display and/or LCD; or it 
takes commands from the terminal and sends them to the nRF.  The only
part of the game that is tracked are the available spaces.

------------------------------------------------------------------------------------------------------------------------
GLOBALS
- NONE

CONSTANTS
- NONE

TYPES
- NONE

PUBLIC FUNCTIONS
- NONE

PROTECTED FUNCTIONS
- 


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>TermTacToe"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32TermTacToeFlags;                            /*!< @brief Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;                   /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;                    /*!< @brief From main.c */

extern u8 G_au8DebugScanfBuffer[DEBUG_SCANF_BUFFER_SIZE]; /* From debug.c */
extern u8 G_u8DebugScanfCharCount;                        /* From debug.c */

extern u8 G_au8UtilMessageOK[];                           /*!< @brief From utilities.c */
extern u8 G_au8UtilMessageFAIL[];                         /*!< @brief From utilities.c */
extern u8 G_au8UtilMessageON[];                           /*!< @brief From utilities.c */
extern u8 G_au8UtilMessageOFF[];                          /*!< @brief From utilities.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "TermTacToe_<type>" and be declared as static.
***********************************************************************************************************************/
static fnCode_type TermTacToe_pfStateMachine;               /*!< @brief The state machine function pointer */
static u32 TermTacToe_u32Timeout;                           /*!< @brief Timeout counter used across states */
static u32 TermTacToe_u32Flags;
static bool TermTacToe_bLocalIsEx;
static bool TermTacToe_bLocalTurn;

//static u8 TermTacToe_au8UartBuffer[U8_NRF_BUFFER_SIZE];     /*!< @brief Space for verified received application messages */
//static u8* TermTacToe_pu8RxBufferNextChar;                  /* Pointer to next char to be written in the buffer */

static u8 TermTacToe_au8AppMessage[U8_ANTTT_MAX_MESSAGE_LENGTH];        /*!< @brief Latest received application message */

static u8 TermTacToe_au8TerminalInputBuffer[U8_TERMINAL_INPUT_BUFFER_SIZE];

static u8 TermTacToe_au8GameBoard[] = "     |     |\n\r  0  |  1  |  2\n\r     |     |\n\r-----|-----|-----\n\r     |     |\n\r  3  |  4  |  5 \n\r     |     |\n\r-----|-----|-----\n\r     |     |\n\r  6  |  7  |  8\n\r     |     |\n\n\n\r";

                                            /*12345678901234567*/
static u8 TermTacToe_au8UserMessage[][18] = {" WAIT TO CONNECT ",
                                             "    YOUR TURN    ",
                                             "    THEIR TURN   ",
                                             "    YOU WIN!!!   ",
                                             "    YOU LOSE!!   ",
                                             "    DRAW GAME    ",
                                            };

static bool TermTacToe_abAvailableSpaces[9];

static u8 TermTacToe_au8SpiGameMove[] = {NRF_SYNC_BYTE, ANTTT_APP_MSG_GAME_MOVE_LENGTH, 
                                         ANTTT_APP_MSG_GAME_MOVE, 0, GAME_MOVE_DATA_STATUS_UNKNOWN};


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/*!--------------------------------------------------------------------------------------------------------------------
@fn void TermTacToeInitialize(void)
@brief Sets up the game board

Should only be called once in main init section.

Requires:
- Ensure the terminal program is open and connected to the development board.
- For best results, set the terminal window size to 18x16

Promises:
- NONE

*/
void TermTacToeInitialize(void)
{ 
  /* Startup message */
  DebugPrintf("### TIC-TAC-TOE ###\n\rPlease set terminal to 17 x 14\n\r");
  
  /* Turn off Debug command processing and echo */
  DebugSetPassthrough();
  DebugEchoOff();
  DebugPrintf(TERM_CUR_HIDE);
  
  /* Start with a setup state */
  TermTacToe_pfStateMachine = TermTacToeSM_Setup;

} /* end TermTacToeInitialize() */

  
/*!----------------------------------------------------------------------------------------------------------------------
@fn void TermTacToeRunActiveState(void)

@brief Selects and runs one iteration of the current state in the state machine.

All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
- State machine function pointer points at current state

Promises:
- Calls the function to pointed by the state machine function pointer

*/
void TermTacToeRunActiveState(void)
{
  TermTacToe_pfStateMachine();

} /* end TermTacToeRunActiveState */


/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!----------------------------------------------------------------------------------------------------------------------
@fn void TermTacToeWriteSquare(u8 u8Square_, bool bX_)

@brief Draws an X or O at the location

Requires:
- Game board is configured and starts at address 1,1
@PARAM u8Square_ is the to update (0...8); it is assumed
to be vetted for correctness (i.e. it is available)
@PARAM bX_ is TRUE for a X

Promises:
- Requested square is updated to the shape requested
- Associated value in TermTacToe_abAvailableSpaces set to FALSE.

*/
void TermTacToeWriteSquare(u8 u8Square_, bool bX_)
{
  u8 au8CharToWrite[] = "O";
  u8 au8CharColor[]   = TERM_TEXT_RED;
  
  /* There are only ever 3 choices of column and row so we
  can save a little space */
  u8 au8Columns[][3] = {"3", "9", "15"};
  u8 au8Rows[][3]    = {"2", "6", "10"};
  
  /* The column and index is calculated to choose from one of the three */
  u8 u8ColumnIndex = u8Square_ % 3;
  u8 u8RowIndex    = u8Square_ / 3;
  
  u8 au8SetCursor[13] = "\033[r;cH";
  u8* pu8ParserSource;
  u8* pu8ParserDest;
 
  /* Update if we're writing an X */
  if(bX_)
  {
    au8CharToWrite[0] = 'X';
    au8CharColor[3]   = '2';
  }
  
  /* Build the command string in the form "\033[r;cH" */
  pu8ParserSource = &au8Rows[u8RowIndex][0];
  pu8ParserDest   = &au8SetCursor[2];
  
  /* Row position */
  do
  {
    *pu8ParserDest = *pu8ParserSource;
    pu8ParserDest++;
    pu8ParserSource++;
  } while(*pu8ParserSource != '\0');
  
  *pu8ParserDest = ';';
  pu8ParserDest++;
  pu8ParserSource = &au8Columns[u8ColumnIndex][0];

  /* Column position */
  do
  {
    *pu8ParserDest = *pu8ParserSource;
    pu8ParserDest++;
    pu8ParserSource++;
  } while(*pu8ParserSource != '\0');
  
  /* End of the control sequence */
  *pu8ParserDest = 'H';
  pu8ParserDest++;
  *pu8ParserDest = '\0';
  
  
  /* Set format (TERM_FORMAT_RESET - TERM_BKG_BLK - TERM_BOLD)
  cursor position, and then write char */
  DebugPrintf("\033[0m\033[40m\033[1m");
  DebugPrintf(au8CharColor);
  DebugPrintf(au8SetCursor);
  DebugPrintf(au8CharToWrite); 
  
  /* Clear the available space */
  TermTacToe_abAvailableSpaces[u8Square_] = FALSE;
  
} /* end TermTacToeWriteSquare() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void TermTacToeWriteUserMessage(UserMessageType eMessageNumber_)
@brief Prints one of the defined game messages in the user message location

TermTacToe_au8UserMessage is indexed to choose the required user
status message.  Messages are always displayed on row 14 and
are inverted. 

Requires:
- u8MessageNumber_ is a valid message

Promises:
- Requested square is updated to the shape requested

*/
void TermTacToeWriteUserMessage(UserMessageType eMessageNumber_)
{
  /* CursorPos-Reset-Blue-Cursor-Reverse */
  DebugPrintf("\033[12;1H\033[0m\033[34m\033[7m");
  if(eMessageNumber_ == TTT_HOME_MOVE)
  {
    DebugPrintf(TERM_BLINK);
  }
  
  /* Write the message */
  DebugPrintf(TermTacToe_au8UserMessage[eMessageNumber_]);
  
  /* Add user prompt for move */  
  if(eMessageNumber_ == TTT_HOME_MOVE)
  {
    /* CursorPos Reset TERM_BKG_BLK TERM_TEXT_WHT TERM_CUR_HIDE */
    DebugPrintf("\033[13;1H\033[0m\033[40m\033[37m\033[?25l");
    DebugPrintf(" Enter Location");
  }
  /* else clear the move prompt line */  
  else
  {
    /* CursorPos TERM_BKG_BLK TERM_DELETE_RIGHT TERM_TEXT_BLK */
    DebugPrintf("\033[13;1H\033[40m\033[K\033[30m");
  }
  
} /* end TermTacToeWriteUserMessage() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void TermTacToeClearAvailableSpaces(void)
@brief Resets all of the "Available spaces" in the game board array.

Requires:
- NONE

Promises:
- Requested square is updated to the shape requested

*/
void TermTacToeClearAvailableSpaces(void)
{
  for(u8 i = 0; i < 9; i++)
  {
    TermTacToe_abAvailableSpaces[i] = TRUE;
  }
  
} /* end TermTacToeClearAvailableSpaces() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void TermTacToeProcessMove(void)
@brief Processes a GAME_MOVE message during game play.

This should only be called from TermTacToeSM_AwayTurn or TermTacToeSM_HomeTurn

Requires:
- TermTacToe_bLocalIsEx correctly set
- u8Position_ holds valid GAME_MOVE position
- u8GameStatus_ holds a valid GAME_MOVE status

Promises:
- Game board is updated based on status
- If game status is "playing" then SM is not updated; otherwise
  SM next state set to GAME OVER

*/
void TermTacToeProcessMove(CurrentPlayerType ePlayer_, u8 u8Position_, u8 u8GameStatus_)
{
  bool bSymbolToWrite = EX;
  
  /* Determine the correct symbol to write */
  if(ePlayer_ == HOME)
  {
    if(!TermTacToe_bLocalIsEx)
    {
      bSymbolToWrite = OH;
    }
  }
  else
  {
    if(TermTacToe_bLocalIsEx)
    {
      bSymbolToWrite = OH;
    }
  }

  /* Write the symbol */
  TermTacToeWriteSquare(u8Position_, bSymbolToWrite);

  /* Update board and next move depending on game state */
  switch (u8GameStatus_)
  {
    case GAME_MOVE_DATA_STATUS_WINNER_HOME:
    {
      LedOff(YELLOW);
      LedBlink(GREEN, LED_8HZ);
      TermTacToeWriteUserMessage(TTT_WINNER);
      
      TermTacToe_pfStateMachine = TermTacToeSM_GameOver;
      break;
    }

    case GAME_MOVE_DATA_STATUS_WINNER_AWAY:
    {
      LedOff(GREEN);
      LedBlink(YELLOW, LED_8HZ);
      TermTacToeWriteUserMessage(TTT_LOSER);
      
      TermTacToe_pfStateMachine = TermTacToeSM_GameOver;
      break;
    }

    case GAME_MOVE_DATA_STATUS_DRAW:
    {
      LedBlink(YELLOW, LED_1HZ);
      LedBlink(GREEN, LED_1HZ);
      TermTacToeWriteUserMessage(TTT_DRAW);
      
      TermTacToe_pfStateMachine = TermTacToeSM_GameOver;
      break;
    }

    default:
    {
      break;
    }

  } /* end switch */
 
} /* end TermTacToeProcessMove() */


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*!----------------------------------------------------------------------------------------------------------------------
@fn static void TermTacToeSM_Setup(void)
@brief Initialize state during main program 
*/
static void TermTacToeSM_Setup(void)
{
  /* Configure the terminal window */
  DebugPrintf(TERM_BKG_BLK);
  DebugPrintf(TERM_TEXT_YLW);
  DebugPrintf(TERM_CLEAR_SCREEN);
  DebugPrintf(TERM_CUR_HOME);
  
  /* Initialize the game board and game variables */
  DebugPrintf(TermTacToe_au8GameBoard);
  TermTacToeWriteUserMessage(TTT_WAITING);
  TermTacToeClearAvailableSpaces();
  
  /* Advance to start state */
  LedOff(YELLOW);
  LedOff(GREEN);
  LedOn(ORANGE);
  TermTacToe_pfStateMachine = TermTacToeSM_Idle;
  
} /* end TermTacToeSM_Setup() */
 

/*!----------------------------------------------------------------------------------------------------------------------
@fn static void TermTacToeSM_Idle(void)
@brief Monitor the nRF interface for game messages.

Wait for indication for who starts. 
Character input on debug can be ignored (characters are not echoed,
but should be cleared from the buffer if any come in).

Setup: 
Orange LED on to indicate state.  
*/
static void TermTacToeSM_Idle(void)
{
  u8 u8CommandNumber;
  
  /* Look for key presses and read them to ensure the buffer does 
  not overflow; ignore since input is not valid right now. */
  if(G_u8DebugScanfCharCount)
  {
    DebugScanf(TermTacToe_au8TerminalInputBuffer);
  }
  
  /* Wait for game message to indicate which player is starting */
  u8CommandNumber = nrfNewMessageCheck();
  if(u8CommandNumber != NRF_CMD_EMPTY)
  {
    /* Read the message */
    nrfGetAppMessage(TermTacToe_au8AppMessage);
    
    /* The only valid message right now is a game start message */
    if(TermTacToe_au8AppMessage[ANTTT_COMMAND_OFFSET_ID] == ANTTT_APP_MSG_GAME_REQUEST)
    {
      if(TermTacToe_au8AppMessage[ANTTT_COMMAND_OFFSET_DATA0] == GAME_REQUEST_DATA_LOCAL_STARTS)
      {
        LedOn(GREEN);
        TermTacToe_bLocalIsEx = TRUE;
        TermTacToe_bLocalTurn = TRUE;
        TermTacToeWriteUserMessage(TTT_HOME_MOVE);
        TermTacToe_pfStateMachine = TermTacToeSM_HomeTurn;
      }
      else if(TermTacToe_au8AppMessage[ANTTT_COMMAND_OFFSET_DATA0] == GAME_REQUEST_DATA_REMOTE_STARTS)
      {
        LedOn(YELLOW);
        TermTacToe_bLocalIsEx = FALSE;
        TermTacToe_bLocalTurn = FALSE;
        TermTacToeWriteUserMessage(TTT_AWAY_MOVE);
        TermTacToe_pfStateMachine = TermTacToeSM_AwayTurn;
      }
      else
      {
        DebugPrintf("\n\rInvalid GAME_REQUEST data\n\r");
      }
      
      /* For now assume a good GAME_REQUEST so initialize the move information */
      LedOff(ORANGE);
    }
    else
    {
      DebugPrintf("\n\rUnexpected message received\n\r");
    }
  }
  
#ifdef LOCAL_TEST_MODE
  /* Emulate GAME_REQUEST_DATA_LOCAL_STARTS */
  if(WasButtonPressed(BUTTON0))
  {
    ButtonAcknowledge(BUTTON0);

    LedOn(GREEN);
    LedOff(ORANGE);
    TermTacToe_bLocalIsEx = TRUE;
    TermTacToe_bLocalTurn = TRUE;
    TermTacToeWriteUserMessage(TTT_HOME_MOVE);
    TermTacToe_pfStateMachine = TermTacToeSM_HomeTurn;
  }

  /* Emulate GAME_REQUEST_DATA_REMOTE_STARTS */
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    LedOn(YELLOW);
    LedOff(ORANGE);
    TermTacToe_bLocalIsEx = FALSE;
    TermTacToe_bLocalTurn = FALSE;
    TermTacToeWriteUserMessage(TTT_AWAY_MOVE);
    TermTacToe_pfStateMachine = TermTacToeSM_AwayTurn;
  }  
  
#endif /* LOCAL_TEST_MODE */
  

#if 0 /* Character write test */
  if(WasButtonPressed(BUTTON0))
  {
    ButtonAcknowledge(BUTTON0);
    TermTacToeWriteSquare(0, EX);
    TermTacToeWriteUserMessage(TTT_AWAY_MOVE);
  }
  
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    TermTacToeWriteSquare(8, EX);
    TermTacToeWriteUserMessage(TTT_AWAY_MOVE);
  }

  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);
    TermTacToeWriteSquare(0, OH);
    TermTacToeWriteUserMessage(TTT_HOME_MOVE);
  }

  if(WasButtonPressed(BUTTON3))
  {
    ButtonAcknowledge(BUTTON3);
    TermTacToeWriteSquare(7, OH);
    TermTacToeWriteUserMessage(TTT_HOME_MOVE);
  }
#endif /* Character write test */
  
} /* end TermTacToeSM_Idle() */
   

/*!----------------------------------------------------------------------------------------------------------------------
@fn static void TermTacToeSM_HomeTurn(void)          
@brief Play a local turn.

Scan keyboard input.  Allow only characters that are available to advance from this
state.  Do not echo characters.

Setup: 
GREEN LED to indicate state.  
Terminal window should indicate "Your turn".
*/
static void TermTacToeSM_HomeTurn(void)          
{
  u8 u8InputChar = 0;
  u8 u8CommandNumber = 0;
  
  /* Look for terminal input */
  if(G_u8DebugScanfCharCount == 1)
  {
    DebugScanf(&u8InputChar);
    
    /* Range check '0' to '8' */
    if( (u8InputChar >= '0') && (u8InputChar <= '8') )
    {
      /* Convert to binary and check if available space */
      u8InputChar -= NUMBER_ASCII_TO_DEC;
      if(TermTacToe_abAvailableSpaces[u8InputChar] == TRUE)
      {
        /* Valid move, so update message and send to 422 */
        TermTacToe_au8SpiGameMove[GAME_MOVE_OFFSET_POSITION + NRF_OVERHEAD_BYTES] = u8InputChar;
        TermTacToe_pfStateMachine = TermTacToeSM_WaitHomeTurnResponse;

#ifdef LOCAL_TEST_MODE
        TermTacToe_pfStateMachine = TermTacToeSM_AwayTurn;
        TermTacToeProcessMove(HOME, u8InputChar, GAME_MOVE_DATA_STATUS_PLAYING);
        LedOn(YELLOW);
        LedOff(GREEN);
        TermTacToeWriteUserMessage(TTT_AWAY_MOVE);
#endif /* LOCAL_TEST_MODE */
      }
    }
    
  } /* end of terminal input */
  
        
  /* Process any messages that arrive - only RESET applies here */
  u8CommandNumber = nrfNewMessageCheck();
  if(u8CommandNumber != NRF_CMD_EMPTY)
  {
    /* Read the message */
    nrfGetAppMessage(TermTacToe_au8AppMessage);
    
    if(TermTacToe_au8AppMessage[ANTTT_COMMAND_OFFSET_ID] == ANTTT_APP_MSG_RESET)
    {
      TermTacToe_pfStateMachine = TermTacToeSM_Setup;
    }
  }
  
} /* end TermTacToeSM_HomeTurn() */

        
/*!----------------------------------------------------------------------------------------------------------------------
@fn static void TermTacToeSM_WaitHomeTurnResponse(void)          
@brief Wait for the 422 game engine to return the GAME_MOVE message for 
the local move that was just made.
*/
static void TermTacToeSM_WaitHomeTurnResponse(void)
{
  u8 u8CommandNumber;
  
  /* Wait for GAME_MOVE message */
  u8CommandNumber = nrfNewMessageCheck();
  if(u8CommandNumber != NRF_CMD_EMPTY)
  {
    /* Read the message */
    nrfGetAppMessage(TermTacToe_au8AppMessage);
    
    switch (TermTacToe_au8AppMessage[ANTTT_COMMAND_OFFSET_ID])
    {
      case ANTTT_APP_MSG_RESET:
      {
        TermTacToe_pfStateMachine = TermTacToeSM_Setup;
        break;
      }
      
      case ANTTT_APP_MSG_GAME_MOVE:
      {
        TermTacToeProcessMove(HOME, 
                              TermTacToe_au8AppMessage[GAME_MOVE_OFFSET_POSITION],
                              TermTacToe_au8AppMessage[GAME_MOVE_OFFSET_STATUS]);
        
        /* Adjust if game is still playing */
        if(TermTacToe_au8AppMessage[GAME_MOVE_OFFSET_STATUS] == GAME_MOVE_DATA_STATUS_PLAYING)
        {
          LedOn(YELLOW);
          LedOff(GREEN);
          TermTacToeWriteUserMessage(TTT_AWAY_MOVE);
          TermTacToe_pfStateMachine = TermTacToeSM_AwayTurn;
        }
                
        break;
      }
            
      default:
      {
        DebugPrintf("\n\rUnexpected message\n\r");
        break;
      }
    } /* end switch */
  }


  /* Look for key presses and read them to ensure the buffer does 
  not overflow; ignore since input is not valid right now. */
  if(G_u8DebugScanfCharCount)
  {
    DebugScanf(TermTacToe_au8TerminalInputBuffer);
  }
  
} /* end TermTacToeSM_WaitHomeTurnResponse() */

        
/*!----------------------------------------------------------------------------------------------------------------------
@fn static void TermTacToeSM_AwayTurn(void)          
@brief Wait for move or other command.

Setup: YELLOW LED on to indicate state
*/
static void TermTacToeSM_AwayTurn(void)          
{
  u8 u8CommandNumber;
   
  /* Wait for message to indicate remote player has moved */
  u8CommandNumber = nrfNewMessageCheck();
  if(u8CommandNumber != NRF_CMD_EMPTY)
  {
    /* Read the message */
    nrfGetAppMessage(TermTacToe_au8AppMessage);
    
    switch (TermTacToe_au8AppMessage[ANTTT_COMMAND_OFFSET_ID])
    {
      case ANTTT_APP_MSG_RESET:
      {
        TermTacToe_pfStateMachine = TermTacToeSM_Setup;
        break;
      }
      
      case ANTTT_APP_MSG_GAME_MOVE:
      {
        TermTacToeProcessMove(AWAY,
                              TermTacToe_au8AppMessage[GAME_MOVE_OFFSET_POSITION],
                              TermTacToe_au8AppMessage[GAME_MOVE_OFFSET_STATUS]);
        
        /* Adjust if game is still playing */
        if(TermTacToe_au8AppMessage[GAME_MOVE_OFFSET_STATUS] == GAME_MOVE_DATA_STATUS_PLAYING)
        {
          LedOn(GREEN);
          LedOff(YELLOW);
          TermTacToeWriteUserMessage(TTT_HOME_MOVE);
          TermTacToe_pfStateMachine = TermTacToeSM_HomeTurn;
        }
                
        break;
      }
            
      default:
      {
        DebugPrintf("\n\rUnexpected message\n\r");
        break;
      }
    } /* end switch */
  }

#ifdef LOCAL_TEST_MODE
  u8 u8InputChar = 4;

  /* BUTTON 0 emulates a move.  Choose a location with BUTTON1-3 prior to BUTTON0. 4 is default. */
  if(WasButtonPressed(BUTTON0))
  {
    ButtonAcknowledge(BUTTON0);
    
    /* Change the move location if a different button was pressed before */
    if(WasButtonPressed(BUTTON1))
    {
      ButtonAcknowledge(BUTTON1);
      u8InputChar = 0;
    }
       
    if(WasButtonPressed(BUTTON2))
    {
      ButtonAcknowledge(BUTTON2);
      u8InputChar = 1;
    }

    if(WasButtonPressed(BUTTON3))
    {
      ButtonAcknowledge(BUTTON3);
      u8InputChar = 2;
    }
    
    TermTacToe_pfStateMachine = TermTacToeSM_HomeTurn;
    LedOff(YELLOW);
    LedOn(GREEN);
    TermTacToeProcessMove(AWAY, u8InputChar, GAME_MOVE_DATA_STATUS_PLAYING);
    TermTacToeWriteUserMessage(TTT_HOME_MOVE);
  }
#endif /* LOCAL_TEST_MODE */

  /* Look for key presses and read them to ensure the buffer does 
  not overflow; ignore since input is not valid right now. */
  if(G_u8DebugScanfCharCount)
  {
    DebugScanf(TermTacToe_au8TerminalInputBuffer);
  }
        
} /* end TermTacToeSM_AwayTurn() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn static void TermTacToeSM_GameOver(void)          
@brief Wait state 
*/
static void TermTacToeSM_GameOver(void)          
{
  
} /* end TermTacToeSM_GameOver() */



/*!----------------------------------------------------------------------------------------------------------------------
@fn static void TermTacToeSM_Error(void)          
@brief Handle an error 
*/
static void TermTacToeSM_Error(void)          
{
  
} /* end TermTacToeSM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
