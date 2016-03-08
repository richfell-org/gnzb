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
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/
#ifndef __BULLET_POINT_RENDERER_HEADER__
#define __BULLET_POINT_RENDERER_HEADER__

#include <glibmm/property.h>
#include <glibmm/propertyproxy.h>
#include <gdkmm/rgba.h>
#include <gtkmm/cellrenderer.h>

/*
 * 
 */
class BulletPointRenderer : public Gtk::CellRenderer
{
public:

	BulletPointRenderer();
	~BulletPointRenderer();

// attributes
public:

	// getter/setter for the color property
	BulletPointRenderer& set_color(const char *colorText) { m_prop_color = Gdk::RGBA(colorText); return *this; }
	BulletPointRenderer& set_color(const Gdk::RGBA& rgba) { m_prop_color = rgba; return *this; }
	Gdk::RGBA get_color() const { return m_prop_color.get_value(); }

	// color for the bullet
	Glib::PropertyProxy<Gdk::RGBA> property_color() { return m_prop_color.get_proxy(); }
	Glib::PropertyProxy_ReadOnly<Gdk::RGBA> property_color() const;

// implementation
protected:

	// sizing overrides
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

	// the bullet color
	Glib::Property<Gdk::RGBA> m_prop_color;
};

#endif  /* __BULLET_POINT_RENDERER_HEADER__ */