#ifndef __PREFS_SCRIPTS_INTERFACE_HEADER__
#define __PREFS_SCRIPTS_INTERFACE_HEADER__

#include <string>
#include "preferencespage.h"

class AppPreferences;
class FileListStore;

namespace Glib {
	template <typename T> class RefPtr;
}

namespace Gtk {
	class Builder;
	class ToggleButton;
	class ComboBox;
	class ComboBoxText;
}

/*
 * 
 */
class PrefsScriptsInterface : public PreferencesPage
{
// construction
public:

	PrefsScriptsInterface();
	~PrefsScriptsInterface();

// attributes
public:

	Gtk::ToggleButton& getRunScriptOnNzbAddedWidget() { return *mpToggleNzbAdded; }
	const Gtk::ToggleButton& getRunScriptOnNzbAddedWidget() const { return *mpToggleNzbAdded; }

	Gtk::ComboBox& getNzbAddedScriptWidget() { return *mpComboNzbAdded; }
	const Gtk::ComboBox& getNzbAddedScriptWidget() const { return *mpComboNzbAdded; }

	Gtk::ToggleButton& getRunScriptOnNzbFinishedWidget() { return *mpToggleNzbFinished; }
	const Gtk::ToggleButton& getRunScriptOnNzbFinishedWidget() const { return *mpToggleNzbFinished; }

	Gtk::ComboBox& getNzbFinishedScriptWidget() { return *mpComboNzbFinished; }
	const Gtk::ComboBox& getNzbFinishedScriptWidget() const { return *mpComboNzbFinished; }

	Gtk::ToggleButton& getRunScriptOnNzbCancelledWidget() { return *mpToggleNzbCancelled; }
	const Gtk::ToggleButton& getRunScriptOnNzbCancelledWidget() const { return *mpToggleNzbCancelled; }

	Gtk::ComboBox& getNzbCancelledScriptWidget() { return *mpComboNzbCancelled; }
	const Gtk::ComboBox& getNzbCancelledScriptWidget() const { return *mpComboNzbCancelled; }

// operations
public:

	void init(const AppPreferences& app_prefs, Glib::RefPtr<Gtk::Builder>& refBuilder);
	void save(AppPreferences& app_prefs);

	bool getScriptingEnabled() const;
	void setScriptinEnabled(bool enableScripting = false);

	std::string getPythonModule() const;
	void setPythonModule(const std::string& pythonModule);

	bool getRunScriptOnNzbAdded() const;
	void setRunScriptOnNzbAdded(bool bDoScript = true);

	std::string getScriptOnNzbAdded() const;
	void setScriptOnNzbAdded(const std::string& script);

	bool getRunScriptOnNzbFinished() const;
	void setRunScriptOnNzbFinished(bool bDoScript = true);

	std::string getScriptOnNzbFinished() const;
	void setScriptOnNzbFinished(const std::string& script);

	bool getRunScriptOnNzbCancelled() const;
	void setRunScriptOnNzbCancelled(bool bDoScript = true);

	std::string getScriptOnNzbCancelled() const;
	void setScriptOnNzbCancelled(const std::string& script);

// implementation
private:

	void onScriptingEnabledChanged();
	void onFileSelectionChanged(Gtk::ComboBox *pComboBox);

	// Widget pointers
	Gtk::ToggleButton *mpToggleEnableScripting;
	Gtk::ComboBoxText *mpComboPythonVersion;
	Gtk::ToggleButton *mpToggleNzbAdded;
	Gtk::ComboBox *mpComboNzbAdded;
	Gtk::ToggleButton *mpToggleNzbFinished;
	Gtk::ComboBox *mpComboNzbFinished;
	Gtk::ToggleButton *mpToggleNzbCancelled;
	Gtk::ComboBox *mpComboNzbCancelled;
};

#endif  /* __PREFS_SCRIPTS_INTERFACE_HEADER__ */