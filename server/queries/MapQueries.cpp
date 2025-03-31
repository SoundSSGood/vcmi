/*
 * MapQueries.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"
#include "MapQueries.h"

#include "QueriesProcessor.h"
#include "../CGameHandler.h"
#include "../TurnTimerHandler.h"
#include "../../lib/mapObjects/CGHeroInstance.h"
#include "../../lib/mapObjects/MiscObjects.h"
#include "../../lib/networkPacks/PacksForServer.h"

TimerPauseQuery::TimerPauseQuery(CGameHandler * owner, PlayerColor player):
	CQuery(owner)
{
	addPlayer(player);
}

bool TimerPauseQuery::blocksPack(const CPackForServer *pack) const
{
	return blockAllButReply(pack);
}

void TimerPauseQuery::onAdding(PlayerColor color)
{
	gh->turnTimerHandler->setTimerEnabled(color, false);
}

void TimerPauseQuery::onRemoval(PlayerColor color)
{
	gh->turnTimerHandler->setTimerEnabled(color, true);
}

bool TimerPauseQuery::endsByPlayerAnswer() const
{
	return true;
}

CGarrisonDialogQuery::CGarrisonDialogQuery(CGameHandler * owner, const CArmedInstance * up, const CArmedInstance * down):
	CDialogQuery(owner, down->tempOwner)
{
	exchangingArmies[0] = up;
	exchangingArmies[1] = down;

	addPlayer(up->tempOwner);
}

bool CGarrisonDialogQuery::blocksPack(const CPackForServer * pack) const
{
	std::set<ObjectInstanceID> ourIds;
	ourIds.insert(this->exchangingArmies[0]->id);
	ourIds.insert(this->exchangingArmies[1]->id);

	if(auto stacks = dynamic_cast<const ArrangeStacks*>(pack))
		return !vstd::contains(ourIds, stacks->id1) || !vstd::contains(ourIds, stacks->id2);

	if(auto stacks = dynamic_cast<const BulkSplitStack*>(pack))
		return !vstd::contains(ourIds, stacks->srcOwner);

	if(auto stacks = dynamic_cast<const BulkMergeStacks*>(pack))
		return !vstd::contains(ourIds, stacks->srcOwner);

	if(auto stacks = dynamic_cast<const BulkSmartSplitStack*>(pack))
		return !vstd::contains(ourIds, stacks->srcOwner);

	if(auto stacks = dynamic_cast<const BulkMoveArmy*>(pack))
		return !vstd::contains(ourIds, stacks->srcArmy) || !vstd::contains(ourIds, stacks->destArmy);

	if(auto arts = dynamic_cast<const ExchangeArtifacts*>(pack))
	{
		if(auto id1 = arts->src.artHolder)
			if(!vstd::contains(ourIds, id1))
				return true;

		if(auto id2 = arts->dst.artHolder)
			if(!vstd::contains(ourIds, id2))
				return true;
		return false;
	}
	if(auto dismiss = dynamic_cast<const DisbandCreature*>(pack))
		return !vstd::contains(ourIds, dismiss->id);

	if(auto arts = dynamic_cast<const BulkExchangeArtifacts*>(pack))
		return !vstd::contains(ourIds, arts->srcHero) || !vstd::contains(ourIds, arts->dstHero);

	if(auto arts = dynamic_cast<const ManageBackpackArtifacts*>(pack))
		return !vstd::contains(ourIds, arts->artHolder);

	if(auto art = dynamic_cast<const EraseArtifactByClient*>(pack))
	{
		if(auto id = art->al.artHolder)
			return !vstd::contains(ourIds, id);
	}

	if(auto dismiss = dynamic_cast<const AssembleArtifacts*>(pack))
		return !vstd::contains(ourIds, dismiss->heroID);

	if(auto upgrade = dynamic_cast<const UpgradeCreature*>(pack))
		return !vstd::contains(ourIds, upgrade->id);

	if(auto formation = dynamic_cast<const SetFormation*>(pack))
		return !vstd::contains(ourIds, formation->hid);

	return CDialogQuery::blocksPack(pack);
}

OpenWindowQuery::OpenWindowQuery(CGameHandler * owner, const CGHeroInstance *hero, EOpenWindowMode mode):
	CDialogQuery(owner, hero->getOwner()),
	mode(mode)
{
}

bool OpenWindowQuery::blocksPack(const CPackForServer *pack) const
{
	if (mode == EOpenWindowMode::RECRUITMENT_FIRST || mode == EOpenWindowMode::RECRUITMENT_ALL)
	{
		if(dynamic_cast<const RecruitCreatures*>(pack) != nullptr)
			return false;

		// If hero has no free slots, he might get some stacks merged automatically
		if(dynamic_cast<const ArrangeStacks*>(pack) != nullptr)
			return false;
	}

	if (mode == EOpenWindowMode::TAVERN_WINDOW)
	{
		if(dynamic_cast<const HireHero*>(pack) != nullptr)
			return false;
	}

	if (mode == EOpenWindowMode::UNIVERSITY_WINDOW)
	{
		if(dynamic_cast<const TradeOnMarketplace*>(pack) != nullptr)
			return false;
	}

	if (mode == EOpenWindowMode::MARKET_WINDOW)
	{
		if(dynamic_cast<const ExchangeArtifacts*>(pack) != nullptr)
			return false;

		if(dynamic_cast<const BulkExchangeArtifacts*>(pack) != nullptr)
			return false;

		if(dynamic_cast<const ManageBackpackArtifacts*>(pack) != nullptr)
			return false;

		if(dynamic_cast<const AssembleArtifacts*>(pack))
			return false;

		if(dynamic_cast<const EraseArtifactByClient*>(pack))
			return false;

		if(dynamic_cast<const TradeOnMarketplace*>(pack) != nullptr)
			return false;
	}

	return CDialogQuery::blocksPack(pack);
}

void CHeroMovementQuery::onExposure(QueryPtr topQuery)
{
	owner->popIfTop(*this);
}

CHeroMovementQuery::CHeroMovementQuery(CGameHandler * owner, const TryMoveHero & Tmh, const CGHeroInstance * Hero, bool VisitDestAfterVictory):
	CQuery(owner), tmh(Tmh), visitDestAfterVictory(VisitDestAfterVictory), hero(Hero)
{
	players.push_back(hero->tempOwner);
}

void CHeroMovementQuery::onRemoval(PlayerColor color)
{
	PlayerBlocked pb;
	pb.player = color;
	pb.reason = PlayerBlocked::ONGOING_MOVEMENT;
	pb.startOrEnd = PlayerBlocked::BLOCKADE_ENDED;
	gh->sendAndApply(pb);
}

void CHeroMovementQuery::onAdding(PlayerColor color)
{
	PlayerBlocked pb;
	pb.player = color;
	pb.reason = PlayerBlocked::ONGOING_MOVEMENT;
	pb.startOrEnd = PlayerBlocked::BLOCKADE_STARTED;
	gh->sendAndApply(pb);
}
