/*
 * VisitQueries.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"
#include "VisitQueries.h"

#include "../../lib/mapObjects/CGHeroInstance.h"
#include "../../lib/mapObjects/CGTownInstance.h"
#include "../../lib/mapObjects/TownBuildingInstance.h"
#include "../CGameHandler.h"
#include "QueriesProcessor.h"

VisitQuery::VisitQuery(CGameHandler * owner, const ObjectInstanceID & objId, const ObjectInstanceID & heroId)
	: CQuery(owner, false)
	, visitedObject(objId)
	, visitingHero(heroId)
{
	addPlayer(gh->getOwner(heroId));
}

bool VisitQuery::blocksPack(const CPackForServer * pack) const
{
	//During the visit itself ALL actions are blocked.
	//(However, the visit may trigger a query above that'll pass some.)
	return true;
}

MapObjectVisitQuery::MapObjectVisitQuery(CGameHandler * owner, const CGObjectInstance * Obj, const CGHeroInstance * Hero)
	: VisitQuery(owner, Obj->id, Hero->id)
	, removeObjectAfterVisit(false)
{
}

void MapObjectVisitQuery::onRemoval(PlayerColor color)
{
	auto obj = gh->getObj(visitedObject);
	auto hero = gh->getHero(visitingHero);

	gh->objectVisitEnded(hero, players.front());

	//Can object visit affect 2 players and what would be desired behavior?
	if(removeObjectAfterVisit)
		gh->removeObject(obj, color);
}

TownBuildingVisitQuery::TownBuildingVisitQuery(CGameHandler * owner, const CGTownInstance * Obj, std::vector<const CGHeroInstance *> heroes, std::vector<BuildingID> buildingToVisit)
	: VisitQuery(owner, Obj->id, heroes.front()->id)
	, visitedTown(Obj)
{
	// generate in reverse order - first building-hero pair to handle must be in the end of vector
	for (auto const * hero : boost::adaptors::reverse(heroes))
		for (auto const & building : boost::adaptors::reverse(buildingToVisit))
			visitedBuilding.push_back({ hero, building});
}

void TownBuildingVisitQuery::onAdding(PlayerColor color)
{
	while(!visitedBuilding.empty())
	{
		visitingHero = visitedBuilding.back().hero->id;
		const auto & building = visitedTown->rewardableBuildings.at(visitedBuilding.back().building);
		building->onHeroVisit(visitedBuilding.back().hero);
		visitedBuilding.pop_back();
	}
}
