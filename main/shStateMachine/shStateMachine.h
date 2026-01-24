/*
 * shStateMachine.h
 *
 *  Created on: 2018. nov. 14.
 *      Author: tothpetiszilard
 */

#ifndef SHSTATEMACHINE_H_
#define SHSTATEMACHINE_H_

typedef enum
{
	WEEKLY_MODE = 0u,
	DAILY_MODE = 1u,
	SUNSETD_MODE = 2u,
	SUNSETW_MODE = 3u,
	MANUAL_MODE = 4u
}ShSM_States_ten;

extern void ShSM_Init(void);
//extern void ShSM_Cyclic(void);
extern void ShSM_SetState(ShSM_States_ten state_en);
extern ShSM_States_ten ShSM_GetState();

#endif /* SHSTATEMACHINE_H_ */
