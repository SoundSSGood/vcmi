/*
 * BattleQueries.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include "CQuery.h"
#include "../../lib/networkPacks/PacksForClientBattle.h"
#include "../../lib/battle/BattleSide.h"

VCMI_LIB_NAMESPACE_BEGIN
class IBattleInfo;
struct SideInBattle;
VCMI_LIB_NAMESPACE_END

class CBattleQuery : public CQuery
{
public:
	BattleSideArray<int> initialHeroMana;

	BattleID battleID;
	std::optional<BattleResult> result;

	CBattleQuery(CGameHandler * owner, const IBattleInfo * Bi);
	void setOnRemovalCallback(const std::function<void(const PlayerColor & player, const BattleResult & result)> & onRemovalCallback);
	bool blocksPack(const CPackForServer *pack) const override;
	bool getAnswerRequired() const override;
};

// Used for requesting battle replay
class CBattleDialogQuery : public CDialogQuery
{
	bool resultProcessed = false;
	const IBattleInfo * bi;

public:
	CBattleDialogQuery(CGameHandler * owner, const IBattleInfo * Bi);
	void onRemoval(PlayerColor color) override;
};
