/*
 * CQuery.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"
#include "CQuery.h"

#include "QueriesProcessor.h"

#include "../CGameHandler.h"

#include "../../lib/networkPacks/PacksForServer.h"

std::ostream & operator<<(std::ostream & out, const CQuery & query)
{
	return out << query.toString();
}

std::ostream & operator<<(std::ostream & out, QueryPtr query)
{
	return out << "[" << query.get() << "] " << query->toString();
}

CQuery::CQuery(CGameHandler * gameHandler, const bool isAnswerRequired)
	: owner(gameHandler->queries.get())
	, gh(gameHandler)
	, isAnswerRequired(isAnswerRequired)
{
	static QueryID QID = QueryID(0);

	queryID = ++QID;
	logGlobal->trace("Created a new query with id %d", queryID);
}

CQuery::~CQuery()
{
	logGlobal->trace("Destructed the query with id %d", queryID);
}

void CQuery::addPlayer(PlayerColor color)
{
	if(color.isValidPlayer())
		players.push_back(color);
}

std::string CQuery::toString() const
{
	const auto size = players.size();
	const std::string plural = size > 1 ? "s" : "";
	std::string names;

	for(size_t i = 0; i < size; i++)
	{
		names += boost::to_upper_copy<std::string>(players[i].toString());

		if(i < size - 2)
			names += ", ";
		else if(size > 1 && i == size - 2)
			names += " and ";
	}
	std::string ret = boost::str(boost::format("A query of type '%s' and qid = %d affecting player%s %s")
		% typeid(*this).name()
		% queryID 
		% plural
		% names
	);
	return ret;
}

bool CQuery::endsByPlayerAnswer() const
{
	return isAnswerRequired;
}

void CQuery::onRemoval(PlayerColor color)
{
	if(onRemovalCallback)
		onRemovalCallback(color);
}

bool CQuery::blocksPack(const CPackForServer * pack) const
{
	return false;
}

void CQuery::setOnRemovalCallback(const std::function<void(const PlayerColor & player)> & onRemovalCallback)
{
	this->onRemovalCallback = onRemovalCallback;
}

void CQuery::onExposure(QueryPtr topQuery)
{
	logGlobal->trace("Exposed query with id %d", queryID);
}

void CQuery::onAdding(PlayerColor color)
{

}

void CQuery::onAdded(PlayerColor color)
{

}

void CQuery::setReply(std::optional<int32_t> reply)
{

}

bool CQuery::blockAllButReply(const CPackForServer * pack) const
{
	//We accept only query replies from correct player
	if(auto reply = dynamic_cast<const QueryReply*>(pack))
		return !vstd::contains(players, reply->player);

	return true;
}

CDialogQuery::CDialogQuery(CGameHandler * owner, const PlayerColor & player):
	CQuery(owner, true)
{
	addPlayer(player);
}

bool CDialogQuery::blocksPack(const CPackForServer * pack) const
{
	return blockAllButReply(pack);
}

void CDialogQuery::setReply(std::optional<int32_t> reply)
{
	if(reply.has_value())
		answer = *reply;
}

void CDialogQuery::setOnRemovalCallback(const std::function<void(const PlayerColor & player, const std::optional<int32_t> & answer)> & onRemovalCallback)
{
	CQuery::setOnRemovalCallback([onRemovalCallback, this](const PlayerColor & player)
	{
		onRemovalCallback(player, answer);
	});
}
