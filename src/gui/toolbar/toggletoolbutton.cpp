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
#include <gdkmm/pixbuf.h>
#include "toggletoolbutton.h"
#include "../../uiresource.h"

GNzbToggleToolButton::GNzbToggleToolButton(
	const Glib::ustring& tooltip,
	const std::string& image_resource_path,
    const std::string& image_selected_resource_path,
	int minWidth/* = -1*/)
:   m_image(Gdk::Pixbuf::create_from_resource(image_resource_path)),
	m_image_sel(Gdk::Pixbuf::create_from_resource(image_selected_resource_path))
{
	set_icon_widget(m_image);
	set_tooltip_text(tooltip);
	set_size_request(minWidth);
	set_homogeneous();
	m_image.show();
	m_image_sel.show();
	show();
}

void GNzbToggleToolButton::on_toggled()
{
	if(get_active())
		set_icon_widget(m_image_sel);
	else
		set_icon_widget(m_image);
}