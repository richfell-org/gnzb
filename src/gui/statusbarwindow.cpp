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
#include "statusbarwindow.h"

NameValueBox::NameValueBox()
:   Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 3)
{
	pack_start(m_name, Gtk::PACK_SHRINK);
	pack_start(m_value, Gtk::PACK_SHRINK);
}

void NameValueBox::set_name(const Glib::ustring& name)
{
	m_name.set_text(name);
}

void NameValueBox::set_value(const Glib::ustring& value)
{
	m_value.set_text(value);
}

void NameValueBox::set_value(int value)
{
	set_value(Glib::ustring::compose("%1", value));
}

// StatusBarWindow item labels
const Glib::ustring StatusBarWindow::sItemLabels[ITEM_COUNT] = {
	Glib::ustring("Downloading:"),
	Glib::ustring("Queued:"),
	Glib::ustring("Paused:"),
	Glib::ustring("Finished:"),
};

StatusBarWindow::StatusBarWindow()
{
	for(int i = 0; i < ITEM_COUNT; ++i)
		m_values[i] = 0;
	init();
}

StatusBarWindow::StatusBarWindow(const Gdk::RGBA& bgColor)
{
	for(int i = 0; i < ITEM_COUNT; ++i)
		m_values[i] = 0;
	init();
	override_background_color(bgColor);
}

void StatusBarWindow::set_item_value(Item i, int value)
{
	m_values[i] = value;
	m_items[i].set_value(m_values[i]);
}

int StatusBarWindow::incrementItemValue(Item i)
{
	m_items[i].set_value(++m_values[i]);
	return m_values[i];
}

int StatusBarWindow::decrementItemValue(Item i)
{
	if(0 < m_values[i])
		m_items[i].set_value(--m_values[i]);
	return m_values[i];
}

void StatusBarWindow::init()
{
	m_main_box.set_spacing(12);

	for(int i = 0; i < ITEM_COUNT; ++i)
	{
		m_items[i].set_name(sItemLabels[i]);
		m_items[i].set_value(m_values[i]);
		m_main_box.pack_start(m_items[i], Gtk::PACK_SHRINK);
	}

	add(m_main_box);
	m_main_box.show_all_children();
	m_main_box.show();
}