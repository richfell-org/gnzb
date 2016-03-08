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
#include "gnzbactioncellrenderer.h"
#include <mutex>
#include <gdkmm/pixbuf.h>
#include "../../uiresource.h"

// class static icon lists
std::vector<Glib::RefPtr<Gdk::Pixbuf>> GNzbActionCellRenderer::s_action_icons;
std::vector<Glib::RefPtr<Gdk::Pixbuf>> GNzbActionCellRenderer::s_action_icons_pressed;

/**
 * 
 */
void GNzbActionCellRenderer::init_icons()
{
	static bool init_complete = false;
	static std::mutex init_mutex;

	std::lock_guard<std::mutex> init_lock(init_mutex);
	if(!init_complete)
	{
		s_action_icons.clear();
		s_action_icons.push_back(Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/actions/start.png")));
		s_action_icons.push_back(Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/actions/pause.png")));
		s_action_icons.push_back(Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/actions/delete.png")));
		s_action_icons.push_back(Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/actions/show.png")));
		s_action_icons.push_back(Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/actions/finished.png")));

		s_action_icons_pressed.clear();
		s_action_icons_pressed.push_back(Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/actions/start-pressed.png")));
		s_action_icons_pressed.push_back(Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/actions/pause-pressed.png")));
		s_action_icons_pressed.push_back(Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/actions/delete-pressed.png")));
		s_action_icons_pressed.push_back(Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/actions/show-pressed.png")));
		s_action_icons_pressed.push_back(Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/actions/finished-pressed.png")));

		init_complete = true;
	}
}

GNzbActionCellRenderer::GNzbActionCellRenderer()
:   Glib::ObjectBase("nzbactioncellrenderer"),
	Gtk::CellRendererPixbuf(),
	m_prop_action(*this, "action", GNzbAction::START),
	m_prop_pressed(*this, "pressed", false)
{
	property_xpad() = 0;
	property_ypad() = 3;
	set_sensitive();

	init_icons();
}

int GNzbActionCellRenderer::get_image_width() const
{
	return s_action_icons[0]->get_width();
}

int GNzbActionCellRenderer::get_image_height() const
{
	return s_action_icons[0]->get_height();
}

Glib::PropertyProxy_ReadOnly<GNzbAction> GNzbActionCellRenderer::property_action() const
{
	return Glib::PropertyProxy_ReadOnly<GNzbAction>(this, m_prop_action.get_name().c_str());
}

Glib::PropertyProxy_ReadOnly<bool> GNzbActionCellRenderer::property_pressed() const
{
	return Glib::PropertyProxy_ReadOnly<bool>(this, m_prop_pressed.get_name().c_str());
}

Gtk::SizeRequestMode GNzbActionCellRenderer::get_request_mode_vfunc() const
{
	return Gtk::CellRendererPixbuf::get_request_mode_vfunc();
}

void GNzbActionCellRenderer::get_preferred_width_vfunc(
	Gtk::Widget& widget, int& minimum_width, int& natural_width) const
{
	minimum_width = natural_width = (s_action_icons[0]->get_width() + (2 * property_xpad()));
}

void GNzbActionCellRenderer::get_preferred_height_for_width_vfunc(
	Gtk::Widget& widget, int width, int& minimum_height, int& natural_height) const
{
	minimum_height = natural_height = (s_action_icons[0]->get_height() + (2 * property_ypad()));
}

void GNzbActionCellRenderer::get_preferred_height_vfunc(
	Gtk::Widget& widget, int& minimum_height, int& natural_height) const
{
	minimum_height = natural_height = (s_action_icons[0]->get_height() + (2 * property_ypad()));
}

void GNzbActionCellRenderer::get_preferred_width_for_height_vfunc(
	Gtk::Widget& widget, int height, int& minimum_width, int& natural_width) const
{
	minimum_width = natural_width = (s_action_icons[0]->get_width() + (2 * property_xpad()));
}

void GNzbActionCellRenderer::render_vfunc(
	const Cairo::RefPtr<Cairo::Context>& cr,
	Gtk::Widget& widget,
	const Gdk::Rectangle& background_area,
	const Gdk::Rectangle& cell_area,
	Gtk::CellRendererState flags)
{
	// set the needed icon
	if(property_pressed())
		property_pixbuf() = s_action_icons_pressed[int(m_prop_action.get_value())];
	else
		property_pixbuf() = s_action_icons[int(m_prop_action.get_value())];

	// render the icon
	Gtk::CellRendererPixbuf::render_vfunc(cr, widget, background_area, cell_area, flags);
}