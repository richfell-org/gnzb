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
#ifndef __GNZB_APPLICATION_WINDOW_HEADER__
#define __GNZB_APPLICATION_WINDOW_HEADER__

#include <memory>
#include <vector>
#include <glibmm/refptr.h>
#include <gtkmm/treeiter.h>
#include <gtkmm/treepath.h>
#include <gtkmm/applicationwindow.h>
#include "nzbdlinfobar.h"
#include "nzbsummarywindow.h"
#include "statusbarwindow.h"
#include "filedrophandler.h"
#include "groups/gnzbgroupsmenu.h"
#include "../db/gnzbgroups.h"

namespace Gio {
	class SimpleAction;
}

namespace Gtk {
	class Alignment;
	class Box;
	class Builder;
	class Paned;
	class Revealer;
	class Stack;
	class TreeView;
}

class NzbTreeView;
class NzbFileTreeView;
class GNzbToolbar;
class MenuAction;
class GNzb;
enum class GNzbAction;

/**
 *
 *
 */
class GNzbApplicationWindow : public Gtk::ApplicationWindow
{
public:

	GNzbApplicationWindow(GtkApplicationWindow *p_object, const Glib::RefPtr<Gtk::Builder>& ref_builder);
	~GNzbApplicationWindow();

// attributes
public: 

	GNzbToolbar* get_toolbar() { return mp_toolbar; }
	const GNzbToolbar* get_toolbar() const { return mp_toolbar; }

	NzbTreeView *get_tree_view() { return mp_treeview; }
	const NzbTreeView *get_tree_view() const { return mp_treeview; }

	NzbSummaryWindow& summary_window() { return m_win_summary; }
	const NzbSummaryWindow& summary_window() const { return m_win_summary; }

	NzbDlInfoBar& info_bar() { return m_info_bar; }
	const NzbDlInfoBar& info_bar() const { return m_info_bar; }

// operations
public:

	// open NZB file
	void open_nzb_file(const std::string& file_path);

	// get a GNzb from a path or oterator
	std::shared_ptr<GNzb> get_gnzb(const Gtk::TreePath& path);
	std::shared_ptr<GNzb> get_gnzb(const Gtk::TreeIter& iter);

	// get the display name for a GNzb
	std::string get_display_name(const GNzb *p_gnzb);
	std::string get_display_name(const Gtk::TreePath& gnzb_path);
	std::string get_display_name(const Gtk::TreeIter& gnzb_iter);

	// put a GNzb into a certain GNzbState
	void move_gnzb_to_state(GNzb* p_gnzb, GNzbState new_state);
	void move_gnzb_to_state(const Gtk::TreePath& gnzb_path, GNzbState new_state);
	void move_gnzb_to_state(const Gtk::TreeIter& gnzb_iter, GNzbState new_state);

	// remove/delete GNzb
	void remove_gnzb(const GNzb *p_gnzb, bool delete_files = false);
	void remove_gnzb(const Gtk::TreePath& gnzb_path, bool delete_files = false);
	void remove_gnzb(const Gtk::TreeIter& gnzb_iter, bool delete_files = false);

	// indicate that some aspect of the GNzb changed
	void gnzb_updated(const GNzb *p_gnzb);
	void gnzb_updated(const Gtk::TreePath& gnzb_path);
	void gnzb_updated(const Gtk::TreeIter& gnzb_iter);

	// GNzb list maipulation
	Gtk::TreeIter find_next_queued_nzb();
	void move_gnzb_to_top(const GNzb *p_gnzb);
	void move_gnzb_to_top(const Gtk::TreeIter& gnzb_iter);

	void update_gnzb_filesview(const std::shared_ptr<GNzb>& ptr_gnzb);

	// updates the NNTP connection count
	void update_connection_info();

	bool is_details_view() const;

// implementation
protected:

	// action handlers
	void on_file_open();
	void on_resume_all();
	void on_pause_all();
	void on_cancel_all();
	void flip_statistics_view();
	void flip_details_view();
	void flip_summary_view();

	// action management
	void set_action_enabled(const MenuAction& ma, bool enabled = true);
	void set_action_state(const MenuAction& ma, bool state);
	void manipulate_action(const Glib::ustring& name, std::function<void (Glib::RefPtr<Gio::SimpleAction>&)> manip_func);

	// signal handlers
	void on_realize();
	void on_hide();
	void on_nzb_selection_changed();
	void on_gnzb_group(const Gtk::TreePath& gnzb_path, GdkEventButton *p_event);
	void on_gnzb_group_selected(Gtk::TreePath gnzb_path, int id_group);
	void on_gnzb_group_configure();

	// initialization
	void create_actions();
	void init_toolbar();

	void add_file_treeview(const std::shared_ptr<GNzb>& ptr_gnzb);
	void remove_file_treeview(const std::shared_ptr<GNzb>& ptr_gnzb);
	Gtk::TreeView *find_file_treeview(const std::shared_ptr<GNzb>& ptr_gnzb);
	void update_gui_state();

	void store_state();
	void restore_state();

	// popup context-type menu for the "groups" button on NZB listed items
	GNzbGroupsMenu m_groups_popup;
	std::vector<GNzbGroup> m_gnzb_groups;

	// widgets
	NzbDlInfoBar m_info_bar;
	NzbSummaryWindow m_win_summary;
	StatusBarWindow m_status_bar;
	GNzbToolbar *mp_toolbar;
	Gtk::Stack *mp_nzb_stack, *mp_files_stack;
	Gtk::Alignment *mp_align_normal, *mp_align_details;
	Gtk::Box *mp_nzb_box;
	Gtk::Paned *mp_details_paned;
	NzbTreeView *mp_treeview;
	Gtk::Revealer *mp_summary_revealer, *mp_info_revealer;

	FileDropHandler m_dnd_handler;
};

#endif	/* __GNZB_APPLICATION_WINDOW_HEADER__ */
