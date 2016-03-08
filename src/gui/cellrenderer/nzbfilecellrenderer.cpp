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
#include "nzbfilecellrenderer.h"
#include "../../gnzb.h"
#include "../../util/recttools.h"
#include "../../util/sizeformat.h"
#include <algorithm>
#include <cstdio>

NzbFileCellRenderer::NzbFileCellRenderer()
:   Gtk::CellRenderer(),
	Glib::ObjectBase("nzbfilecellrenderer"),
	m_prop_file_tuple(*this, "file_tuple"),
	m_prop_name(*this, "name"),
	m_prop_margin(*this, "margin", 0)

{
	// configure the name renderer
	m_name_renderer.property_weight() = Pango::WEIGHT_BOLD;
	m_name_renderer.property_ellipsize() = Pango::ELLIPSIZE_MIDDLE;
	Pango::FontDescription font_desc = m_name_renderer.property_font_desc();
	font_desc.set_size(8 * Pango::SCALE);
	m_name_renderer.property_font_desc() = font_desc;

	// configure the status renderer
	font_desc = m_status_renderer.property_font_desc();
	font_desc.set_size(7 * Pango::SCALE);
	m_status_renderer.property_font_desc() = font_desc;
}

NzbFileCellRenderer::~NzbFileCellRenderer()
{
}

Glib::PropertyProxy_ReadOnly<NzbFileTuple> NzbFileCellRenderer::property_file_tuple() const
{
	return Glib::PropertyProxy_ReadOnly<NzbFileTuple>(this, m_prop_file_tuple.get_name().c_str());
}

Glib::PropertyProxy_ReadOnly<Glib::ustring> NzbFileCellRenderer::property_name() const
{
	return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, m_prop_name.get_name().c_str());
}

Glib::PropertyProxy_ReadOnly<int> NzbFileCellRenderer::property_margin() const
{
	return Glib::PropertyProxy_ReadOnly<int>(this, m_prop_margin.get_name().c_str());
}

Gtk::SizeRequestMode NzbFileCellRenderer::get_request_mode_vfunc() const
{
	return m_name_renderer.get_request_mode();
}

void NzbFileCellRenderer::get_preferred_width_vfunc(
	Gtk::Widget& widget, int& minimum_width, int& natural_width) const
{
	int min{0}, natural{0};

	// max width of the vertically stacked renderers
	m_name_renderer.get_preferred_width(widget, min, natural);
	minimum_width = std::max(minimum_width, min);
	natural_width = std::max(natural_width, natural);

	m_status_renderer.get_preferred_width(widget, min, natural);
	minimum_width = std::max(minimum_width, min);
	natural_width = std::max(natural_width, natural);
}

void NzbFileCellRenderer::get_preferred_height_for_width_vfunc(
	Gtk::Widget& widget, int width, int& minimum_height, int& natural_height) const
{
	int min{0}, natural{0};

	minimum_height = natural_height = (2 * property_ypad());

	// add the widths of the appropiate renderers
	m_name_renderer.get_preferred_height_for_width(widget, width, min, natural);
	minimum_height += (min + m_prop_margin);
	natural_height += (natural + m_prop_margin);

	m_status_renderer.get_preferred_height_for_width(widget, width, min, natural);
	minimum_height += min;
	natural_height += natural;
}

void NzbFileCellRenderer::get_preferred_height_vfunc(
	Gtk::Widget& widget, int& minimum_height, int& natural_height) const
{
	int min{0}, natural{0};

	minimum_height = natural_height = (2 * property_ypad());

	// add the widths of the appropiate renderers
	m_name_renderer.get_preferred_height(widget, min, natural);
	minimum_height += (min + m_prop_margin);
	natural_height += (natural + m_prop_margin);

	m_status_renderer.get_preferred_height(widget, min, natural);
	minimum_height += min;
	natural_height += natural;
}

void NzbFileCellRenderer::get_preferred_width_for_height_vfunc(
	Gtk::Widget& widget, int height, int& minimum_width, int& natural_width) const
{
	int min{0}, natural{0};

	// max width of the verticall stacked renderers
	m_name_renderer.get_preferred_width_for_height(widget, height, min, natural);
	minimum_width = std::max(minimum_width, min);
	natural_width = std::max(natural_width, natural);

	m_status_renderer.get_preferred_width_for_height(widget, height, min, natural);
	minimum_width = std::max(minimum_width, min);
	natural_width = std::max(natural_width, natural);
}

void NzbFileCellRenderer::render_vfunc(
	const Cairo::RefPtr<Cairo::Context>& cr,
	Gtk::Widget& widget,
	const Gdk::Rectangle& background_area,
	const Gdk::Rectangle& cell_area,
	Gtk::CellRendererState flags)
{
	cr->save();

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

	int height = get_cellrenderer_height(m_name_renderer, widget);

	// render the file name
	Gdk::Rectangle text_area(
		cell_area.get_x() + property_xpad(),
		cell_area.get_y() + property_ypad(),
		cell_area.get_width() - (2 * property_xpad()),
		height);
	m_name_renderer.property_text() = m_prop_name.get_value();
	m_name_renderer.render(cr, widget, text_area, text_area, flags);

	// adjust the rectangle for the status area
	rectMoveVertical(text_area, height + m_prop_margin);
	text_area.set_height(get_cellrenderer_height(m_status_renderer, widget));

	const NZB::File& nzb_file = *NzbFileTuple(m_prop_file_tuple).nzb_file();
	const FileMeta& file_meta = *NzbFileTuple(m_prop_file_tuple).file_meta();

	// render the state
	if(m_prop_file_tuple.get_value().file_meta() != nullptr)
	{
		char size_buf[16];
		unsigned long scale = formatULongBytes(size_buf, file_meta.total_size());

		if(0 == file_meta.download_size())
		{
			m_status_renderer.property_text() = Glib::ustring::compose(
				"%1 (%2)", (file_meta.state() == GNzbState::CONDITIONAL ? "If needed" : "Waiting"), size_buf);
		}
		else if(file_meta.state() == GNzbState::DOWNLOADING)
		{
			char so_far[16];
			sprintf(so_far, "%.02f", double(file_meta.download_size()) / double(scale));
			Glib::ustring state = (file_meta.state() == GNzbState::DOWNLOADING)
				? "Downloading"
				: (file_meta.state() == GNzbState::STOPPED) ? "Partial" : "Waiting";
			m_status_renderer.property_text() = Glib::ustring::compose("%1 (%2/%3)", state, so_far, size_buf);
			//get_gnzb_state_display_name(file_meta.state());
		}
		else
		{
			m_status_renderer.property_text() = "Complete";
		}

		m_status_renderer.property_xalign() = 0.0;
		m_status_renderer.render(cr, widget, text_area, text_area, flags);
	}

	// render the article info
	if(m_prop_file_tuple.get_value().nzb_file() != nullptr)
	{
		// downloading?
		if(file_meta.state() == GNzbState::DOWNLOADING)
		{
			m_status_renderer.property_text() = Glib::ustring::compose(
				"%1 of %2 article%3",
				file_meta.completed_segments(),
				nzb_file.getSegmentCount(),
				(nzb_file.getSegmentCount() > 1 ? "s" : ""));
		}
		else
		{
			m_status_renderer.property_text() = Glib::ustring::compose(
				"%1 article%2", nzb_file.getSegmentCount(), (nzb_file.getSegmentCount() > 1 ? "s" : ""));
		}

		m_status_renderer.property_xalign() = 1.0;
		m_status_renderer.render(cr, widget, text_area, text_area, flags);
	}

	cr->restore();
}

// class helper function
int NzbFileCellRenderer::get_cellrenderer_height(const Gtk::CellRenderer& cell_renderer, Gtk::Widget& w)
{
	int min_height, nat_height;
	cell_renderer.get_preferred_height(w, min_height, nat_height);
	return min_height;
}