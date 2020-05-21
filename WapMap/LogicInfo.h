class LogicInfo {
    const char* logic;
    enum {
        NOT_TESTED,
        YES,
        NO
    } soundTrigger = NOT_TESTED,
      enemy = NOT_TESTED,
      humanEnemy = NOT_TESTED,
      boss = NOT_TESTED;

public:
    LogicInfo(const char* logic): logic(logic) {}

    bool IsSoundTrigger() {
        if (soundTrigger == NOT_TESTED) {
            soundTrigger = strstr(logic, "SoundTrigger") ? YES : NO;
        }
        return soundTrigger == YES;
    }

    bool IsBoss() {
        if (boss == NOT_TESTED) {
            static std::set<std::string> BossLogicSet = {
                "Raux",
                "Katherine",
                "Wolvington",
                "Gabriel",
                "Marrow",
                "Aquatis",
                "RedTail",
                "Omar",
            };
            boss = BossLogicSet.contains(logic) ? YES : NO;
            if (BossLogicSet.contains(logic)) {
                enemy = YES;
                humanEnemy = YES;
                return true;
            }
        }
        return boss == YES;
    }

    bool IsHumanEnemy() {
        if (humanEnemy == NOT_TESTED) {
            static std::set<std::string> EnemyLogicSet = {
                "Officer",
                "Soldier",
                "RobberThief",
                "CutThroat",
                "TownGuard1",
                "TownGuard2",
                "RedTailPirate",
                "BearSailor",
                "PegLeg",
                "CrazyHook",
                "Mercat",
                "Siren",
                "TigerGuard",
            };

            if (EnemyLogicSet.contains(logic)) {
                boss = NO;
                enemy = YES;
                humanEnemy = YES;

                return true;
            } else return IsBoss();
        }

        return humanEnemy == YES;
    }

    bool IsSingleCrate() {
        return !strcmp(logic, "FrontCrate") || !strcmp(logic, "BehindCrate");
    }

    bool CanDropTreasure() {
        return IsHumanEnemy() || !strcmp(logic, "Chameleon") || !strcmp(logic, "Seagull") || !strcmp(logic, "HermitCrab")
        || IsSingleCrate() || !strcmp(logic, "BackStackedCrates") || !strcmp(logic, "FrontStackedCrates")
        || !strcmp(logic, "Statue") || !strcmp(logic, "CrabNest") ;
    }

    bool IsEnemy() {
        if (enemy == NOT_TESTED) {
            if (!strcmp(logic, "Rat") || !strcmp(logic, "PunkRat") || !strcmp(logic, "HermitCrab")
            || !strcmp(logic, "Chameleon") || !strcmp(logic, "Seagull") || !strcmp(logic, "Fish")) {
                boss = NO;
                enemy = YES;
                humanEnemy = NO;

                return true;
            } else return IsHumanEnemy();
        }

        return enemy == YES;
    }

    static void GetEnemyButtonIcons(std::vector<int>& enemyButtons, int base) {
        if (base < 5 || base == 6 || base == 8) {
            enemyButtons.push_back(Icon_NPC_Rat);
        }

        if (base == 7) {
            enemyButtons.push_back(Icon_NPC_Crab);
        }

        if (base > 4 && base < 11) {
            enemyButtons.push_back(Icon_NPC_Seagull);
        }

        if (base == 1 || base == 2) {
            enemyButtons.push_back(Icon_NPC_Officer);
            enemyButtons.push_back(Icon_NPC_Soldier);
            if (base == 2) {
                enemyButtons.push_back(Icon_LaRauxe);
            }
        } else if (base == 3 || base == 4) {
            enemyButtons.push_back(Icon_NPC_RobberThief);
            enemyButtons.push_back(Icon_NPC_CutThroat);
            if (base == 4) {
                enemyButtons.push_back(Icon_Catherine);
            }
        } else if (base == 5 || base == 6) {
            enemyButtons.push_back(Icon_NPC_TownGuard1);
            enemyButtons.push_back(Icon_NPC_TownGuard2);
            if (base == 6) {
                enemyButtons.push_back(Icon_Wolvington);
            }
        } else if (base == 7 || base == 8) {
            enemyButtons.push_back(Icon_NPC_RedTailPirate);
            enemyButtons.push_back(Icon_NPC_BearSailor);
            if (base == 8) {
                enemyButtons.push_back(Icon_Gabriel);
            }
        } else if (base == 9 || base == 10) {
            enemyButtons.push_back(Icon_NPC_Pegleg);
            enemyButtons.push_back(Icon_NPC_CrazyHook);
            if (base == 10) {
                enemyButtons.push_back(Icon_Marrow);
            }
        } else if (base == 11 || base == 12) {
            enemyButtons.push_back(Icon_NPC_Fish);
            enemyButtons.push_back(Icon_NPC_Merkat);
            enemyButtons.push_back(Icon_NPC_Siren);
            if (base == 12) {
                enemyButtons.push_back(Icon_Aquatis);
            }
        } else if (base == 13) {
            enemyButtons.push_back(Icon_NPC_Chameleon);
            enemyButtons.push_back(Icon_NPC_RedTailPirate);
            enemyButtons.push_back(Icon_NPC_BearSailor);
            enemyButtons.push_back(Icon_RedTail);
        } else if (base == 14) {
            enemyButtons.push_back(Icon_NPC_Chameleon);
            enemyButtons.push_back(Icon_NPC_TigerGuard1);
            enemyButtons.push_back(Icon_NPC_TigerGuard2);
            enemyButtons.push_back(Icon_Omar);
        }
    }

#define DEFAULT_ENEMY(x, y) (std::pair<std::string,std::string>(x, y))

    static void GetEnemyLogicPairs(std::vector<std::pair<std::string, std::string>>& vstrpTypes, int base) {
        if (base < 5 || base == 6 || base == 8) {
            if (base == 2 || base == 8) {
                vstrpTypes.push_back(DEFAULT_ENEMY("PunkRat", "LEVEL_PUNKRAT"));
            } else {
                vstrpTypes.push_back(DEFAULT_ENEMY("Rat", "LEVEL_RAT"));
            }
        }

        if (base == 7) {
            vstrpTypes.push_back(DEFAULT_ENEMY("HermitCrab", "LEVEL_HERMITCRAB"));
            vstrpTypes.push_back(DEFAULT_ENEMY("HermitCrab", "LEVEL_BOMBERCRAB"));
        }

        if (base > 4 && base < 11) {
            vstrpTypes.push_back(DEFAULT_ENEMY("Seagull", "LEVEL_SEAGULL"));
        }

        if (base == 1 || base == 2) {
            vstrpTypes.push_back(DEFAULT_ENEMY("Officer", "LEVEL_OFFICER"));
            vstrpTypes.push_back(DEFAULT_ENEMY("Soldier", "LEVEL_SOLDIER"));
            if (base == 2) {
                vstrpTypes.push_back(DEFAULT_ENEMY("Raux", "LEVEL_RAUX"));
            }
        } else if (base == 3 || base == 4) {
            vstrpTypes.push_back(DEFAULT_ENEMY("RobberThief", "LEVEL_ROBBERTHIEF"));
            vstrpTypes.push_back(DEFAULT_ENEMY("CutThroat", "LEVEL_CUTTHROAT"));
            if (base == 4) {
                vstrpTypes.push_back(DEFAULT_ENEMY("Katherine", "LEVEL_KATHERINE"));
            }
        } else if (base == 5 || base == 6) {
            vstrpTypes.push_back(DEFAULT_ENEMY("TownGuard1", "LEVEL_TOWNGUARD1"));
            vstrpTypes.push_back(DEFAULT_ENEMY("TownGuard2", "LEVEL_TOWNGUARD2"));
            if (base == 6) {
                vstrpTypes.push_back(DEFAULT_ENEMY("Wolvington", "LEVEL_WOLVINGTON"));
            }
        } else if (base == 7 || base == 8) {
            vstrpTypes.push_back(DEFAULT_ENEMY("RedTailPirate", "LEVEL_REDTAILPIRATE"));
            vstrpTypes.push_back(DEFAULT_ENEMY("BearSailor", "LEVEL_BEARSAILOR"));
            if (base == 8) {
                vstrpTypes.push_back(DEFAULT_ENEMY("Gabriel", "LEVEL_GABRIEL"));
            }
        } else if (base == 9 || base == 10) {
            vstrpTypes.push_back(DEFAULT_ENEMY("PegLeg", "LEVEL_PEGLEG"));
            vstrpTypes.push_back(DEFAULT_ENEMY("CrazyHook", "LEVEL_CRAZYHOOK"));
            if (base == 10) {
                vstrpTypes.push_back(DEFAULT_ENEMY("Marrow", "LEVEL_MARROW"));
            }
        } else if (base == 11 || base == 12) {
            vstrpTypes.push_back(DEFAULT_ENEMY("Fish", "LEVEL_FISH"));
            vstrpTypes.push_back(DEFAULT_ENEMY("Mercat", "LEVEL_MERCAT"));
            vstrpTypes.push_back(DEFAULT_ENEMY("Siren", "LEVEL_SIREN"));
            if (base == 12) {
                vstrpTypes.push_back(DEFAULT_ENEMY("Aquatis", "LEVEL_KINGAQUATIS"));
            }
        } else if (base == 13) {
            vstrpTypes.push_back(DEFAULT_ENEMY("Chameleon", "LEVEL_CHAMELEON"));
            vstrpTypes.push_back(DEFAULT_ENEMY("RedTailPirate", "LEVEL_REDTAILPIRATE"));
            vstrpTypes.push_back(DEFAULT_ENEMY("BearSailor", "LEVEL_BEARSAILOR"));
            vstrpTypes.push_back(DEFAULT_ENEMY("RedTail", "LEVEL_REDTAIL"));
        } else if (base == 14) {
            vstrpTypes.push_back(DEFAULT_ENEMY("Chameleon", "LEVEL_CHAMELEON"));
            vstrpTypes.push_back(DEFAULT_ENEMY("TigerGuard", "LEVEL_TIGER"));
            vstrpTypes.push_back(DEFAULT_ENEMY("TigerGuard", "LEVEL_TIGERWHITE"));
            vstrpTypes.push_back(DEFAULT_ENEMY("Omar", "LEVEL_OMAR"));
        }
    }
};