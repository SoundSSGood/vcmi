/*
 * CUnivercity.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include "CMarketBase.h"

class CUnivercity : public CMarketTraderText
{
public:
	CUnivercity(const IMarket * market, const CGHeroInstance * hero);
	void deselect() override;
	void makeDeal() override;

private:
	std::shared_ptr<CAnimation> bars;
	std::shared_ptr<CAnimImage> titlePic;
	std::shared_ptr<CPicture> showcaseBack;

	MarketShowcasesParams getShowcasesParams() const override;
	void onSlotClickPressed(const std::shared_ptr<CTradeableItem> & newSlot, std::shared_ptr<TradePanelBase> & curPanel);
	std::string getTraderText() override;
};
