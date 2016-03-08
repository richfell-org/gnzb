/*
	gnzb Usenet downloads using NZB index files

    Copyright (C) 2016  Richard J. Fellinger, Jr

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, see <http://www.gnu.org/licenses/> or write
	to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301 USA.
*/
#ifndef __APP_MENU_HEADER__
#define __APP_MENU_HEADER__

#include <vector>
#include <initializer_list>
#include <glibmm/ustring.h>
#include <giomm/menu.h>

class AppMenu;

/*
 * Base class for a menu item
 */
struct MenuItem
{
	Glib::ustring title;

	MenuItem(const char *t) : title(t) {}
	MenuItem(Glib::ustring&& t) : title(t) {}
	MenuItem(const Glib::ustring& t) : title(t) {}
	MenuItem(MenuItem&& that) : title(std::move(that.title)) {}
	MenuItem(const MenuItem& that) : title(that.title) {}
	~MenuItem() {}

	virtual void add_to_menu(Gio::Menu& menu) const = 0;

	MenuItem& operator =(MenuItem&& that) { title = std::move(that.title); return *this; }
	MenuItem& operator =(const MenuItem& that) { title = that.title; return *this; }
};

/*
 * This is an individual menu item with tha action
 * string in 'name' and the accelerator string in
 * 'accel'
 */
struct MenuAction : public MenuItem
{
	Glib::ustring name, accel;

	MenuAction(const char *t, const char *n, const char *a = nullptr)
		: MenuItem(t), name(n), accel((nullptr == a ? "" : a)) {}
	MenuAction(MenuAction&& that)
		: MenuItem((MenuItem&&)that), name(std::move(that.name)), accel(std::move(that.accel)) {}
	MenuAction(const MenuAction& that)
		: MenuItem((const MenuItem&)that), name(that.name), accel(that.accel) {}
	~MenuAction() {}

	MenuAction& operator =(MenuAction&& that)
	{
		MenuItem::operator =(std::move(that));
		name = std::move(that.name);
		accel = std::move(that.accel);
		return *this;
	}

	MenuAction& operator =(const MenuAction& that)
	{
		MenuItem::operator =(that);
		name = that.name;
		accel = that.accel; return *this;
	}
};

/*
 * MenuItem which targets the Gtk::Application instance
 */
struct AppMenuAction : public MenuAction
{
	using MenuAction::MenuAction;
	using MenuAction::operator =;

	void add_to_menu(Gio::Menu& menu) const;
};

/*
 * MenuItem which targets the Gtk::ApplicationWindow instance
 * by prepending "win." to the name as the item is inserted
 * into the menu instance passed to add_to_menu
 */
struct WinMenuAction : public MenuAction
{
	using MenuAction::MenuAction;
	using MenuAction::operator =;

	void add_to_menu(Gio::Menu& menu) const;
};

/*
 * Base class for menu sections and sub-menus  
 */
struct MenuGroup : public MenuItem
{
	std::vector<const MenuItem*> items;

	MenuGroup(const char *t, std::initializer_list<const MenuItem*> mitems = {})
		: MenuItem(t) { for(auto& i : mitems) items.push_back(i); }
	MenuGroup(std::initializer_list<const MenuItem*> mitems)
		: MenuItem("") { for(auto& i : mitems) items.push_back(i); }
	MenuGroup(MenuGroup&& that)
		: MenuItem((MenuGroup&&)that), items(std::move(that.items)) {}
	MenuGroup(const MenuGroup& that)
		: MenuItem((const MenuGroup&)that), items(that.items) {}
	~MenuGroup() {}

	MenuGroup& operator =(MenuGroup&& that)
	{
		MenuItem::operator =(std::move(that));
		items = std::move(that.items);
		return *this;
	}

	MenuGroup& operator =(const MenuGroup& that)
	{
		MenuItem::operator =(that);
		items = that.items;
		return *this;
	}
};

/*
 * This class puts all of its MenuItems into
 * a Gio::Menu and then calls Gio::Menu::append_section
 * on the instance passed into add_to_menu
 */
struct MenuSection : public MenuGroup
{
	using MenuGroup::MenuGroup;
	using MenuGroup::operator =;

	void add_to_menu(Gio::Menu& menu) const;
};

/*
 * A SubMenu MenuItem.  This class adds all of its
 * items to a Gio::Menu and then inserts that menu
 * into the instance passed to add_to_menu
 */
struct SubMenu : public MenuGroup
{
	using MenuGroup::MenuGroup;
	using MenuGroup::operator =;

	void add_to_menu(Gio::Menu& menu) const;
};

/*
 * 
 */
class AppMenu : public Gio::Menu
{
// construction
public:

	AppMenu();
	AppMenu(std::initializer_list<const MenuItem*> l);
	~AppMenu();
};

#endif  /* __APP_MENU_HEADER__ */