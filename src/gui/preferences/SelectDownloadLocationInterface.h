#ifndef __SELECT_DOWNLOAD_LOCATION_INTERFACE_HEADER__
#define __SELECT_DOWNLOAD_LOCATION_INTERFACE_HEADER__

#include "../AppPreferences.h"
#include "PrefsLocationsInterface.h"

namespace Gtk {
	class Window;
	class Button;
	class Dialog;
}

/*
 * 
 */
class SelectDownloadLocationInterface
{
// construction
public:

	SelectDownloadLocationInterface();
	~SelectDownloadLocationInterface();

// operations
public:

	bool run(Gtk::Window *pParentWin = 0);

// implementation
private:

	void on_save();
	void on_cancel();

	void on_selected();

	AppPreferences m_app_prefs;

	Gtk::Dialog *mp_dialog;
	Gtk::Button *mp_btn_ok;
	PrefsLocationsInterface m_locations;
};

#endif  /* __SELECT_DOWNLOAD_LOCATION_INTERFACE_HEADER__ */