/*
 * shStateMachine.cpp
 *
 *  Created on: 2018. nov. 14.
 *      Author: tothpetiszilard
 */

#include "shStateMachine.h"
#include "shStateMachine_Cfg.h"

#include "../shInfo.h"
#if (10 < SH_SW_PATCH_VER)
#include "../Det/Det.h"
#endif


static ShSM_States_ten ramState_en;

void ShSM_Init(void)
{
	StdReturnType result = E_NOT_OK;
	uint8_t flashState_u8;
	result = ShCfg_Read(SHCFG_SMSTATE_CH, &flashState_u8);
	if (E_OK == result)
	{
		ramState_en = (ShSM_States_ten)flashState_u8;
	}
	else
	{
#if (10 < SH_SW_PATCH_VER)
		Det_ReportError(DET_MODULEID_SHSTATEMACHINE, "Nvm read failed");
#endif
		ramState_en = MANUAL_MODE;
		ShSM_SetState(ramState_en);
	}

}


void ShSM_SetState(ShSM_States_ten state_en)
{
	ramState_en = state_en;
	ShCfg_Write(SHCFG_SMSTATE_CH,(uint8_t *)&ramState_en);
}
ShSM_States_ten ShSM_GetState()
{
	return ramState_en;
}
