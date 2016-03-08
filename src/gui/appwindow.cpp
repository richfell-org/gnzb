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
#include "appwindow.h"
#include "guiutil.h"
#include "../application.h"
#include "../gnzb.h"
#include "nntp/fetch.h"
#include "nzbtreeview.h"
#include "nzbfiletreeview.h"
#include "nzbliststore.h"
#include "nzbfileliststore.h"
#include "nzbfilechooser.h"
#include "menus/menuactions.h"
#include "toolbar/toolbar.h"
#include "groups/groupsinterface.h"
#include <memory>
#include <algorithm>
#include <cstring>
#include <giomm/actionmap.h>
#include <giomm/simpleaction.h>
#include <gtkmm/alignment.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/paned.h>
#include <gtkmm/revealer.h>
#include <gtkmm/stack.h>
#include <gtkmm/treeselection.h>
#include "../uiresource.h"

GNzbApplicationWindow::GNzbApplicationWindow(GtkApplicationWindow *p_object, const Glib::RefPtr<Gtk::Builder>& ref_builder)
:	Gtk::ApplicationWindow(p_object),
	Glib::ObjectBase("gnzbapplicationwindow"),
	mp_toolbar(get_managed_widget_derived<GNzbToolbar>(ref_builder, "gnzb.toolbar")),
	mp_nzb_stack(get_managed_widget<Gtk::Stack>(ref_builder, "gnzb.view.stack")),
	mp_files_stack(get_managed_widget<Gtk::Stack>(ref_builder, "gnzb.files.treeview.stack")),
	mp_align_normal(get_managed_widget<Gtk::Alignment>(ref_builder, "gnzb.nzbtreeview.align")),
	mp_align_details(get_managed_widget<Gtk::Alignment>(ref_builder, "gnzb.nzbtreevirew.details.align")),
	mp_nzb_box(get_managed_widget<Gtk::Box>(ref_builder, "gnzb.nzbtreeview.box")),
	mp_details_paned(get_managed_widget<Gtk::Paned>(ref_builder, "gnzb.list.paned")),
	mp_treeview(get_managed_widget_derived<NzbTreeView>(ref_builder, "gnzb.nzb.treeview")),
	mp_summary_revealer(get_managed_widget<Gtk::Revealer>(ref_builder, "gnzb.summary.revealer")),
	mp_info_revealer(get_managed_widget<Gtk::Revealer>(ref_builder, "gnzb.info.revealer")),
	m_dnd_handler(*this)
{
	// add a reference to the toolbar
	mp_toolbar->reference();

	// place the download info window
	set_widget(m_info_bar, ref_builder, "gnzb.infobar.align");
	m_info_bar.show_all_children();

	// place the summary window
	set_widget(m_win_summary, ref_builder, "gnzb.summsry.align");
	m_win_summary.show_all_children();
	m_win_summary.clear();

	// NZB state status window
	set_widget(m_status_bar, ref_builder, "gnzb.status.align");
	m_status_bar.show_all_children();

	// set handlers for signals
	mp_treeview->get_selection()->signal_changed().connect(
		sigc::mem_fun(*this, &GNzbApplicationWindow::on_nzb_selection_changed));
	//mp_treeview->signal_action().connect(sigc::mem_fun(*this, &GNzbApplicationWindow::on_gnzb_action));
	mp_treeview->signal_group().connect(sigc::mem_fun(*this, &GNzbApplicationWindow::on_gnzb_group));

	// connect Drag-n-Drop handler signal to open NZB file handler
	m_dnd_handler.signal_dropped().connect(sigc::mem_fun(*this, &GNzbApplicationWindow::open_nzb_file));

	// init actions and toolbar
	create_actions();
	init_toolbar();

	update_connection_info();
	update_gui_state();

	// configure the GNzbGroups popup menu
	m_groups_popup.signal_group_selected().connect(sigc::mem_fun(*this, &GNzbApplicationWindow::on_gnzb_group_selected));
	m_groups_popup.setConfigureSlot(sigc::mem_fun(*this, &GNzbApplicationWindow::on_gnzb_group_configure));

	// load the groups from the DB
	m_gnzb_groups = GNzbGroups().get_groups();
}

GNzbApplicationWindow::~GNzbApplicationWindow()
{
	Glib::RefPtr<NzbListStore> ref_gnzb_store = mp_treeview->get_nzb_model();
	for(auto& iter : ref_gnzb_store->children())
		remove_file_treeview((*iter)[ref_gnzb_store->columns().gnzb()]);
}

void GNzbApplicationWindow::open_nzb_file(const std::string& file_path)
{
	// parse the NZB file and add it to the main NZB list
	std::shared_ptr<GNzb> ptr_gnzb = std::make_shared<GNzb>(file_path);

	// find matching group, default to none
	ptr_gnzb->ptr_group(&m_gnzb_groups[0]);
	for(auto& group : m_gnzb_groups)
	{
		if(group.doAutoAssignGroup() && group.doesMatchInclude(file_path.c_str())
		   && !group.doesMatchExclude(file_path.c_str()))
		{
			ptr_gnzb->ptr_group(&group);
		}
	}

	// add to the GNzb treeview
	Gtk::TreeIter iter = mp_treeview->get_nzb_model()->append(ptr_gnzb);
	if(ptr_gnzb->download_data().is_complete())
		move_gnzb_to_state(iter, GNzbState::COMPLETE);

	// add an NZB::File treeview for this NZB file
	add_file_treeview(ptr_gnzb);

	// set the summary window
	m_win_summary.setNzbSummary(*ptr_gnzb);

	update_gui_state();
}

void GNzbApplicationWindow::move_gnzb_to_state(GNzb* p_gnzb, GNzbState new_state)
{
	move_gnzb_to_state(mp_treeview->get_nzb_model()->find_gnzb(p_gnzb), new_state);
}

void GNzbApplicationWindow::move_gnzb_to_state(const Gtk::TreePath& gnzb_path, GNzbState new_state)
{
	move_gnzb_to_state(mp_treeview->get_nzb_model()->get_iter(gnzb_path), new_state);
}

void GNzbApplicationWindow::move_gnzb_to_state(const Gtk::TreeIter& gnzb_iter, GNzbState new_state)
{
	Glib::RefPtr<NzbListStore> ref_store = mp_treeview->get_nzb_model();

	if(!gnzb_iter || gnzb_iter == ref_store->children().end())
		return;

	switch(new_state)
	{
		case GNzbState::DOWNLOADING:
		case GNzbState::WAITING:
			(*gnzb_iter)[ref_store->columns().action1()] = GNzbAction::PAUSE;
			(*gnzb_iter)[ref_store->columns().action2()] = GNzbAction::DELETE;
			break;
		case GNzbState::PAUSED:
		case GNzbState::STOPPED:
			(*gnzb_iter)[ref_store->columns().action1()] = GNzbAction::START;
			(*gnzb_iter)[ref_store->columns().action2()] = GNzbAction::DELETE;
			break;
		case GNzbState::COMPLETE:
			(*gnzb_iter)[ref_store->columns().action1()] = GNzbAction::SHOW;
			(*gnzb_iter)[ref_store->columns().action2()] = GNzbAction::FINISHED;
			break;
		// nothing is done for these states
		case GNzbState::CONDITIONAL:
		case GNzbState::STATE_COUNT:
		default:
			break;
	}

	std::shared_ptr<GNzb> ptr_gnzb = (*gnzb_iter)[ref_store->columns().gnzb()];
	ptr_gnzb->state(new_state);

	ref_store->row_changed(Gtk::TreePath(gnzb_iter), gnzb_iter);
	update_gui_state();
}

std::shared_ptr<GNzb> GNzbApplicationWindow::get_gnzb(const Gtk::TreePath& path)
{
	// get the GNzb shared ptr using the given path
	Glib::RefPtr<NzbListStore> ref_store = mp_treeview->get_nzb_model();
	Gtk::TreeIter iter = ref_store->get_iter(path);
	return (*iter)[ref_store->columns().gnzb()];
}

std::shared_ptr<GNzb> GNzbApplicationWindow::get_gnzb(const Gtk::TreeIter& iter)
{
	return (*iter)[mp_treeview->get_nzb_model()->columns().gnzb()];
}

void GNzbApplicationWindow::remove_gnzb(const GNzb *p_gnzb, bool delete_files/* = false*/)
{
	Gtk::TreeIter iter = mp_treeview->get_nzb_model()->find_gnzb(p_gnzb);
	remove_gnzb(iter, delete_files);
}

std::string GNzbApplicationWindow::get_display_name(const GNzb *p_gnzb)
{
	return get_display_name(mp_treeview->get_nzb_model()->find_gnzb(p_gnzb));
}

std::string GNzbApplicationWindow::get_display_name(const Gtk::TreePath& gnzb_path)
{
	return get_display_name(mp_treeview->get_nzb_model()->get_iter(gnzb_path));
}

std::string GNzbApplicationWindow::get_display_name(const Gtk::TreeIter& gnzb_iter)
{
	return Glib::ustring((*gnzb_iter)[mp_treeview->get_nzb_model()->columns().name()]);
}

void GNzbApplicationWindow::remove_gnzb(const Gtk::TreePath& gnzb_path, bool delete_files/* = false*/)
{
	remove_gnzb(mp_treeview->get_nzb_model()->get_iter(gnzb_path), delete_files);
}

void GNzbApplicationWindow::remove_gnzb(const Gtk::TreeIter& gnzb_iter, bool delete_files/* = false*/)
{
	if(!gnzb_iter) return;

	Glib::RefPtr<NzbListStore> ref_model = mp_treeview->get_nzb_model();
	std::shared_ptr<GNzb> ptr_gnzb = (*gnzb_iter)[ref_model->columns().gnzb()];

	if(delete_files)
		system(Glib::ustring::compose("rm -rf \"%1\"", ptr_gnzb->output_path()).c_str());

	// remove the NZB::Files view for the GNzb
	remove_file_treeview(ptr_gnzb);

	// remove the GNzb from the main list
	ref_model->erase(gnzb_iter);

	// if no more GNzb are listed close the details view
	if(ref_model->children().size() == 0)
	{
		if(is_details_view())
			flip_details_view();
	}
	// else show NZB::Files for the top GNzb if no selection
	else if(mp_treeview->get_selection()->count_selected_rows() == 0)
	{
		ptr_gnzb = (*ref_model->children().begin())[ref_model->columns().gnzb()];
		mp_files_stack->set_visible_child(ptr_gnzb->path());
	}

	update_gui_state();
}

void GNzbApplicationWindow::gnzb_updated(const GNzb *p_gnzb)
{
	gnzb_updated(mp_treeview->get_nzb_model()->find_gnzb(p_gnzb));
}

void GNzbApplicationWindow::gnzb_updated(const Gtk::TreePath& gnzb_path)
{
	mp_treeview->get_model()->row_changed(gnzb_path, mp_treeview->get_model()->get_iter(gnzb_path));
	update_gui_state();
}

void GNzbApplicationWindow::gnzb_updated(const Gtk::TreeIter& gnzb_iter)
{
	mp_treeview->get_model()->row_changed(Gtk::TreePath(gnzb_iter), gnzb_iter);
	update_gui_state();
}

Gtk::TreeIter GNzbApplicationWindow::find_next_queued_nzb()
{
	Glib::RefPtr<NzbListStore> ref_model = mp_treeview->get_nzb_model();
	for(auto iter : ref_model->children())
	{
		std::shared_ptr<GNzb> ptr_gnzb = (*iter)[ref_model->columns().gnzb()];
		if(ptr_gnzb->state() == GNzbState::WAITING)
			return iter;
	}
	
	return Gtk::TreeIter();
}

void GNzbApplicationWindow::move_gnzb_to_top(const GNzb *p_gnzb)
{
	move_gnzb_to_top(mp_treeview->get_nzb_model()->find_gnzb(p_gnzb));
}

void GNzbApplicationWindow::move_gnzb_to_top(const Gtk::TreeIter& gnzb_iter)
{
	Glib::RefPtr<NzbListStore> ref_model = mp_treeview->get_nzb_model();
	if(gnzb_iter != ref_model->children().end())
		ref_model->move(gnzb_iter, ref_model->children().begin());
}

void GNzbApplicationWindow::update_gnzb_filesview(const std::shared_ptr<GNzb>& ptr_gnzb)
{
	// don't waste time if the file details are not displayed
	if(!is_details_view()) return;

	Gtk::TreeView *p_files_treeview = find_file_treeview(ptr_gnzb);
	if(p_files_treeview != nullptr)
	{
		Glib::RefPtr<Gtk::TreeModel> ref_files_model = p_files_treeview->get_model();

		int file_count = ptr_gnzb->nzb_files().getFileCount();
		std::vector<FileMeta>& file_meta = ptr_gnzb->files_meta();
		std::lock_guard<std::mutex> updateLock(ptr_gnzb->update_mutex());
		for(int i = 0; i < file_count; ++i)
		{
			if(file_meta[i].updated())
			{
				file_meta[i].updated(false);
				Gtk::TreeModel::iterator file_iter = ref_files_model->children()[i];
				ref_files_model->row_changed(Gtk::TreePath(file_iter), file_iter);
			}
		}
	}

}

bool GNzbApplicationWindow::is_details_view() const
{
	return mp_nzb_stack->get_visible_child() == mp_details_paned;
}

void GNzbApplicationWindow::on_file_open()
{
	try
	{
		std::vector<std::string> file_names;

		// open a chooser dialog to get the user's file choice
		NzbFileChooser file_chooser(*this);
		int result = file_chooser.run();
		WaitCursorTool waitTool(get_window());
		switch(result)
		{
			case Gtk::RESPONSE_ACCEPT:
				file_names = file_chooser.get_filenames();
				for(unsigned int i = 0; i < file_names.size(); ++i)
					open_nzb_file(file_names[i]);
				break;
			default:
				break;
		}
	}
	catch(const Glib::Error& e)
	{
		show_error(*this, Glib::ustring("Error opening file"), e.what());
	}
	catch(const std::exception& e)
	{
		show_error(*this, Glib::ustring("Error opening file"), e.what());
	}
	catch(...)
	{
		show_error(*this, Glib::ustring("Error opening file"), Glib::ustring("Unknown error type"));
	}
}

void GNzbApplicationWindow::on_gnzb_group(const Gtk::TreePath& gnzb_path, GdkEventButton *p_event)
{
	Glib::RefPtr<NzbListStore> ref_store = mp_treeview->get_nzb_model();
	Gtk::TreeIter iter = ref_store->get_iter(gnzb_path);

	std::shared_ptr<GNzb> ptr_gnzb = (*iter)[ref_store->columns().gnzb()];

	m_groups_popup.set_target(gnzb_path);
	m_groups_popup.resetAppNzbGroupItems(GNzbGroups().get_groups());
	m_groups_popup.setActiveGroup(*ptr_gnzb->ptr_group());
	m_groups_popup.popup(p_event->time, p_event->x_root, p_event->y_root);
}

void GNzbApplicationWindow::on_gnzb_group_selected(Gtk::TreePath gnzb_path, int id_group)
{
	Glib::RefPtr<NzbListStore> ref_store = mp_treeview->get_nzb_model();
	Gtk::TreeIter iter = ref_store->get_iter(gnzb_path);

	std::shared_ptr<GNzb> ptr_gnzb = (*iter)[ref_store->columns().gnzb()];

	for(auto& group : m_gnzb_groups)
	{
		if(id_group == group.get_db_id())
		{
			ptr_gnzb->ptr_group(&group);
			(*iter)[ref_store->columns().group_color_text()] = group.getColorText();
			break;
		}
	}

	ref_store->row_changed(gnzb_path, iter);
}

void GNzbApplicationWindow::on_gnzb_group_configure()
{
	GroupsInterface ui_interface;
	if(Gtk::RESPONSE_ACCEPT == ui_interface.run(this))
	{
	}
}

void GNzbApplicationWindow::on_resume_all()
{
	std::shared_ptr<GNzb> ptr_gnzb_ready;

	Glib::RefPtr<NzbListStore> ref_model = mp_treeview->get_nzb_model();
	for(auto& iter : ref_model->children())
	{
		std::shared_ptr<GNzb> ptr_gnzb = (*iter)[ref_model->columns().gnzb()];
		if(ptr_gnzb->state() == GNzbState::PAUSED)
		{
			if(!ptr_gnzb_ready) ptr_gnzb_ready = ptr_gnzb;
			move_gnzb_to_state(iter, GNzbState::WAITING);
		}
	}

	Glib::RefPtr<GNzbApplication> ref_app = GNzbApplication::get_instance();
	ref_app->reference();
	if(ptr_gnzb_ready && !ref_app->is_download_active())
		ref_app->start_download(ptr_gnzb_ready);
}

void GNzbApplicationWindow::on_pause_all()
{
}

void GNzbApplicationWindow::on_cancel_all()
{
}

void GNzbApplicationWindow::on_nzb_selection_changed()
{
	// if there is a selection then set the summary and file list
	// according to the selected NZB, else set those items according
	// to the first NZB in the list

	Glib::RefPtr<Gtk::TreeSelection> ref_selection = mp_treeview->get_selection();
	Glib::RefPtr<NzbListStore> ref_nzb_store = mp_treeview->get_nzb_model();

	Gtk::TreeModel::iterator nzb_iter = (0 < ref_selection->count_selected_rows())
		? ref_selection->get_selected()
		: ref_nzb_store->children().begin();

	// if there are no items on the list then make sure
	// the file treeview is hidden and update action state
	if(nzb_iter == ref_nzb_store->children().end())
	{
		if(is_details_view())
			flip_details_view();
		update_gui_state();
	}
	else
	{
		// get the selected NZB
		std::shared_ptr<GNzb> ptr_gnzb = (*nzb_iter)[ref_nzb_store->columns().gnzb()];

		// update the summary window with the selected NBZ
		m_win_summary.setNzbSummary(*ptr_gnzb);
		mp_files_stack->set_visible_child(ptr_gnzb->path());
	}
}

/**
 * Toggle the "info" bar.
 */
void GNzbApplicationWindow::flip_statistics_view()
{
	// determine is the new state
	bool flipped_state = !mp_info_revealer->get_reveal_child();

	// make it so
	mp_info_revealer->set_reveal_child(flipped_state);

	// update the menu action state
	set_action_state(ma_view_statistics, flipped_state);
}

/**
 * Toggle the details view.  The details view contains
 * the sidebar with the content of the selected NZB
 * file.  The non-details view contains just the list
 * of NZB files themselves.
 * 
 */
void GNzbApplicationWindow::flip_details_view()
{
	bool new_state_is_details = mp_align_normal == mp_nzb_stack->get_visible_child();

	// switch to details?
	if(new_state_is_details)
	{
		// reparent the NZB view to be within the "details view"
		// Gtk::Paned instance and display the details view
		mp_nzb_box->reparent(*mp_align_details);
		mp_nzb_stack->set_visible_child(*mp_details_paned);
	}
	// switch to normal
	else
	{
		// reparent the NZB treeview to be within the "normal view"'s
		// Gtk::Alignment instance and display the non-details view
		mp_nzb_box->reparent(*mp_align_normal);
		mp_nzb_stack->set_visible_child(*mp_align_normal);
	}

	// update the menu action state
	set_action_state(ma_view_details, new_state_is_details);
}

void GNzbApplicationWindow::flip_summary_view()
{
	// determine is the new state
	bool flipped_state = !mp_summary_revealer->get_reveal_child();

	// make it so
	mp_summary_revealer->set_reveal_child(flipped_state);

	// update the menu action state
	set_action_state(ma_view_summary, flipped_state);
}

void GNzbApplicationWindow::create_actions()
{
	// file/open
	add_action(
		ma_file_open.name,
		sigc::mem_fun(*this, &GNzbApplicationWindow::on_file_open)
	)->reference();

	// file/resume all
	add_action(
		ma_file_resume_all.name,
		sigc::mem_fun(*this, &GNzbApplicationWindow::on_resume_all)
	)->reference();

	// file/pause all
	add_action(
		ma_file_pause_all.name,
		sigc::mem_fun(*this, &GNzbApplicationWindow::on_pause_all)
	)->reference();

	// file/cancel all
	add_action(
		ma_file_cancel_all.name,
		sigc::mem_fun(*this, &GNzbApplicationWindow::on_cancel_all)
	)->reference();

	// view/statistics
	add_action_bool(
		ma_view_statistics.name,
		sigc::mem_fun(*this, &GNzbApplicationWindow::flip_statistics_view)
	)->reference();

	// view/details
	add_action_bool(
		ma_view_details.name,
		sigc::mem_fun(*this, &GNzbApplicationWindow::flip_details_view)
	)->reference();

	// view/summary
	add_action_bool(
		ma_view_summary.name,
        sigc::mem_fun(*this, &GNzbApplicationWindow::flip_summary_view)
	)->reference();
}

void GNzbApplicationWindow::init_toolbar()
{
	mp_toolbar->set_clicked_action(GNzbToolbar::ADD_FILE, lookup_action(ma_file_open.name));
	mp_toolbar->set_clicked_action(GNzbToolbar::RESUME, lookup_action(ma_file_resume_all.name));
	mp_toolbar->set_clicked_action(GNzbToolbar::PAUSE, lookup_action(ma_file_pause_all.name));
	mp_toolbar->set_clicked_action(GNzbToolbar::CANCEL, lookup_action(ma_file_cancel_all.name));
	mp_toolbar->set_toggled_action(GNzbToolbar::DETAILS, lookup_action(ma_view_details.name));
	mp_toolbar->set_toggled_action(GNzbToolbar::SUMMARY, lookup_action(ma_view_summary.name));
}

void GNzbApplicationWindow::set_action_enabled(const MenuAction& ma, bool enabled/* = true*/)
{
	Glib::RefPtr<Gio::Action> ref_action = lookup_action(ma.name);
	if(ref_action)
	{
		ref_action->reference();
		change_action_state(ma.name, Glib::Variant<bool>::create(enabled));
	}
}

void GNzbApplicationWindow::set_action_state(const MenuAction& ma, bool state)
{
	static Glib::Variant<bool> STATE_TRUE = Glib::Variant<bool>::create(true); 
	static Glib::Variant<bool> STATE_FALSE = Glib::Variant<bool>::create(false);

	change_action_state(ma.name, (state ? STATE_TRUE : STATE_FALSE));
}

void GNzbApplicationWindow::manipulate_action(
	const Glib::ustring& name, std::function<void (Glib::RefPtr<Gio::SimpleAction>&)> manip_func)
{
	Glib::RefPtr<Gio::SimpleAction> ref_action
		= Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(lookup_action(name));
	if(ref_action)
	{
		ref_action->reference();
		manip_func(ref_action);
	}
}

void GNzbApplicationWindow::add_file_treeview(const std::shared_ptr<GNzb>& ptr_gnzb)
{
	Gtk::ScrolledWindow *p_scrolled{nullptr};
	NzbFileTreeView *p_files_treeview{nullptr};
	try
	{
		// create a ScrolledWindow and the NzbFileTreeView instances
		p_scrolled = new Gtk::ScrolledWindow();
		p_files_treeview = new NzbFileTreeView();

		// add the NzbFileTreeView to the ScrolledWindow
		p_scrolled->add(*p_files_treeview);

		Glib::RefPtr<Gdk::Pixbuf> ref_icon
			= Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/filetype/24x24/generic.png"));

		// load the NZB::Files into the tree view
		Glib::RefPtr<NzbFileListStore> ref_file_store = p_files_treeview->get_nzbfile_model();
		ref_file_store->clear();
		ref_file_store->append_nzb_files(*ptr_gnzb);

		// add to the files stack view, these instances will be retrieved
		// from the stack view when it is time to delete them
		mp_files_stack->add(*p_scrolled, ptr_gnzb->path());
		p_files_treeview->set_visible();
		p_scrolled->set_visible();
	}
	catch(const Glib::Error& e)
	{
		show_error(*this, "GNzbApplicationWindow::add_file_treeview", e.what());
	}
	catch(const std::exception& e)
	{
		show_error(*this, "GNzbApplicationWindow::add_file_treeview", e.what());
	}
	catch(...)
	{
		show_error(*this, "GNzbApplicationWindow::add_file_treeview", "Unknow error type");
	}
}

void GNzbApplicationWindow::remove_file_treeview(const std::shared_ptr<GNzb>& ptr_gnzb)
{
	// look for a ScrolledWindow by name in the files treeview stack
	Gtk::ScrolledWindow *p_scrolled = dynamic_cast<Gtk::ScrolledWindow*>(mp_files_stack->get_child_by_name(ptr_gnzb->path()));
	if(nullptr == p_scrolled) return;

	// get the NzbFileTreeView from the ScrolledWindow
	NzbFileTreeView *p_files_treeview = dynamic_cast<NzbFileTreeView*>(p_scrolled->get_child());
	if(nullptr != p_files_treeview)
	{
		p_scrolled->remove();
		delete p_files_treeview;
	}

	delete p_scrolled;
}

Gtk::TreeView *GNzbApplicationWindow::find_file_treeview(const std::shared_ptr<GNzb>& ptr_gnzb)
{
	// look for a ScrolledWindow by name in the files treeview stack
	Gtk::ScrolledWindow *p_scrolled = dynamic_cast<Gtk::ScrolledWindow*>(mp_files_stack->get_child_by_name(ptr_gnzb->path()));
	if(nullptr != p_scrolled)
		return dynamic_cast<NzbFileTreeView*>(p_scrolled->get_child());
	return nullptr;
}

void GNzbApplicationWindow::update_connection_info()
{
	m_info_bar.connections().set_text(Glib::ustring::compose(
		"%1/%2", NntpFetch::get_active_connection_count(), NntpFetch::get_max_connections()));
}

void GNzbApplicationWindow::update_gui_state()
{
	int nDl(0), nQueued(0), nPaused(0), nComplete(0), nGnzb(0);

	Glib::RefPtr<NzbListStore> ref_store = mp_treeview->get_nzb_model();
	for(auto& iter : ref_store->children())
	{
		++nGnzb;
		std::shared_ptr<GNzb> ptr_gnzb = (*iter)[ref_store->columns().gnzb()];
		switch(ptr_gnzb->state())
		{
			case GNzbState::WAITING:
				++nQueued;
				break;
			case GNzbState::PAUSED:
				++nPaused;
				break;
			case GNzbState::COMPLETE:
				++nComplete;
				break;
			case GNzbState::DOWNLOADING:
				++nDl;
				break;
			// nothing is counted for these states
			case GNzbState::STOPPED:
			case GNzbState::CONDITIONAL:
			case GNzbState::STATE_COUNT:
			default:
				break;
		}
	}

	// update the status bar
	m_status_bar.setDownloadCount(nDl);
	m_status_bar.setQueuedCount(nQueued);
	m_status_bar.setPausedCount(nPaused);
	m_status_bar.setFinishedCount(nComplete);

	bool enabled = false;
	auto action_manip = [&enabled](Glib::RefPtr<Gio::SimpleAction>& ref_action) { ref_action->set_enabled(enabled); };

	// NZB details enabled?
	enabled = nGnzb > 0;
	manipulate_action(ma_view_details.name, action_manip);

	// resume all
	enabled = nPaused > 0;
	manipulate_action(ma_file_resume_all.name, action_manip);

	// pause all
	enabled = (nQueued + nDl) > 0;
	manipulate_action(ma_file_pause_all.name, action_manip);

	// cancel all
	enabled = (nQueued + nDl + nPaused) > 0;
	manipulate_action(ma_file_cancel_all.name , action_manip);
}

void GNzbApplicationWindow::on_realize()
{
	// restore the last saved state
	restore_state();
	Gtk::ApplicationWindow::on_realize();
}

void GNzbApplicationWindow::on_hide()
{
	// save the current state
	store_state();
	Gtk::ApplicationWindow::on_hide();
}

/**
 * structure with the data needed to remember the dislay appearance.
 */
typedef struct {
	// window position
	struct {
		int x, y, width, height;
	} winpos;

	// the position for NZB::Files treeview Gtk::Paned
	int nzb_files_position;

	// if the NZB::File treeview is visible
	bool is_details_mode;

	// NZB summary revealed state
	bool nzb_summary_revealed;

	// statictics bar revealed state
	bool statistics_revealed;
} GnzbGuiSettings;

#include "../db/gnzbdb.h"

void GNzbApplicationWindow::store_state()
{
	GnzbGuiSettings settings;

	// main window position & size
	get_position(settings.winpos.x, settings.winpos.y);
	settings.winpos.width = get_allocated_width();
	settings.winpos.height = get_allocated_height();

	// the position of the Gtk::Paned for the NZB::File treeview
	settings.nzb_files_position = mp_details_paned->get_position();

	// NZB details mode?
	settings.is_details_mode = mp_align_details == mp_nzb_stack->get_visible_child();

	settings.nzb_summary_revealed = mp_summary_revealer->get_child_revealed();
	settings.statistics_revealed = mp_info_revealer->get_child_revealed();

	try
	{
		// wrtie the settings
		Sqlite3::Db appdb = open_app_db();
		Sqlite3::Stmt stmt = appdb.prepareStatement("update SystemAttributes set data = ? where id = ?");
		if(stmt)
		{
			stmt.bindBlob(1, &settings, sizeof(settings));
			stmt.bindInt(2, SA_WINSTATE);
			stmt.step();
			stmt.reset();
		}
	}
	catch(const std::exception& e)
	{
		show_error(*this, "Error while storing the current state:", e.what());
	}
	catch(...)
	{
		show_error(*this, "Unknow error type while storing the current state");
	}
}

void GNzbApplicationWindow::restore_state()
{
	GnzbGuiSettings settings;

	try
	{
		// open a connection to the DB and prepare a statement
		Sqlite3::Db appdb = open_app_db();
		Sqlite3::Stmt stmt = appdb.prepareStatement("select data from SystemAttributes where id = ?");
		if(!stmt) return;

		// execute the query
		stmt.bindInt(1, SA_WINSTATE);
		stmt.step();

		// copy the blob data into our structure
		memcpy(&settings, stmt.getResultBlob(0), std::min(stmt.getResultBytes(0), int(sizeof(settings))));

		// match the recalled settings
		move(settings.winpos.x, settings.winpos.y);
		resize(settings.winpos.width, settings.winpos.height);
		mp_details_paned->set_position(settings.nzb_files_position);

		// statistics revealer
		mp_info_revealer->set_reveal_child(settings.statistics_revealed);
		set_action_state(ma_view_statistics, settings.statistics_revealed);

		// NZB summary revealer
		mp_summary_revealer->set_reveal_child(settings.nzb_summary_revealed);
		set_action_state(ma_view_summary, settings.nzb_summary_revealed);
	}
	catch(const std::exception& e)
	{
		show_error(*this, "Error while restoring the current state:", e.what());
	}
	catch(...)
	{
		show_error(*this, "Unknow error type while restoring the current state");
	}
}