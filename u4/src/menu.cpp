/*
 * $Id$
 */

#include "vc6.h" // Fixes things if you're using VC6, does nothing if otherwise

#include "menu.h"

#include "error.h"
#include "event.h"
#include "textview.h"

Menu::Menu() : 
    closed(false),
    title(""),
    titleX(0),
    titleY(0)
{
}

Menu::~Menu() {
    for (MenuItemList::iterator i = items.begin(); i != items.end(); i++)
        delete *i;
}

/**
 * Adds an item to the menu list and returns the menu
 */
void Menu::add(int id, string text, short x, short y, int sc) {
    MenuItem *item = new MenuItem(text, x, y, sc);
    item->setId(id);
    items.push_back(item);
}

MenuItem *Menu::add(MenuItem *item) {
    items.push_back(item);
    return item;
}

void Menu::addShortcutKey(int id, int shortcutKey) {
    for (MenuItemList::iterator i = items.begin(); i != items.end(); i++) {
        if ((*i)->getId() == id) {
            (*i)->addShortcutKey(shortcutKey);
            break;
        }
    }    
}

void Menu::setClosesMenu(int id) {
    for (MenuItemList::iterator i = items.begin(); i != items.end(); i++) {
        if ((*i)->getId() == id) {
            (*i)->setClosesMenu(true);
            break;
        }
    }
}

/**
 * Returns the menu item that is currently selected/highlighted
 */
Menu::MenuItemList::iterator Menu::getCurrent() {
    return selected;
}

/**
 * Sets the current menu item to the one indicated by the iterator
 */
void Menu::setCurrent(MenuItemList::iterator i) {
    selected = i;
    highlight(*selected);

    MenuEvent event(this, MenuEvent::SELECT);
    setChanged();
    notifyObservers(event);
}

void Menu::setCurrent(int id) {
    setCurrent(getById(id));
}

void Menu::show(TextView *view) {
    if (title.length() > 0)
        view->textAt(titleX, titleY, "%s", title.c_str());

    for (current = items.begin(); current != items.end(); current++) {
        MenuItem *mi = *current;

        if (mi->isVisible()) {
            if (mi->isSelected())
                view->textAt(mi->getX()-1, mi->getY(), "\010%s", mi->getText().c_str());
            else
                view->textAt(mi->getX(), mi->getY(), mi->getText().c_str());

            if (mi->isHighlighted()) {
                view->setCursorPos(mi->getX() - 2, mi->getY(), true);
                view->enableCursor();
            }
        }
    }
}

/**
 * Checks the menu to ensure that there is at least 1 visible
 * item in the list.  Returns true if there is at least 1 visible
 * item, false if nothing is visible.
 */
bool Menu::isVisible() {
    bool visible = false;

    for (current = items.begin(); current != items.end(); current++) {
        if ((*current)->isVisible())
            visible = true;
    }

    return visible;
}

/**
 * Sets the selected iterator to the next visible menu item and highlights it
 */
void Menu::next() {
    MenuItemList::iterator i = selected;
    if (isVisible()) {
        if (++i == items.end())
            i = items.begin();
        while (!(*i)->isVisible()) {
            if (++i == items.end())
                i = items.begin();
        }
    }

    setCurrent(i);
}

/**
 * Sets the selected iterator to the previous visible menu item and highlights it
 */
void Menu::prev() {
    MenuItemList::iterator i = selected;
    if (isVisible()) {
        if (i == items.begin())
            i = items.end();
        i--;
        while (!(*i)->isVisible()) {
            if (i == items.begin())
                i = items.end();
            i--;
        }
    }

    setCurrent(i);
}

/**
 * Highlights a single menu item, un-highlighting any others
 */
void Menu::highlight(MenuItem *item) {
    // unhighlight all menu items first
    for (current = items.begin(); current != items.end(); current++)
        (*current)->setHighlighted(false);
    if (item)
        item->setHighlighted(true);
}

/**
 * Returns an iterator pointing to the first menu item
 */
Menu::MenuItemList::iterator Menu::begin() {
    return items.begin();
}

/**
 * Returns an iterator pointing just past the last menu item
 */
Menu::MenuItemList::iterator Menu::end() {
    return items.end();
}

/**
 * Returns an iterator pointing to the first visible menu item
 */
Menu::MenuItemList::iterator Menu::begin_visible() {
    if (!isVisible())
        return items.end();

    current = items.begin();
    while (!(*current)->isVisible() && current != items.end())
        current++;

    return current;
}

/**
 * 'Resets' the menu.  This does the following:
 *      - un-highlights all menu items
 *      - highlights the first menu item
 *      - selects the first visible menu item
 */
void Menu::reset(bool highlightFirst) {
    closed = false;

    /* get the first visible menu item */
    selected = begin_visible();

    /* un-highlight and deselect each menu item */
    for (current = items.begin(); current != items.end(); current++) {
        (*current)->setHighlighted(false);
        (*current)->setSelected(false);
    }

    /* highlight the first visible menu item */
    if (highlightFirst)
        highlight(*selected);

    MenuEvent event(this, MenuEvent::RESET);
    setChanged();
    notifyObservers(event);
}

/**
 * Returns an iterator pointing to the item associated with the given 'id'
 */
Menu::MenuItemList::iterator Menu::getById(int id) {
    if (id == -1)
        return getCurrent();

    for (current = items.begin(); current != items.end(); current++) {
        if ((*current)->getId() == id)
            return current;
    }
    return items.end();
}

/**
 * Returns the menu item associated with the given 'id'
 */
MenuItem *Menu::getItemById(int id) {
    current = getById(id);
    if (current != items.end())
        return *current;
    return NULL;
}

/**
 * Activates the menu item given by 'id', using 'action' to
 * activate it.  If the menu item cannot be activated using
 * 'action', then it is not activated.  This also un-highlights
 * the menu item given by 'menu' and highlights the new menu
 * item that was found for 'id'.
 */
void Menu::activateItem(int id, MenuEvent::Type action) {
    MenuItem *mi;
    
    /* find the given menu item by id */
    if (id >= 0)
        mi = getItemById(id);
    /* or use the current item */
    else mi = *getCurrent();
       
    if (!mi)
        errorFatal("Error: Unable to find menu item with id '%d'", id);

    /* make sure the action given will activate the menu item */
    if (mi->getClosesMenu())
        setClosed(true);

    MenuEvent event(this, (MenuEvent::Type) action, mi);
    mi->activate(event);
    setChanged();
    notifyObservers(event);
}

/**
 * Activates a menu item by it's shortcut key.  True is returned if a
 * menu item get activated, false otherwise.
 */
bool Menu::activateItemByShortcut(int key, MenuEvent::Type action) {
    for (MenuItemList::iterator i = items.begin(); i != items.end(); i++) {
        const set<int> &shortcuts = (*i)->getShortcutKeys();
        if (shortcuts.find(key) != shortcuts.end()) {
            activateItem((*i)->getId(), action);
            return true;
        }
    }
    return false;
}

/**
 * Returns true if the menu has been closed.
 */
bool Menu::getClosed() const {
    return closed;
}

/**
 * Update whether the menu has been closed.
 */
void Menu::setClosed(bool closed) {
    this->closed = closed;
}

void Menu::setTitle(const string &text, int x, int y) {
    title = text;
    titleX = x;
    titleY = y;
}

MenuController::MenuController(Menu *menu, TextView *view) {
    this->menu = menu;
    this->view = view;
}

bool MenuController::keyPressed(int key) {
    bool handled = true;
    bool cursorOn = view->getCursorEnabled();

    if (cursorOn)
        view->disableCursor();

    switch(key) {
    case U4_UP:
        menu->prev();
        break;
    case U4_DOWN:
        menu->next();
        break;
    case U4_LEFT:
    case U4_RIGHT:
    case U4_ENTER:
        {
            MenuEvent::Type action = MenuEvent::ACTIVATE;

            if (key == U4_LEFT)
                action = MenuEvent::DECREMENT;
            else if (key == U4_RIGHT)
                action = MenuEvent::INCREMENT;
            menu->activateItem(-1, action);
        }
        break;
    default:
        handled = menu->activateItemByShortcut(key, MenuEvent::ACTIVATE);
    }

    menu->show(view);

    if (cursorOn)
        view->enableCursor();
    view->update();

    if (menu->getClosed())
        doneWaiting();

    return handled;
}

