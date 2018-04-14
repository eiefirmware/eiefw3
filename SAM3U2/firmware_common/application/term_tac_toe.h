/*!*********************************************************************************************************************
@file term_tac_toe.h                                                                
@brief Header file for term_tac_toe

**********************************************************************************************************************/

#ifndef __TERM_TAC_TOE_H
#define __TERM_TAC_TOE_H

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/
typedef enum {TTT_WAITING, TTT_LOCAL_MOVE, TTT_REMOTE_MOVE} UserMessageType;


/*******************************************************************************
* Macros 
*******************************************************************************/


/**********************************************************************************************************************
Function Declarations
**********************************************************************************************************************/

/*------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/
void TermTacToeInitialize(void);
void TermTacToeRunActiveState(void);


/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/
void TermTacToeWriteSquare(u8 u8Square_, bool bX_);
void TermTacToeWriteUserMessage(UserMessageType eMessageNumber_);


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
static void TermTacToeSM_Setup(void);
static void TermTacToeSM_Idle(void);    

static void TermTacToeSM_Error(void);         


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
/* TermTacToe_u32Flags */
/* end TermTacToe_u32Flags */

#define EX              (bool)TRUE
#define OH              (bool)FALSE


#endif /* __TERM_TAC_TOE_H */





/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
