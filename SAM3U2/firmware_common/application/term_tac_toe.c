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

static u8 TermTacToe_au8UartBuffer[U8_NRF_BUFFER_SIZE];     /*!< @brief Space for verified received application messages */
//static u8* TermTacToe_pu8RxBufferNextChar;                  /* Pointer to next char to be written in the buffer */

static u8 TermTacToe_au8Message[U8_NRF_BUFFER_SIZE];        /*!< @brief Latest received application message */

static u8 TermTacToe_au8TerminalInputBuffer[U8_TERMINAL_INPUT_BUFFER_SIZE];

static u8 TermTacToe_au8GameBoard[] = "     |     |\n\r  0  |  1  |  2\n\r     |     |\n\r-----|-----|-----\n\r     |     |\n\r  3  |  4  |  5 \n\r     |     |\n\r-----|-----|-----\n\r     |     |\n\r  6  |  7  |  8\n\r     |     |\n\n\n\r";

                                            /*12345678901234567*/
static u8 TermTacToe_au8UserMessage[][18] = {" WAIT TO CONNECT ",
                                             "    YOUR TURN    ",
                                             "    THEIR TURN   " };

static bool TermTacToe_abAvailableSpaces[9];


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!--------------------------------------------------------------------------------------------------------------------
@fn 
@brief Determines if a new message is available from the nRF Interface.


Requires:
- 

Promises:
- 

*/
/* end  */


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
  
  /* Turn off Debug command processing */
  DebugSetPassthrough();
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
  u8 au8CharColor[] = TERM_TEXT_RED;
  
  /* There are only ever 3 choices of column and row so we
  can save a little space */
  u8 au8Columns[][3] = {"3", "9", "15"};
  u8 au8Rows[][3]    = {"2", "6", "10"};
  
  /* The column and index is calculated to choose from one of the three */
  u8 u8ColumnIndex = u8Square_ % 3;
  u8 u8RowIndex = u8Square_ / 3;
  
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
  if(eMessageNumber_ == TTT_LOCAL_MOVE)
  {
    DebugPrintf(TERM_BLINK);
  }
  
  /* Write the message */
  DebugPrintf(TermTacToe_au8UserMessage[eMessageNumber_]);
  
  /* Add user prompt for move */  
  if(eMessageNumber_ == TTT_LOCAL_MOVE)
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
  
} /* end TermTacToeWriteUserMessage() */


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
  LedOn(GREEN);
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
  /* Look for key presses and read them to ensure the buffer does not overflow */
  if(G_u8DebugScanfCharCount)
  {
    DebugScanf(TermTacToe_au8TerminalInputBuffer);
  }
  
  /* Wait for game message to indicate which player is starting */
  if(nrfNewMessageCheck != NRF_CMD_EMPTY)
  {
    /* The only valid message right now is a game start message */
    
  }
  
  /* Character write test */
  if(WasButtonPressed(BUTTON0))
  {
    ButtonAcknowledge(BUTTON0);
    TermTacToeWriteSquare(0, EX);
    TermTacToeWriteUserMessage(TTT_REMOTE_MOVE);
  }
  
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    TermTacToeWriteSquare(8, EX);
    TermTacToeWriteUserMessage(TTT_REMOTE_MOVE);
  }

  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);
    TermTacToeWriteSquare(0, OH);
    TermTacToeWriteUserMessage(TTT_LOCAL_MOVE);
  }

  if(WasButtonPressed(BUTTON3))
  {
    ButtonAcknowledge(BUTTON3);
    TermTacToeWriteSquare(7, OH);
    TermTacToeWriteUserMessage(TTT_LOCAL_MOVE);
  }
  
} /* end TermTacToeSM_Idle() */
   

/*!----------------------------------------------------------------------------------------------------------------------
@fn static void TermTacToeSM_LocalTurn(void)          
@brief Play a local turn.

Scan keyboard input.  Allow only characters that are available to advance from this
state.  Do not echo characters.

Setup: 
Yellow LED to indicate state.  
Terminal window should indicate "Your turn".
*/
static void TermTacToeSM_LocalTurn(void)          
{
  
} /* end TermTacToeSM_LocalTurn() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn static void TermTacToeSM_RemoteTurn(void)          
@brief Wait for move or other command.

Setup: GREEN LED on to indicate state
*/
static void TermTacToeSM_LocalTurn(void)          
{
  
} /* end TermTacToeSM_LocalTurn() */

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
