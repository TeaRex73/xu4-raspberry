/*
 * $Id$
 */

#include <typeinfo>
#include "city.h"

#include "monster.h"
#include "object.h"
#include "person.h"
#include "player.h"
#include "types.h"

/**
 * Returns the name of the city
 */ 
string City::getName() {
    return name;
}

/**
 * Adds a person object to the map
 */
Person *City::addPerson(Person *person) {    
    Person *p = new Person;
    
    // Make a copy of the person before adding them, so 
    // things like angering the guards, etc. will be
    // forgotten the next time you visit :)
    *p = *person;

    /* set the start coordinates for the person */
    p->setCoords(p->start);    

    objects.push_back(p);    
    return p;
}

/**
 * Add people to the map
 */ 
void City::addPeople() {
    PersonList::iterator current;
    for (current = persons.begin(); current != persons.end(); current++) {
        Person *p = *current;
        if ((p->getTile() != 0) && 
            !(playerCanPersonJoin(p->name, NULL) && playerIsPersonJoined(p->name)))
            addPerson(p);
    }
}

/**
 * Returns the person object at the given (x,y,z) coords, if one exists.
 * Otherwise, returns NULL.
 */
const Person *City::personAt(MapCoords coords) {
    Object *obj;

    obj = objectAt(coords);
    if (isPerson(obj))
        return dynamic_cast<Person*>(obj);    
    else
        return NULL;
}

/**
 * Returns true if the Map pointed to by 'punknown'
 * is a City map
 */ 
bool isCity(Map *punknown) {
    City *pCity;
    if ((pCity = dynamic_cast<City*>(punknown)) != NULL)
        return true;
    else 
        return false;
}