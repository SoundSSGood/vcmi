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
#include "../../CPlayerInterface.h"

#include "../../../CCallback.h"

#include "../../../lib/entities/building/CBuilding.h"
#include "../../../lib/texts/CGeneralTextHandler.h"
#include "../../../lib/CSkillHandler.h"
#include "../../../lib/mapObjects/CGHeroInstance.h"
#include "../../../lib/mapObjects/CGMarket.h"
#include "../../../lib/mapObjects/CGTownInstance.h"
#include "../../../lib/mapObjects/ObjectTemplate.h"
#include "../../../lib/entities/faction/CTownHandler.h"

CUnivercity::CUnivercity(const IMarket * market, const CGHeroInstance * hero, const EMarketMode & mode)
	: CMarketBase(market, hero)
	, CMarketTraderText(Point(24, 129))
	, mode(mode)
{
	assert(mode == EMarketMode::GOLD_SECSKILL_BASIC || mode == EMarketMode::GOLD_SECSKILL_EXPERT);
	OBJECT_CONSTRUCTION_CUSTOM_CAPTURING(255 - DISPOSE);

	if(auto town = dynamic_cast<const CGTownInstance*>(market))
		titlePic = std::make_shared<CAnimImage>((*CGI->townh)[town->town->faction->getId()]->town->clientInfo.buildingsIcons,
			town->town->getSpecialBuilding(BuildingSubID::MAGIC_UNIVERSITY)->bid);
	else if(auto university = dynamic_cast<const CGUniversity*>(market); university && university->appearance)
		titlePic = std::make_shared<CAnimImage>(university->appearance->animationFile, 0);
	titlePic->moveTo(pos.topLeft() + Point(157, 39));

	state = getSkillsState();
	std::shared_ptr<SecondarySkillsPanel> skillsPanel = std::make_shared<SecondarySkillsPanel>(
		[this](const std::shared_ptr<CTradeableItem> & skillSlot)
		{
			CUnivercity::onSlotClickPressed(skillSlot, offerTradePanel);
		}, 
		[this, &skillsPanel]()
		{
			
		}, state);
	offerTradePanel = skillsPanel;
	offerTradePanel->moveTo(pos.topLeft() + Point(54, 234));

	for(const auto & tradeSlot : offerTradePanel->slots)
	{
		auto skillName = labels.emplace_back(std::make_shared<CLabel>(0, 0, FONT_SMALL, ETextAlignment::CENTER, Colors::WHITE,
			CGI->skillh->getByIndex(tradeSlot->id)->getNameTranslated()));
		skillName->moveTo(tradeSlot->pos.topLeft() + Point(23, -13));
	}

	// Showcase stuff
	showcaseBackground = std::make_shared<CPicture>(ImagePath::builtin("UNIVERS2.PCX"));
	
	deal = std::make_shared<CButton>(Point(148, 299), AnimationPath::builtin("IBY6432.DEF"),
		CGI->generaltexth->zelp[595], [this]()
		{
			CUnivercity::makeDeal();
		}, EShortcut::GLOBAL_ACCEPT);
	showcaseClose = std::make_shared<CButton>(Point(252, 299), AnimationPath::builtin("ICANCEL.DEF"),
		CGI->generaltexth->zelp[631], [this]()
		{
			CUnivercity::deselect();
			GH.statusbar()->clear(); // TODO should not be here
			redraw();
		}, EShortcut::GLOBAL_CANCEL);

	traderText->pos.w = 413;
	traderText->pos.h = 70;

	addChild(offerTradePanel->showcaseSlot.get());
	moveChildForeground(traderText.get());
	CUnivercity::deselect();
}

void CUnivercity::deselect()
{
	CMarketBase::deselect();
	CMarketTraderText::deselect();
	showcaseBackground->disable();
	deal->disable();
	showcaseClose->disable();
}

void CUnivercity::makeDeal()
{

}

CMarketBase::MarketShowcasesParams CUnivercity::getShowcasesParams() const
{
	if(offerTradePanel->isHighlighted())
		return MarketShowcasesParams
		{
			std::nullopt,
			ShowcaseParams {offerTradePanel->highlightedSlot->subtitle->getText(),
			offerTradePanel->getHighlightedItemId().value() * 3 + 2 + state[offerTradePanel->highlightedSlot->serial].level}
		};
	else
		return MarketShowcasesParams{std::nullopt, std::nullopt};
}

void CUnivercity::highlightingChanged()
{
	CMarketBase::highlightingChanged();
	CMarketTraderText::highlightingChanged();
}

void CUnivercity::onSlotClickPressed(const std::shared_ptr<CTradeableItem> & slot, std::shared_ptr<TradePanelBase> & curPanel)
{
	if(state[slot->serial].availability == SecondarySkillsPanel::SkillAvailability::canBeLearned)
	{
		showcaseBackground->enable();
		deal->enable();
		showcaseClose->enable();

		market->getOffer(GameResID::GOLD, state[slot->serial].skill.num, bidQty, offerQty, mode);
		deal->block(LOCPLINT->cb->getResourceAmount(GameResID::GOLD) < bidQty || !LOCPLINT->makingTurn);

		curPanel->highlightedSlot = slot;
		highlightingChanged();
	}
}

std::string CUnivercity::getTraderText()
{
	if(offerTradePanel->isHighlighted())
	{
		MetaString message = MetaString::createFromRawString(CGI->generaltexth->allTexts[608]);
		message.replaceRawString(offerTradePanel->highlightedSlot->subtitle->getText());
		message.replaceName(SecondarySkill(offerTradePanel->getHighlightedItemId().value()));
		message.replaceNumber(bidQty);
		return message.toString();
	}
	else
	{
		return CGI->generaltexth->allTexts[603];
	}
}

CUnivercity::SkillsState CUnivercity::getSkillsState()
{
	std::vector<SecondarySkillsPanel::SkillState> state;
	ui8 maxSkillLevel = mode == EMarketMode::GOLD_SECSKILL_BASIC ? 1 : 3;

	for(const auto & skill : market->availableItemsIds(EMarketMode::GOLD_SECSKILL_BASIC))
	{
		auto skillAvailability = SecondarySkillsPanel::SkillAvailability::canBeLearned;
		auto skillLevel = hero->getSecSkillLevel(skill.getNum());
		skillLevel += 1;	// next level
		if(skillLevel == 1)
		{
			if(!hero->canLearnSkill(skill.getNum()))
				skillAvailability = SecondarySkillsPanel::SkillAvailability::cantBeLearned;
		}
		else if(skillLevel > maxSkillLevel)
		{
			skillAvailability = SecondarySkillsPanel::SkillAvailability::alreadyLearned;
			skillLevel = maxSkillLevel;
		}
		state.emplace_back(SecondarySkillsPanel::SkillState{skill.getNum(), skillLevel, skillAvailability});
	}
	return state;
}
 