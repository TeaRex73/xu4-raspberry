/*
 * $Id$
 */

#include "vc6.h" // Fixes things if you're using VC6, does nothing if otherwise

#include "savegame.h"

#include "io.h"
#include "object.h"
#include "types.h"

char *partySavFilename() {
    char *fname;
    
#if defined(MACOSX)
    char *home;

    home = getenv("HOME");
    if (home && home[0]) {
        fname = new char[strlen(home) + strlen(MACOSX_USER_FILES_PATH) + strlen(PARTY_SAV_BASE_FILENAME) + 2];
        strcpy(fname, home);
        strcat(fname, MACOSX_USER_FILES_PATH);
        strcat(fname, "/");
        strcat(fname, PARTY_SAV_BASE_FILENAME);
    } else
        fname = strdup(PARTY_SAV_BASE_FILENAME);
#else
    fname = strdup(PARTY_SAV_BASE_FILENAME);
#endif
    
    return fname;
}

char *monstersSavFilename(const char *base) {
    char *fname;
    
#if defined(MACOSX)
    char *home;

    home = getenv("HOME");
    if (home && home[0]) {
        fname = new char[strlen(home) + strlen(MACOSX_USER_FILES_PATH) + strlen(base) + 2];
        strcpy(fname, home);
        strcat(fname, MACOSX_USER_FILES_PATH);
        strcat(fname, "/");
        strcat(fname, base);
    } else
        fname = strdup(base);
#else
    fname = strdup(base);
#endif
    
    return fname;
}
    
FILE *saveGameOpenForWriting() {
    char *fname;
    FILE *f;
    
    fname = partySavFilename();
    f = fopen(fname, "wb");
    delete fname;

    return f;
}

FILE *saveGameOpenForReading() {
    char *fname;
    FILE *f;
    
    fname = partySavFilename();
    f = fopen(fname, "rb");
    delete fname;

    return f;
}

FILE *saveGameMonstersOpenForWriting(const char *filename) {
    char *fname;
    FILE *f;
    
    fname = monstersSavFilename(filename);
    f = fopen(fname, "wb");
    delete fname;

    return f;
}

FILE *saveGameMonstersOpenForReading(const char *filename) {
    char *fname;
    FILE *f;
    
    fname = monstersSavFilename(filename);
    f = fopen(fname, "rb");
    delete fname;

    return f;
}

int SaveGame::write(FILE *f) const {
    int i;

    if (!writeInt(unknown1, f) ||
        !writeInt(moves, f))
        return 0;

    for (i = 0; i < 8; i++) {
        if (!players[i].write(f))
            return 0;
    }

    if (!writeInt(food, f) ||
        !writeShort(gold, f))
        return 0;

    for (i = 0; i < 8; i++) {
        if (!writeShort(karma[i], f))
            return 0;
    }

    if (!writeShort(torches, f) ||
        !writeShort(gems, f) ||
        !writeShort(keys, f) ||
        !writeShort(sextants, f))
        return 0;

    for (i = 0; i < ARMR_MAX; i++) {
        if (!writeShort(armor[i], f))
            return 0;
    }

    for (i = 0; i < WEAP_MAX; i++) {
        if (!writeShort(weapons[i], f))
            return 0;
    }

    for (i = 0; i < REAG_MAX; i++) {
        if (!writeShort(reagents[i], f))
            return 0;
    }

    for (i = 0; i < SPELL_MAX; i++) {
        if (!writeShort(mixtures[i], f))
            return 0;
    }

    if (!writeShort(items, f) ||
        !writeChar(x, f) ||
        !writeChar(y, f) ||
        !writeChar(stones, f) ||
        !writeChar(runes, f) ||
        !writeShort(members, f) ||
        !writeShort(transport, f) ||
        !writeShort(balloonstate, f) ||
        !writeShort(trammelphase, f) ||
        !writeShort(feluccaphase, f) ||
        !writeShort(shiphull, f) ||
        !writeShort(lbintro, f) ||
        !writeShort(lastcamp, f) ||
        !writeShort(lastreagent, f) ||
        !writeShort(lastmeditation, f) ||
        !writeShort(lastvirtue, f) ||
        !writeChar(dngx, f) ||
        !writeChar(dngy, f) ||
        !writeShort(orientation, f) ||
        !writeShort(dnglevel, f) ||
        !writeShort(location, f))
        return 0;

    return 1;
}

int SaveGame::read(FILE *f) {
    int i;

    if (!readInt(&unknown1, f) ||
        !readInt(&moves, f))
        return 0;

    for (i = 0; i < 8; i++) {
        if (!players[i].read(f))
            return 0;
    }

    if (!readInt((unsigned int*)&food, f) ||
        !readShort((unsigned short*)&gold, f))
        return 0;

    for (i = 0; i < 8; i++) {
        if (!readShort((unsigned short*)&(karma[i]), f))
            return 0;
    }

    if (!readShort((unsigned short*)&torches, f) ||
        !readShort((unsigned short*)&gems, f) ||
        !readShort((unsigned short*)&keys, f) ||
        !readShort((unsigned short*)&sextants, f))
        return 0;

    for (i = 0; i < ARMR_MAX; i++) {
        if (!readShort((unsigned short*)&(armor[i]), f))
            return 0;
    }

    for (i = 0; i < WEAP_MAX; i++) {
        if (!readShort((unsigned short*)&(weapons[i]), f))
            return 0;
    }

    for (i = 0; i < REAG_MAX; i++) {
        if (!readShort((unsigned short*)&(reagents[i]), f))
            return 0;
    }

    for (i = 0; i < SPELL_MAX; i++) {
        if (!readShort((unsigned short*)&(mixtures[i]), f))
            return 0;
    }

    if (!readShort(&items, f) ||
        !readChar(&x, f) ||
        !readChar(&y, f) ||
        !readChar(&stones, f) ||
        !readChar(&runes, f) ||
        !readShort(&members, f) ||
        !readShort(&transport, f) ||
        !readShort(&balloonstate, f) ||
        !readShort(&trammelphase, f) ||
        !readShort(&feluccaphase, f) ||
        !readShort(&shiphull, f) ||
        !readShort(&lbintro, f) ||
        !readShort(&lastcamp, f) ||
        !readShort(&lastreagent, f) ||
        !readShort(&lastmeditation, f) ||
        !readShort(&lastvirtue, f) ||
        !readChar(&dngx, f) ||
        !readChar(&dngy, f) ||
        !readShort(&orientation, f) ||
        !readShort(&dnglevel, f) ||
        !readShort(&location, f))
        return 0;

    /* workaround of U4DOS bug to retain savegame compatibility */
    if (location == 0 && dnglevel == 0)
        dnglevel = 0xFFFF;

    return 1;
}

void SaveGame::init(const SaveGamePlayerRecord *avatarInfo) {
    int i;

    unknown1 = 0;
    moves = 0;

    memcpy(&(players[0]), avatarInfo, sizeof(SaveGamePlayerRecord));
    for (i = 1; i < 8; i++)
        players[i].init();

    food = 0;
    gold = 0;

    for (i = 0; i < 8; i++)
        karma[i] = 20;

    torches = 0;
    gems = 0;
    keys = 0;
    sextants = 0;

    for (i = 0; i < ARMR_MAX; i++)
        armor[i] = 0;

    for (i = 0; i < WEAP_MAX; i++)
        weapons[i] = 0;

    for (i = 0; i < REAG_MAX; i++)
        reagents[i] = 0;

    for (i = 0; i < SPELL_MAX; i++)
        mixtures[i] = 0;

    items = 0;
    x = 0;
    y = 0;
    stones = 0;
    runes = 0;
    members = 1;
    transport = 0x1f;
    balloonstate = 0;
    trammelphase = 0;
    feluccaphase = 0;
    shiphull = 50;
    lbintro = 0;
    lastcamp = 0;
    lastreagent = 0;
    lastmeditation = 0;
    lastvirtue = 0;
    dngx = 0;
    dngy = 0;
    orientation = 0;
    dnglevel = 0xFFFF;
    location = 0;
}

int SaveGamePlayerRecord::write(FILE *f) const {
    int i;

    if (!writeShort(hp, f) ||
        !writeShort(hpMax, f) ||
        !writeShort(xp, f) ||
        !writeShort(str, f) ||
        !writeShort(dex, f) ||
        !writeShort(intel, f) ||
        !writeShort(mp, f) ||
        !writeShort(unknown, f) ||
        !writeShort((unsigned short)weapon, f) ||
        !writeShort((unsigned short)armor, f))
        return 0;

    for (i = 0; i < 16; i++) {
        if (!writeChar(name[i], f))
            return 0;
    }

    if (!writeChar((unsigned char)sex, f) ||
        !writeChar((unsigned char)klass, f) ||
        !writeChar((unsigned char)status, f))
        return 0;

    return 1;
}

int SaveGamePlayerRecord::read(FILE *f) {
    int i;
    unsigned char ch;
    unsigned short s;

    if (!readShort(&hp, f) ||
        !readShort(&hpMax, f) ||
        !readShort(&xp, f) ||
        !readShort(&str, f) ||
        !readShort(&dex, f) ||
        !readShort(&intel, f) ||
        !readShort(&mp, f) ||
        !readShort(&unknown, f))
        return 0;
        
    if (!readShort(&s, f))
        return 0;
    weapon = (WeaponType) s;
    if (!readShort(&s, f))
        return 0;
    armor = (ArmorType) s;

    for (i = 0; i < 16; i++) {
        if (!readChar((unsigned char *) &(name[i]), f))
            return 0;
    }

    if (!readChar(&ch, f))
        return 0;
    sex = (SexType) ch;
    if (!readChar(&ch, f))
      return 0;
    klass = (ClassType) ch;
    if (!readChar(&ch, f))
        return 0;
    status = (StatusType) ch;

    return 1;
}

void SaveGamePlayerRecord::init() {
    int i;

    hp = 0;
    hpMax = 0;
    xp = 0;
    str = 0;
    dex = 0;
    intel = 0;
    mp = 0;
    unknown = 0;
    weapon = WEAP_HANDS;
    armor = ARMR_NONE;

    for (i = 0; i < 16; i++)
      name[i] = '\0';

    sex = SEX_MALE;
    klass = CLASS_MAGE;
    status = STAT_GOOD;
}

int SaveGameMonsterRecord::read(FILE *f) {	
    if (!readChar(&prevTile, f) ||
        !readChar(&x, f) ||
        !readChar(&y, f) ||
        !readChar(&tile, f) ||
        !readChar(&prevx, f) ||
        !readChar(&prevy, f) ||
		!readChar(&unused1, f) ||
		!readChar(&unused2, f))
        return 0;
    return 1;
}

int SaveGameMonsterRecord::write(FILE *f) const {
	if (!writeChar(prevTile, f) ||
        !writeChar(x, f) ||
        !writeChar(y, f) ||
        !writeChar(tile, f) ||
        !writeChar(prevx, f) ||
        !writeChar(prevy, f) ||
		!writeChar(unused1, f) ||
		!writeChar(unused2, f))
        return 0;
    return 1;
}

int saveGameMonstersWrite(SaveGameMonsterRecord *monsterTable, FILE *f) {
    if (monsterTable) {
		for (int i = 0; i < MONSTERTABLE_SIZE; i++) {		
			if (!monsterTable[i].write(f))
				return 0;
		}		
	}
	else {
		for (int i = 0; i < MONSTERTABLE_SIZE; i++) {
			SaveGameMonsterRecord empty;
			if (!empty.write(f))
				return 0;
		}
	}
	return 1;
}

int saveGameMonstersRead(SaveGameMonsterRecord *monsterTable, FILE *f) {    
	for (int i = 0; i < MONSTERTABLE_SIZE; i++) {
		if (!monsterTable[i].read(f))
			return 0;
	}
	return 1;    
}
