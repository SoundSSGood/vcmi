/*
 * CUnivercity.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#include "StdInc.h"
#include "CUnivercity.h"

#include "../../gui/CGuiHandler.h"
#include "../../gui/Shortcut.h"
#include "../../widgets/Buttons.h"
#include "../../widgets/TextControls.h"

#include "../../CGameInfo.h"

#include "../../../lib/CGeneralTextHandler.h"
#include "../../../lib/CSkillHandler.h"
#include "../../../lib/mapObjects/CGMarket.h"
#include "../../../lib/mapObjects/CGTownInstance.h"
#include "../../../lib/mapObjects/ObjectTemplate.h"

CUnivercity::CUnivercity(const IMarket * market, const CGHeroInstance * hero)
	: CMarketBase(market, hero)
	, CMarketTraderText(Point(24, 129))
{
	OBJECT_CONSTRUCTION_CUSTOM_CAPTURING(255 - DISPOSE);

	if(auto town = dynamic_cast<const CGTownInstance*>(market))
		titlePic = std::make_shared<CAnimImage>((*CGI->townh)[town->town->faction->getId()]->town->clientInfo.buildingsIcons,
			town->town->getSpecialBuilding(BuildingSubID::MAGIC_UNIVERSITY)->bid);
	else if(auto university = dynamic_cast<const CGUniversity*>(market); university && university->appearance)
		titlePic = std::make_shared<CAnimImage>(university->appearance->animationFile, 0);
	titlePic->moveTo(pos.topLeft() + Point(157, 39));

	offerTradePanel = std::make_shared<SecondarySkillsPanel>([this](const std::shared_ptr<CTradeableItem> & skillSlot)
		{
			CUnivercity::onSlotClickPressed(skillSlot, offerTradePanel);
		}, market->availableItemsIds(EMarketMode::GOLD_SECSKILL_BASIC));
	offerTradePanel->moveTo(pos.topLeft() + Point(54, 234));

	for(const auto & tradeSlot : offerTradePanel->slots)
	{
		auto skillName = labels.emplace_back(std::make_shared<CLabel>(0, 0, FONT_SMALL, ETextAlignment::CENTER, Colors::WHITE,
			CGI->skillh->getByIndex(tradeSlot->id)->getNameTranslated()));
		skillName->moveTo(tradeSlot->pos.topLeft() + Point(23, -13));
	}
	

	// Showcase stuff
	showcaseBack = std::make_shared<CPicture>(ImagePath::builtin("UNIVERS2.PCX"));
	showcaseBack->disable();
	
	deal = std::make_shared<CButton>(Point(200, 313), AnimationPath::builtin("IBY6432.DEF"),
		CGI->generaltexth->zelp[595], [this]() {}, EShortcut::GLOBAL_ACCEPT);
	deal->disable();

	traderText->pos.w = 413;
	traderText->pos.h = 70;

	CUnivercity::deselect();
}

void CUnivercity::deselect()
{
	CMarketBase::deselect();
	CMarketTraderText::deselect();
}

void CUnivercity::makeDeal()
{

}

CMarketBase::MarketShowcasesParams CUnivercity::getShowcasesParams() const
{
	/*if (offerTradePanel->isHighlighted())
		return MarketShowcasesParams
	{
		ShowcaseParams {std::to_string(bidQty * offerSlider->getValue()), CGI->creatures()->getByIndex(bidTradePanel->getHighlightedItemId())->getIconIndex()},
		ShowcaseParams {std::to_string(offerQty * offerSlider->getValue()), offerTradePanel->getHighlightedItemId()}
	};
	else*/
	return MarketShowcasesParams{std::nullopt, std::nullopt};
}

void CUnivercity::onSlotClickPressed(const std::shared_ptr<CTradeableItem> & newSlot, std::shared_ptr<TradePanelBase> & curPanel)
{

}

std::string CUnivercity::getTraderText()
{
	/*if (offerTradePanel->isHighlighted())
	{
		MetaString message = MetaString::createFromTextID("core.genrltxt.269");
		message.replaceNumber(offerQty);
		message.replaceRawString(offerQty == 1 ? CGI->generaltexth->allTexts[161] : CGI->generaltexth->allTexts[160]);
		message.replaceName(GameResID(offerTradePanel->getHighlightedItemId()));
		message.replaceNumber(bidQty);
		if (bidQty == 1)
			message.replaceNameSingular(bidTradePanel->getHighlightedItemId());
		else
			message.replaceNamePlural(bidTradePanel->getHighlightedItemId());
		return message.toString();
	}
	else
	{
		return madeTransaction ? CGI->generaltexth->allTexts[162] : CGI->generaltexth->allTexts[163];
	}*/
	return CGI->generaltexth->allTexts[603];
}
 