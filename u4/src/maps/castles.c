/*
 * $Id$
 */

#include <stddef.h>

#include "../u4.h"
#include "../map.h"
#include "../city.h"
#include "../portal.h"

City empath_city;

Map empath_map = {
    "empath.ult", /* fname */
    CITY_WIDTH, CITY_HEIGHT, /* width, height */
    15, 31, /* startx, starty */
    BORDER_EXIT2PARENT, /* border_behavior */
    0, /* n_portals */
    NULL, /* portals */
    SHOW_AVATAR, /* flags */
    NULL, /* data */
    &empath_city /* city */
};

City empath_city  = {
    "Empath Abbey", /* name */
    0, /* n_persons */
    NULL, /* persons */
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /* person_types */
    "empath.tlk", /* tlk_fname */
    &empath_map /* map */
};

City lycaeum_city;

Map lycaeum_map = {
    "lycaeum.ult", /* fname */
    CITY_WIDTH, CITY_HEIGHT, /* width, height */
    15, 31, /* startx, starty */
    BORDER_EXIT2PARENT, /* border_behavior */
    0, /* n_portals */
    NULL, /* portals */
    SHOW_AVATAR, /* flags */
    NULL, /* data */
    &lycaeum_city /* city */
};

City lycaeum_city  = {
    "Lycaeum", /* name */
    0, /* n_persons */
    NULL, /* persons */
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /* person_types */
    "lycaeum.tlk", /* tlk_fname */
    &lycaeum_map /* map */
};

City serpent_city;

Map serpent_map = {
    "serpent.ult", /* fname */
    CITY_WIDTH, CITY_HEIGHT, /* width, height */
    15, 31, /* startx, starty */
    BORDER_EXIT2PARENT, /* border_behavior */
    0, /* n_portals */
    NULL, /* portals */
    SHOW_AVATAR, /* flags */
    NULL, /* data */
    &serpent_city /* city */
};

City serpent_city  = {
    "Serpents Hold", /* name */
    0, /* n_persons */
    NULL, /* persons */
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /* person_types */
    "serpent.tlk", /* tlk_fname */
    &serpent_map /* map */
};

Map lcb_2_map;

const Portal lcb_1_portals[] = {
    { 3, 3, &lcb_2_map, ACTION_KLIMB },
    { 27, 3, &lcb_2_map, ACTION_KLIMB }
};

City lcb_1_city;

Map lcb_1_map = {
    "lcb_1.ult", /* fname */
    CITY_WIDTH, CITY_HEIGHT, /* width, height */
    15, 31, /* startx, starty */
    BORDER_EXIT2PARENT, /* border_behavior */
    2, /* n_portals */
    lcb_1_portals, /* portals */
    SHOW_AVATAR, /* flags */
    NULL, /* data */
    &lcb_1_city /* city */
};

City lcb_1_city  = {
    "Britannia", /* name */
    0, /* n_persons */
    NULL, /* persons */
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13 }, /* person_types */
    "lcb.tlk", /* tlk_fname */
    &lcb_1_map /* map */
};

const Portal lcb_2_portals[] = {
    { 3, 3, &lcb_1_map, ACTION_DESCEND },
    { 27, 3, &lcb_1_map, ACTION_DESCEND }
};

City lcb_2_city;

Map lcb_2_map = {
    "lcb_2.ult", /* fname */
    CITY_WIDTH, CITY_HEIGHT, /* width, height */
    15, 31, /* startx, starty */
    BORDER_EXIT2PARENT, /* border_behavior */
    2, /* n_portals */
    lcb_2_portals, /* portals */
    SHOW_AVATAR, /* flags */
    NULL, /* data */
    &lcb_2_city /* city */
};

City lcb_2_city  = {
    "Britannia", /* name */
    0, /* n_persons */
    NULL, /* persons */
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 0 }, /* person_types */
    "lcb.tlk", /* tlk_fname */
    &lcb_2_map /* map */
};
