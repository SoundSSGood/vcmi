/*
 * CArtifact.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include "ArtBearer.h"
#include "EArtifactClass.h"

#include "../../bonuses/CBonusSystemNode.h"
#include "../../networkPacks/ArtifactLocation.h"

#include <vcmi/Artifact.h>

VCMI_LIB_NAMESPACE_BEGIN

class CArtifactSet;

class DLL_LINKAGE CCombinedArtifact
{
protected:
	CCombinedArtifact()
		: fused(false){};

	std::vector<const CArtifact *> constituents; // Artifacts IDs a combined artifact consists of, or nullptr.
	std::set<const CArtifact *> partOf; // Reverse map of constituents - combined arts that include this art
	bool fused;

public:
	bool isCombined() const;
	const std::vector<const CArtifact *> & getConstituents() const;
	const std::set<const CArtifact *> & getPartOf() const;
	void setFused(bool isFused);
	bool isFused() const;
	bool hasParts() const;
};

class DLL_LINKAGE CScrollArtifact
{
protected:
	CScrollArtifact() = default;

public:
	bool isScroll() const;
};

class DLL_LINKAGE CGrowingArtifact
{
public:
    CGrowingArtifact() = default;

    struct growingBonus
    {
        ui16 level;
        Bonus bonus;
        std::set<GrowingUpCondition> conditions;

        growingBonus(const ui16 level, const Bonus & bonus)
            : level(level), bonus(bonus)
        {}
    };

	bool isGrowing() const;

    std::vector<CGrowingArtifact::growingBonus> & getBonusesPerLevel();
    const std::vector<CGrowingArtifact::growingBonus> & getBonusesPerLevel() const;
    std::vector<CGrowingArtifact::growingBonus> & getThresholdBonuses();
    const std::vector<CGrowingArtifact::growingBonus> & getThresholdBonuses() const;

protected:

    std::vector<growingBonus> bonusesPerLevel; // Bonus given each n levels
    std::vector<growingBonus> thresholdBonuses; // After certain level they will be added once
};

// Container for artifacts. Not for instances.
class DLL_LINKAGE CArtifact final : public Artifact, public CBonusSystemNode, public CCombinedArtifact, public CScrollArtifact, public CGrowingArtifact
{
	ArtifactID id;
	std::string image;
	std::string large; // big image for custom artifacts, used in drag & drop
	std::string advMapDef; // used for adventure map object
	std::string modScope;
	std::string identifier;
	int32_t iconIndex;
	uint32_t price;
	CreatureID warMachine;
	// Bearer Type => ids of slots where artifact can be placed
	std::map<ArtBearer::ArtBearer, std::vector<ArtifactPosition>> possibleSlots;

public:
	EArtifactClass::Type aClass = EArtifactClass::ART_SPECIAL;
	bool onlyOnWaterMap;

	int32_t getIndex() const override;
	int32_t getIconIndex() const override;
	std::string getJsonKey() const override;
	std::string getModScope() const override;
	void registerIcons(const IconRegistar & cb) const override;
	ArtifactID getId() const override;
	const IBonusBearer * getBonusBearer() const override;

	std::string getDescriptionTranslated() const override;
	std::string getEventTranslated() const override;
	std::string getNameTranslated() const override;

	std::string getDescriptionTextID() const override;
	std::string getEventTextID() const override;
	std::string getNameTextID() const override;

	uint32_t getPrice() const override;
	CreatureID getWarMachine() const override;
	bool isBig() const override;
	bool isTradable() const override;

	int getArtClassSerial() const; //0 - treasure, 1 - minor, 2 - major, 3 - relic, 4 - spell scroll, 5 - other
	std::string nodeName() const override;
	void addNewBonus(const std::shared_ptr<Bonus> & b) override;
	const std::map<ArtBearer::ArtBearer, std::vector<ArtifactPosition>> & getPossibleSlots() const;

	virtual bool canBePutAt(const CArtifactSet * artSet, ArtifactPosition slot = ArtifactPosition::FIRST_AVAILABLE, bool assumeDestRemoved = false) const;
	void updateFrom(const JsonNode & data);
	// Is used for testing purposes only
	void setImage(int32_t iconIndex, std::string image, std::string large);

	CArtifact();
	~CArtifact();

	friend class CArtHandler;
};

VCMI_LIB_NAMESPACE_END
