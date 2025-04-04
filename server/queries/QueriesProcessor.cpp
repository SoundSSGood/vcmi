/*
 * CQuery.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "CQuery.h"
#include "VisitQueries.h"

#include "QueriesProcessor.h"

void QueriesProcessor::popQuery(PlayerColor player, QueryPtr query)
{
	LOG_TRACE_PARAMS(logGlobal, "player='%s', query='%s'", player % query);
	if(topQuery(player) != query)
	{
		logGlobal->trace("Cannot remove, not a top!");
		return;
	}

	queries[player] -= query;
	std::cout << "deleted " << query->queryID << " " << player << std::endl;
	query->onRemoval(player);

	// Try to pop the next query
	if(auto nextQuery = topQuery(player); nextQuery && nextQuery->getReadyForRemoval())
		popQuery(nextQuery);
}

void QueriesProcessor::popQuery(const CQuery &query)
{
	LOG_TRACE_PARAMS(logGlobal, "query='%s'", query);

	assert(query.players.size());
	for(auto player : query.players)
	{
		auto top = topQuery(player);
		if(top.get() == &query)
			popQuery(top);
		else
		{
			logGlobal->trace("Cannot remove query %s", query.toString());
			logGlobal->trace("Queries found:");
			for(auto q : queries[player])
			{
				logGlobal->trace(q->toString());
			}
		}
	}
}

void QueriesProcessor::popQuery(QueryPtr query)
{
	for(auto player : query->players)
		popQuery(player, query);
}

void QueriesProcessor::addQuery(QueryPtr query, const std::vector<PlayerColor> & players)
{
	for(const auto & player : players)
		addQuery(query, player);
}

void QueriesProcessor::addQuery(QueryPtr query, const PlayerColor & player)
{
	if(player.isValidPlayer())
	{
		LOG_TRACE_PARAMS(logGlobal, "player='%d', query='%s'", player.getNum() % query);
		query->addPlayer(player);
		std::cout << "add Q " << query << std::endl;
		query->onAdding(player);
		queries[player].push_back(query);
	}
}

QueryPtr QueriesProcessor::topQuery(PlayerColor player)
{
	return vstd::backOrNull(queries[player]);
}

void QueriesProcessor::popIfTop(QueryPtr query)
{
	LOG_TRACE_PARAMS(logGlobal, "query='%d'", query);
	if(!query)
		logGlobal->error("The query is nullptr! Ignoring.");

	popIfTop(*query);
}

void QueriesProcessor::popIfTop(const CQuery & query)
{
	for(PlayerColor color : query.players)
		if(topQuery(color).get() == &query)
			popQuery(color, topQuery(color));
}

std::vector<std::shared_ptr<const CQuery>> QueriesProcessor::allQueries() const
{
	std::vector<std::shared_ptr<const CQuery>> ret;
	for(auto & playerQueries : queries)
		for(auto & query : playerQueries.second)
			ret.push_back(query);

	return ret;
}

std::vector<QueryPtr> QueriesProcessor::allQueries()
{
	//TODO code duplication with const function :(
	std::vector<QueryPtr> ret;
	for(auto & playerQueries : queries)
		for(auto & query : playerQueries.second)
			ret.push_back(query);

	return ret;
}

QueryPtr QueriesProcessor::getQuery(QueryID queryID)
{
	for(auto & playerQueries : queries)
		for(auto & query : playerQueries.second)
			if(query->queryID == queryID)
				return query;
	return nullptr;
}

const std::tuple<const ObjectInstanceID, const ObjectInstanceID> QueriesProcessor::getActiveVisitorAndObj(const PlayerColor & player)
{
	for(const auto & query : queries[player])
	{
		if(const auto visitingQuery = std::dynamic_pointer_cast<VisitQuery>(query))
			return std::make_tuple(visitingQuery->visitingHero, visitingQuery->visitedObject);
	}
	return std::make_tuple(ObjectInstanceID::NONE, ObjectInstanceID::NONE);
}

void QueriesProcessor::execWhileOnTop(const QueryID & Id, const PlayerColor & player)
{
	auto query = topQuery(player);
	while(query->queryID == Id && !query->getReadyForRemoval())
	{
		//query->onTopExecuteFunctor(player);
	}
	if(query->getReadyForRemoval())
		popIfTop(query);
}
