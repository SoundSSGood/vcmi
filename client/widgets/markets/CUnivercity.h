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
	CUnivercity(const IMarket * market, const CGHeroInstance * hero, const EMarketMode & mode);
	void deselect() override;
	void makeDeal() override;

private:
	using SkillsState = std::vector<SecondarySkillsPanel::SkillState>;

	std::shared_ptr<CAnimImage> titlePic;
	std::shared_ptr<CPicture> showcaseBackground;
	std::shared_ptr<CButton> showcaseClose;
	const EMarketMode mode;
	SkillsState state;

	MarketShowcasesParams getShowcasesParams() const override;
	void highlightingChanged() override;
	void onSlotClickPressed(const std::shared_ptr<CTradeableItem> & slot, std::shared_ptr<TradePanelBase> & curPanel);
	std::string getTraderText() override;
	SkillsState getSkillsState();
};
