#ifndef __NZB_FILE_CHOOSER_HEADER__
#define __NZB_FILE_CHOOSER_HEADER__

#include <gtkmm/filechooserdialog.h>
#include <glibmm/ustring.h>

class NzbFileChooser : public Gtk::FileChooserDialog
{
public:
	NzbFileChooser(Gtk::Window& parent, const Glib::ustring title = "Open NZB File");
	~NzbFileChooser() {}

protected:

	Glib::RefPtr<Gtk::FileFilter> m_nzb_filter;
};

#endif  /* __NZB_FILE_CHOOSER_HEADER__ */