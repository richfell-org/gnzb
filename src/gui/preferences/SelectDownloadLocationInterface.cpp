#include <gtkmm/builder.h>
#include <gtkmm/alignment.h>
#include <gtkmm/grid.h>
#include <gtkmm/combobox.h>
#include <gtkmm/dialog.h>
#include "uiresource.h"
#include "SelectDownloadLocationInterface.h"

// the GUI definition for the preferences dialog box
extern Glib::ustring enzybeeui_prefs;

SelectDownloadLocationInterface::SelectDownloadLocationInterface()
:   mp_dialog(0)
{
}

SelectDownloadLocationInterface::~SelectDownloadLocationInterface()
{
}

bool SelectDownloadLocationInterface::run(Gtk::Window *pParentWin/* = 0*/)
{
	// build the GUI from the XML definition
	Glib::RefPtr<Gtk::Builder> refBuilder = Gtk::Builder::create_from_resource(UiResourcePath("prefs-ui/preferences.ui"));

	// get the instantiated Gtk::Dialog instance
	refBuilder->get_widget("selectDownloadLocationDialog", mp_dialog);
	if(0 != pParentWin)
		mp_dialog->set_transient_for(*pParentWin);

	// initalize the signal handlers
	// ...
	refBuilder->get_widget("btnSelectOk", mp_btn_ok);
	mp_btn_ok->set_sensitive(false);
	sigc::connection okConn = mp_btn_ok->signal_clicked().connect(sigc::mem_fun(*this, &SelectDownloadLocationInterface::on_save));

	Gtk::Button *pButton = 0;
	refBuilder->get_widget("btnSelectCancel", pButton);
	sigc::connection cancelConn = pButton->signal_clicked().connect(sigc::mem_fun(*this, &SelectDownloadLocationInterface::on_cancel));

	// Init the locations widgets and reparent them to the selectDownloadLocationDialog
	m_locations.init(m_app_prefs, refBuilder);
	Gtk::Alignment *p_locations_alignment = 0;
	Gtk::Grid *p_locations_grid = 0;
	refBuilder->get_widget("contentAlignment", p_locations_alignment);
	refBuilder->get_widget("locationsGrid", p_locations_grid);
	p_locations_grid->reparent(*p_locations_alignment);

	// connect to the dir selectors change signal so the OK button can be
	// enabled/disabled based on have a selection made by the user
	sigc::connection selConn = m_locations.getLocationWidget().signal_changed().connect(sigc::mem_fun(*this, &SelectDownloadLocationInterface::on_selected));

	// run the dialog
	int dialogResult = mp_dialog->run();
	okConn.disconnect();
	cancelConn.disconnect();
	selConn.disconnect();
	delete mp_dialog;
	mp_dialog = 0;

	return dialogResult == 1;
}

void SelectDownloadLocationInterface::on_save()
{
	std::string dir = m_locations.getDownloadDir().c_str();
	if(!dir.empty())
	{
		m_app_prefs.setDownloadDir(dir.c_str());
		mp_dialog->response(1);
	}
}

void SelectDownloadLocationInterface::on_cancel()
{
	mp_dialog->response(0);
}

void SelectDownloadLocationInterface::on_selected()
{
	std::string dir = m_locations.getDownloadDir().c_str();
	mp_btn_ok->set_sensitive(!dir.empty());
}