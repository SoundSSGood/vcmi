/*
 * MapQueries.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include "CQuery.h"
#include "../../lib/networkPacks/PacksForClient.h"

VCMI_LIB_NAMESPACE_BEGIN
class CGHeroInstance;
class CGObjectInstance;
class IObjectInterface;
VCMI_LIB_NAMESPACE_END

//Created when player starts turn or when player puts game on [ause
//Removed when player accepts a turn or continur play
class TimerPauseQuery : public CQuery
{
public:	
	TimerPauseQuery(CGameHandler * owner, PlayerColor player);
	
	bool blocksPack(const CPackForServer *pack) const override;
	void onAdding(PlayerColor color) override;
	void onRemoval(PlayerColor color) override;
};

//Created when hero attempts move and something happens
//(not necessarily position change, could be just an object interaction).
class CHeroMovementQuery : public CQuery
{
public:
	TryMoveHero tmh;
	const CGHeroInstance *hero;

	CHeroMovementQuery(CGameHandler * owner, const TryMoveHero & Tmh, const CGHeroInstance * Hero);
	void onAdding(PlayerColor color) override;
	void onRemoval(PlayerColor color) override;
	void onExposure(QueryPtr topQuery) override;
};

class CGarrisonDialogQuery : public CDialogQuery //used also for hero exchange dialogs
{
public:
	std::array<const CArmedInstance *,2> exchangingArmies;

	CGarrisonDialogQuery(CGameHandler * owner, const CArmedInstance *up, const CArmedInstance *down);
	bool blocksPack(const CPackForServer *pack) const override;
};

class OpenWindowQuery : public CDialogQuery
{
	EOpenWindowMode mode;
public:
	OpenWindowQuery(CGameHandler * owner, const CGHeroInstance *hero, EOpenWindowMode mode);

	bool blocksPack(const CPackForServer *pack) const override;
};

//yes/no and component selection dialogs
class CBlockingDialogQuery : public CDialogQuery
{
public:	CBlockingDialogQuery(CGameHandler * owner, const PlayerColor & player): CDialogQuery(owner, player) {};
};

class CTeleportDialogQuery : public CDialogQuery
{
public: CTeleportDialogQuery(CGameHandler * owner, const PlayerColor & player): CDialogQuery(owner, player) {};
};

class CHeroLevelUpDialogQuery : public CDialogQuery
{
public: CHeroLevelUpDialogQuery(CGameHandler * owner, const PlayerColor & player): CDialogQuery(owner, player) {};
};

class CCommanderLevelUpDialogQuery : public CDialogQuery
{
public: CCommanderLevelUpDialogQuery(CGameHandler * owner, const PlayerColor & player): CDialogQuery(owner, player) {};
};

class CTownGateDialogQuery : public CDialogQuery
{
public : CTownGateDialogQuery(CGameHandler * owner, const PlayerColor & player): CDialogQuery(owner, player) {};
};
