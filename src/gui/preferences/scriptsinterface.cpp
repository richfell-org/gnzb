#include "scriptsinterface.h"
#include "widgets/singlefileliststore.h"
#include "../guiutil.h"
#include "../../db/gnzbdb.h"
#include "../../db/preferences.h"
#include <gtkmm/builder.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/filechooserdialog.h>

#include "config.h"

PrefsScriptsInterface::PrefsScriptsInterface()
:   mpToggleEnableScripting(nullptr), mpComboPythonVersion(nullptr),
	mpToggleNzbAdded(nullptr), mpComboNzbAdded(nullptr),
	mpToggleNzbFinished(nullptr), mpComboNzbFinished(nullptr),
	mpToggleNzbCancelled(nullptr), mpComboNzbCancelled(nullptr)
{
}

PrefsScriptsInterface::~PrefsScriptsInterface()
{
}

static void initSelectionCombo(Gtk::ComboBox *pComboBox, Glib::RefPtr<SingleFileListStore>& refModel, bool bSensitive)
{
	pComboBox->set_model(refModel);
	pComboBox->set_row_separator_func(sigc::mem_fun(*refModel.operator->(), &SingleFileListStore::isRowSeparator));
	pComboBox->set_sensitive(bSensitive);

	Gtk::CellRendererPixbuf *pPixbufRenderer = new Gtk::CellRendererPixbuf;
	Gtk::CellRendererText *pTextRenderer = new Gtk::CellRendererText;

	pComboBox->pack_start(*pPixbufRenderer, false);
	pComboBox->pack_end(*pTextRenderer);
	pComboBox->add_attribute(*pPixbufRenderer, "gicon", refModel->columns().icon());
	pComboBox->add_attribute(*pTextRenderer, "text", refModel->columns().text());
}

void PrefsScriptsInterface::init(const AppPreferences& app_prefs, Glib::RefPtr<Gtk::Builder>& ref_builder)
{
	mpToggleNzbAdded = mpToggleNzbFinished = mpToggleNzbCancelled = nullptr;
	mpComboNzbAdded = mpComboNzbFinished = mpComboNzbCancelled = nullptr;

	ref_builder->get_widget("preferences.scripting.enable.chk", mpToggleEnableScripting);
	ref_builder->get_widget("preferences.scripting.pythonversion.combo", mpComboPythonVersion);
	ref_builder->get_widget("preferences.script.nzbadded.chk", mpToggleNzbAdded);
	ref_builder->get_widget("preferences.script.nzbadded.combo", mpComboNzbAdded);
	ref_builder->get_widget("preferences.script.nzbfinished.chk", mpToggleNzbFinished);
	ref_builder->get_widget("preferences.script.nzbfinished.combo", mpComboNzbFinished);
	ref_builder->get_widget("preferences.script.nzbcancelled.chk", mpToggleNzbCancelled);
	ref_builder->get_widget("preferences.script.nzbcancelled.combo", mpComboNzbCancelled);

	// is scripting enabled?
	mpToggleEnableScripting->set_active(app_prefs.getScriptingEnabled());
	mpToggleEnableScripting->signal_toggled().connect(sigc::mem_fun(*this, &PrefsScriptsInterface::onScriptingEnabledChanged));

#ifdef GNZB_PLUGIN_PY2
	mpComboPythonVersion->append(Glib::ustring("libgnzbpy2.so"), Glib::ustring("2"));
#endif  /* GNZB_PLUGIN_PY2 */
#ifdef GNZB_PLUGIN_PY3
	mpComboPythonVersion->append(Glib::ustring("libgnzbpy3.so"), Glib::ustring("3"));
#endif  /* GNZB_PLUGIN_PY3 */

	// get the python version TODO: read python version from preferences system attributes
	std::string cur_module = app_prefs.getSysAttrString(SA_PYMODULE);
	if(cur_module.empty())
		mpComboPythonVersion->set_active(0);
	else
		mpComboPythonVersion->set_active_id(cur_module);

	// set sensitivity for the current state
	onScriptingEnabledChanged();

	// will set the locations area as "dirty" and, if provided, set a peer widget's
	// sensitivity according to the active state of the toggle button
	auto setDirtyLambda = [this](Gtk::ToggleButton *pTgl = 0, Gtk::Widget* pPeer = nullptr)
		{ set_modified(); if(pTgl && pPeer) pPeer->set_sensitive(pTgl->get_active()); };

	mpToggleNzbAdded->set_active(app_prefs.getRunScriptOnNzbAdded());
	mpToggleNzbAdded->signal_toggled().connect(sigc::bind<Gtk::ToggleButton*>(setDirtyLambda, mpToggleNzbAdded, mpComboNzbAdded));

	mpToggleNzbFinished->set_active(app_prefs.getRunScriptOnNzbFinished());
	mpToggleNzbFinished->signal_toggled().connect(sigc::bind<Gtk::ToggleButton*>(setDirtyLambda, mpToggleNzbFinished, mpComboNzbFinished));

	mpToggleNzbCancelled->set_active(app_prefs.getRunScriptOnNzbCancelled());
	mpToggleNzbCancelled->signal_toggled().connect(sigc::bind<Gtk::ToggleButton*>(setDirtyLambda, mpToggleNzbCancelled, mpComboNzbCancelled));

	Glib::RefPtr<SingleFileListStore> refListStore = Glib::RefPtr<SingleFileListStore>(new SingleFileListStore);
	initSelectionCombo(mpComboNzbAdded, refListStore, mpToggleNzbAdded->get_active());
	setScriptOnNzbAdded(app_prefs.getNzbAddedScript());
	mpComboNzbAdded->set_active(0);
	mpComboNzbAdded->signal_changed().connect(sigc::bind<Gtk::ComboBox*>(sigc::mem_fun(*this, &PrefsScriptsInterface::onFileSelectionChanged), mpComboNzbAdded));

	refListStore = Glib::RefPtr<SingleFileListStore>(new SingleFileListStore);
	initSelectionCombo(mpComboNzbFinished, refListStore, mpToggleNzbFinished->get_active());
	setScriptOnNzbFinished(app_prefs.getNzbFinishedScript());
	mpComboNzbFinished->set_active(0);
	mpComboNzbFinished->signal_changed().connect(sigc::bind<Gtk::ComboBox*>(sigc::mem_fun(*this, &PrefsScriptsInterface::onFileSelectionChanged), mpComboNzbFinished));

	refListStore = Glib::RefPtr<SingleFileListStore>(new SingleFileListStore);
	initSelectionCombo(mpComboNzbCancelled, refListStore, mpToggleNzbCancelled->get_active());
	setScriptOnNzbCancelled(app_prefs.getNzbCancelledScript());
	mpComboNzbCancelled->set_active(0);
	mpComboNzbCancelled->signal_changed().connect(sigc::bind<Gtk::ComboBox*>(sigc::mem_fun(*this, &PrefsScriptsInterface::onFileSelectionChanged), mpComboNzbCancelled));

	// start in a fresh state
	set_modified(false);
}

void PrefsScriptsInterface::save(AppPreferences& app_prefs)
{
	app_prefs.setScriptingEnabled(getScriptingEnabled());
	app_prefs.setSysAttrSring(SA_PYMODULE, getPythonModule());
	app_prefs.setRunScriptOnNzbAdded(getRunScriptOnNzbAdded());
	app_prefs.setNzbAddedScript(getScriptOnNzbAdded().c_str());
	app_prefs.setRunScriptOnNzbFinished(getRunScriptOnNzbFinished());
	app_prefs.setNzbFinishedScript(getScriptOnNzbFinished().c_str());
	app_prefs.setRunScriptOnNzbCancelled(getRunScriptOnNzbCancelled());
	app_prefs.setNzbCancelledScript(getScriptOnNzbCancelled().c_str());
}

void PrefsScriptsInterface::onScriptingEnabledChanged()
{
	set_modified();

	bool isScriptingEnabled = mpToggleEnableScripting->get_active();
	mpComboPythonVersion->set_sensitive(isScriptingEnabled);
	mpToggleNzbAdded->set_sensitive(isScriptingEnabled);
	mpToggleNzbFinished->set_sensitive(isScriptingEnabled);
	mpToggleNzbCancelled->set_sensitive(isScriptingEnabled);
}

void PrefsScriptsInterface::onFileSelectionChanged(Gtk::ComboBox *pComboBox)
{
	Glib::RefPtr<SingleFileListStore> comboModel
		= Glib::RefPtr<SingleFileListStore>::cast_dynamic(pComboBox->get_model());

	Gtk::TreeIter iter = pComboBox->get_active();
	if((*iter)[comboModel->columns().type()] == SingleFileListStore::SELECT)
	{
		Gtk::FileChooserDialog dirChooser("Select Location...", Gtk::FILE_CHOOSER_ACTION_OPEN);
		dirChooser.set_create_folders();
		dirChooser.add_button("Cancel", Gtk::RESPONSE_CANCEL);
		dirChooser.add_button("Select", Gtk::RESPONSE_ACCEPT);
		switch(dirChooser.run())
		{
			case Gtk::RESPONSE_ACCEPT:
				comboModel->set_file(dirChooser.get_filename().c_str());
				comboModel->set_file_icon(get_icon_for_path(dirChooser.get_filename()));
				set_modified();
				// fall through, because the first item is to always be selected
			default:
				pComboBox->set_active(comboModel->children().begin());
				break;
		}
	}
}

bool PrefsScriptsInterface::getScriptingEnabled() const
{
	bool result = false;
	if(nullptr != mpToggleEnableScripting)
		result = mpToggleEnableScripting->get_active();
	return result;
}

void PrefsScriptsInterface::setScriptinEnabled(bool enableScripting/* = false*/)
{
	if(0 != mpToggleEnableScripting)
		mpToggleEnableScripting->set_active(enableScripting);
}

std::string PrefsScriptsInterface::getPythonModule() const
{
	std::string result("");
	if(nullptr != mpComboPythonVersion)
		result = mpComboPythonVersion->get_active_id();
	return result;
}

void PrefsScriptsInterface::setPythonModule(const std::string& pythonModule)
{
	if(nullptr != mpComboPythonVersion)
		mpComboPythonVersion->set_active_id(pythonModule);
}

bool PrefsScriptsInterface::getRunScriptOnNzbAdded() const
{
	bool result = false;
	if(0 != mpToggleNzbAdded)
		result = mpToggleNzbAdded->get_active();
	return result;
}

void PrefsScriptsInterface::setRunScriptOnNzbAdded(bool bDoScript/* = true*/)
{
	if(0 != mpToggleNzbAdded)
		mpToggleNzbAdded->set_active(bDoScript);
}

std::string PrefsScriptsInterface::getScriptOnNzbAdded() const
{
	std::string result("");
	if(0 != mpComboNzbAdded)
	{
		Glib::RefPtr<SingleFileListStore> ref_model
			= Glib::RefPtr<SingleFileListStore>::cast_dynamic(mpComboNzbAdded->get_model());
		result = ref_model->get_file();
	}
	return result;
}

void PrefsScriptsInterface::setScriptOnNzbAdded(const std::string& filename)
{
	if(0 != mpComboNzbAdded)
	{
		Glib::RefPtr<SingleFileListStore> ref_model
			= Glib::RefPtr<SingleFileListStore>::cast_dynamic(mpComboNzbAdded->get_model());
		if(filename.empty())
			ref_model->clear_selection();
		else
		{
			ref_model->set_file(filename);
			ref_model->set_file_icon(get_icon_for_path(filename));
		}
	}
}

bool PrefsScriptsInterface::getRunScriptOnNzbFinished() const
{
	bool result = false;
	if(0 != mpToggleNzbFinished)
		result = mpToggleNzbFinished->get_active();
	return result;
}

void PrefsScriptsInterface::setRunScriptOnNzbFinished(bool bDoScript/* = true*/)
{
	if(0 != mpToggleNzbFinished)
		mpToggleNzbFinished->set_active(bDoScript);
}

std::string PrefsScriptsInterface::getScriptOnNzbFinished() const
{
	std::string result("");
	if(0 != mpComboNzbFinished)
	{
		Glib::RefPtr<SingleFileListStore> ref_model
			= Glib::RefPtr<SingleFileListStore>::cast_dynamic(mpComboNzbFinished->get_model());
		result = ref_model->get_file();
	}
	return result;
}

void PrefsScriptsInterface::setScriptOnNzbFinished(const std::string& filename)
{
	if(0 != mpComboNzbFinished)
	{
		Glib::RefPtr<SingleFileListStore> ref_model
			= Glib::RefPtr<SingleFileListStore>::cast_dynamic(mpComboNzbFinished->get_model());
		if(filename.empty())
			ref_model->clear_selection();
		else
		{
			ref_model->set_file(filename);
			ref_model->set_file_icon(get_icon_for_path(filename));
		}
	}
}

bool PrefsScriptsInterface::getRunScriptOnNzbCancelled() const
{
	bool result = false;
	if(0 != mpToggleNzbCancelled)
		result = mpToggleNzbCancelled->get_active();
	return result;
}

void PrefsScriptsInterface::setRunScriptOnNzbCancelled(bool bDoScript/* = true*/)
{
	if(0 != mpToggleNzbCancelled)
		mpToggleNzbCancelled->set_active(bDoScript);
}

std::string PrefsScriptsInterface::getScriptOnNzbCancelled() const
{
	std::string result("");
	if(0 != mpComboNzbCancelled)
	{
		Glib::RefPtr<SingleFileListStore> ref_model
			= Glib::RefPtr<SingleFileListStore>::cast_dynamic(mpComboNzbCancelled->get_model());
		result = ref_model->get_file();
	}
	return result;
}

void PrefsScriptsInterface::setScriptOnNzbCancelled(const std::string& filename)
{
	if(0 != mpComboNzbCancelled)
	{
		Glib::RefPtr<SingleFileListStore> ref_model
			= Glib::RefPtr<SingleFileListStore>::cast_dynamic(mpComboNzbCancelled->get_model());
		if(filename.empty())
			ref_model->clear_selection();
		else
		{
			ref_model->set_file(filename);
			ref_model->set_file_icon(get_icon_for_path(filename));
		}
	}
}