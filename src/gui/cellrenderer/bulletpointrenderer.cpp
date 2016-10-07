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
#include "bulletpointrenderer.h"

BulletPointRenderer::BulletPointRenderer()
:   Glib::ObjectBase("bulletpointrenderer"),
	Gtk::CellRenderer(),
	m_prop_color(*this, "color"),
	//m_highlight(Cairo::RadialGradient::create(0.45, 0.45, 0.1, 0.5, 0.5, 0.5))
	m_highlight()
{
	Cairo::RefPtr<Cairo::RadialGradient> refGradient(Cairo::RadialGradient::create(0.45, 0.45, 0.1, 0.5, 0.5, 0.5));
	refGradient->add_color_stop_rgba(0, 1, 1, 1, 0.2);
	refGradient->add_color_stop_rgba(1, 0, 0, 0, 0.3);
	m_highlight = Cairo::RefPtr<const Cairo::Pattern>::cast_dynamic(refGradient);

	// set the color stops on the highlight radial pattern
	//m_highlight->add_color_stop_rgba(0, 1, 1, 1, 0.2);
	//m_highlight->add_color_stop_rgba(1, 0, 0, 0, 0.3);
}

BulletPointRenderer::~BulletPointRenderer()
{
}

Glib::PropertyProxy_ReadOnly<Gdk::RGBA> BulletPointRenderer::property_color() const
{
	return Glib::PropertyProxy_ReadOnly<Gdk::RGBA>(this, m_prop_color.get_name().c_str());
}

Gtk::SizeRequestMode BulletPointRenderer::get_request_mode_vfunc() const
{
	return Gtk::SIZE_REQUEST_CONSTANT_SIZE;
}

void BulletPointRenderer::get_preferred_width_vfunc(
	Gtk::Widget& widget, int& minimum_width, int& natural_width) const
{
	// set the results
	minimum_width = natural_width = property_width() + (2 * property_xpad());
}

void BulletPointRenderer::get_preferred_height_for_width_vfunc(
	Gtk::Widget& widget, int width, int& minimum_height, int& natural_height) const
{
	// we naturally want a square area
	minimum_height = natural_height = property_height() + (2 * property_ypad());
}

void BulletPointRenderer::get_preferred_height_vfunc(
	Gtk::Widget& widget, int& minimum_height, int& natural_height) const
{
	// we naturally want a square area
	minimum_height = natural_height = property_height() + (2 * property_ypad());
}

void BulletPointRenderer::get_preferred_width_for_height_vfunc(
	Gtk::Widget& widget, int height, int& minimum_width, int& natural_width) const
{
	// we naturally want a square area
	minimum_width = natural_width = property_width() + (2 * property_xpad());
}

void BulletPointRenderer::render_vfunc(
	const Cairo::RefPtr<Cairo::Context>& cr,
	Gtk::Widget& widget,
	const Gdk::Rectangle& background_area,
	const Gdk::Rectangle& cell_area,
	Gtk::CellRendererState flags)
{
	// paint background
	if(property_cell_background_set())
	{
		Gdk::RGBA color = property_cell_background_rgba();
		//cr->begin_new_sub_path();
		cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());
		cr->rectangle(
			background_area.get_x(), background_area.get_y(),
			background_area.get_width(), background_area.get_height());
		cr->fill();
	}

	// calculate the usable cell width/height
	int usable_cell_width = cell_area.get_width() - (2 * property_xpad());
	int usable_cell_height = cell_area.get_height() - (2 * property_ypad());

	// get the width and height for the bullet
	int width = std::min(property_width().get_value(), usable_cell_width);
	int height = std::min(property_height().get_value(), usable_cell_height);

	// get the translation offset adjustment based on the alignment values
	double xalign_offset = double(usable_cell_width - width) * property_xalign();
	double yalign_offset = double(usable_cell_height - height) * property_yalign();

	// set scaling so we can work with coords of 0.0 thru 1.0
	cr->translate(
		cell_area.get_x() + property_xpad() + xalign_offset,
		cell_area.get_y() + property_ypad() + yalign_offset);
	cr->scale(width, height);

	// bullet point base
	cr->begin_new_sub_path();
	cr->set_source_rgba(0.6, 0.6, 0.6, 1.0);
	cr->arc(0.5, 0.5, 0.5, 0.0, 2 * M_PI);
	cr->fill();

	// main bullet point color
	Gdk::RGBA color = m_prop_color;
	cr->begin_new_sub_path();
	cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());
	cr->arc(0.5, 0.5, 0.40, 0.0, 2 * M_PI);
	cr->fill();

	//int exception_count = 0;

	// shading accent
	cr->begin_new_sub_path();
	cr->set_source(m_highlight);
	cr->arc(0.5, 0.5, 0.4, 0.0, 2 * M_PI);
	try { cr->fill(); }
	catch(...)
	{
		//++exception_count;
		throw;
	}
}
