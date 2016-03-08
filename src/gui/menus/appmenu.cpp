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
#include "appmenu.h"
#include <string>

void MenuItem::add_to_menu(Gio::Menu& menu) const
{
}

void AppMenuAction::add_to_menu(Gio::Menu& menu) const
{
	Glib::RefPtr<Gio::MenuItem> ref_item = Gio::MenuItem::create(title, std::string("app.") + name);
	if(!accel.empty())
		ref_item->set_attribute("accel", Glib::Variant<Glib::ustring>::create(accel.c_str()));
	menu.append_item(ref_item);
}

void WinMenuAction::add_to_menu(Gio::Menu& menu) const
{
	Glib::RefPtr<Gio::MenuItem> ref_item = Gio::MenuItem::create(title, std::string("win.") + name);
	if(!accel.empty())
		ref_item->set_attribute("accel", Glib::Variant<Glib::ustring>::create(accel.c_str()));
	menu.append_item(ref_item);
}

void MenuSection::add_to_menu(Gio::Menu& menu) const
{
	Glib::RefPtr<Gio::Menu> section = Gio::Menu::create();
	for(const MenuItem *p_m : items)
		p_m->add_to_menu(*section.operator ->());
	menu.append_section(section);
}

void SubMenu::add_to_menu(Gio::Menu& menu) const
{
	Glib::RefPtr<Gio::Menu> sub = Gio::Menu::create();
	for(const MenuItem *p_i : items)
		p_i->add_to_menu(*sub.operator ->());
	menu.append_item(Gio::MenuItem::create(title, sub));
}

AppMenu::AppMenu()
:   Gio::Menu()
{
}

AppMenu::AppMenu(std::initializer_list<const MenuItem*> l)
:   Gio::Menu()
{
	std::string app_base("app.");
	for(const MenuItem *p_m : l)
		p_m->add_to_menu(*this);
}


AppMenu::~AppMenu()
{
}