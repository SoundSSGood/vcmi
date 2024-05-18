/*
 * ShortcutHandler.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#include "StdInc.h"

#include "ShortcutHandler.h"
#include "Shortcut.h"

#include "../../lib/json/JsonUtils.h"

ShortcutHandler::ShortcutHandler()
{
	const JsonNode config = JsonUtils::assembleFromFiles("config/shortcutsConfig");

	mappedKeyboardShortcuts = loadShortcuts(config["keyboard"]);
	mappedJoystickShortcuts = loadShortcuts(config["joystickButtons"]);
	mappedJoystickAxes = loadShortcuts(config["joystickAxes"]);
}

std::multimap<std::string, EShortcut> ShortcutHandler::loadShortcuts(const JsonNode & data) const
{
	std::multimap<std::string, EShortcut> result;

	for (auto const & entry : data.Struct())
	{
		std::string shortcutName = entry.first;
		EShortcut shortcutID = findShortcut(shortcutName);

		if (shortcutID == EShortcut::NONE)
		{
			logGlobal->warn("Unknown shortcut '%s' found when loading shortcuts config!", shortcutName);
			continue;
		}

		if (entry.second.isString())
		{
			result.emplace(entry.second.String(), shortcutID);
		}

		if (entry.second.isVector())
		{
			for (auto const & entryVector : entry.second.Vector())
				result.emplace(entryVector.String(), shortcutID);
		}
	}

	return result;
}

std::vector<EShortcut> ShortcutHandler::translateShortcut(const std::multimap<std::string, EShortcut> & options, const std::string & key) const
{
	auto range = options.equal_range(key);

	// FIXME: some code expects calls to keyPressed / captureThisKey even without defined hotkeys
	if (range.first == range.second)
		return {EShortcut::NONE};

	std::vector<EShortcut> result;

	for (auto it = range.first; it != range.second; ++it)
		result.push_back(it->second);

	return result;
}

std::vector<EShortcut> ShortcutHandler::translateKeycode(const std::string & key) const
{
	return translateShortcut(mappedKeyboardShortcuts, key);
}

std::vector<EShortcut> ShortcutHandler::translateJoystickButton(const std::string & key) const
{
	return translateShortcut(mappedJoystickShortcuts, key);
}

std::vector<EShortcut> ShortcutHandler::translateJoystickAxis(const std::string & key) const
{
	return translateShortcut(mappedJoystickAxes, key);
}

EShortcut ShortcutHandler::findShortcut(const std::string & identifier ) const
{
	static const std::map<std::string, EShortcut> shortcutNames = {
		{"mouseClickLeft",           EShortcut::MOUSE_LEFT                },
		{"mouseClickRight",          EShortcut::MOUSE_RIGHT               },
		{"mouseCursorX",             EShortcut::MOUSE_CURSOR_X,           },
		{"mouseCursorY",             EShortcut::MOUSE_CURSOR_Y,           },
		{"mouseSwipeX",              EShortcut::MOUSE_SWIPE_X,            },
		{"mouseSwipeY",              EShortcut::MOUSE_SWIPE_Y,            },
		{"globalAccept",             EShortcut::GLOBAL_ACCEPT             },
		{"globalCancel",             EShortcut::GLOBAL_CANCEL             },
		{"globalReturn",             EShortcut::GLOBAL_RETURN             },
		{"globalFullscreen",         EShortcut::GLOBAL_FULLSCREEN         },
		{"globalOptions",            EShortcut::GLOBAL_OPTIONS            },
		{"globalBackspace",          EShortcut::GLOBAL_BACKSPACE          },
		{"globalMoveFocus",          EShortcut::GLOBAL_MOVE_FOCUS         },
		{"moveLeft",                 EShortcut::MOVE_LEFT                 },
		{"moveRight",                EShortcut::MOVE_RIGHT                },
		{"moveUp",                   EShortcut::MOVE_UP                   },
		{"moveDown",                 EShortcut::MOVE_DOWN                 },
		{"moveFirst",                EShortcut::MOVE_FIRST                },
		{"moveLast",                 EShortcut::MOVE_LAST                 },
		{"movePageUp",               EShortcut::MOVE_PAGE_UP              },
		{"movePageDown",             EShortcut::MOVE_PAGE_DOWN            },
		{"selectIndex1",             EShortcut::SELECT_INDEX_1            },
		{"selectIndex2",             EShortcut::SELECT_INDEX_2            },
		{"selectIndex3",             EShortcut::SELECT_INDEX_3            },
		{"selectIndex4",             EShortcut::SELECT_INDEX_4            },
		{"selectIndex5",             EShortcut::SELECT_INDEX_5            },
		{"selectIndex6",             EShortcut::SELECT_INDEX_6            },
		{"selectIndex7",             EShortcut::SELECT_INDEX_7            },
		{"selectIndex8",             EShortcut::SELECT_INDEX_8            },
		{"mainMenuNewGame",          EShortcut::MAIN_MENU_NEW_GAME        },
		{"mainMenuLoadGame",         EShortcut::MAIN_MENU_LOAD_GAME       },
		{"mainMenuHighScores",       EShortcut::MAIN_MENU_HIGH_SCORES     },
		{"mainMenuCredits",          EShortcut::MAIN_MENU_CREDITS         },
		{"mainMenuQuit",             EShortcut::MAIN_MENU_QUIT            },
		{"mainMenuBack",             EShortcut::MAIN_MENU_BACK            },
		{"mainMenuSingleplayer",     EShortcut::MAIN_MENU_SINGLEPLAYER    },
		{"mainMenuMultiplayer",      EShortcut::MAIN_MENU_MULTIPLAYER     },
		{"mainMenuCampaign",         EShortcut::MAIN_MENU_CAMPAIGN        },
		{"mainMenuTutorial",         EShortcut::MAIN_MENU_TUTORIAL        },
		{"mainMenuCampaignSod",      EShortcut::MAIN_MENU_CAMPAIGN_SOD    },
		{"mainMenuCampaignRoe",      EShortcut::MAIN_MENU_CAMPAIGN_ROE    },
		{"mainMenuCampaignAb",       EShortcut::MAIN_MENU_CAMPAIGN_AB     },
		{"mainMenuCampaignCustom",   EShortcut::MAIN_MENU_CAMPAIGN_CUSTOM },
		{"mainMenuLobby",            EShortcut::MAIN_MENU_LOBBY           },
		{"lobbyBeginStandardGame",   EShortcut::LOBBY_BEGIN_STANDARD_GAME },
		{"lobbyBeginCampaign",       EShortcut::LOBBY_BEGIN_CAMPAIGN      },
		{"lobbyLoadGame",            EShortcut::LOBBY_LOAD_GAME           },
		{"lobbySaveGame",            EShortcut::LOBBY_SAVE_GAME           },
		{"lobbyRandomMap",           EShortcut::LOBBY_RANDOM_MAP          },
		{"lobbyToggleChat",          EShortcut::LOBBY_TOGGLE_CHAT         },
		{"lobbyAdditionalOptions",   EShortcut::LOBBY_ADDITIONAL_OPTIONS  },
		{"lobbySelectScenario",      EShortcut::LOBBY_SELECT_SCENARIO     },
		{"gameEndTurn",              EShortcut::GAME_END_TURN             },
		{"gameLoadGame",             EShortcut::GAME_LOAD_GAME            },
		{"gameSaveGame",             EShortcut::GAME_SAVE_GAME            },
		{"gameRestartGame",          EShortcut::GAME_RESTART_GAME         },
		{"gameMainMenu",             EShortcut::GAME_TO_MAIN_MENU         },
		{"gameQuitGame",             EShortcut::GAME_QUIT_GAME            },
		{"gameOpenMarketplace",      EShortcut::GAME_OPEN_MARKETPLACE     },
		{"gameOpenThievesGuild",     EShortcut::GAME_OPEN_THIEVES_GUILD   },
		{"gameActivateConsole",      EShortcut::GAME_ACTIVATE_CONSOLE     },
		{"adventureGameOptions",     EShortcut::ADVENTURE_GAME_OPTIONS    },
		{"adventureToggleGrid",      EShortcut::ADVENTURE_TOGGLE_GRID     },
		{"adventureToggleSleep",     EShortcut::ADVENTURE_TOGGLE_SLEEP    },
		{"adventureSetHeroAsleep",   EShortcut::ADVENTURE_SET_HERO_ASLEEP },
		{"adventureSetHeroAwake",    EShortcut::ADVENTURE_SET_HERO_AWAKE  },
		{"adventureMoveHero",        EShortcut::ADVENTURE_MOVE_HERO       },
		{"adventureVisitObject",     EShortcut::ADVENTURE_VISIT_OBJECT    },
		{"adventureMoveHeroSW",      EShortcut::ADVENTURE_MOVE_HERO_SW    },
		{"adventureMoveHeroSS",      EShortcut::ADVENTURE_MOVE_HERO_SS    },
		{"adventureMoveHeroSE",      EShortcut::ADVENTURE_MOVE_HERO_SE    },
		{"adventureMoveHeroWW",      EShortcut::ADVENTURE_MOVE_HERO_WW    },
		{"adventureMoveHeroEE",      EShortcut::ADVENTURE_MOVE_HERO_EE    },
		{"adventureMoveHeroNW",      EShortcut::ADVENTURE_MOVE_HERO_NW    },
		{"adventureMoveHeroNN",      EShortcut::ADVENTURE_MOVE_HERO_NN    },
		{"adventureMoveHeroNE",      EShortcut::ADVENTURE_MOVE_HERO_NE    },
		{"adventureViewSelected",    EShortcut::ADVENTURE_VIEW_SELECTED   },
		{"adventureNextObject",      EShortcut::ADVENTURE_NEXT_OBJECT     },
		{"adventureNextTown",        EShortcut::ADVENTURE_NEXT_TOWN       },
		{"adventureNextHero",        EShortcut::ADVENTURE_NEXT_HERO       },
		{"adventureFirstTown",       EShortcut::ADVENTURE_FIRST_TOWN      },
		{"adventureFirstHero",       EShortcut::ADVENTURE_FIRST_HERO      },
		{"adventureViewScenario",    EShortcut::ADVENTURE_VIEW_SCENARIO   },
		{"adventureDigGrail",        EShortcut::ADVENTURE_DIG_GRAIL       },
		{"adventureViewPuzzle",      EShortcut::ADVENTURE_VIEW_PUZZLE     },
		{"adventureViewWorld",       EShortcut::ADVENTURE_VIEW_WORLD      },
		{"adventureViewWorld1",      EShortcut::ADVENTURE_VIEW_WORLD_X1   },
		{"adventureViewWorld2",      EShortcut::ADVENTURE_VIEW_WORLD_X2   },
		{"adventureViewWorld4",      EShortcut::ADVENTURE_VIEW_WORLD_X4   },
		{"adventureTrackHero",       EShortcut::ADVENTURE_TRACK_HERO,     },
		{"adventureToggleMapLevel",  EShortcut::ADVENTURE_TOGGLE_MAP_LEVEL},
		{"adventureKingdomOverview", EShortcut::ADVENTURE_KINGDOM_OVERVIEW},
		{"adventureQuestLog",        EShortcut::ADVENTURE_QUEST_LOG       },
		{"adventureCastSpell",       EShortcut::ADVENTURE_CAST_SPELL      },
		{"adventureThievesGuild",    EShortcut::ADVENTURE_THIEVES_GUILD   },
		{"adventureExitWorldView",   EShortcut::ADVENTURE_EXIT_WORLD_VIEW },
		{"adventureZoomIn",          EShortcut::ADVENTURE_ZOOM_IN         },
		{"adventureZoomOut",         EShortcut::ADVENTURE_ZOOM_OUT        },
		{"adventureZoomReset",       EShortcut::ADVENTURE_ZOOM_RESET      },
		{"battleToggleHeroesStats",  EShortcut::BATTLE_TOGGLE_HEROES_STATS},
		{"battleToggleQueue",        EShortcut::BATTLE_TOGGLE_QUEUE       },
		{"battleUseCreatureSpell",   EShortcut::BATTLE_USE_CREATURE_SPELL },
		{"battleSurrender",          EShortcut::BATTLE_SURRENDER          },
		{"battleRetreat",            EShortcut::BATTLE_RETREAT            },
		{"battleAutocombat",         EShortcut::BATTLE_AUTOCOMBAT         },
		{"battleAutocombatEnd",      EShortcut::BATTLE_END_WITH_AUTOCOMBAT},
		{"battleCastSpell",          EShortcut::BATTLE_CAST_SPELL         },
		{"battleWait",               EShortcut::BATTLE_WAIT               },
		{"battleDefend",             EShortcut::BATTLE_DEFEND             },
		{"battleConsoleUp",          EShortcut::BATTLE_CONSOLE_UP         },
		{"battleConsoleDown",        EShortcut::BATTLE_CONSOLE_DOWN       },
		{"battleTacticsNext",        EShortcut::BATTLE_TACTICS_NEXT       },
		{"battleTacticsEnd",         EShortcut::BATTLE_TACTICS_END        },
		{"battleSelectAction",       EShortcut::BATTLE_SELECT_ACTION      },
		{"spectateTrackHero",        EShortcut::SPECTATE_TRACK_HERO       },
		{"spectateSkipBattle",       EShortcut::SPECTATE_SKIP_BATTLE      },
		{"spectateSkipBattleResult", EShortcut::SPECTATE_SKIP_BATTLE_RESULT },
		{"townOpenTavern",           EShortcut::TOWN_OPEN_TAVERN          },
		{"townSwapArmies",           EShortcut::TOWN_SWAP_ARMIES          },
		{"recruitmentMax",           EShortcut::RECRUITMENT_MAX           },
		{"recruitmentMin",           EShortcut::RECRUITMENT_MIN           },
		{"recruitmentUpgrade",       EShortcut::RECRUITMENT_UPGRADE       },
		{"recruitmentUpgradeAll",    EShortcut::RECRUITMENT_UPGRADE_ALL   },
		{"kingdomHeroesTab",         EShortcut::KINGDOM_HEROES_TAB        },
		{"kingdomTownsTab",          EShortcut::KINGDOM_TOWNS_TAB         },
		{"heroDismiss",              EShortcut::HERO_DISMISS              },
		{"heroCommander",            EShortcut::HERO_COMMANDER            },
		{"heroLooseFormation",       EShortcut::HERO_LOOSE_FORMATION      },
		{"heroTightFormation",       EShortcut::HERO_TIGHT_FORMATION      },
		{"heroToggleTactics",        EShortcut::HERO_TOGGLE_TACTICS       },
		{"heroCostumeSave0",         EShortcut::HERO_COSTUME_SAVE_0       },
		{"heroCostumeSave1",         EShortcut::HERO_COSTUME_SAVE_1       },
		{"heroCostumeSave2",         EShortcut::HERO_COSTUME_SAVE_2       },
		{"heroCostumeSave3",         EShortcut::HERO_COSTUME_SAVE_3       },
		{"heroCostumeSave4",         EShortcut::HERO_COSTUME_SAVE_4       },
		{"heroCostumeSave5",         EShortcut::HERO_COSTUME_SAVE_5       },
		{"heroCostumeSave6",         EShortcut::HERO_COSTUME_SAVE_6       },
		{"heroCostumeSave7",         EShortcut::HERO_COSTUME_SAVE_7       },
		{"heroCostumeSave8",         EShortcut::HERO_COSTUME_SAVE_8       },
		{"heroCostumeSave9",         EShortcut::HERO_COSTUME_SAVE_9       },
		{"heroCostumeLoad0",         EShortcut::HERO_COSTUME_LOAD_0       },
		{"heroCostumeLoad1",         EShortcut::HERO_COSTUME_LOAD_1       },
		{"heroCostumeLoad2",         EShortcut::HERO_COSTUME_LOAD_2       },
		{"heroCostumeLoad3",         EShortcut::HERO_COSTUME_LOAD_3       },
		{"heroCostumeLoad4",         EShortcut::HERO_COSTUME_LOAD_4       },
		{"heroCostumeLoad5",         EShortcut::HERO_COSTUME_LOAD_5       },
		{"heroCostumeLoad6",         EShortcut::HERO_COSTUME_LOAD_6       },
		{"heroCostumeLoad7",         EShortcut::HERO_COSTUME_LOAD_7       },
		{"heroCostumeLoad8",         EShortcut::HERO_COSTUME_LOAD_8       },
		{"heroCostumeLoad9",         EShortcut::HERO_COSTUME_LOAD_9       },
		{"spellbookTabAdventure",    EShortcut::SPELLBOOK_TAB_ADVENTURE   },
		{"spellbookTabCombat",       EShortcut::SPELLBOOK_TAB_COMBAT      }
	};

	if (shortcutNames.count(identifier))
		return shortcutNames.at(identifier);
	return EShortcut::NONE;
}
