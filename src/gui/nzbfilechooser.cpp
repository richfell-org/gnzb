#include "nzbfilechooser.h"
#include <gtkmm/stock.h>

NzbFileChooser::NzbFileChooser(Gtk::Window& parent, const Glib::ustring title/* = "Open NZB File"*/)
:   Gtk::FileChooserDialog(parent, title),
	m_nzb_filter(Gtk::FileFilter::create())
{
	set_select_multiple();
	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	add_button("Open", Gtk::RESPONSE_ACCEPT);
	m_nzb_filter->set_name("NZB Files");
	m_nzb_filter->add_pattern("*.nzb");
	add_filter(m_nzb_filter);
}