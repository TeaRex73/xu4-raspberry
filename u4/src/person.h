/*
 * $Id$
 */

#ifndef PERSON_H
#define PERSON_H

#include "object.h"

struct _Conversation;

typedef enum {
    QTRIGGER_NONE = 0,
    QTRIGGER_JOB = 3,
    QTRIGGER_HEALTH = 4,
    QTRIGGER_KEYWORD1 = 5,
    QTRIGGER_KEYWORD2 = 6
} PersonQuestionTrigger;

typedef enum {
    QUESTION_SHOULDSAYYES,
    QUESTION_SHOULDSAYNO
} PersonQuestionType;

typedef enum {
   NPC_EMPTY,
   NPC_TALKER,
   NPC_TALKER_BEGGAR,
   NPC_TALKER_GUARD,
   NPC_TALKER_COMPANION,
   NPC_VENDOR_WEAPONS,
   NPC_VENDOR_ARMOR,
   NPC_VENDOR_FOOD,
   NPC_VENDOR_TAVERN,
   NPC_VENDOR_REAGENTS,
   NPC_VENDOR_HEALER,
   NPC_VENDOR_INN,
   NPC_VENDOR_GUILD,
   NPC_VENDOR_STABLE,
   NPC_LORD_BRITISH,
   NPC_HAWKWIND,
   NPC_MAX
} PersonNpcType;

typedef enum {
    CONV_INTRO,
    CONV_TALK,
    CONV_ASK,
    CONV_BUYSELL,
    CONV_BUY,
    CONV_SELL,
    CONV_QUANTITY,
    CONV_DONE
} ConversationState;

typedef struct _PersonType {
    char *(*getIntro)(struct _Conversation *cnv);
    char *(*getResponse)(struct _Conversation *cnv, const char *inquiry);
    char *(*getQuestionResponse)(struct _Conversation *cnv, const char *answer);
    char *(*getBuySellResponse)(struct _Conversation *cnv, const char *inquiry);
    char *(*getBuyResponse)(struct _Conversation *cnv, const char *inquiry);
    char *(*getSellResponse)(struct _Conversation *cnv, const char *inquiry);
    char *(*getQuantityResponse)(struct _Conversation *cnv, const char *inquiry);
    char *(*getPrompt)(const struct _Conversation *cnv);
} PersonType;

typedef struct _Person {
    char *name;
    char *pronoun;
    char *description;
    char *job;
    char *health;
    char *response1;
    char *response2;
    char *question;
    char *yesresp;
    char *noresp;
    char *keyword1;
    char *keyword2;
    PersonQuestionTrigger questionTrigger;
    PersonQuestionType questionType;
    int turnAwayProb;
    unsigned char tile0, tile1;
    unsigned int startx, starty;
    ObjectMovementBehavior movement_behavior;
    PersonNpcType npcType;
    int vendorIndex;
} Person;

int personInit(void);
void personGetConversationText(struct _Conversation *cnv, const char *inquiry, char **response);
void personGetPrompt(const struct _Conversation *cnv, char **prompt);
char *concat(const char *str, ...);

#endif
