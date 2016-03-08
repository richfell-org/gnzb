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
#include "preferencesinterface.h"
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <gtkmm/builder.h>
#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/messagedialog.h>
#include "../guiutil.h"
#include "../../uiresource.h"

PreferencesInterface::PreferencesInterface()
:   mDirtyCategories(0), mpPrefsDialog(nullptr)
{
}

PreferencesInterface::~PreferencesInterface()
{
	if(nullptr != mpPrefsDialog)
	{
		delete mpPrefsDialog;
		mpPrefsDialog = nullptr;
	}
}

PreferencesInterface::CategoryBitset PreferencesInterface::run(Gtk::Window *pParentWin/* = 0*/)
{
	// build the GUI from the XML definition
	Glib::RefPtr<Gtk::Builder> ref_builder
		= Gtk::Builder::create_from_resource(UiResourcePath("gui/preferences/preferences.ui"));

	// get the instantiated Gtk::Dialog instance
	ref_builder->get_widget("preferences.dialog", mpPrefsDialog);
	if(0 != pParentWin)
		mpPrefsDialog->set_transient_for(*pParentWin);

	// initalize the signal handlers
	// ...
	Gtk::Button *pButton = 0;
	ref_builder->get_widget("preferences.save.btn", pButton);
	pButton->signal_clicked().connect(sigc::bind<Gtk::Dialog*>(
		sigc::mem_fun(*this, &PreferencesInterface::on_save_preferences), mpPrefsDialog));

	ref_builder->get_widget("preferences.cancel.btn", pButton);
	pButton->signal_clicked().connect(sigc::bind<Gtk::Dialog*>(
		sigc::mem_fun(*this, &PreferencesInterface::on_cancel_preferences), mpPrefsDialog));

	// initialize dialog box widgets with the current settings
	m_servers_tab.init(m_app_prefs, ref_builder);
	mLocationsTab.init(m_app_prefs, ref_builder);
	mNotificationsTab.init(m_app_prefs, ref_builder);
	mScriptsTab.init(m_app_prefs, ref_builder);

	// run the dialog
	int dialogResult = mpPrefsDialog->run();
	switch(dialogResult)
	{
		case Gtk::RESPONSE_CANCEL:
			mDirtyCategories.reset();   // on cancel remove all "dirty" indicators
		case Gtk::RESPONSE_ACCEPT:
			mpPrefsDialog->hide();
			delete mpPrefsDialog;
			mpPrefsDialog = 0;
			break;
		default:
			break;
	}

	return mDirtyCategories;
}

void PreferencesInterface::on_save_preferences(Gtk::Dialog *pPrefsDialog)
{
	WaitCursorTool waitCursor(*pPrefsDialog);

	// NNTP servers modified?
	if(m_servers_tab.is_modified())
	{
		mDirtyCategories.set(PREF_SERVERS, true);
		m_servers_tab.save(m_app_prefs);
	}

	// download directories/actions modified
	if(mLocationsTab.is_modified())
	{
		mDirtyCategories[PREF_LOCATION] = true;
		mLocationsTab.save(m_app_prefs);
	}

	// notifications modified
	if(mNotificationsTab.is_modified())
	{
		mDirtyCategories[PREF_NOTIFICATIONS] = true;
		mNotificationsTab.save(m_app_prefs);
	}

	// sctipting options modified?
	if(mScriptsTab.is_modified())
	{
		mDirtyCategories[PREF_SCRIPTS] = true;
		mScriptsTab.save(m_app_prefs);
	}

	// set the response for the dialog
	pPrefsDialog->response(Gtk::RESPONSE_ACCEPT);
}

void PreferencesInterface::on_cancel_preferences(Gtk::Dialog *pPrefsDialog)
{
	pPrefsDialog->response(Gtk::RESPONSE_CANCEL);
}