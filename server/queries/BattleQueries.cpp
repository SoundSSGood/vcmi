/*
 * BattleQueries.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"
#include "BattleQueries.h"
#include "QueriesProcessor.h"

#include "../CGameHandler.h"
#include "../battles/BattleProcessor.h"

#include "../../lib/battle/IBattleState.h"
#include "../../lib/battle/BattleLayout.h"

CBattleQuery::CBattleQuery(CGameHandler * owner, const IBattleInfo * bi):
	CQuery(owner, "BattleQuery", false),
	battleID(bi->getBattleID())
{
	addPlayer(bi->getSidePlayer(BattleSide::ATTACKER));
	addPlayer(bi->getSidePlayer(BattleSide::DEFENDER));
}

void CBattleQuery::setOnRemovalCallback(const std::function<void(const PlayerColor & player, const BattleResult & result)> & onRemovalCallback)
{
	CQuery::setOnRemovalCallback([onRemovalCallback, this](const PlayerColor & player)
	{
		assert(result);
		onRemovalCallback(player, result.value());
	});
}

bool CBattleQuery::blocksPack(const CPackForServer * pack) const
{
	if(dynamic_cast<const MakeAction*>(pack) != nullptr)
		return false;

	if(dynamic_cast<const GamePause*>(pack) != nullptr)
		return false;

	return true;
}

void CBattleQuery::onExposure(QueryPtr topQuery)
{
	// this method may be called in two cases:
	// 1) when requesting battle replay (but before replay starts -> no valid result)
	// 2) when aswering on levelup queries after accepting battle result -> valid result
	CQuery::onExposure(topQuery);
	if(result)
		owner->popQuery(*this);
}

CBattleDialogQuery::CBattleDialogQuery(CGameHandler * owner, const IBattleInfo * bi):
	CDialogQuery(owner, bi->getSidePlayer(BattleSide::ATTACKER), "BattleDialogQuery"),
	bi(bi)
{
	addPlayer(bi->getSidePlayer(BattleSide::DEFENDER));
}

void CBattleDialogQuery::onRemoval(PlayerColor color)
{
	// answer to this query was already processed when handling 1st player
	// this removal call for 2nd player which can be safely ignored
	if (resultProcessed)
		return;

	assert(answer);
	if(*answer == 1)
	{
		gh->battles->restartBattle(
			bi->getBattleID(),
			bi->getSideArmy(BattleSide::ATTACKER),
			bi->getSideArmy(BattleSide::DEFENDER),
			bi->getLocation(),
			bi->getSideHero(BattleSide::ATTACKER),
			bi->getSideHero(BattleSide::DEFENDER),
			bi->getLayout(),
			bi->getDefendedTown()
		);
	}
	else
	{
		gh->battles->endBattleConfirm(bi->getBattleID());
	}
	resultProcessed = true;
}
