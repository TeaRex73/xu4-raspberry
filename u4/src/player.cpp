/*
 * $Id$
 */

#include "vc6.h" // Fixes things if you're using VC6, does nothing if otherwise

#include "player.h"

#include "annotation.h"
#include "armor.h"
#include "combat.h"
#include "context.h"
#include "debug.h"
#include "game.h"
#include "location.h"
#include "screen.h" /* FIXME: remove dependency on this */
#include "tileset.h"
#include "types.h"
#include "utils.h"
#include "weapon.h"

LostEighthCallback lostEighthCallback = NULL;
AdvanceLevelCallback advanceLevelCallback = NULL;
PartyStarvingCallback partyStarvingCallback = NULL;

/**
 * Sets up a callback to handle player losing an eighth of his or her
 * avatarhood.
 */
void playerSetLostEighthCallback(LostEighthCallback callback) {
    lostEighthCallback = callback;
}

void playerSetAdvanceLevelCallback(AdvanceLevelCallback callback) {
    advanceLevelCallback = callback;
}

void playerSetPartyStarvingCallback(PartyStarvingCallback callback) {
    partyStarvingCallback = callback;
}

bool isPartyMember(Object *punknown) {
    PartyMember *pm;
    if ((pm = dynamic_cast<PartyMember*>(punknown)) != NULL)
        return true;
    else 
        return false;
}

/**
 * PartyMember class implementation
 */ 
PartyMember::PartyMember(class Party *p, SaveGamePlayerRecord *pr) : 
    Creature(MapTile::tileForClass(pr->klass)),    
    player(pr),
    party(p)
{
    /* FIXME: we need to rename movement behaviors */
    setMovementBehavior(MOVEMENT_ATTACK_AVATAR);
    this->ranged = Weapon::get(pr->weapon)->getRange() ? 1 : 0;
    setStatus(pr->status);
}

/**
 * Notify the party that this player has changed somehow
 */
void PartyMember::notifyOfChange(string arg) {
    if (party) {
        party->setChanged();
        party->notifyObservers(arg);
    }
}

int PartyMember::getHp() const      { return player->hp; }
int PartyMember::getMaxHp() const   { return player->hpMax; }
int PartyMember::getExp() const     { return player->xp; }
int PartyMember::getStr() const     { return player->str; }
int PartyMember::getDex() const     { return player->dex; }
int PartyMember::getInt() const     { return player->intel; }
int PartyMember::getMp() const      { return player->mp; }

/**
 * Determine the most magic points a character could have
 * given his class and intelligence.
 */
int PartyMember::getMaxMp() const {
    int max_mp = -1;

    switch (player->klass) {
    case CLASS_MAGE:            /*  mage: 200% of int */
        max_mp = player->intel * 2;
        break;

    case CLASS_DRUID:           /* druid: 150% of int */
        max_mp = player->intel * 3 / 2;
        break;

    case CLASS_BARD:            /* bard, paladin, ranger: 100% of int */
    case CLASS_PALADIN:
    case CLASS_RANGER:
        max_mp = player->intel;
        break;

    case CLASS_TINKER:          /* tinker: 50% of int */
        max_mp = player->intel / 2;
        break;

    case CLASS_FIGHTER:         /* fighter, shepherd: no mp at all */
    case CLASS_SHEPHERD:
        max_mp = 0;
        break;

    default:
        ASSERT(0, "invalid player class: %d", player->klass);
    }

    /* mp always maxes out at 99 */
    if (max_mp > 99)
        max_mp = 99;

    return max_mp;
}

WeaponType PartyMember::getWeapon() const   { return player->weapon; }
ArmorType PartyMember::getArmor() const     { return player->armor; }
string PartyMember::getName() const         { return player->name; }
SexType PartyMember::getSex() const         { return player->sex; }
ClassType PartyMember::getClass() const     { return player->klass; }
StatusType PartyMember::getStatus() const   { return status.back(); }

/**
 * Determine what level a character has.
 */
int PartyMember::getRealLevel() const {
    return player->hpMax / 100;
}

/**
 * Determine the highest level a character could have with the number
 * of experience points he has.
 */
int PartyMember::getMaxLevel() const {
    int level = 1;
    int next = 100;

    while (player->xp >= next && level < 8) {
        level++;
        next <<= 1;
    }

    return level;
}

/**
 * Adds a status effect to the player
 */ 
void PartyMember::addStatus(StatusType s) {
    Creature::addStatus(s);
    player->status = status.back();
    notifyOfChange("PartyMember::addStatus");
}

/**
 * Adjusts the player's mp by 'pts'
 */
void PartyMember::adjustMp(int pts) {
    AdjustValueMax(player->mp, pts, getMaxMp());
    notifyOfChange("PartyMember::adjustMp");
}

/**
 * Advances the player to the next level if they have enough experience
 */
void PartyMember::advanceLevel() {
    if (getRealLevel() == getMaxLevel())
        return;
    setStatus(STAT_GOOD);    
    player->hpMax = getMaxLevel() * 100;
    player->hp = player->hpMax;

    /* improve stats by 1-8 each */
    player->str   += xu4_random(8) + 1;
    player->dex   += xu4_random(8) + 1;
    player->intel += xu4_random(8) + 1;

    if (player->str > 50) player->str = 50;
    if (player->dex > 50) player->dex = 50;
    if (player->intel > 50) player->intel = 50;

    if (advanceLevelCallback)
        (*advanceLevelCallback)(this);

    notifyOfChange("PartyMember::advanceLevel");
}

/**
 * Apply an effect to the party member
 */
void PartyMember::applyEffect(TileEffect effect) {
    if (getStatus() == STAT_DEAD)
        return;

    switch (effect) {
    case EFFECT_NONE:
        break;
    case EFFECT_LAVA:
    case EFFECT_FIRE:
        applyDamage(16 + (xu4_random(32)));

        /*else if (player == ALL_PLAYERS && xu4_random(2) == 0)
            playerApplyDamage(&(c->saveGame->players[i]), 10 + (xu4_random(25)));*/
        break;
    case EFFECT_SLEEP:        
        putToSleep();
        break;
    case EFFECT_POISONFIELD:
    case EFFECT_POISON:
        addStatus(STAT_POISONED);
        break;
    case EFFECT_ELECTRICITY: break;
    default:
        ASSERT(0, "invalid effect: %d", effect);
    }

    if (party && effect != EFFECT_NONE) {
        party->setChanged();
        party->notifyObservers("PartyMember::applyEffect");
    }
}

/**
 * Award a player experience points.  Maxs out the players xp at 9999.
 */
void PartyMember::awardXp(int xp) {
    AdjustValueMax(player->xp, xp, 9999);
    notifyOfChange("PartyMember::awardXp");
}

/**
 * Perform a certain type of healing on the party member
 */
bool PartyMember::heal(HealType type) {
    switch(type) {

    case HT_NONE:
        return true;

    case HT_CURE:
        if (getStatus() != STAT_POISONED)
            return false;
        removeStatus(STAT_POISONED);
        break;

    case HT_FULLHEAL:
        if (getStatus() == STAT_DEAD ||
            player->hp == player->hpMax)
            return false;
        player->hp = player->hpMax;
        break;

    case HT_RESURRECT:
        if (getStatus() != STAT_DEAD)
            return false;
        setStatus(STAT_GOOD);
        break;

    case HT_HEAL:
        if (getStatus() == STAT_DEAD ||
            player->hp == player->hpMax)
            return false;        

        player->hp += 75 + (xu4_random(0x100) % 0x19);
        break;

    case HT_CAMPHEAL:
        if (getStatus() == STAT_DEAD ||
            player->hp == player->hpMax)
            return false;        
        player->hp += 99 + (xu4_random(0x100) & 0x77);
        break;

    case HT_INNHEAL:
        if (getStatus() == STAT_DEAD ||
            player->hp == player->hpMax)
            return false;        
        player->hp += 100 + (xu4_random(50) * 2);
        break;

    default:
        return false;
    }

    if (player->hp > player->hpMax)
        player->hp = player->hpMax;
    
    notifyOfChange("PartyMember::heal");
    
    return true;
}

/**
 * Remove status effects from the party member
 */
void PartyMember::removeStatus(StatusType s) {
    Creature::removeStatus(s);
    player->status = status.back();
    notifyOfChange("PartyMember::removeStatus");
}

void PartyMember::setHp(int hp) {
    player->hp = hp;
    notifyOfChange("PartyMember::setHp");
}

void PartyMember::setMp(int mp) {
    player->mp = mp;
    notifyOfChange("PartyMember::setMp");
}

void PartyMember::setArmor(ArmorType a) {
    player->armor = a;
    notifyOfChange("PartyMember::setArmor");
}

void PartyMember::setWeapon(WeaponType w) {
    player->weapon = w;
    notifyOfChange("PartyMember::setWeapon");
}

/**
 * Applies damage to a player, and changes status to dead if hit
 * points drop to zero or below.
 */
bool PartyMember::applyDamage(int damage) {
    int newHp = player->hp;

    if (getStatus() == STAT_DEAD)
        return false;

    newHp -= damage;

    if (newHp < 0) {
        setStatus(STAT_DEAD);
        newHp = 0;
    }
    
    player->hp = newHp;
    notifyOfChange("PartyMember::applyDamage");

    if (isCombatMap(c->location->map) && getStatus() == STAT_DEAD) {
        Coords p = getCoords();                    
        Map *map = getMap();

        screenMessage("%s is Killed!\n", getName().c_str());
        map->annotations->add(p, Tileset::findTileByName("corpse")->id)->setTTL(party->size());

        /* remove yourself from the map */
        remove();        
        return false;
    }

    return true;
}

/**
 * Determine whether a player's attack hits or not.
 */
bool PartyMember::attackHit(Creature *m) {
    if (Weapon::get(player->weapon)->alwaysHits() || player->dex >= 40)
        return true;

    return(m->isHit(player->dex));
}

bool PartyMember::dealDamage(Creature *m, int damage) {
    /* we have to record these now, because if we
       kill the target, it gets destroyed */
    bool isEvil = m->isEvil();
    int m_xp = m->xp;

    if (!Creature::dealDamage(m, damage)) {
        /* half the time you kill an evil creature you get a karma boost */
        if (isEvil && xu4_random(2) == 0)
            c->party->adjustKarma(KA_KILLED_EVIL);
        awardXp(m_xp);
        return false;
    }
    return true;
}

/**
 * Calculate damage for an attack.
 */
int PartyMember::getDamage() {
    int maxDamage;

    maxDamage = Weapon::get(player->weapon)->getDamage();
    maxDamage += player->str;
    if (maxDamage > 255)
        maxDamage = 255;

    return xu4_random(maxDamage);
}

/**
 * Returns the tile that will be displayed when the party
 * member's attack hits
 */
MapTile PartyMember::getHitTile() const {
    return Weapon::get(getWeapon())->getHitTile();
}

/**
 * Returns the tile that will be displayed when the party
 * member's attack fails
 */
MapTile PartyMember::getMissTile() const {
    return Weapon::get(getWeapon())->getMissTile();
}

/**
 * Determine whether a player is hit by a melee attack.
 */
bool PartyMember::isHit(int hit_offset) {
    return xu4_random(0x100) + hit_offset > Armor::get(player->armor)->getDefense();
}

bool PartyMember::isDead() {
    return getStatus() == STAT_DEAD;
}

bool PartyMember::isDisabled() {
    return (getStatus() == STAT_GOOD ||
        getStatus() == STAT_POISONED) ? false : true;
}

/**
 * Lose the equipped weapon for the player (flaming oil, ranged daggers, etc.)
 * Returns the number of weapons left of that type, including the one in
 * the players hand
 */
int PartyMember::loseWeapon() {
    int weapon = player->weapon;
    
    notifyOfChange("PartyMember::loseWeapon");

    if (party->saveGame->weapons[weapon] > 0)
        return (--party->saveGame->weapons[weapon]) + 1;
    else {
        player->weapon = WEAP_HANDS;
        return 0;
    }
}

/**
 * Put the party member to sleep
 */
void PartyMember::putToSleep() {    
    if (getStatus() != STAT_DEAD) {
        addStatus(STAT_SLEEPING);
        setTile(Tileset::findTileByName("corpse")->id);
    }
}

/**
 * Wakes up the party member
 */
void PartyMember::wakeUp() {
    removeStatus(STAT_SLEEPING);    
    setTile(MapTile::tileForClass(getClass()));
}


/**
 * Party class implementation
 */ 
Party::Party(SaveGame *s) : saveGame(s), torchduration(0) {
    for (int i = 0; i < saveGame->members; i++) {
        // add the members to the party
        members.push_back(new PartyMember(this, &saveGame->players[i]));
    }    
}

void Party::adjustFood(int food) {
    int oldFood = saveGame->food;    
    AdjustValue(saveGame->food, food, 999900, 0);
    if ((saveGame->food / 100) != (oldFood / 100)) {
        setChanged();
        notifyObservers("Party::adjustFood");
    }
}

void Party::adjustGold(int gold) {
    AdjustValue(saveGame->gold, gold, 9999, 0);    
    setChanged();
    notifyObservers("Party::adjustGold");
}

/**
 * Adjusts the avatar's karma level for the given action.  Activate
 * the lost eighth callback if the player has lost avatarhood.
 */
void Party::adjustKarma(KarmaAction action) {
    int timeLimited = 0;
    int v, newKarma[VIRT_MAX], maxVal[VIRT_MAX];

    /*
     * make a local copy of all virtues, and adjust it according to
     * the game rules
     */
    for (v = 0; v < VIRT_MAX; v++) {
        newKarma[v] = saveGame->karma[v] == 0 ? 100 : saveGame->karma[v];
        maxVal[v] = saveGame->karma[v] == 0 ? 100 : 99;
    }

    switch (action) {
    case KA_FOUND_ITEM:
        AdjustValueMax(newKarma[VIRT_HONOR], 5, maxVal[VIRT_HONOR]);
        break;
    case KA_STOLE_CHEST:
        AdjustValueMin(newKarma[VIRT_HONESTY], -1, 1);
        AdjustValueMin(newKarma[VIRT_JUSTICE], -1, 1);
        AdjustValueMin(newKarma[VIRT_HONOR], -1, 1);
        break;
    case KA_GAVE_TO_BEGGAR:
        timeLimited = 1;
        AdjustValueMax(newKarma[VIRT_COMPASSION], 2, maxVal[VIRT_COMPASSION]);
        AdjustValueMax(newKarma[VIRT_HONOR], 3, maxVal[VIRT_HONOR]); /* FIXME: verify if honor goes up */
        break;
    case KA_BRAGGED:
        AdjustValueMin(newKarma[VIRT_HUMILITY], -5, 1);
        break;
    case KA_HUMBLE:
        timeLimited = 1;
        AdjustValueMax(newKarma[VIRT_HUMILITY], 10, maxVal[VIRT_HUMILITY]);
        break;
    case KA_HAWKWIND:
    case KA_MEDITATION:
        AdjustValueMax(newKarma[VIRT_SPIRITUALITY], 3, maxVal[VIRT_SPIRITUALITY]);
        break;
    case KA_BAD_MANTRA:
        AdjustValueMin(newKarma[VIRT_SPIRITUALITY], -3, 1);
        break;
    case KA_ATTACKED_GOOD:
        AdjustValueMin(newKarma[VIRT_COMPASSION], -5, 1);
        AdjustValueMin(newKarma[VIRT_JUSTICE], -5, 1);
        AdjustValueMin(newKarma[VIRT_HONOR], -5, 1);
        break;
    case KA_FLED_EVIL:
        AdjustValueMin(newKarma[VIRT_VALOR], -2, 1);
        break;
    case KA_HEALTHY_FLED_EVIL:
        AdjustValueMin(newKarma[VIRT_VALOR], -2, 1);
        AdjustValueMin(newKarma[VIRT_SACRIFICE], -2, 1);
        break;
    case KA_KILLED_EVIL:
        AdjustValueMax(newKarma[VIRT_VALOR], xu4_random(2), maxVal[VIRT_VALOR]); /* gain one valor half the time, zero the rest */
        break;
    case KA_SPARED_GOOD:        
        AdjustValueMax(newKarma[VIRT_COMPASSION], 1, maxVal[VIRT_COMPASSION]);
        AdjustValueMax(newKarma[VIRT_JUSTICE], 1, maxVal[VIRT_JUSTICE]);
        break;    
    case KA_DONATED_BLOOD:
        AdjustValueMax(newKarma[VIRT_SACRIFICE], 5, maxVal[VIRT_SACRIFICE]);
        break;
    case KA_DIDNT_DONATE_BLOOD:
        AdjustValueMin(newKarma[VIRT_SACRIFICE], -5, 1);
        break;
    case KA_CHEAT_REAGENTS:
        AdjustValueMin(newKarma[VIRT_HONESTY], -10, 1);
        AdjustValueMin(newKarma[VIRT_JUSTICE], -10, 1);
        AdjustValueMin(newKarma[VIRT_HONOR], -10, 1);
        break;
    case KA_DIDNT_CHEAT_REAGENTS:
        timeLimited = 1;
        AdjustValueMax(newKarma[VIRT_HONESTY], 2, maxVal[VIRT_HONESTY]);
        AdjustValueMax(newKarma[VIRT_JUSTICE], 2, maxVal[VIRT_JUSTICE]);
        AdjustValueMax(newKarma[VIRT_HONOR], 2, maxVal[VIRT_HONOR]);
        break;
    case KA_USED_SKULL:
        /* using the skull is very, very bad... */
        for (v = 0; v < VIRT_MAX; v++)
            AdjustValueMin(newKarma[v], -5, -1);
        break;
    case KA_DESTROYED_SKULL:
        /* ...but destroying it is very, very good */
        for (v = 0; v < VIRT_MAX; v++)
            AdjustValueMax(newKarma[v], 10, maxVal[v]);
        break;
    }

    /*
     * check if enough time has passed since last virtue award if
     * action is time limited -- if not, throw away new values
     */
    if (timeLimited) {
        if (((saveGame->moves / 16) >= 0x10000) || (((saveGame->moves / 16) & 0xFFFF) != saveGame->lastvirtue))
            saveGame->lastvirtue = (saveGame->moves / 16) & 0xFFFF;
        else
            return;
    }

    /* something changed */
    setChanged();
    notifyObservers("Party::adjustKarma");

    /*
     * return to u4dos compatibility and handle losing of eighths
     */
    for (v = 0; v < VIRT_MAX; v++) {
        if (maxVal[v] == 100) { /* already an avatar */
            if (newKarma[v] < 100) { /* but lost it */
                if (lostEighthCallback)
                    (*lostEighthCallback)((Virtue)v);
                saveGame->karma[v] = newKarma[v];
            }
            else saveGame->karma[v] = 0; /* return to u4dos compatibility */
        }
        else saveGame->karma[v] = newKarma[v];
    }
}

/**
 * Apply effects to the entire party
 */
void Party::applyEffect(TileEffect effect) {
    int i;

    for (i = 0; i < size(); i++) {
        switch(effect) {
        case EFFECT_NONE:
        case EFFECT_ELECTRICITY:
            members[i]->applyEffect(effect);
        case EFFECT_LAVA:
        case EFFECT_FIRE:        
        case EFFECT_SLEEP:
            if (xu4_random(2) == 0)
                members[i]->applyEffect(effect);
        case EFFECT_POISONFIELD:
        case EFFECT_POISON:
            if (xu4_random(5) == 0)
                members[i]->applyEffect(effect);
        }        
    }
}

/**
 * Attempt to elevate in the given virtue
 */
bool Party::attemptElevation(Virtue virtue) {
    if (saveGame->karma[virtue] == 99) {
        saveGame->karma[virtue] = 0;
        setChanged();
        notifyObservers("Party::attemptElevation");
        return true;
    } else
        return false;
}

/**
 * Burns a torch's duration down a certain number of turns
 */
void Party::burnTorch(int turns) {
    torchduration -= turns;
    if (torchduration <= 0)
        torchduration = 0;

    saveGame->torchduration = torchduration;

    setChanged();
    notifyObservers("Party::burnTorch");
}

/**
 * Returns true if the party can enter the shrine
 */
bool Party::canEnterShrine(Virtue virtue) {
    if (saveGame->runes & (1 << (int) virtue))
        return true;
    else
        return false;
}

/**
 * Returns true if the person can join the party
 */
bool Party::canPersonJoin(string name, Virtue *v) {
    int i;

    if (name.empty())
        return 0;    

    for (i = 1; i < 8; i++) {
        if (name == saveGame->players[i].name) {
            if (v)
                *v = (Virtue) saveGame->players[i].klass;
            return true;
        }
    }
    return false;
}

/**
 * Damages the party's ship
 */
void Party::damageShip(unsigned int pts) {
    saveGame->shiphull -= pts;
    if ((short)saveGame->shiphull < 0)
        saveGame->shiphull = 0;
    
    setChanged();
    notifyObservers("Party::damageShip");
}

/**
 * Donates 'quantity' gold. Returns true if the donation succeeded,
 * or false if there was not enough gold to make the donation
 */
bool Party::donate(int quantity) {
    if (quantity > saveGame->gold)
        return false;

    adjustGold(-quantity);
    adjustKarma(KA_GAVE_TO_BEGGAR);

    return true;
}

/**
 * Ends the party's turn
 */
void Party::endTurn() {
    int i;    
    
    saveGame->moves++;   
   
    for (i = 0; i < size(); i++) {

        /* Handle player status (only for non-combat turns) */
        if ((c->location->context & CTX_NON_COMBAT) == c->location->context) {
            
            /* party members eat food (also non-combat) */
            if (!members[i]->isDead())
                adjustFood(-1);

            switch (members[i]->getStatus()) {
            case STAT_SLEEPING:
                if (xu4_random(5) == 0)
                    members[i]->wakeUp();                    
                break;

            case STAT_POISONED:            
                members[i]->applyDamage(2);
                break;

            default:
                break;
            }
        }

        /* regenerate magic points */
        if (!members[i]->isDisabled() && members[i]->getMp() < members[i]->getMaxMp())
            saveGame->players[i].mp++;
    }

    /* The party is starving! */
    if ((saveGame->food == 0) && ((c->location->context & CTX_NON_COMBAT) == c->location->context))
        (*partyStarvingCallback)();
    
    /* heal ship (25% chance it is healed each turn) */
    if ((c->location->context == CTX_WORLDMAP) && (saveGame->shiphull < 50) && xu4_random(4) == 0)
        healShip(1);
}

/**
 * Adds a chest worth of gold to the party's inventory
 */
int Party::getChest() {
    int gold = xu4_random(50) + xu4_random(8) + 10;
    adjustGold(gold);    

    return gold;
}

/**
 * Returns the number of turns a currently lit torch will last (or 0 if no torch lit)
 */
int Party::getTorchDuration() const {
    return torchduration;
}

/**
 * Heals the ship's hull strength by 'pts' points
 */
void Party::healShip(unsigned int pts) {
    saveGame->shiphull += pts;
    if (saveGame->shiphull > 50)
        saveGame->shiphull = 50;

    setChanged();
    notifyObservers("Party::healShip");
}

/**
 * Whether or not the party can make an action.
 */
bool Party::isImmobilized() {
    int i;
    bool immobile = true;

    for (i = 0; i < saveGame->members; i++) {
        if (!members[i]->isDisabled())
            immobile = false;
    }

    return immobile;
}

/**
 * Whether or not all the party members are dead.
 */
bool Party::isDead() {
    int i;
    bool dead = true;

    for (i = 0; i < saveGame->members; i++) {
        if (!members[i]->isDead()) {
            dead = false;
        }
    }

    return dead;
}

/**
 * Returns true if the person with that name
 * is already in the party
 */
bool Party::isPersonJoined(string name) {
    int i;

    if (name.empty())
        return false;

    for (i = 1; i < saveGame->members; i++) {
        if (name == saveGame->players[i].name)
            return true;
    }
    return false;
}

/**
 * Attempts to add the person to the party.
 * Returns JOIN_SUCCEEDED if successful.
 */
CannotJoinError Party::join(string name) {
    int i;
    SaveGamePlayerRecord tmp;

    for (i = saveGame->members; i < 8; i++) {
        if (name == saveGame->players[i].name) {

            /* ensure avatar is experienced enough */
            if (saveGame->members + 1 > (saveGame->players[0].hpMax / 100))
                return JOIN_NOT_EXPERIENCED;

            /* ensure character has enough karma */
            if ((saveGame->karma[saveGame->players[i].klass] > 0) &&
                (saveGame->karma[saveGame->players[i].klass] < 40))
                return JOIN_NOT_VIRTUOUS;

            tmp = saveGame->players[saveGame->members];
            saveGame->players[saveGame->members] = saveGame->players[i];
            saveGame->players[i] = tmp;            
            setChanged();
            notifyObservers("Party::join");

            members.push_back(new PartyMember(this, &saveGame->players[saveGame->members++]));

            return JOIN_SUCCEEDED;
        }
    }

    return JOIN_NOT_EXPERIENCED;
}

/**
 * Lights a torch with a default duration of 100
 */
void Party::lightTorch(int duration, bool loseTorch) {
    if (loseTorch) { 
        if (!c->saveGame->torches) {
            screenMessage("None left!\n");
            return;
        }
        else c->saveGame->torches--;
    }
    
    torchduration += duration;
    saveGame->torchduration = torchduration;

    setChanged();
    notifyObservers("Party::lightTorch");
}

/**
 * Extinguishes a torch
 */
void Party::quenchTorch() {
    torchduration = saveGame->torchduration = 0;

    setChanged();
    notifyObservers("Party::quenchTorch");
}

/**
 * Revives the party after the entire party has been killed
 */
void Party::reviveParty() {
    int i;

    for (i = 0; i < size(); i++) {
        members[i]->setStatus(STAT_GOOD);
        saveGame->players[i].hp = saveGame->players[i].hpMax;
    }
    
    saveGame->food = 20099;
    saveGame->gold = 200;
    setTransport(Tileset::findTileByName("avatar")->id);
    setChanged();
    notifyObservers("Party::reviveParty");
}

void Party::setTransport(MapTile tile) {
    saveGame->transport = tile.getIndex();
    transport = tile;
    
    if (tile.isHorse())
        c->transportContext = TRANSPORT_HORSE;
    else if (tile.isShip())
        c->transportContext = TRANSPORT_SHIP;
    else if (tile.isBalloon())
        c->transportContext = TRANSPORT_BALLOON;
    else c->transportContext = TRANSPORT_FOOT;

    setChanged();
    notifyObservers("Party::setTransport");
}

void Party::setShipHull(int str) {
    int newStr = str;
    AdjustValue(newStr, 0, 99, 0);

    if (saveGame->shiphull != newStr) {
        saveGame->shiphull = newStr;
        setChanged();
        notifyObservers("Party::setShipHull");
    }
}

void Party::adjustReagent(int reagent, int amt) {
    int oldVal = c->saveGame->reagents[reagent];
    AdjustValue(c->saveGame->reagents[reagent], amt, 99, 0);

    if (oldVal != c->saveGame->reagents[reagent]) {        
        setChanged();
        notifyObservers("Party::adjustReagent");
    }
}

int Party::reagents(int reagent) const {
    return c->saveGame->reagents[reagent];
}

/**
 * Returns the size of the party
 */
int Party::size() const {
    return members.size();
}

/**
 * Returns a pointer to the party member indicated
 */
PartyMember *Party::member(int index) const {
    return members[index];
}
