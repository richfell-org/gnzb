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
#ifndef __NZB_FILE_CELLRENDERER_HEADER__
#define __NZB_FILE_CELLRENDERER_HEADER__

#include <glibmm/property.h>
#include <glibmm/propertyproxy.h>
#include <gtkmm/cellrenderer.h>
#include <gtkmm/cellrenderertext.h>
#include <libusenet/nzb.h>
#include "../guignzb.h"

/**
 * 
 * 
 */
class NzbFileCellRenderer : public Gtk::CellRenderer
{
// construction/destruction
public:

	NzbFileCellRenderer();
	~NzbFileCellRenderer();

// attributes
public:

	// NZB::File and FileMeta property
	Glib::PropertyProxy<NzbFileTuple> property_file_tuple() { return m_prop_file_tuple.get_proxy(); }
	Glib::PropertyProxy_ReadOnly<NzbFileTuple> property_file_tuple() const;

	// name property
	Glib::PropertyProxy<Glib::ustring> property_name() { return m_prop_name.get_proxy(); }
	Glib::PropertyProxy_ReadOnly<Glib::ustring> property_name() const;

	Glib::PropertyProxy<int> property_margin() { return m_prop_margin.get_proxy(); }
	Glib::PropertyProxy_ReadOnly<int> property_margin() const;

// implementation
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

	// properties
	Glib::Property<NzbFileTuple> m_prop_file_tuple;
	Glib::Property<Glib::ustring> m_prop_name;
	Glib::Property<int> m_prop_margin;

	// helper renderers
	Gtk::CellRendererText m_name_renderer, m_status_renderer;

	static int get_cellrenderer_height(const Gtk::CellRenderer& cell_renderer, Gtk::Widget& w);
};

#endif  /* __NZB_FILE_CELLRENDERER_HEADER__ */