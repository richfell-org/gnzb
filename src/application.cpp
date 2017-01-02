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
#include <vector>
#include <gtkmm/settings.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/aboutdialog.h>
#include "application.h"
#include "uiresource.h"
#include "runtimesettings.h"
#include "nntp/fetch.h"
#include "gui/guiutil.h"
#include "gui/appwindow.h"
#include "gui/toolbar/toolbar.h"
#include "gui/nzbtreeview.h"
#include "gui/nzbliststore.h"
#include "gui/menus/menuactions.h"
#include "gui/preferences/preferencesinterface.h"

#include <iostream>

Glib::RefPtr<GNzbApplication> GNzbApplication::get_instance()
{
	return Glib::RefPtr<GNzbApplication>::cast_dynamic(Gtk::Application::get_default());
}

/**
 * Application construction.
 * 
 * The application registers as a unique instance and
 * as such will only have one active GUI running at
 * a time.  Requests to open NZB files are all handled
 * by the one unique instance.
 */
GNzbApplication::GNzbApplication()
:	Glib::ObjectBase("gnzbapplication"),
	Gtk::Application(Glib::ustring("org.richfell.gnzb"), Gio::APPLICATION_HANDLES_OPEN)
{
//	if(!is_registered())
		register_application();
}

GNzbApplication::~GNzbApplication()
{
}

/**
 * Allocates the main window for this instance.
 * 
 */
void GNzbApplication::allocate_main_window()
{
	std::cout << "GNzbApplication::allocate_main_window" << std::endl;

	try
	{
		// load the main UI definition resource
		Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_resource(UiResourcePath("gui/mainwin.ui"));

		// get the main window and add the application icons to its icon list
		m_ptr_mainwin = get_widget_derived<GNzbApplicationWindow>(builder, "gnzb.mainwin");
		m_ptr_mainwin->set_icon_list({
			Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/app/appicon24.png")),
			Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/app/appicon32.png")),
			Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/app/appicon64.png"))});

		// add the main window to this application instance
		add_window(*m_ptr_mainwin.get());

		// connect with the GNzbAction signal
		m_ptr_mainwin->get_tree_view()->signal_action().connect(
			sigc::mem_fun(*this, &GNzbApplication::on_gnzb_action));

		// set toolbar handlers for application level actions
		GNzbToolbar *p_toolbar = m_ptr_mainwin->get_toolbar();
		if(nullptr != p_toolbar)
		{
			// edit/preferences
			Glib::RefPtr<Gio::Action> ref_action = lookup_action(ma_prefs.name);
			if(ref_action)
			{
				ref_action->reference();
				p_toolbar->set_clicked_action(GNzbToolbar::PREFS, ref_action);
			}

			// help/about
			ref_action = lookup_action(ma_help_about.name);
			if(ref_action)
			{
				ref_action->reference();
				p_toolbar->set_clicked_action(GNzbToolbar::ABOUT, ref_action);
			}
		}
	}
	catch(const Glib::Error& e)
	{
		Gtk::MessageDialog error_dialog(e.what(), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
		error_dialog.run();
	}
}

/**
 * Called only once (via framework) and only for the
 * primary instance.
 * 
 */
void GNzbApplication::on_startup()
{
	std::cout << "GNzbApplication::on_startup" << std::endl;

	Gtk::Application::on_startup();
	create_actions();
	Gtk::Settings::get_default()->property_gtk_shell_shows_app_menu() = true;
	create_app_menu();
	create_menubar();
}

/**
 * Called when the application is launched without
 * any non-option command line arguments.
 */
void GNzbApplication::on_activate()
{
	std::cout << "GNzbApplication::on_activate" << std::endl;

	//Gtk::Application::on_activate();
	allocate_main_window();
	m_ptr_mainwin->set_visible();
}

/**
 * Called when the application is launched with
 * some non-option command line arguments.  The
 * arguments are considered to be file names.
 */
void GNzbApplication::on_open_file(const type_vec_files& files, const Glib::ustring& hint)
{
	std::cout << "GNzbApplication::on_open_file" << std::endl;

	if(!m_ptr_mainwin)
	{
		allocate_main_window();
		m_ptr_mainwin->set_visible();
	}

	// open given files
	for(auto& file : files)
		m_ptr_mainwin->open_nzb_file(file->get_path());
}

/**
 * Handler for the Help/About menu item
 */
void GNzbApplication::on_help_about()
{
	try
	{
		// build the dialog instance from the UI definition
		Glib::RefPtr<Gtk::Builder> builder
			= Gtk::Builder::create_from_resource(UiResourcePath("gui/about/about.ui"));

		// access the about dialog box instance
		std::unique_ptr<Gtk::AboutDialog> ptr_about = get_widget<Gtk::AboutDialog>(builder, "about_dialog");

		// set the icon
		ptr_about->set_logo(Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/app/appicon64.png")));
		ptr_about->set_transient_for(*m_ptr_mainwin.get());

		// run the dialog, hide it when done
		ptr_about->run();
		ptr_about->hide();
	}
	catch(const Glib::Error& e)
	{
		//std::cerr << "EnzybeeApplication::on_help_about: " << e.what() << std::endl;
		Gtk::MessageDialog errDialog(
			*m_ptr_mainwin.get(),
			Glib::ustring(e.what()),
			false,
			Gtk::MESSAGE_ERROR,
			Gtk::BUTTONS_OK,
			true);
		errDialog.run();
	}
}

/**
 * Handler for the Edit/Preferences menu item
 */
void GNzbApplication::on_preferences()
{
	/*
	Gtk::MessageDialog errDialog(
		*m_ptr_mainwin.get(),
		Glib::ustring("PREFERENCES!"),
		false,
		Gtk::MESSAGE_ERROR,
		Gtk::BUTTONS_OK,
		true);
	errDialog.run();
	*/

	// run the preferences dialog window
	PreferencesInterface prefs;
	PreferencesInterface::CategoryBitset dirty_prefs = prefs.run(m_ptr_mainwin.get());

	// no changes?
	if(!dirty_prefs.any()) return;

	AppPreferences app_prefs;

	// check for changes to the NNTP servers
	if(dirty_prefs[PreferencesInterface::PREF_SERVERS])
	{
		std::vector<PrefsNntpServer> nntp_servers = app_prefs.get_nntp_servers();

		// purge current fetch pools
		NntpFetch::purge_server_pools(nntp_servers);

		// add/modify NNTP server fetch pools
		NntpFetch::update_server_pools(nntp_servers);

		// update the NNTP connections info
		m_ptr_mainwin->update_connection_info();
	}

	// changes to other settings?
	if(dirty_prefs[PreferencesInterface::PREF_LOCATION])
		RuntimeSettings::locations().load(app_prefs);
	if(dirty_prefs[PreferencesInterface::PREF_NOTIFICATIONS])
		RuntimeSettings::notifications().load(app_prefs);
	if(dirty_prefs[PreferencesInterface::PREF_SCRIPTS])
		RuntimeSettings::scripting().load(app_prefs);
}

/**
 * Handler for the File/Quit menu item
 */
void GNzbApplication::on_quit()
{
	if(m_ptr_mainwin)
		m_ptr_mainwin->hide();	
}

/**
 * Creates Gio::Actions for all items handled by the
 * Gtk::Application instance:
 *
 * - Preferences
 * - About
 * - Quit
 * 
 */
void GNzbApplication::create_actions()
{
	add_action(ma_prefs.name, sigc::mem_fun(*this, &GNzbApplication::on_preferences));
	add_action(ma_help_about.name, sigc::mem_fun(*this, &GNzbApplication::on_help_about));
	add_action(ma_quit.name, sigc::mem_fun(*this, &GNzbApplication::on_quit));
}

/**
 * Creates the application level menu.  That is
 * the one shown on the shell tool bar.
 */
void GNzbApplication::create_app_menu()
{
	MenuSection prefs_section({&ma_prefs, &ma_help_about});
	MenuSection quit_section({&ma_quit});

	// create the application menu
	Glib::RefPtr<AppMenu> ref_appmenu = Glib::RefPtr<AppMenu>(new AppMenu({ &ma_file_open, &prefs_section, &quit_section }));
	ref_appmenu->reference();

	// set the app menu
	set_app_menu(Glib::RefPtr<Gio::Menu>::cast_dynamic(ref_appmenu));
}

/**
 * Creates the menu which is displayed within the
 * application's main window.
 * 
 */
void GNzbApplication::create_menubar()
{
	MenuSection p_r_section({&ma_file_pause_all, &ma_file_resume_all, &ma_file_cancel_all});
	MenuSection quit_section({&ma_quit});

	SubMenu file("File", {&ma_file_open, &p_r_section, &quit_section});
	SubMenu edit("Edit", {&ma_prefs});
	SubMenu view("View", {&ma_view_statistics, &ma_view_details, &ma_view_summary});
	SubMenu help("Help", {&ma_help_about});

	Glib::RefPtr<AppMenu> ref_menubar = Glib::RefPtr<AppMenu>(new AppMenu({&file, &edit, &view, &help}));
	ref_menubar->reference();
	set_menubar(ref_menubar);
}
