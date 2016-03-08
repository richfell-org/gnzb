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
#include "gnzbgroupsmenu.h"
#include <algorithm>
#include <gdkmm/rgba.h>
#include <gtkmm/box.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/label.h>
#include <gtkmm/checkmenuitem.h>
#include "../../gnzb.h"
#include "../cellrenderer/bulletpointrenderer.h"

class BulletPoint : public Gtk::DrawingArea
{
public:

	BulletPoint(const Gdk::RGBA& color, int min_width = 15) : m_color(color)
	{
		set_size_request(min_width);
	}

	~BulletPoint() {}

protected:

	bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
	{
		int width = get_allocated_width();
		int height = get_allocated_height();
		int size = std::min(width, height);

		Gdk::Rectangle rectBullet(0, 0, width, height);
		BulletPointRenderer renderer;
		renderer.set_fixed_size(size, size);
		renderer.set_color(m_color);
		cr->save();
		renderer.render(cr, *this, rectBullet, rectBullet, Gtk::CellRendererState(0));
		cr->restore();

		return true;
	}

private:

	Gdk::RGBA m_color;
};

class GroupCheckMenuItem : public Gtk::CheckMenuItem
{
public:

	GroupCheckMenuItem(const GNzbGroup& group)
	:   Gtk::CheckMenuItem(),
		m_box(Gtk::ORIENTATION_HORIZONTAL, 6),
		m_bullet(Gdk::RGBA(group.getColorText())),
		m_label(group.getName())
	{
		m_box.pack_start(m_bullet, Gtk::PACK_SHRINK);
		m_box.pack_start(m_label, Gtk::PACK_SHRINK);
		m_bullet.show();
		m_label.show();
		m_box.show();
		add(m_box);
	}
	~GroupCheckMenuItem() {}

private:

	Gtk::Box m_box;
	BulletPoint m_bullet;
	Gtk::Label m_label;
};

GNzbGroupsMenu::GNzbGroupsMenu()
:   m_configure_item("Configure...")
{
	m_group_items.clear();

	// set up the items that are always present:
	// - The separator
	// - The "Configure..." option
	// +--------------------+
	// |  <NZB group>       |
	// |  <NZB group>       |
	// |  <...>             |
	// |____________________|
	// |  Configure...      |
	// +--------------------+
	m_separator.show();
	m_configure_item.show();

	// add these "constant" items to the menu
	append(m_separator);
	append(m_configure_item);
}

GNzbGroupsMenu::~GNzbGroupsMenu()
{
}

void GNzbGroupsMenu::resetAppNzbGroupItems(std::vector<GNzbGroup> groups)
{
	// remove existing items
	for(auto& item_tuple : m_group_items)
		remove(*std::get<1>(item_tuple));
	m_group_items.clear();

	// allocate menu items for each NZB group provided
	int i = 0;
	for(auto& group : groups)
	{
		// allocate the check item
		Gtk::CheckMenuItem *p_chk_item = new GroupCheckMenuItem(group);
		p_chk_item->signal_activate().connect(
			sigc::bind<int>(sigc::mem_fun(*this, &GNzbGroupsMenu::onMenuItemSelected), i));
		p_chk_item->show();

		// Insert the check item ito the menu...
		insert(*p_chk_item, i++);

		// save the NZB group and check item instance
		m_group_items.emplace_back(group, std::unique_ptr<Gtk::CheckMenuItem>(p_chk_item));
	}
}

void GNzbGroupsMenu::setConfigureSlot(sigc::slot<void> callback)
{
	m_configure_item.signal_activate().connect(callback);
}

void GNzbGroupsMenu::setActiveGroup(GNzbGroup& group)
{
	// looking to set the given group as the selected group
	for(auto& item_tuple : m_group_items)
		std::get<1>(item_tuple)->set_active(group == std::get<0>(item_tuple));
}

void GNzbGroupsMenu::popup(guint32 eTime, int x, int y)
{
	Gtk::Menu::popup(
		[x, y](int& mx, int& my, bool& push_in){ mx = x + 2; my = y; push_in = true; }, 1, eTime);
}

void GNzbGroupsMenu::onMenuItemSelected(int i_item)
{
	GNzbGroup& group = std::get<0>(m_group_items[i_item]);
	m_sig_group_selected(m_target, group.get_db_id());
}