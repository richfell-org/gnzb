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
#ifndef __GNZB_ACTION_CELL_RENDERER_HEADER__
#define __GNZB_ACTION_CELL_RENDERER_HEADER__

#include <vector>
#include <glibmm/refptr.h>
#include <gtkmm/cellrendererpixbuf.h>
#include "../guignzb.h"
#include <glibmm/property.h>

namespace Gdk {
	class Pixbuf;
}

/**
 * 
 * 
 */
class GNzbActionCellRenderer : public Gtk::CellRendererPixbuf
{
// construction
public:

	GNzbActionCellRenderer();
	~GNzbActionCellRenderer() {}

// attributes
public:

	// image attrubutes
	int get_image_width() const;
	int get_image_height() const;

	// The GNzbAction value
	Glib::PropertyProxy<GNzbAction> property_action() { return m_prop_action.get_proxy(); }
	Glib::PropertyProxy_ReadOnly<GNzbAction> property_action() const;

	// Indicator for rendering a pressed state button
	Glib::PropertyProxy<bool> property_pressed() { return m_prop_pressed.get_proxy(); }
	Glib::PropertyProxy_ReadOnly<bool> property_pressed() const;

// implementation
protected:

	// sizing methods
	Gtk::SizeRequestMode get_request_mode_vfunc() const;
	void get_preferred_width_vfunc(Gtk::Widget& widget, int& minimum_width, int& natural_width) const;
	void get_preferred_height_for_width_vfunc(Gtk::Widget& widget, int width, int& minimum_height, int& natural_height) const;
	void get_preferred_height_vfunc(Gtk::Widget& widget, int& minimum_height, int& natural_height) const;
	void get_preferred_width_for_height_vfunc(Gtk::Widget& widget, int height, int& minimum_width, int& natural_width) const;

	// rendering method
	void render_vfunc(
		const Cairo::RefPtr<Cairo::Context>& cr,
		Gtk::Widget& widget,
		const Gdk::Rectangle& background_area,
		const Gdk::Rectangle& cell_area,
		Gtk::CellRendererState flags);

	// properties
	Glib::Property<GNzbAction> m_prop_action;
	Glib::Property<bool> m_prop_pressed;

	// class global action icons
	static std::vector<Glib::RefPtr<Gdk::Pixbuf>> s_action_icons;
	static std::vector<Glib::RefPtr<Gdk::Pixbuf>> s_action_icons_pressed;

	static void init_icons();
};

#endif  /* __GNZB_ACTION_CELL_RENDERER_HEADER__ */