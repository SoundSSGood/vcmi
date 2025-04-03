/*
 * CQuery.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include "StdInc.h"
#include "../../lib/GameConstants.h"

VCMI_LIB_NAMESPACE_BEGIN

struct CPackForServer;
class CGObjectInstance;
class CGHeroInstance;

VCMI_LIB_NAMESPACE_END

class CObjectVisitQuery;
class QueriesProcessor;
class CQuery;
class CGameHandler;

using QueryPtr = std::shared_ptr<CQuery>;

// This class represents any kind of prolonged interaction that may need to do something special after it is over.
// It does not necessarily has to be "query" requiring player action, it can be also used internally within server.
// Examples:
// - all kinds of blocking dialog windows
// - battle
// - object visit
// - hero movement
// Queries can cause another queries, forming a stack of queries for each player. Eg: hero movement -> object visit -> dialog.
class CQuery
{
public:
	std::vector<PlayerColor> players; //players that are affected (often "blocked") by query
	QueryID queryID;

	CQuery(CGameHandler * gh, const bool isAnswerRequired);

	/// query can block attempting actions by player. Eg. he can't move hero during the battle.
	virtual bool blocksPack(const CPackForServer *pack) const;

	/// query is removed after player gives answer (like dialogs)
	virtual bool getAnswerRequired() const;

	/// called just before query is pushed on stack
	virtual void onAdding(PlayerColor color);

	/// called after query is removed from stack
	virtual void onRemoval(PlayerColor color);

	void setOnRemovalCallback(const std::function<void(const PlayerColor & player)> & onRemovalCallback);
	void addPlayer(PlayerColor color);

	virtual void setReply(std::optional<int32_t> reply);
	virtual std::string toString() const;

	virtual ~CQuery();
protected:
	CGameHandler * gh;
	bool blockAllButReply(const CPackForServer * pack) const;
	std::function<void(const PlayerColor & player)> onRemovalCallback;
	const bool isAnswerRequired;
};

std::ostream &operator<<(std::ostream &out, const CQuery &query);
std::ostream &operator<<(std::ostream &out, QueryPtr query);

class CDialogQuery : public CQuery
{
public:
	CDialogQuery(CGameHandler * owner);
	bool blocksPack(const CPackForServer *pack) const override;
	void setReply(std::optional<int32_t> reply) override;
	void setOnRemovalCallback(const std::function<void(const PlayerColor & player, const std::optional<int32_t> & answer)> & onRemovalCallback);

protected:
	std::optional<int32_t> answer;
};
