/*
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "savegame.h"
#include "io.h"
#include "object.h"

#define MONSTERTABLE_SIZE 32

int saveGameWrite(const SaveGame *save, FILE *f) {
    int i;

    if (!writeInt(save->unknown1, f) ||
        !writeInt(save->moves, f))
        return 0;

    for (i = 0; i < 8; i++) {
        if (!saveGamePlayerRecordWrite(&(save->players[i]), f))
            return 0;
    }

    if (!writeInt(save->food, f) ||
        !writeShort(save->gold, f))
        return 0;

    for (i = 0; i < 8; i++) {
        if (!writeShort(save->karma[i], f))
            return 0;
    }

    if (!writeShort(save->torches, f) ||
        !writeShort(save->gems, f) ||
        !writeShort(save->keys, f) ||
        !writeShort(save->sextants, f))
        return 0;

    for (i = 0; i < ARMR_MAX; i++) {
        if (!writeShort(save->armor[i], f))
            return 0;
    }

    for (i = 0; i < WEAP_MAX; i++) {
        if (!writeShort(save->weapons[i], f))
            return 0;
    }

    for (i = 0; i < REAG_MAX; i++) {
        if (!writeShort(save->reagents[i], f))
            return 0;
    }

    for (i = 0; i < 26; i++) {
        if (!writeShort(save->mixtures[i], f))
            return 0;
    }

    if (!writeShort(save->items, f) ||
        !writeChar(save->x, f) ||
        !writeChar(save->y, f) ||
        !writeChar(save->stones, f) ||
        !writeChar(save->runes, f) ||
        !writeShort(save->members, f) ||
        !writeShort(save->transport, f) ||
        !writeShort(save->balloonstate, f) ||
        !writeShort(save->trammelphase, f) ||
        !writeShort(save->feluccaphase, f) ||
        !writeShort(save->shiphull, f) ||
        !writeShort(save->lbintro, f) ||
        !writeShort(save->lastcamp, f) ||
        !writeShort(save->lastreagent, f) ||
        !writeShort(save->lastmeditation, f) ||
        !writeShort(save->lastvirtue, f) ||
        !writeChar(save->dngx, f) ||
        !writeChar(save->dngy, f) ||
        !writeShort(save->orientation, f) ||
        !writeShort(save->dnglevel, f) ||
        !writeShort(save->location, f))
        return 0;

    return 1;
}

int saveGameRead(SaveGame *save, FILE *f) {
    int i;

    if (!readInt(&(save->unknown1), f) ||
        !readInt(&(save->moves), f))
        return 0;

    for (i = 0; i < 8; i++) {
        if (!saveGamePlayerRecordRead(&(save->players[i]), f))
            return 0;
    }

    if (!readInt(&(save->food), f) ||
        !readShort(&(save->gold), f))
        return 0;

    for (i = 0; i < 8; i++) {
        if (!readShort(&(save->karma[i]), f))
            return 0;
    }

    if (!readShort(&(save->torches), f) ||
        !readShort(&(save->gems), f) ||
        !readShort(&(save->keys), f) ||
        !readShort(&(save->sextants), f))
        return 0;

    for (i = 0; i < ARMR_MAX; i++) {
        if (!readShort(&(save->armor[i]), f))
            return 0;
    }

    for (i = 0; i < WEAP_MAX; i++) {
        if (!readShort(&(save->weapons[i]), f))
            return 0;
    }

    for (i = 0; i < REAG_MAX; i++) {
        if (!readShort(&(save->reagents[i]), f))
            return 0;
    }

    for (i = 0; i < 26; i++) {
        if (!readShort(&(save->mixtures[i]), f))
            return 0;
    }

    if (!readShort(&(save->items), f) ||
        !readChar(&(save->x), f) ||
        !readChar(&(save->y), f) ||
        !readChar(&(save->stones), f) ||
        !readChar(&(save->runes), f) ||
        !readShort(&(save->members), f) ||
        !readShort(&(save->transport), f) ||
        !readShort(&(save->balloonstate), f) ||
        !readShort(&(save->trammelphase), f) ||
        !readShort(&(save->feluccaphase), f) ||
        !readShort(&(save->shiphull), f) ||
        !readShort(&(save->lbintro), f) ||
        !readShort(&(save->lastcamp), f) ||
        !readShort(&(save->lastreagent), f) ||
        !readShort(&(save->lastmeditation), f) ||
        !readShort(&(save->lastvirtue), f) ||
        !readChar(&(save->dngx), f) ||
        !readChar(&(save->dngy), f) ||
        !readShort(&(save->orientation), f) ||
        !readShort(&(save->dnglevel), f) ||
        !readShort(&(save->location), f))
        return 0;

    return 1;
}

void saveGameInit(SaveGame *save, const SaveGamePlayerRecord *avatarInfo) {
    int i;

    save->unknown1 = 0;
    save->moves = 0;

    memcpy(&(save->players[0]), avatarInfo, sizeof(SaveGamePlayerRecord));
    for (i = 1; i < 8; i++)
        saveGamePlayerRecordInit(&(save->players[i]));

    save->food = 0;
    save->gold = 0;

    for (i = 0; i < 8; i++)
        save->karma[i] = 20;

    save->torches = 0;
    save->gems = 0;
    save->keys = 0;
    save->sextants = 0;

    for (i = 0; i < ARMR_MAX; i++)
        save->armor[i] = 0;

    for (i = 0; i < WEAP_MAX; i++)
        save->weapons[i] = 0;

    for (i = 0; i < REAG_MAX; i++)
        save->reagents[i] = 0;

    for (i = 0; i < 26; i++)
        save->mixtures[i] = 0;

    save->items = 0;
    save->x = 0;
    save->y = 0;
    save->stones = 0;
    save->runes = 0;
    save->members = 1;
    save->transport = 0x1f;
    save->balloonstate = 0;
    save->trammelphase = 0;
    save->feluccaphase = 0;
    save->shiphull = 50;
    save->lbintro = 0;
    save->lastcamp = 0;
    save->lastreagent = 0;
    save->lastmeditation = 0;
    save->lastvirtue = 0;
    save->dngx = 0;
    save->dngy = 0;
    save->orientation = 0;
    save->dnglevel = 0xFFFF;
    save->location = 0;
}

int saveGamePlayerRecordWrite(const SaveGamePlayerRecord *record, FILE *f) {
    int i;

    if (!writeShort(record->hp, f) ||
        !writeShort(record->hpMax, f) ||
        !writeShort(record->xp, f) ||
        !writeShort(record->str, f) ||
        !writeShort(record->dex, f) ||
        !writeShort(record->intel, f) ||
        !writeShort(record->mp, f) ||
        !writeShort(record->unknown, f) ||
        !writeShort((unsigned short)record->weapon, f) ||
        !writeShort((unsigned short)record->armor, f))
        return 0;

    for (i = 0; i < 16; i++) {
        if (!writeChar(record->name[i], f))
            return 0;
    }

    if (!writeChar((unsigned char)record->sex, f) ||
        !writeChar((unsigned char)record->klass, f) ||
        !writeChar((unsigned char)record->status, f))
        return 0;

    return 1;
}

int saveGamePlayerRecordRead(SaveGamePlayerRecord *record, FILE *f) {
    int i;
    unsigned char ch;
    unsigned short s;

    if (!readShort(&(record->hp), f) ||
        !readShort(&(record->hpMax), f) ||
        !readShort(&(record->xp), f) ||
        !readShort(&(record->str), f) ||
        !readShort(&(record->dex), f) ||
        !readShort(&(record->intel), f) ||
        !readShort(&(record->mp), f) ||
        !readShort(&(record->unknown), f))
        return 0;
        
    if (!readShort(&s, f))
        return 0;
    record->weapon = s;
    if (!readShort(&s, f))
        return 0;
    record->armor = s;

    for (i = 0; i < 16; i++) {
        if (!readChar(&(record->name[i]), f))
            return 0;
    }

    if (!readChar(&ch, f))
        return 0;
    record->sex = ch;
    if (!readChar(&ch, f))
      return 0;
    record->klass = ch;
    if (!readChar(&ch, f))
        return 0;
    record->status = ch;

    return 1;
}

void saveGamePlayerRecordInit(SaveGamePlayerRecord *record) {
    int i;

    record->hp = 0;
    record->hpMax = 0;
    record->xp = 0;
    record->str = 0;
    record->dex = 0;
    record->intel = 0;
    record->mp = 0;
    record->unknown = 0;
    record->weapon = WEAP_HANDS;
    record->armor = ARMR_NONE;

    for (i = 0; i < 16; i++)
      record->name[i] = '\0';

    record->sex = SEX_MALE;
    record->klass = CLASS_MAGE;
    record->status = STAT_GOOD;
}

int saveGameMonstersWrite(const Object *objs, FILE *f) {
    const Object *obj;
    const Object *monsterTable[MONSTERTABLE_SIZE];
    int anim, inanim;
    int i;
    int r;

    memset(monsterTable, 0, MONSTERTABLE_SIZE * sizeof(Object *));
    anim = 0;
    inanim = MONSTERTABLE_SIZE;
    obj = objs;
    for (i = 0; i < MONSTERTABLE_SIZE; i++) {
        if (obj) {
            if (obj->isAvatar)
                /* skip */;
            else if (/* is animate */ 0)
                monsterTable[anim++] = obj;
            else
                monsterTable[--inanim] = obj;
            obj = obj->next;
        }
    }

    for (i = 0; i < MONSTERTABLE_SIZE; i++) {
        if (monsterTable[i])
            r = writeChar(monsterTable[i]->tile, f);
        else
            r = writeChar(0, f);
        if (!r) return 0;
    }
        
    for (i = 0; i < MONSTERTABLE_SIZE; i++) {
        if (monsterTable[i])
            r = writeChar(monsterTable[i]->x, f);
        else
            r = writeChar(0, f);
        if (!r) return 0;
    }
        
    for (i = 0; i < MONSTERTABLE_SIZE; i++) {
        if (monsterTable[i])
            r = writeChar(monsterTable[i]->y, f);
        else
            r = writeChar(0, f);
        if (!r) return 0;
    }
        
    for (i = 0; i < MONSTERTABLE_SIZE; i++) {
        if (monsterTable[i])
            r = writeChar(monsterTable[i]->prevtile, f);
        else
            r = writeChar(0, f);
        if (!r) return 0;
    }
        
    for (i = 0; i < MONSTERTABLE_SIZE; i++) {
        if (monsterTable[i])
            r = writeChar(monsterTable[i]->prevx, f);
        else
            r = writeChar(0, f);
        if (!r) return 0;
    }
        
    for (i = 0; i < MONSTERTABLE_SIZE; i++) {
        if (monsterTable[i])
            r = writeChar(monsterTable[i]->prevy, f);
        else
            r = writeChar(0, f);
        if (!r) return 0;
    }
        
    for (i = 0; i < MONSTERTABLE_SIZE; i++) {
        if (!writeChar(0, f))
            return 0;
    }

    obj = objs;
    for (i = 0; i < MONSTERTABLE_SIZE; i++) {
        if (!writeChar(0, f))
            return 0;
    }

    return 1;
}

int saveGameMonstersRead(Object **objs, FILE *f) {
    Object *obj;
    Object monsterTable[MONSTERTABLE_SIZE];
    int i;
    unsigned char ch;

    for (i = 0; i < MONSTERTABLE_SIZE; i++) {
        if (!readChar(&monsterTable[i].tile, f))
            return 0;
    }

    for (i = 0; i < MONSTERTABLE_SIZE; i++) {
        if (!readChar(&ch, f))
            return 0;
        monsterTable[i].x = ch;
    }

    for (i = 0; i < MONSTERTABLE_SIZE; i++) {
        if (!readChar(&ch, f))
            return 0;
        monsterTable[i].y = ch;
    }

    for (i = 0; i < MONSTERTABLE_SIZE; i++) {
        if (!readChar(&monsterTable[i].prevtile, f))
            return 0;
    }

    for (i = 0; i < MONSTERTABLE_SIZE; i++) {
        if (!readChar(&ch, f))
            return 0;
        monsterTable[i].prevx = ch;
    }

    for (i = 0; i < MONSTERTABLE_SIZE; i++) {
        if (!readChar(&ch, f))
            return 0;
        monsterTable[i].prevy = ch;
    }

    for (i = MONSTERTABLE_SIZE - 1; i >= 0; i--) {
        if (monsterTable[i].tile != 0 &&
            monsterTable[i].prevtile != 0) {
            obj = malloc(sizeof(Object));
            obj->tile = monsterTable[i].tile;
            obj->x = monsterTable[i].x;
            obj->y = monsterTable[i].y;
            obj->prevtile = monsterTable[i].prevtile;
            obj->prevx = monsterTable[i].prevx;
            obj->prevy = monsterTable[i].prevy;
            obj->movement_behavior = MOVEMENT_FIXED; /* FIXME: monsters should be MOVEMENT_ATTACK_AVATAR */
            obj->person = NULL;
            obj->isAvatar = 0;
            obj->hasFocus = 0;
            obj->next = *objs;
            *objs = obj;
        }
    }

    return 1;
}

