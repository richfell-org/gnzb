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
#ifndef __STATUS_BAR_WINDOW_HEADER__
#define __STATUS_BAR_WINDOW_HEADER__

#include <glibmm/ustring.h>
#include <gdkmm/rgba.h>
#include <gtkmm/box.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/label.h>

/*
 * 
 */
class NameValueBox : public Gtk::Box
{
public:

	NameValueBox();
	~NameValueBox() {}

// attributes
public:

	Gtk::Label& getNameLabel() { return m_name; }
	const Gtk::Label& getNameLabel() const { return m_name; }

	Gtk::Label& getValueLabel() { return m_value; }
	const Gtk::Label& getValueLabel() const { return m_value; }

// operations
public:

	void set_name(const Glib::ustring& name);
	Glib::ustring get_name() const { return m_name.get_text(); }

	void set_value(int value);
	void set_value(const Glib::ustring& value);
	Glib::ustring get_value() const { return m_value.get_text(); }

// implementation
protected:

	Gtk::Label m_name, m_value;
};

/*
 * 
 */
class StatusBarWindow : public Gtk::EventBox
{
// construction
public:

	StatusBarWindow();
	StatusBarWindow(const Gdk::RGBA& bgColor);
	~StatusBarWindow() {}

// operations
public:

	// download count manipulation
	int getDownloadCount() const { return m_values[DOWNLOADING]; }
	void setDownloadCount(int value) { set_item_value(DOWNLOADING, value); }
	int incrementDownloadCount() { return incrementItemValue(DOWNLOADING); }
	int decrementDownloadCount() { return decrementItemValue(DOWNLOADING); }

	// queued count manipulation
	int getQueuedCount() const { return m_values[QUEUED]; }
	void setQueuedCount(int value) { set_item_value(QUEUED, value); }
	int incrementQueuedCount() { return incrementItemValue(QUEUED); }
	int decrementQueuedCount() { return decrementItemValue(QUEUED); }

	// paused count manipulation
	int getPausedCount() const { return m_values[PAUSED]; }
	void setPausedCount(int value) { set_item_value(PAUSED, value); }
	int incrementPausedCount() { return incrementItemValue(PAUSED); }
	int decrementPausedCount() { return decrementItemValue(PAUSED); }

	// paused count manipulation
	int getFinishedCount() const { return m_values[FINISHED]; }
	void setFinishedCount(int value) { set_item_value(FINISHED, value); }
	int incrementFinishedCount() { return incrementItemValue(FINISHED); }
	int decrementFinishedCount() { return decrementItemValue(FINISHED); }

// implementation
protected:

	enum Item { DOWNLOADING, QUEUED, PAUSED, FINISHED, ITEM_COUNT, };

	void init();
	void set_item_value(Item i, int value);
	int incrementItemValue(Item i);
	int decrementItemValue(Item i);

	static const Glib::ustring sItemLabels[ITEM_COUNT];

	Gtk::Box m_main_box;
	NameValueBox m_items[ITEM_COUNT];
	int m_values[ITEM_COUNT];
};

#endif  /* __STATUS_BAR_WINDOW_HEADER__ */