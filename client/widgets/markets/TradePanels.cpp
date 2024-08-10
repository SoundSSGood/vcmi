/*
 * TradePanels.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"
#include "TradePanels.h"

#include "../../gui/CGuiHandler.h"
#include "../../render/Canvas.h"
#include "../../widgets/TextControls.h"
#include "../../windows/InfoWindows.h"

#include "../../CGameInfo.h"
#include "../../CPlayerInterface.h"

#include "../../../CCallback.h"

#include "../../../lib/texts/CGeneralTextHandler.h"
#include "../../../lib/CSkillHandler.h"
#include "../../../lib/mapObjects/CGHeroInstance.h"

CTradeableItem::CTradeableItem(const Rect & area, EType Type, int ID, int Serial)
	: SelectableSlot(area, Point(1, 1))
	, type(EType(-1)) // set to invalid, will be corrected in setType
	, id(ID)
	, serial(Serial)
{
	OBJECT_CONSTRUCTION;

	addUsedEvents(LCLICK);
	addUsedEvents(HOVER);
	addUsedEvents(SHOW_POPUP);
	
	subtitle = std::make_shared<CLabel>(0, 0, FONT_SMALL, ETextAlignment::CENTER, Colors::WHITE);
	setType(Type);

	this->pos.w = area.w;
	this->pos.h = area.h;
}

void CTradeableItem::setType(EType newType)
{
	if(type != newType)
	{
		OBJECT_CONSTRUCTION;
		type = newType;

		if(getIndex() < 0)
		{
			image = std::make_shared<CAnimImage>(getFilename(), 0);
			image->disable();
		}
		else
		{
			image = std::make_shared<CAnimImage>(getFilename(), getIndex());
		}

		switch(type)
		{
		case EType::RESOURCE:
			subtitle->moveTo(pos.topLeft() + Point(35, 55));
			image->moveTo(pos.topLeft() + Point(19, 8));
			break;
		case EType::CREATURE:
			subtitle->moveTo(pos.topLeft() + Point(30, 77));
			break;
		case EType::PLAYER:
			subtitle->moveTo(pos.topLeft() + Point(31, 76));
			break;
		case EType::ARTIFACT_PLACEHOLDER:
			subtitle->moveTo(pos.topLeft() + Point(21, 55));
			break;
		case EType::ARTIFACT_TYPE:
			subtitle->moveTo(pos.topLeft() + Point(35, 57));
			image->moveTo(pos.topLeft() + Point(13, 0));
		case EType::SECSKILL:
			break;
		}
	}
}

void CTradeableItem::setID(int newID)
{
	if(id != newID)
	{
		id = newID;
		if(image)
		{
			int index = getIndex();
			if(index < 0)
				image->disable();
			else
			{
				image->enable();
				image->setFrame(index);
			}
		}
	}
}

void CTradeableItem::clear()
{
	setID(-1);
	image->setFrame(0);
	image->disable();
	subtitle->clear();
}

AnimationPath CTradeableItem::getFilename()
{
	switch(type)
	{
	case EType::RESOURCE:
		return AnimationPath::builtin("RESOURCE");
	case EType::PLAYER:
		return AnimationPath::builtin("CREST58");
	case EType::ARTIFACT_TYPE:
	case EType::ARTIFACT_PLACEHOLDER:
		return AnimationPath::builtin("artifact");
	case EType::CREATURE:
		return AnimationPath::builtin("TWCRPORT");
	case EType::SECSKILL:
		return AnimationPath::builtin("SECSKILL");
	default:
		return {};
	}
}

int CTradeableItem::getIndex()
{
	if(id < 0)
		return -1;

	switch(type)
	{
	case EType::RESOURCE:
	case EType::PLAYER:
		return id;
	case EType::ARTIFACT_TYPE:
	case EType::ARTIFACT_PLACEHOLDER:
		return CGI->artifacts()->getByIndex(id)->getIconIndex();
	case EType::CREATURE:
		return CGI->creatures()->getByIndex(id)->getIconIndex();
	case EType::SECSKILL:
		return id * 3 + 3;
	default:
		return -1;
	}
}

void CTradeableItem::clickPressed(const Point & cursorPosition)
{
	if(clickPressedCallback)
		clickPressedCallback(shared_from_this());
}

void CTradeableItem::hover(bool on)
{
	if(!on || id == -1)
	{
		GH.statusbar()->clear();
		return;
	}

	switch(type)
	{
	case EType::CREATURE:
		GH.statusbar()->write(boost::str(boost::format(CGI->generaltexth->allTexts[481]) % CGI->creh->objects[id]->getNamePluralTranslated()));
		break;
	case EType::ARTIFACT_TYPE:
	case EType::ARTIFACT_PLACEHOLDER:
		if(id < 0)
			GH.statusbar()->write(CGI->generaltexth->zelp[582].first);
		else
			GH.statusbar()->write(CGI->artifacts()->getByIndex(id)->getNameTranslated());
		break;
	case EType::RESOURCE:
		GH.statusbar()->write(CGI->generaltexth->restypes[id]);
		break;
	case EType::PLAYER:
		GH.statusbar()->write(CGI->generaltexth->capColors[id]);
		break;
	case EType::SECSKILL:
		GH.statusbar()->write(CGI->skills()->getByIndex(id)->getNameTranslated());
		break;
	}
}

void CTradeableItem::showPopupWindow(const Point & cursorPosition)
{
	switch(type)
	{
	case EType::CREATURE:
		break;
	case EType::ARTIFACT_TYPE:
	case EType::ARTIFACT_PLACEHOLDER:
		//TODO: it's would be better for market to contain actual CArtifactInstance and not just ids of certain artifact type so we can use getEffectiveDescription.
		if (id >= 0)
			CRClickPopup::createAndPush(CGI->artifacts()->getByIndex(id)->getDescriptionTranslated());
		break;
	}
}

void TradePanelBase::update()
{
	if(deleteSlotsCheck)
		slots.erase(std::remove_if(slots.begin(), slots.end(), deleteSlotsCheck), slots.end());

	if(updateSlotsCallback)
		updateSlotsCallback();
}

void TradePanelBase::deselect()
{
	for(const auto & slot : slots)
		slot->selectSlot(false);
}

void TradePanelBase::clearSubtitles()
{
	for(const auto & slot : slots)
		slot->subtitle->clear();
}

void TradePanelBase::updateOffer(CTradeableItem & slot, int cost, int qty)
{
	std::string subtitle = std::to_string(qty);
	if(cost != 1)
	{
		subtitle.append("/");
		subtitle.append(std::to_string(cost));
	}
	slot.subtitle->setText(subtitle);
}

void TradePanelBase::setShowcaseSubtitle(const std::string & text)
{
	showcaseSlot->subtitle->setText(text);
}

std::optional<int> TradePanelBase::getHighlightedItemId() const
{
	if(highlightedSlot)
		return highlightedSlot->id;
	else
		return std::nullopt;
}

void TradePanelBase::onSlotClickPressed(const std::shared_ptr<CTradeableItem> & newSlot)
{
	assert(vstd::contains(slots, newSlot));
	if(newSlot == highlightedSlot)
		return;

	if(highlightedSlot)
		highlightedSlot->selectSlot(false);
	highlightedSlot = newSlot;
	newSlot->selectSlot(true);
}

bool TradePanelBase::isHighlighted() const
{
	return highlightedSlot != nullptr;
}

ResourcesPanel::ResourcesPanel(const CTradeableItem::ClickPressedFunctor & clickPressedCallback,
	const UpdateSlotsFunctor & updateSubtitles)
{
	assert(resourcesForTrade.size() == slotsPos.size());
	OBJECT_CONSTRUCTION;

	for(const auto & res : resourcesForTrade)
	{
		auto slot = slots.emplace_back(std::make_shared<CTradeableItem>(Rect(slotsPos[res.num], slotDimension), EType::RESOURCE, res.num, res.num));
		slot->clickPressedCallback = clickPressedCallback;
		slot->setSelectionWidth(highlightWidth);
	}
	updateSlotsCallback = updateSubtitles;
	showcaseSlot = std::make_shared<CTradeableItem>(Rect(showcasePos, slotDimension), EType::RESOURCE, 0, 0);
}

ArtifactsPanel::ArtifactsPanel(const CTradeableItem::ClickPressedFunctor & clickPressedCallback,
	const UpdateSlotsFunctor & updateSubtitles, const std::vector<TradeItemBuy> & arts)
{
	assert(slotsForTrade == slotsPos.size());
	assert(slotsForTrade == arts.size());
	OBJECT_CONSTRUCTION;

	for(auto slotIdx = 0; slotIdx < slotsForTrade; slotIdx++)
	{
		auto artType = arts[slotIdx].getNum();
		if(artType != ArtifactID::NONE)
		{
			auto slot = slots.emplace_back(std::make_shared<CTradeableItem>(Rect(slotsPos[slotIdx], slotDimension),
				EType::ARTIFACT_TYPE, artType, slotIdx));
			slot->clickPressedCallback = clickPressedCallback;
			slot->setSelectionWidth(highlightWidth);
		}
	}
	updateSlotsCallback = updateSubtitles;
	showcaseSlot = std::make_shared<CTradeableItem>(Rect(showcasePos, slotDimension), EType::ARTIFACT_TYPE, 0, 0);
	showcaseSlot->subtitle->moveBy(Point(0, 1));
}

PlayersPanel::PlayersPanel(const CTradeableItem::ClickPressedFunctor & clickPressedCallback)
{
	assert(PlayerColor::PLAYER_LIMIT_I <= slotsPos.size() + 1);
	OBJECT_CONSTRUCTION;

	std::vector<PlayerColor> players;
	for(auto player = PlayerColor(0); player < PlayerColor::PLAYER_LIMIT_I; player++)
	{
		if(player != LOCPLINT->playerID && LOCPLINT->cb->getPlayerStatus(player) == EPlayerStatus::INGAME)
			players.emplace_back(player);
	}

	slots.resize(players.size());
	int slotNum = 0;
	for(auto & slot : slots)
	{
		slot = std::make_shared<CTradeableItem>(Rect(slotsPos[slotNum], slotDimension), EType::PLAYER, players[slotNum].num, slotNum);
		slot->clickPressedCallback = clickPressedCallback;
		slot->setSelectionWidth(highlightWidth);
		slot->subtitle->setText(CGI->generaltexth->capColors[players[slotNum].num]);
		slotNum++;
	}
	showcaseSlot = std::make_shared<CTradeableItem>(Rect(showcasePos, slotDimension), EType::PLAYER, 0, 0);
}

CreaturesPanel::CreaturesPanel(const CTradeableItem::ClickPressedFunctor & clickPressedCallback, const slotsData & initialSlots)
{
	assert(initialSlots.size() <= GameConstants::ARMY_SIZE);
	assert(slotsPos.size() <= GameConstants::ARMY_SIZE);
	OBJECT_CONSTRUCTION;

	for(const auto & [creatureId, slotId, creaturesNum] : initialSlots)
	{
		auto slot = slots.emplace_back(std::make_shared<CTradeableItem>(Rect(slotsPos[slotId.num], slotDimension),
			 EType::CREATURE, creaturesNum == 0 ? -1 : creatureId.num, slotId));
		slot->clickPressedCallback = clickPressedCallback;
		if(creaturesNum != 0)
			slot->subtitle->setText(std::to_string(creaturesNum));
		slot->setSelectionWidth(highlightWidth);
	}
	showcaseSlot = std::make_shared<CTradeableItem>(Rect(showcasePos, slotDimension), EType::CREATURE, 0, 0);
}

CreaturesPanel::CreaturesPanel(const CTradeableItem::ClickPressedFunctor & clickPressedCallback,
	const std::vector<std::shared_ptr<CTradeableItem>> & srcSlots, bool emptySlots)
{
	assert(slots.size() <= GameConstants::ARMY_SIZE);
	OBJECT_CONSTRUCTION;

	for(const auto & srcSlot : srcSlots)
	{
		auto slot = slots.emplace_back(std::make_shared<CTradeableItem>(Rect(slotsPos[srcSlot->serial], srcSlot->pos.dimensions()),
			EType::CREATURE, emptySlots ? -1 : srcSlot->id, srcSlot->serial));
		slot->clickPressedCallback = clickPressedCallback;
		slot->subtitle->setText(emptySlots ? "" : srcSlot->subtitle->getText());
		slot->setSelectionWidth(highlightWidth);
	}
	showcaseSlot = std::make_shared<CTradeableItem>(Rect(showcasePos, slotDimension), EType::CREATURE, 0, 0);
}

ArtifactsAltarPanel::ArtifactsAltarPanel(const CTradeableItem::ClickPressedFunctor & clickPressedCallback)
{
	OBJECT_CONSTRUCTION;

	int slotNum = 0;
	for(auto & altarSlotPos : slotsPos)
	{
		auto slot = slots.emplace_back(std::make_shared<CTradeableItem>(Rect(altarSlotPos, Point(44, 44)), EType::ARTIFACT_PLACEHOLDER, -1, slotNum));
		slot->clickPressedCallback = clickPressedCallback;
		slot->subtitle->clear();
		slot->subtitle->moveBy(Point(0, -1));
		slotNum++;
	}
	showcaseSlot = std::make_shared<CTradeableItem>(Rect(showcasePos, slotDimension), EType::ARTIFACT_TYPE, 0, 0);
	showcaseSlot->subtitle->moveBy(Point(0, 3));
}

SecondarySkillsPanel::SecondarySkillsPanel(const CTradeableItem::ClickPressedFunctor & clickPressedCallback,
	const UpdateSlotsFunctor & updateSlot, const std::vector<SkillState> & skillsState)
{
	assert(skillsState.size() <= slotsPos.size());
	OBJECT_CONSTRUCTION;

	updateSlotsCallback = updateSlot;
	int slotNum = 0;
	slots.resize(skillsState.size());
	setSkillsState(skillsState);
	for(auto & slot : slots)
	{
		slot = std::make_shared<CTradeableItem>(Rect(slotsPos[slotNum], slotDimension), EType::SECSKILL, skillsState[slotNum].skill.num, slotNum);
		slot->clickPressedCallback = clickPressedCallback;
		slot->subtitle->setText(skillsState[slotNum].level > 0 ? CGI->generaltexth->levels[skillsState[slotNum].level - 1] : "");
		slot->subtitle->moveBy(Point(23, 56));
		slotNum++;
	}
	showcaseSlot = std::make_shared<CTradeableItem>(Rect(showcasePos, slotDimension), EType::SECSKILL, -1, 0);
	showcaseSlot->moveTo(pos.topLeft() + showcasePos);
	showcaseSlot->subtitle->moveBy(Point(23, 56));
}

void SecondarySkillsPanel::setSkillsState(const std::vector<SkillState> & state)
{
	assert(slots.size() == state.size());

	int slotNum = 0;
	for(auto & slot : slots)
	{
		ImagePath image;
		switch(state[slotNum].availability)
		{
		case SkillAvailability::alreadyLearned:
			image = ImagePath::builtin("UNIVGOLD");
			break;
		case SkillAvailability::canBeLearned:
			image = ImagePath::builtin("UNIVGREN");
			break;
		default:
			image = ImagePath::builtin("UNIVRED");
		}

		OBJECT_CONSTRUCTION;
		bars.emplace_back(std::make_shared<CPicture>(image))->moveTo(pos.topLeft() + slotsPos[slotNum] + Point(-28, -22));
		bars.emplace_back(std::make_shared<CPicture>(image))->moveTo(pos.topLeft() + slotsPos[slotNum] + Point(-28, 48));
		slotNum++;
	}
}
