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
#include <algorithm>
#include <stdio.h>
#include "gnzbcellrenderer.h"
#include "../../gnzb.h"
#include "../../util/recttools.h"
#include "../../util/sizeformat.h"
#include <gdkmm/rgba.h>
#include <gtkmm/settings.h>

//#include <iostream>

//static std::ostream& operator <<(std::ostream& out, const Gdk::Rectangle& r)
//{
//	out << '(' << r.get_x() << ',' << r.get_y() << ") " << r.get_width() << 'x' << r.get_height();
//	return out;
//}

GNzbCellRenderer::GNzbCellRenderer(int bullet_dim/* = 18*/)
:   Glib::ObjectBase("gnzbcellrenderer"),
	Gtk::CellRenderer(),
	m_prop_nzb(*this, "nzb"),
	m_prop_nzb_name(*this, "nzb_name"),
	m_prop_bullet_rgba_text(*this, "bullet_rgba_text", "rgba(128,128,128,1.0)"),
	m_prop_margin(*this, "margin", 2)
{

	// configure the name renderer
	m_name_renderer.property_font_desc() = Pango::FontDescription(Gtk::Settings::get_default()->property_gtk_font_name());
	m_name_renderer.property_size() = (m_name_renderer.property_size() + (2 * PANGO_SCALE));
	m_name_renderer.property_weight() = Pango::WEIGHT_MEDIUM;
	m_name_renderer.property_ellipsize() = Pango::ELLIPSIZE_MIDDLE;

	// configure the status line renderer
	m_status_renderer.property_size() = (m_name_renderer.property_font_desc().get_value().get_size() / 1.5);

	// configure the bullet renderer
	m_bullet_renderer.set_fixed_size(bullet_dim, bullet_dim);
	m_bullet_renderer.set_padding(2, 2);
	m_bullet_renderer.set_alignment(0.0, 0.5);
}

GNzbCellRenderer::~GNzbCellRenderer()
{
}

Glib::PropertyProxy_ReadOnly<std::shared_ptr<GNzb>> GNzbCellRenderer::property_nzb() const
{
	return Glib::PropertyProxy_ReadOnly<std::shared_ptr<GNzb>>(this, m_prop_nzb.get_name().c_str());
}

Glib::PropertyProxy_ReadOnly<Glib::ustring> GNzbCellRenderer::property_nzb_name() const
{
	return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, m_prop_nzb_name.get_name().c_str());
}

Glib::PropertyProxy_ReadOnly<Glib::ustring> GNzbCellRenderer::property_bullet_rgba_text() const
{
	return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, m_prop_bullet_rgba_text.get_name().c_str());
}

Glib::PropertyProxy_ReadOnly<int> GNzbCellRenderer::property_margin() const
{
	return Glib::PropertyProxy_ReadOnly<int>(this, m_prop_margin.get_name().c_str());
}

Gtk::SizeRequestMode GNzbCellRenderer::get_request_mode_vfunc() const
{
	return m_name_renderer.get_request_mode();
}

void GNzbCellRenderer::get_preferred_width_vfunc(
	Gtk::Widget& widget, int& minimum_width, int& natural_width) const
{
	int min{0}, natural{0};

	// reset to 0
	minimum_width = natural_width = 0;

	// add the widths of the appropiate renderers
	// name and bullet widths need to be added together
	m_name_renderer.get_preferred_width(widget, min, natural);
	minimum_width = min;
	natural_width = natural;
	m_bullet_renderer.get_preferred_width(widget, min, natural);
	minimum_width += min;
	natural_width += natural;

	// progress 
	m_progress_renderer.get_preferred_width(widget, min, natural);
	minimum_width = std::max(min, minimum_width);
	natural_width = std::max(natural, natural_width);

	// status
	m_status_renderer.get_preferred_width(widget, min, natural);
	minimum_width = std::max(min, minimum_width);
	natural_width = std::max(natural, natural_width);

	const int pad_width = 2 * property_xpad();
	minimum_width += pad_width;
	natural_width += pad_width;
}

void GNzbCellRenderer::get_preferred_height_for_width_vfunc(
	Gtk::Widget& widget, int width, int& minimum_height, int& natural_height) const
{
	int min{0}, natural{0};

	// reset to 0
	minimum_height = natural_height = 0;

	// add the heights of the appropiate renderers
	// take max height of name or bullet render as their contibution to height
	m_name_renderer.get_preferred_height_for_width(widget, width, min, natural);
	minimum_height = min + m_prop_margin;
	natural_height = natural + m_prop_margin;
	m_bullet_renderer.get_preferred_height_for_width(widget, width, min, natural);
	minimum_height = std::max(min + m_prop_margin, minimum_height);
	natural_height = std::max(natural + m_prop_margin, natural_height);
	

	m_progress_renderer.get_preferred_height_for_width(widget, width, min, natural);
	minimum_height += (min + m_prop_margin);
	natural_height += (natural + m_prop_margin);

	m_status_renderer.get_preferred_height_for_width(widget, width, min, natural);
	minimum_height += min;
	natural_height += natural;

	const int pad_height = 2 * property_ypad();
	minimum_height += pad_height;
	natural_height += pad_height;
}

void GNzbCellRenderer::get_preferred_height_vfunc(
	Gtk::Widget& widget, int& minimum_height, int& natural_height) const
{
	int min{0}, natural{0};

	// reset to 0
	minimum_height = natural_height = 0;

	// add the heights of the appropiate renderers
	m_name_renderer.get_preferred_height(widget, min, natural);
	minimum_height = (min + m_prop_margin);
	natural_height = (natural + m_prop_margin);
	m_bullet_renderer.get_preferred_height(widget, min, natural);
	minimum_height = std::max(min + m_prop_margin, minimum_height);
	natural_height = std::max(natural + m_prop_margin, natural_height);

	m_progress_renderer.get_preferred_height(widget, min, natural);
	minimum_height += (min + m_prop_margin);
	natural_height += (natural + m_prop_margin);

	m_status_renderer.get_preferred_height(widget, min, natural);
	minimum_height += min;
	natural_height += natural;

	const int pad_height = 2 * property_ypad();
	minimum_height += pad_height;
	natural_height += pad_height;
}

void GNzbCellRenderer::get_preferred_width_for_height_vfunc(
	Gtk::Widget& widget, int height, int& minimum_width, int& natural_width) const
{
	int min{0}, natural{0};

	// reset to 0
	minimum_width = natural_width = 0;

	// add the widths of the appropiate renderers
	m_name_renderer.get_preferred_width_for_height(widget, height, min, natural);
	minimum_width = min;
	natural_width = natural;
	m_bullet_renderer.get_preferred_width(widget, min, natural);
	minimum_width += min;
	natural_width += natural;

	m_progress_renderer.get_preferred_width_for_height(widget, height, min, natural);
	minimum_width = std::max(min, minimum_width);
	natural_width = std::max(natural, natural_width);

	m_status_renderer.get_preferred_width_for_height(widget, height, min, natural);
	minimum_width = std::max(min, minimum_width);
	natural_width = std::max(natural, natural_width);

	const int pad_width = 2 * property_xpad();
	minimum_width += pad_width;
	natural_width += pad_width;
}

void GNzbCellRenderer::render_vfunc(
	const Cairo::RefPtr<Cairo::Context>& cr,
	Gtk::Widget& widget,
	const Gdk::Rectangle& background_area,
	const Gdk::Rectangle& cell_area,
	Gtk::CellRendererState flags)
{
	//cr->save();

	// paint the background if necessary
	if(property_cell_background_set())
	{
		Gdk::RGBA bg_color = property_cell_background_rgba();
		cr->begin_new_sub_path();
		cr->set_source_rgba(bg_color.get_red(), bg_color.get_green(), bg_color.get_blue(), bg_color.get_alpha());
		cr->rectangle(
			background_area.get_x(), background_area.get_y(),
			background_area.get_width(), background_area.get_height());
		cr->fill();
	}

	// get the height for the name renderer
	int area_height = std::max(get_cellrenderer_height(m_name_renderer, widget), get_cellrenderer_height(m_bullet_renderer, widget));
	
	// BG and Cell rect for the bullet and name
	Gdk::Rectangle upper_area(
		cell_area.get_x(), cell_area.get_y(),
		cell_area.get_width(), area_height);

	// render the bullet
	const int bullet_width = m_bullet_renderer.property_width() + (2 * m_bullet_renderer.property_xpad());
	Gdk::Rectangle bullet_area(upper_area.get_x(), upper_area.get_y(), bullet_width, upper_area.get_height());
	m_bullet_renderer.property_color() = Gdk::RGBA(m_prop_bullet_rgba_text.get_value());
	cr->save();
	m_bullet_renderer.render(cr, widget, bullet_area, bullet_area, flags);
	cr->restore();

	// render the name
	rectAdjustArea(upper_area, -bullet_width, 0);
	rectAdjustOrigin(upper_area, bullet_width, 0);
	m_name_renderer.property_text() = m_prop_nzb_name.get_value();
	m_name_renderer.render(cr, widget, upper_area, upper_area, flags);

	// reference our GNzb instance
	std::shared_ptr<GNzb> ptr_gnzb = m_prop_nzb;

	// render the progress
	area_height = get_cellrenderer_height(m_progress_renderer, widget);
	Gdk::Rectangle prog_area(
		cell_area.get_x(), rectGetBottom(upper_area) + m_prop_margin,
		cell_area.get_width(), area_height);
	m_progress_renderer.property_value() = ptr_gnzb->download_data().get_percentage_complete();
	m_progress_renderer.render(cr, widget, prog_area, prog_area, flags);

	// render the status items
	int status_top = rectGetBottom(prog_area) + m_prop_margin;
	Gdk::Rectangle status_area(
		cell_area.get_x(), status_top,
		cell_area.get_width(), rectGetBottom(cell_area) - status_top);

	// the state
	m_status_renderer.property_text() = get_gnzb_state_display_name(ptr_gnzb->state());
	m_status_renderer.property_xalign() = 0.0;
	m_status_renderer.render(cr, widget, status_area, status_area, flags);

	// the DL and total size info
	char total[16], so_far[16];
	unsigned long scale_factor = formatULongBytes(total, ptr_gnzb->collection_meta().total_size());
	sprintf(so_far, "%.02f", double(ptr_gnzb->download_data().current_size()) / double(scale_factor));

	m_status_renderer.property_text()
		= Glib::ustring::compose("%1/%2    %3%%", so_far, total, m_progress_renderer.property_value().get_value()); 
	m_status_renderer.property_xalign() = 1.0;
	m_status_renderer.render(cr, widget, status_area, status_area, flags);

	//cr->restore();
}

// class helper function
int GNzbCellRenderer::get_cellrenderer_height(const Gtk::CellRenderer& cell_renderer, Gtk::Widget& w)
{
	int min_height, nat_height;
	cell_renderer.get_preferred_height(w, min_height, nat_height);
	return min_height;
}
