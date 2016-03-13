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
#include <gtkmm/builder.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/cellrendererpixbuf.h>
#include <gtkmm/filechooserdialog.h>
#include "../guiutil.h"
#include "locationsinterface.h"
#include "widgets/folderselectionliststore.h"

PrefsLocationsInterface::PrefsLocationsInterface()
:   mpComboLocation(0), mpToggleAlwaysAsk(0), mpToggleMoveDownloaded(0), mpComboMoveToLocation(0)
{
}

PrefsLocationsInterface::~PrefsLocationsInterface()
{
}

void PrefsLocationsInterface::onDlLocationChanged(Gtk::ComboBox *pComboBox)
{
	Glib::RefPtr<FolderSelectionListStore> dlLocationModel
		= Glib::RefPtr<FolderSelectionListStore>::cast_dynamic(pComboBox->get_model());

	Gtk::TreeIter iter = pComboBox->get_active();
	if((*iter)[dlLocationModel->columns().type()] == FolderSelectionListStore::SELECT)
	{
		//Gtk::FileChooserDialog dirChooser(*mpPrefsDialog, "Select Location...", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
		Gtk::FileChooserDialog dirChooser("Select Location...", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
		dirChooser.set_create_folders();
		dirChooser.add_button("Cancel", Gtk::RESPONSE_CANCEL);
		dirChooser.add_button("Select", Gtk::RESPONSE_ACCEPT);
		switch(dirChooser.run())
		{
			case Gtk::RESPONSE_ACCEPT:
				dlLocationModel->setDirectory(dirChooser.get_filename().c_str());
				dlLocationModel->setDirectoryIcon(get_icon_for_path(dirChooser.get_filename()));
				set_modified();
				// fall through, because the first item is to always be selected
			default:
				pComboBox->set_active(dlLocationModel->children().begin());
				break;
		}
	}
}

void PrefsLocationsInterface::init(const AppPreferences& app_prefs, Glib::RefPtr<Gtk::Builder>& ref_builder)
{
	mpComboLocation = mpComboMoveToLocation = 0;
	mpToggleAlwaysAsk = mpToggleMoveDownloaded = 0;

	ref_builder->get_widget("preferences.locations.downloadlocation.combo", mpComboLocation);
	ref_builder->get_widget("preferences.locations.downloadlocationask.chk", mpToggleAlwaysAsk);
	ref_builder->get_widget("preferences.locations.movecompleted.chk", mpToggleMoveDownloaded);
	ref_builder->get_widget("preferences.locations.moveto.combo", mpComboMoveToLocation);

	// Download location
	Glib::RefPtr<FolderSelectionListStore> dlLocationModel = Glib::RefPtr<FolderSelectionListStore>(new FolderSelectionListStore);

	std::string dlDir = app_prefs.getDownloadDir();
	dlLocationModel->setDirectory(dlDir);
	dlLocationModel->setDirectoryIcon(get_icon_for_path(dlDir));

	mpComboLocation->set_model(dlLocationModel);
	mpComboLocation->set_row_separator_func(sigc::mem_fun(*dlLocationModel.operator->(), &FolderSelectionListStore::isRowSeparator));

	Gtk::CellRendererPixbuf *pPixbufRenderer = new Gtk::CellRendererPixbuf;
	Gtk::CellRendererText *pTextRenderer = new Gtk::CellRendererText;

	mpComboLocation->pack_start(*pPixbufRenderer, false);
	mpComboLocation->pack_end(*pTextRenderer);
	mpComboLocation->add_attribute(*pPixbufRenderer, "gicon", dlLocationModel->columns().icon());
	mpComboLocation->add_attribute(*pTextRenderer, "text", dlLocationModel->columns().text());

	mpComboLocation->set_active(dlLocationModel->children().begin());
	mpComboLocation->signal_changed().connect(
		sigc::bind<Gtk::ComboBox*>(sigc::mem_fun(*this, &PrefsLocationsInterface::onDlLocationChanged), mpComboLocation));

	// will set the locations area as "dirty" and, if provided, set a peer widget's
	// sensitivity according to the active state of the toggle button
	auto setDirtyLambda = [this](Gtk::ToggleButton *pTgl = 0, Gtk::Widget *pPeer = 0)
		{ set_modified(); if(pTgl && pPeer) pPeer->set_sensitive(pTgl->get_active()); };

	// always ask for DL location
	mpToggleAlwaysAsk->set_active(app_prefs.getAlwayAskForDir());
	mpToggleAlwaysAsk->signal_toggled().connect(setDirtyLambda);

	// "Move to" location
	mpToggleMoveDownloaded->set_active(app_prefs.getDoMoveToDir());
	mpComboMoveToLocation->set_sensitive(mpToggleMoveDownloaded->get_active());
	mpToggleMoveDownloaded->signal_toggled().connect(
		sigc::bind<Gtk::ToggleButton*,Gtk::Widget*>(setDirtyLambda, mpToggleMoveDownloaded, mpComboMoveToLocation));

	dlLocationModel = Glib::RefPtr<FolderSelectionListStore>(new FolderSelectionListStore);
	dlDir = app_prefs.getMoveToDir();
	if(!dlDir.empty())
	{
		dlLocationModel->setDirectory(dlDir);
		dlLocationModel->setDirectoryIcon(get_icon_for_path(dlDir));
	}

	mpComboMoveToLocation->set_model(dlLocationModel);
	mpComboMoveToLocation->set_row_separator_func(sigc::mem_fun(*dlLocationModel.operator->(), &FolderSelectionListStore::isRowSeparator));

	pPixbufRenderer = new Gtk::CellRendererPixbuf;
	pTextRenderer = new Gtk::CellRendererText;

	mpComboMoveToLocation->pack_start(*pPixbufRenderer, false);
	mpComboMoveToLocation->pack_end(*pTextRenderer);
	mpComboMoveToLocation->add_attribute(*pPixbufRenderer, "gicon", dlLocationModel->columns().icon());
	mpComboMoveToLocation->add_attribute(*pTextRenderer, "text", dlLocationModel->columns().text());

	mpComboMoveToLocation->set_active(dlLocationModel->children().begin());
	mpComboMoveToLocation->signal_changed().connect(
		sigc::bind<Gtk::ComboBox*>(sigc::mem_fun(*this, &PrefsLocationsInterface::onDlLocationChanged), mpComboMoveToLocation));

	set_modified(false);
}

void PrefsLocationsInterface::save(AppPreferences& app_prefs)
{
	app_prefs.setDownloadDir(getDownloadDir().c_str());
	app_prefs.setMoveToDir(getMoveToDir().c_str());
	app_prefs.setAlwayAskForDir(getDoAlwaysAskForDownloadLocation());
	app_prefs.setDoMoveToDir(getDoMoveDownloaded());
}

std::string PrefsLocationsInterface::getDownloadDir() const
{
	std::string result("");
	if(0 != mpComboLocation)
	{
		Glib::RefPtr<FolderSelectionListStore> dlLocationModel
			= Glib::RefPtr<FolderSelectionListStore>::cast_dynamic(mpComboLocation->get_model());
		result = dlLocationModel->getDirectory();
	}
	return result;
}

void PrefsLocationsInterface::setDownloadDir(const std::string& dir)
{
	if(0 != mpComboLocation)
	{
		Glib::RefPtr<FolderSelectionListStore> dlLocationModel
			= Glib::RefPtr<FolderSelectionListStore>::cast_dynamic(mpComboLocation->get_model());
		dlLocationModel->setDirectory(dir);
		dlLocationModel->setDirectoryIcon(get_icon_for_path(dir.empty() ? std::string("/") : dir));
	}
}

bool PrefsLocationsInterface::getDoAlwaysAskForDownloadLocation() const
{
	bool result = false;
	if(0 != mpToggleAlwaysAsk)
		result = mpToggleAlwaysAsk->get_active();
	return result;
}

void PrefsLocationsInterface::setDoAlwaysAskForDownloadLocation(bool bDoAlwaysAsk/* = true*/)
{
	if(0 != mpToggleAlwaysAsk)
		mpToggleAlwaysAsk->set_active(bDoAlwaysAsk);
}

bool PrefsLocationsInterface::getDoMoveDownloaded() const
{
	bool result = false;
	if(0 != mpToggleMoveDownloaded)
		result = mpToggleMoveDownloaded->get_active();
	return result;
}

void PrefsLocationsInterface::setDoMoveDownloaded(bool bDoMoveDownloaded/* = true*/)
{
	if(0 != mpToggleMoveDownloaded)
		mpToggleMoveDownloaded->set_active(bDoMoveDownloaded);
}

std::string PrefsLocationsInterface::getMoveToDir() const
{
	std::string result("");
	if(0 != mpComboLocation)
	{
		Glib::RefPtr<FolderSelectionListStore> dlLocationModel
			= Glib::RefPtr<FolderSelectionListStore>::cast_dynamic(mpComboMoveToLocation->get_model());
		result = dlLocationModel->getDirectory();
	}
	return result;
}

void PrefsLocationsInterface::setMoveToDir(const std::string& dir)
{
	if(0 != mpComboLocation)
	{
		Glib::RefPtr<FolderSelectionListStore> dlLocationModel
			= Glib::RefPtr<FolderSelectionListStore>::cast_dynamic(mpComboMoveToLocation->get_model());
		dlLocationModel->setDirectory(dir);
		dlLocationModel->setDirectoryIcon(get_icon_for_path(dir.empty() ? std::string("/") : dir));
	}
}