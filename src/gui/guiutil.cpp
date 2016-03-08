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
#include "guiutil.h"
#include <stdexcept>
#include <glibmm/ustring.h>
#include <giomm/file.h>
#include <giomm/fileinfo.h>
#include <giomm/contenttype.h>
#include <gtkmm/container.h>
#include <gtkmm/widget.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/application.h>
#include <gtkmm/main.h>
#include <gstreamermm/playbin2.h>

void set_widget(Gtk::Widget& w, const Glib::RefPtr<Gtk::Builder>& ref_builder, const char *parent_name)
{
	Gtk::Container *pParent = 0;
	ref_builder->get_widget(parent_name, pParent);
	if(0 == pParent)
		//std::cerr << "setWidget: container \"" << parentName << "\" not found." << std::endl;
		throw std::runtime_error(std::string("widget not found in builder: ") + parent_name);
	else
	{
		pParent->add(w);
		w.show();
	}
}

WaitCursorTool::WaitCursorTool(Gdk::CursorType cursorType/* = Gdk::WATCH*/)
{
	Glib::RefPtr<Gtk::Application> ref_app = Glib::RefPtr<Gtk::Application>::cast_dynamic(Gtk::Application::get_default());
	if(ref_app)
	{
		ref_app->reference();
		mRefGdkWin = (ref_app->get_windows()[0])->get_window();
		if(mRefGdkWin)
		{
			mRefGdkWin->set_cursor(Gdk::Cursor::create(mRefGdkWin->get_display(), cursorType));
			mRefGdkWin->flush();
			for(int i = 0; i < 5 && Gtk::Main::events_pending(); ++i)
				Gtk::Main::iteration();
		}
	}
}

WaitCursorTool::WaitCursorTool(Gtk::Widget& widget, Gdk::CursorType cursorType/* = Gdk::WATCH*/)
:   WaitCursorTool(widget.get_window(), cursorType)
{
}

WaitCursorTool::WaitCursorTool(Glib::RefPtr<Gdk::Window> refGdkWin, Gdk::CursorType cursorType/* = Gdk::WATCH*/)
:   mRefGdkWin(refGdkWin)
{
	if(mRefGdkWin)
	{
		mRefGdkWin->set_cursor(Gdk::Cursor::create(mRefGdkWin->get_display(), cursorType));
		mRefGdkWin->flush();
		for(int i = 0; i < 5 && Gtk::Main::events_pending(); ++i)
			Gtk::Main::iteration();
	}
}

WaitCursorTool::~WaitCursorTool()
{
	if(mRefGdkWin)
	{
		mRefGdkWin->set_cursor();
		for(int i = 0; i < 5 && Gtk::Main::events_pending(); ++i)
			Gtk::Main::iteration();
	}
}

Glib::RefPtr<Gio::Icon> get_icon_for_path(const std::string& path)
{
	Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(path);
	Glib::RefPtr<Gio::FileInfo> fileInfo = file->query_info();
	return fileInfo->get_icon();
}

Glib::RefPtr<Gio::Icon> get_icon_for_name(const std::string& name)
{
	bool is_uncertain;
	Glib::ustring mime_type = Gio::content_type_guess(name, "", is_uncertain);
	return Gio::content_type_get_icon(mime_type);
}

// PlayBin2 instance for sound notifications
static Glib::RefPtr<Gst::PlayBin2> ref_playbin(nullptr);

void play_sound(const std::string& path)
{
	if(!path.empty())
	{
		if(!ref_playbin)
			ref_playbin = Gst::PlayBin2::create();
		else
			ref_playbin->set_state(Gst::STATE_READY);
		ref_playbin->property_uri() = "file://" + path;
		ref_playbin->set_state(Gst::STATE_PLAYING);
	}
}

void show_info(const Glib::ustring& msg, bool useMarkup/* = false*/, bool modal/* = true*/)
{
	Glib::RefPtr<Gtk::Application> ref_app = Glib::RefPtr<Gtk::Application>::cast_dynamic(Gtk::Application::get_default());
	if(ref_app)
	{
		ref_app->reference();
		show_info(*(ref_app->get_windows()[0]), msg, useMarkup, modal);
	}
	else
	{
		Gtk::MessageDialog dialog(msg, useMarkup, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, modal);
		dialog.run();
	}
}

void show_info(const Glib::ustring& msg, const Glib::ustring& secondaryMsg, bool useMarkup/* = false*/, bool modal/* = true*/)
{
	Glib::RefPtr<Gtk::Application> ref_app = Glib::RefPtr<Gtk::Application>::cast_dynamic(Gtk::Application::get_default());
	if(ref_app)
	{
		ref_app->reference();
		show_info(*(ref_app->get_windows()[0]), msg, secondaryMsg, useMarkup, modal);
	}
	else
	{
		Gtk::MessageDialog dialog(msg, useMarkup, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, modal);
		dialog.set_secondary_text(secondaryMsg);
		dialog.run();
	}
}

void show_info(Gtk::Window& parent, const Glib::ustring& msg, bool useMarkup/* = false*/, bool modal/* = true*/)
{
	Gtk::MessageDialog dialog(parent, msg, useMarkup, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, modal);
	dialog.run();
}

void show_info(Gtk::Window& parent, const Glib::ustring& msg, const Glib::ustring& secondaryMsg, bool useMarkup/* = false*/, bool modal/* = true*/)
{
	Gtk::MessageDialog dialog(parent, msg, useMarkup, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, modal);
	dialog.set_secondary_text(secondaryMsg);
	dialog.run();
}

void show_error(const Glib::ustring& msg, bool useMarkup/* = false*/, bool modal/* = true*/)
{
	Glib::RefPtr<Gtk::Application> ref_app = Glib::RefPtr<Gtk::Application>::cast_dynamic(Gtk::Application::get_default());
	if(ref_app)
	{
		ref_app->reference();
		show_error(*(ref_app->get_windows()[0]), msg, useMarkup, modal);
	}
	else
	{
		Gtk::MessageDialog errDialog(msg, useMarkup, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, modal);
		errDialog.run();
	}
}

void show_error(const Glib::ustring& msg, const Glib::ustring& secondaryMsg, bool useMarkup/* = false*/, bool modal/* = true*/)
{
	Glib::RefPtr<Gtk::Application> ref_app = Glib::RefPtr<Gtk::Application>::cast_dynamic(Gtk::Application::get_default());
	if(ref_app)
	{
		ref_app->reference();
		show_error(*(ref_app->get_windows()[0]), msg, secondaryMsg, useMarkup, modal);
	}
	else
	{
		Gtk::MessageDialog errDialog(msg, useMarkup, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, modal);
		errDialog.set_secondary_text(secondaryMsg);
		errDialog.run();
	}
}

void show_error(Gtk::Window& parent, const Glib::ustring& msg, bool useMarkup/* = false*/, bool modal/* = true*/)
{
	Gtk::MessageDialog errDialog(parent, msg, useMarkup, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, modal);
	errDialog.run();
}

void show_error(Gtk::Window& parent, const Glib::ustring& msg, const Glib::ustring& secondaryMsg, bool useMarkup/* = false*/, bool modal/* = true*/)
{
	Gtk::MessageDialog errDialog(parent, msg, useMarkup, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, modal);
	errDialog.set_secondary_text(secondaryMsg);
	errDialog.run();
}

int get_yes_or_no(Gtk::Window& parent, const Glib::ustring& msg, bool useMarkup/* = false*/, bool modal/* = true*/)
{
	Gtk::MessageDialog dialog(parent, msg, useMarkup, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, modal);
	return dialog.run();
}

int get_yes_or_no(Gtk::Window& parent, const Glib::ustring& msg, const Glib::ustring& secondaryMsg, bool useMarkup/* = false*/, bool modal/* = true*/)
{
	Gtk::MessageDialog dialog(parent, msg, useMarkup, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, modal);
	dialog.set_secondary_text(secondaryMsg);
	return dialog.run();
}

const int YES_WITH_DELETE = Gtk::RESPONSE_ACCEPT;

int get_yes_or_no_or_yeswithdelete(Gtk::Window& parent, const Glib::ustring& msg, bool useMarkup/* = false*/, bool modal/* = true*/)
{
	Gtk::MessageDialog dialog(parent, msg, useMarkup, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE, modal);
	dialog.add_button("No", Gtk::RESPONSE_NO);
	dialog.add_button("Yes", Gtk::RESPONSE_YES);
	dialog.add_button("Yes and delete files", YES_WITH_DELETE);
	return dialog.run();
}

int get_yes_or_no_or_yeswithdelete(Gtk::Window& parent, const Glib::ustring& msg, const Glib::ustring& secondaryMsg, bool useMarkup/* = false*/, bool modal/* = true*/)
{
	Gtk::MessageDialog dialog(parent, msg, useMarkup, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE, modal);
	dialog.add_button("No", Gtk::RESPONSE_NO);
	dialog.add_button("Yes", Gtk::RESPONSE_YES);
	dialog.add_button("Yes and delete files", YES_WITH_DELETE);
	dialog.set_secondary_text(secondaryMsg);
	return dialog.run();
}