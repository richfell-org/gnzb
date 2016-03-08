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
#ifndef __GNZB_CELLRENDERER_HEADER__
#define __GNZB_CELLRENDERER_HEADER__

#include <string>
#include <glibmm/property.h>
#include <glibmm/propertyproxy.h>
#include <gtkmm/cellrenderer.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/cellrendererprogress.h>
#include "bulletpointrenderer.h"

namespace Gtk {
	class Widget;
}

class GNzb;

/**
 * 
 * 
 */
class GNzbCellRenderer : public Gtk::CellRenderer
{
// construction/destruction
public:

	GNzbCellRenderer(int bullet_dim = 18);
	~GNzbCellRenderer();

// attributes
public:

	// GNzb property
	Glib::PropertyProxy<std::shared_ptr<GNzb>> property_nzb() { return m_prop_nzb.get_proxy(); }
	Glib::PropertyProxy_ReadOnly<std::shared_ptr<GNzb>> property_nzb() const;

	// The display name property
	Glib::PropertyProxy<Glib::ustring> property_nzb_name() { return m_prop_nzb_name.get_proxy(); }
	Glib::PropertyProxy_ReadOnly<Glib::ustring> property_nzb_name() const;

	// The bullet color property
	Glib::PropertyProxy<Glib::ustring> property_bullet_rgba_text() { return m_prop_bullet_rgba_text.get_proxy(); }
	Glib::PropertyProxy_ReadOnly<Glib::ustring> property_bullet_rgba_text() const;

	// the space between the widget rows in the cell
	Glib::PropertyProxy<int> property_margin() { return m_prop_margin.get_proxy(); }
	Glib::PropertyProxy_ReadOnly<int> property_margin() const;

	// access to the bullet renderer
	BulletPointRenderer& bullet_renderer() { return m_bullet_renderer; }
	const BulletPointRenderer& bullet_renderer() const { return m_bullet_renderer; }

protected:

	// overrides
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

	// renderer properties
	Glib::Property<std::shared_ptr<GNzb>> m_prop_nzb;
	Glib::Property<Glib::ustring> m_prop_nzb_name;
	Glib::Property<Glib::ustring> m_prop_bullet_rgba_text;
	Glib::Property<int> m_prop_margin;

	// helper cell renderers
	BulletPointRenderer m_bullet_renderer;
	Gtk::CellRendererText m_name_renderer, m_status_renderer;
	Gtk::CellRendererProgress m_progress_renderer;

	static int get_cellrenderer_height(const Gtk::CellRenderer& cell_renderer, Gtk::Widget& w);
};

#endif  /* __GNZB_CELLRENDERER_HEADER__ */