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
#include "notificationsinterface.h"
#include "widgets/fileliststore.h"
#include "../guiutil.h"
#include "../../db/preferences.h"
#include <forward_list>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/filechooserdialog.h>

PrefsNotificationsInterface::PrefsNotificationsInterface()
:	mpToggleShowNotifications(0),
	mpToggleSoundOnNzbFinished(0),mpComboNzbFinishedSound(0), mpBtnPlayNzbFinishedSound(0),
	mpToggleSoundOnQueueFinished(0), mpComboQueueFinishedSound(0), mpBtnPlayQueueFinishedSound(0)
{
}

PrefsNotificationsInterface::~PrefsNotificationsInterface()
{
}

static void initSelectionCombo(Gtk::ComboBox *pComboBox, Glib::RefPtr<FileListStore>& ref_model, bool bSensitive)
{
	pComboBox->set_model(ref_model);
	pComboBox->set_row_separator_func(sigc::mem_fun(*ref_model.operator->(), &FileListStore::isRowSeparator));
	pComboBox->set_sensitive(bSensitive);

	Gtk::CellRendererPixbuf *pPixbufRenderer = new Gtk::CellRendererPixbuf;
	Gtk::CellRendererText *pTextRenderer = new Gtk::CellRendererText;

	pComboBox->pack_start(*pPixbufRenderer, false);
	pComboBox->pack_end(*pTextRenderer);
	pComboBox->add_attribute(*pPixbufRenderer, "gicon", ref_model->cols().col_icon());
	pComboBox->add_attribute(*pTextRenderer, "text", ref_model->cols().col_text());
}

void PrefsNotificationsInterface::init(const AppPreferences& app_prefs, Glib::RefPtr<Gtk::Builder>& ref_builder)
{
	mpToggleShowNotifications = mpToggleSoundOnNzbFinished = 0;
	mpComboNzbFinishedSound = mpComboQueueFinishedSound = 0;
	mpBtnPlayNzbFinishedSound = mpBtnPlayQueueFinishedSound = 0;
	mpToggleSoundOnQueueFinished = 0;

	ref_builder->get_widget("preferences.notifications.desktop.chk", mpToggleShowNotifications);
	ref_builder->get_widget("preferences.soundnzbfinished.chk", mpToggleSoundOnNzbFinished);
	ref_builder->get_widget("preferences.soundnzbfinished.combo", mpComboNzbFinishedSound);
	ref_builder->get_widget("preferences.previewnzbfinished.btn", mpBtnPlayNzbFinishedSound);
	ref_builder->get_widget("preferences.soundqueuefinished.chk", mpToggleSoundOnQueueFinished);
	ref_builder->get_widget("preferences.soundqueuefinished.combo", mpComboQueueFinishedSound);
	ref_builder->get_widget("preferences.previewqueuefinished.btn", mpBtnPlayQueueFinishedSound);

	// will set the locations area as "dirty" and, if provided, set a peer widget's
	// sensitivity according to the active state of the toggle button
	auto setDirtyLambda = [this](Gtk::ToggleButton *pTgl = 0, std::forward_list<Gtk::Widget*> peers =std::forward_list<Gtk::Widget*>{})
		{ set_modified(); if(pTgl) for(auto& pWidget : peers) pWidget->set_sensitive(pTgl->get_active()); };

	// The "Show Notifications" option
	mpToggleShowNotifications->set_active(app_prefs.getShowNotifications());
	mpToggleShowNotifications->signal_toggled().connect(sigc::bind<Gtk::ToggleButton*>(setDirtyLambda, mpToggleShowNotifications));

	// sound on NZB complete
	mpToggleSoundOnNzbFinished->set_active(app_prefs.getSoundOnNzbFinish());
	mpToggleSoundOnNzbFinished->signal_toggled().connect(sigc::bind< Gtk::ToggleButton*,std::forward_list<Gtk::Widget*> >(
			setDirtyLambda, mpToggleSoundOnNzbFinished, {mpComboNzbFinishedSound,mpBtnPlayNzbFinishedSound}));

	Glib::RefPtr<FileListStore> refListStore = Glib::RefPtr<FileListStore>(new FileListStore({"/usr/share/sounds/gnome/default/alerts/"}, {"*"}));
	initSelectionCombo(mpComboNzbFinishedSound, refListStore, mpToggleSoundOnNzbFinished->get_active());
	mpComboNzbFinishedSound->signal_changed().connect(sigc::bind<Gtk::ComboBox*>(
			sigc::mem_fun(*this, &PrefsNotificationsInterface::on_file_selection_changed), mpComboNzbFinishedSound));

	std::string curValue = app_prefs.getNzbFinishSound();
	if(curValue.empty())
		mpComboNzbFinishedSound->set_active(0);
	else
		setSoundOnNzbFinished(curValue);

	// sound on Queue complete
	mpToggleSoundOnQueueFinished->set_active(app_prefs.getSoundOnQueueFinish());
	mpToggleSoundOnQueueFinished->signal_toggled().connect(sigc::bind< Gtk::ToggleButton*,std::forward_list<Gtk::Widget*> >(
		setDirtyLambda, mpToggleSoundOnQueueFinished, {mpComboQueueFinishedSound, mpBtnPlayQueueFinishedSound}));

	refListStore = Glib::RefPtr<FileListStore>(new FileListStore({"/usr/share/sounds/gnome/default/alerts/"}, {"*"}));
	initSelectionCombo(mpComboQueueFinishedSound, refListStore, mpToggleSoundOnQueueFinished->get_active());
	mpComboQueueFinishedSound->signal_changed().connect(sigc::bind<Gtk::ComboBox*>(
			sigc::mem_fun(*this, &PrefsNotificationsInterface::on_file_selection_changed), mpComboQueueFinishedSound));

	curValue = app_prefs.getQueueFinishSound();
	if(curValue.empty())
		mpComboQueueFinishedSound->set_active(0);
	else
		setSoundOnQueueFinished(curValue);

	// set up preview sound button signal handlers
	mpBtnPlayNzbFinishedSound->signal_clicked().connect([this](){play_sound(getSoundOnNzbFinished());});	
	mpBtnPlayQueueFinishedSound->signal_clicked().connect([this](){play_sound(getSoundOnQueueFinished());});

	set_modified(false);
}

void PrefsNotificationsInterface::save(AppPreferences& app_prefs)
{
	app_prefs.setShowNotifications(getDoShowNotifications());
	app_prefs.setSoundOnNzbFinish(getDoSoundOnNzbFinished());
	app_prefs.setNzbFinishSound(getSoundOnNzbFinished().c_str());
	app_prefs.setSoundOnQueueFinish(getDoSoundOnQueueFinished());
	app_prefs.setQueueFinishSound(getSoundOnQueueFinished().c_str());
}

bool PrefsNotificationsInterface::getDoShowNotifications() const
{
	bool result = false;
	if(0 != mpToggleShowNotifications)
		result = mpToggleShowNotifications->get_active();
	return result;
}

void PrefsNotificationsInterface::setDoShowNotifications(bool bDoNotifications/* = true*/)
{
	if(0 != mpToggleShowNotifications)
		mpToggleShowNotifications->set_active(bDoNotifications);
}

bool PrefsNotificationsInterface::getDoSoundOnNzbFinished() const
{
	bool result = false;
	if(0 != mpToggleSoundOnNzbFinished)
		result = mpToggleSoundOnNzbFinished->get_active();
	return result;
}

void PrefsNotificationsInterface::setDoSoundOnNzbFinished(bool bDoSound/* = true*/)
{
	if(0 != mpToggleSoundOnNzbFinished)
		mpToggleSoundOnNzbFinished->set_active(bDoSound);
}

std::string PrefsNotificationsInterface::getSoundOnNzbFinished() const
{
	std::string result("");
	if(0 != mpComboNzbFinishedSound)
	{
		Gtk::TreeIter selIter = mpComboNzbFinishedSound->get_active();
		Glib::RefPtr<FileListStore> refModel = Glib::RefPtr<FileListStore>::cast_dynamic(mpComboNzbFinishedSound->get_model());
		result = refModel->getFileSystemItemPath(selIter);
	}
	return result;
}

void PrefsNotificationsInterface::setSoundOnNzbFinished(const std::string& filename)
{
	if(0 != mpComboNzbFinishedSound)
	{
		Glib::RefPtr<FileListStore> refModel = Glib::RefPtr<FileListStore>::cast_dynamic(mpComboNzbFinishedSound->get_model());
		Gtk::TreeIter iter = refModel->findByPath(filename);
		if(iter == refModel->children().end())
			iter = refModel->prependFileSystemItem(filename, get_icon_for_path(filename));
		mpComboNzbFinishedSound->set_active(iter);
	}
}

bool PrefsNotificationsInterface::getDoSoundOnQueueFinished() const
{
	bool result = false;
	if(0 != mpToggleSoundOnQueueFinished)
		result = mpToggleSoundOnQueueFinished->get_active();
	return result;
}

void PrefsNotificationsInterface::setDoSoundOnQueueFinished(bool bDoSound/* = true*/)
{
	if(0 != mpToggleSoundOnQueueFinished)
		mpToggleSoundOnQueueFinished->set_active(bDoSound);
}

std::string PrefsNotificationsInterface::getSoundOnQueueFinished() const
{
	std::string result("");
	if(0 != mpComboQueueFinishedSound)
	{
		Gtk::TreeIter selIter = mpComboQueueFinishedSound->get_active();
		Glib::RefPtr<FileListStore> refModel = Glib::RefPtr<FileListStore>::cast_dynamic(mpComboQueueFinishedSound->get_model());
		result = refModel->getFileSystemItemPath(selIter);
	}
	return result;
}

void PrefsNotificationsInterface::setSoundOnQueueFinished(const std::string& filename)
{
	if(0 != mpComboQueueFinishedSound)
	{
		Glib::RefPtr<FileListStore> refModel = Glib::RefPtr<FileListStore>::cast_dynamic(mpComboQueueFinishedSound->get_model());
		Gtk::TreeIter iter = refModel->findByPath(filename);
		if(iter == refModel->children().end())
			iter = refModel->prependFileSystemItem(filename, get_icon_for_path(filename));
		mpComboQueueFinishedSound->set_active(iter);
	}
}

void PrefsNotificationsInterface::on_file_selection_changed(Gtk::ComboBox *p_combo)
{
	Glib::RefPtr<FileListStore> ref_model
		= Glib::RefPtr<FileListStore>::cast_dynamic(p_combo->get_model());

	Gtk::TreeIter iter = p_combo->get_active();
	if((*iter)[ref_model->cols().col_type()] != FileListStore::SELECT)
		set_modified();
	else
	{
		Gtk::FileChooserDialog file_chooser("Select sound...", Gtk::FILE_CHOOSER_ACTION_OPEN);
		file_chooser.add_button("Cancel", Gtk::RESPONSE_CANCEL);
		file_chooser.add_button("Select", Gtk::RESPONSE_ACCEPT);
		switch(file_chooser.run())
		{
			case Gtk::RESPONSE_ACCEPT:
				iter = ref_model->findByPath(file_chooser.get_filename());
				if(!iter)
					iter = ref_model->appendFileSystemItem(file_chooser.get_filename(), get_icon_for_path(file_chooser.get_filename()));
				p_combo->set_active(iter);
				set_modified();
				break;
			default:
				break;
		}
	}
}