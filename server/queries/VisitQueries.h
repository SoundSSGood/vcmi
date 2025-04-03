/*
 * VisitQueries.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include "CQuery.h"

VCMI_LIB_NAMESPACE_BEGIN
class CGTownInstance;
VCMI_LIB_NAMESPACE_END

//Created when hero visits object.
//Removed when query above is resolved (or immediately after visit if no queries were created)
class VisitQuery : public CQuery
{
public:
	VisitQuery(CGameHandler * owner, const ObjectInstanceID & objId, const ObjectInstanceID & heroId);
	bool blocksPack(const CPackForServer * pack) const final;

	ObjectInstanceID visitedObject;
	ObjectInstanceID visitingHero;
};

class MapObjectVisitQuery final : public VisitQuery
{
public:
	bool removeObjectAfterVisit;

	MapObjectVisitQuery(CGameHandler * owner, const CGObjectInstance * Obj, const CGHeroInstance * Hero);

	void onRemoval(PlayerColor color) override final;
};

class TownBuildingVisitQuery final : public VisitQuery
{
	struct BuildingVisit
	{
		const CGHeroInstance * hero;
		BuildingID building;
	};

	const CGTownInstance * visitedTown;
	std::vector<BuildingVisit> visitedBuilding;

public:
	TownBuildingVisitQuery(CGameHandler * owner, const CGTownInstance * Obj, std::vector<const CGHeroInstance *> heroes, std::vector<BuildingID> buildingToVisit);
	void onAdding(PlayerColor color) override final;
};
