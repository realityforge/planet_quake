// Copyright (C) 1999-2000 Id Software, Inc.
//

/*****************************************************************************
 * name:		ai_dmnet.h
 *
 * desc:		Quake3 bot AI
 *
 * $Archive: /source/code/botai/ai_chat.c $
 * $Author: root $ 
 * $Revision: 1.1.1.1 $
 * $Modtime: 11/10/99 3:30p $
 * $Date: 2001/06/19 01:11:44 $
 *
 *****************************************************************************/

#define MAX_NODESWITCHES	50

void AIEnter_Intermission(bot_state_t *bs);
void AIEnter_Observer(bot_state_t *bs);
void AIEnter_Respawn(bot_state_t *bs);
void AIEnter_Stand(bot_state_t *bs);
void AIEnter_Seek_ActivateEntity(bot_state_t *bs);
void AIEnter_Seek_NBG(bot_state_t *bs);
void AIEnter_Seek_LTG(bot_state_t *bs);
void AIEnter_Seek_Camp(bot_state_t *bs);
void AIEnter_Battle_Fight(bot_state_t *bs);
void AIEnter_Battle_Chase(bot_state_t *bs);
void AIEnter_Battle_Retreat(bot_state_t *bs);
void AIEnter_Battle_NBG(bot_state_t *bs);
int AINode_Intermission(bot_state_t *bs);
int AINode_Observer(bot_state_t *bs);
int AINode_Respawn(bot_state_t *bs);
int AINode_Stand(bot_state_t *bs);
int AINode_Seek_ActivateEntity(bot_state_t *bs);
int AINode_Seek_NBG(bot_state_t *bs);
int AINode_Seek_LTG(bot_state_t *bs);
int AINode_Battle_Fight(bot_state_t *bs);
int AINode_Battle_Chase(bot_state_t *bs);
int AINode_Battle_Retreat(bot_state_t *bs);
int AINode_Battle_NBG(bot_state_t *bs);

void BotResetNodeSwitches(void);
void BotDumpNodeSwitches(bot_state_t *bs);

