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
#ifndef __GUI_UTIL_HEADER__
#define __GUI_UTIL_HEADER__

#include <memory>
#include <glibmm/refptr.h>
#include <gdkmm/cursor.h>
#include <gtkmm/builder.h>

namespace Glib {
	class ustring;
}
namespace Gio {
	class Icon;
}
namespace Gtk {
	class Widget;
}

void set_widget(Gtk::Widget& w, const Glib::RefPtr<Gtk::Builder>& ref_builder, const char *parent_name);

template <typename T>
static std::unique_ptr<T> get_widget(const Glib::RefPtr<Gtk::Builder>& ref_builder, const char* name)
{
	std::unique_ptr<T> result;
	T* p_widget{nullptr};
	ref_builder->get_widget(name, p_widget);
	result.reset(p_widget);
	return result;
}

template <typename T>
static std::unique_ptr<T> get_widget_derived(const Glib::RefPtr<Gtk::Builder>& ref_builder, const char* name)
{
	std::unique_ptr<T> result;
	T* p_widget{nullptr};
	ref_builder->get_widget_derived(name, p_widget);
	result.reset(p_widget);
	return result;
}

template <typename T>
static T* get_managed_widget(const Glib::RefPtr<Gtk::Builder>& ref_builder, const char* name)
{
	T* p_widget{nullptr};
	ref_builder->get_widget(name, p_widget);
	return p_widget;
}

template <typename T>
static T* get_managed_widget_derived(const Glib::RefPtr<Gtk::Builder>& ref_builder, const char* name)
{
	T* p_widget{nullptr};
	ref_builder->get_widget_derived(name, p_widget);
	return p_widget;
}

/*
 * Constructor sets the wait cursor for the window
 * Destructor resets to the default sursor
 */
class WaitCursorTool
{
public:
	WaitCursorTool(Gdk::CursorType cursorType = Gdk::WATCH);
	WaitCursorTool(Gtk::Widget& widget, Gdk::CursorType cursorType = Gdk::WATCH);
	WaitCursorTool(Glib::RefPtr<Gdk::Window> refGdkWin, Gdk::CursorType cursorType = Gdk::WATCH);
	~WaitCursorTool();

private:

	Glib::RefPtr<Gdk::Window> mRefGdkWin;
};

Glib::RefPtr<Gio::Icon> get_icon_for_path(const std::string& path);
Glib::RefPtr<Gio::Icon> get_icon_for_name(const std::string& name);

void play_sound(const std::string& path);

void show_info(const Glib::ustring& msg, bool useMarkup = false, bool modal = true);
void show_info(const Glib::ustring& msg, const Glib::ustring& secondaryMsg, bool useMarkup = false, bool modal = true);
void show_info(Gtk::Window& parent, const Glib::ustring& msg, bool useMarkup = false, bool modal = true);
void show_info(Gtk::Window& parent, const Glib::ustring& msg, const Glib::ustring& secondaryMsg, bool useMarkup = false, bool modal = true);

void show_error(const Glib::ustring& msg, bool useMarkup = false, bool modal = true);
void show_error(const Glib::ustring& msg, const Glib::ustring& secondaryMsg, bool useMarkup = false, bool modal = true);
void show_error(Gtk::Window& parent, const Glib::ustring& msg, bool useMarkup = false, bool modal = true);
void show_error(Gtk::Window& parent, const Glib::ustring& msg, const Glib::ustring& secondaryMsg, bool useMarkup = false, bool modal = true);

int get_yes_or_no(Gtk::Window& parent, const Glib::ustring& msg, bool useMarkup = false, bool modal = true);
int get_yes_or_no(Gtk::Window& parent, const Glib::ustring& msg, const Glib::ustring& secondaryMsg, bool useMarkup = false, bool modal = true);

extern const int YES_WITH_DELETE;

int get_yes_or_no_or_yeswithdelete(Gtk::Window& parent, const Glib::ustring& msg, bool useMarkup = false, bool modal = true);
int get_yes_or_no_or_yeswithdelete(Gtk::Window& parent, const Glib::ustring& msg, const Glib::ustring& secondaryMsg, bool useMarkup = false, bool modal = true);

#endif	/* __GUI_UTIL_HEADER__ */
