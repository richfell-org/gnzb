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
#include "groupsinterface.h"
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <gtkmm/builder.h>
#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/treeview.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/filechooserdialog.h>
#include "groupsliststore.h"
#include "../cellrenderer/bulletpointrenderer.h"
#include "../preferences/widgets/folderselectionliststore.h"
#include "../guiutil.h"
#include "../../uiresource.h"

// UI definition string
extern Glib::ustring enzybeeui_groups;

GroupsInterface::GroupsInterface()
:   mpBulletRenderer(0), mpGroupsDialog(0)
{
	m_group_tuples.clear();
}

GroupsInterface::~GroupsInterface()
{
	if(0 != mpGroupsDialog)
	{
		delete mpGroupsDialog;
		mpGroupsDialog = 0;
	}
}

int GroupsInterface::run(Gtk::Window *pParentWin/* = 0*/)
{
	// build the GUI from the XML definition
	Glib::RefPtr<Gtk::Builder> refBuilder = Gtk::Builder::create_from_resource(UiResourcePath("gui/groups/groups.ui"));

	// get the instantiated Gtk::Dialog instance
	refBuilder->get_widget("groups.dialog", mpGroupsDialog);
	if(0 != pParentWin)
		mpGroupsDialog->set_transient_for(*pParentWin);
	mpGroupsDialog->set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);

	// initalize the signal handlers
	// ...
	Gtk::Button *pButton = 0;
	refBuilder->get_widget("groups.save.btn", pButton);
	pButton->signal_clicked().connect(sigc::mem_fun(*this, &GroupsInterface::onSaveGroups));

	refBuilder->get_widget("groups.cancel.btn", pButton);
	pButton->signal_clicked().connect(sigc::mem_fun(*this, &GroupsInterface::onCancelGroups));

	// get the widget pointers
	refBuilder->get_widget("groups.gnzbgroup.treeview", mpGroupTreeView);
	refBuilder->get_widget("groups.gnzbgroupadd.btn", mpBtnAddGroup);
	refBuilder->get_widget("groups.gnzbgroupremove.btn", mpBtnRemoveGroup);
	refBuilder->get_widget("groups.gnzbgroupcolor.btn", mpBtnGroupColor);
	refBuilder->get_widget("groups.movecompleted.chk", mpToggleMoveCompleted);
	refBuilder->get_widget("groups.movetofolder.combo", mpComboMoveLocation);
	refBuilder->get_widget("groups.movetvshowepisodes.chk", mpToggleMoveTvSeries);
	refBuilder->get_widget("groups.cleanup.chk", mpToggleCleanup);
	refBuilder->get_widget("groups.autoassigngroup.chk", mpToggleAutoAssign);
	refBuilder->get_widget("groups.groupnameincludes.entry", mpEntryNameIncludes);
	refBuilder->get_widget("groups.groupnameexcludes.entry", mpEntryNameExcludes);
	refBuilder->get_widget("groups.matchingsizemin.entry", mpEntryMinSize);
	refBuilder->get_widget("groups.matchingsizemax.entry", mpEntryMaxSize);

	// initialize dialog box widgets with the current settings
	initWidgets();
	initNzbGroupsList();

	// run the dialog
	int dialogResult = mpGroupsDialog->run();
	return dialogResult;
}

int GroupsInterface::getMetricValueFromListPosition(int iGroup)
{
	int result = -1;

	Glib::RefPtr<GroupsListStore> ref_model
		= Glib::RefPtr<GroupsListStore>::cast_dynamic(mpGroupTreeView->get_model());

	for(auto& iter : ref_model->children())
	{
		// check for a match on the group's index (iGroup is the index in mAppNzbGroups vector)
		if(iGroup != (*iter)[ref_model->columns().col_entry_idx()])
			continue;

		// the path index + 1 is equivalent to the metric value
		result = Gtk::TreePath(iter)[0];
		++result;
	}

	return result;
}

void GroupsInterface::onSaveGroups()
{
	WaitCursorTool waitCursor(*mpGroupsDialog);

	// check for any changes to the groups
	for(unsigned int i = 0; i < m_group_tuples.size(); ++i)
	{
		// reference the GNzbGroup and its EditState
		GNzbGroup& group = std::get<0>(m_group_tuples[i]);
		EditState& state = std::get<1>(m_group_tuples[i]);

		// A groups metric is determined by the position it ended up in
		// TreeView list...this is checked for each group at this time
		const int metric = getMetricValueFromListPosition(i);

		try
		{
			// EditState::DELETED has to be deleted from the DB
			if(EditState::DELETED == state)
			{
				// make sure this was not a new, uncommited to DB, group
				// which was "deleted" by the user (i.e. added as new then
				// deleted before dismissing the dialog)
				if(group.isDbRecord())
					m_groups_db.delete_group(group);
			}
			// EditState::NEW has to be inserted
			else if(EditState::NEW == state)
			{
				group.setMetric(metric);
				m_groups_db.insert_group(group);
			}
			// EditState::DIRTY or metric change hase to be updated
			else if((EditState::DIRTY == state) || (group.getMetric() != metric))
			{
				group.setMetric(metric);
				m_groups_db.update_group(group);
			}
		}
		catch(const Sqlite3::Error& e)
		{
			show_error(
				*mpGroupsDialog,
				"NZB Groups Database Error",
				Glib::ustring::compose("%1: %2", group.getName(), e.what()));
		}

		// A state of EditState::DIRTY requires an update
	}

	mpGroupsDialog->response(Gtk::RESPONSE_ACCEPT);
}

void GroupsInterface::onCancelGroups()
{
	mpGroupsDialog->response(Gtk::RESPONSE_CANCEL);
}

int GroupsInterface::getMaxGroupMetric()
{
	int result = 0;
	for(auto& group_tuple : m_group_tuples)
	{
		GNzbGroup& group = std::get<0>(group_tuple);

		if(result < group.getMetric())
			result = group.getMetric();
	}
	return result;
}

void GroupsInterface::onNzbGroupColorSet()
{
	if(0 < mpGroupTreeView->get_selection()->count_selected_rows())
	{
		Glib::RefPtr<GroupsListStore> ref_model
			= Glib::RefPtr<GroupsListStore>::cast_dynamic(mpGroupTreeView->get_model());

		Gtk::TreeIter sel_iter = mpGroupTreeView->get_selection()->get_selected();
		const int group_idx = (*sel_iter)[ref_model->columns().col_entry_idx()];

		// reference the group and its state
		GNzbGroup& group = std::get<0>(m_group_tuples[group_idx]);
		EditState& state = std::get<1>(m_group_tuples[group_idx]);

		// set the user selected color in the group
		group.setColorText(mpBtnGroupColor->get_rgba().to_string().c_str());
		// if not a NEW group then set the state to DIRTY
		if(EditState::NEW != state)
			state = EditState::DIRTY;

		(*sel_iter)[ref_model->columns().col_color()] = mpBtnGroupColor->get_rgba();
	}
}

void GroupsInterface::onNzbGroupsSelection()
{
	// no selection?
	if(0 == mpGroupTreeView->get_selection()->count_selected_rows())
	{
		// clear all widgets and set them insensitive
		mpBtnGroupColor->set_rgba(Gdk::RGBA("#000000"));
		mpBtnGroupColor->set_sensitive(false);
		mpToggleMoveCompleted->set_active(false);
		mpToggleMoveCompleted->set_sensitive(false);
		setMoveToLocation("");
		mpComboMoveLocation->set_active(0);
		mpComboMoveLocation->set_sensitive(false);
		mpToggleMoveTvSeries->set_active(false);
		mpToggleMoveTvSeries->set_sensitive(false);
		mpToggleCleanup->set_active(false);
		mpToggleCleanup->set_sensitive(false);
		mpToggleAutoAssign->set_active(false);
		mpToggleAutoAssign->set_sensitive(false);
		mpEntryNameIncludes->set_text("");
		mpEntryNameIncludes->set_sensitive(false);
		mpEntryNameExcludes->set_text("");
		mpEntryNameExcludes->set_sensitive(false);
		mpEntryMinSize->set_text("");
		mpEntryMinSize->set_sensitive(false);
		mpEntryMaxSize->set_text("");
		mpEntryMaxSize->set_sensitive(false);
	}
	// server is selected
	else
	{
		Glib::RefPtr<GroupsListStore> ref_model
			= Glib::RefPtr<GroupsListStore>::cast_dynamic(mpGroupTreeView->get_model());

		// get the GNzbGroup tuple index from the selected treeview row
		Gtk::TreeIter sel_iter = mpGroupTreeView->get_selection()->get_selected();
		int idx = (*sel_iter)[ref_model->columns().col_entry_idx()];

		// reference the GNzbGroup
		const GNzbGroup& group = std::get<0>(m_group_tuples[idx]);

		// set the GUI widgets according to the selected GNzbGroup
		mpBtnGroupColor->set_rgba(Gdk::RGBA(group.getColorText()));
		mpBtnGroupColor->set_sensitive();
		mpToggleMoveCompleted->set_active(group.doMoveToDir());
		mpToggleMoveCompleted->set_sensitive();
		setMoveToLocation(group.getMoveToDir());
		mpComboMoveLocation->set_sensitive(group.doMoveToDir());
		mpToggleMoveTvSeries->set_active(group.doMoveToTvSeriesDir());
		mpToggleMoveTvSeries->set_sensitive(group.doMoveToDir());
		mpToggleCleanup->set_active(group.doCleanup());
		mpToggleCleanup->set_sensitive();
		mpToggleAutoAssign->set_active(group.doAutoAssignGroup());
		mpToggleAutoAssign->set_sensitive();
		mpEntryNameIncludes->set_text(group.getNameIncludesCSV().c_str());
		mpEntryNameIncludes->set_sensitive(group.doAutoAssignGroup());
		mpEntryNameExcludes->set_text(group.getNameExcludesCSV().c_str());
		mpEntryNameExcludes->set_sensitive(group.doAutoAssignGroup());
		if(0 == group.getMinSizeMB())
			mpEntryMinSize->set_text("");
		else
			mpEntryMinSize->set_text(Glib::ustring::compose("%1", group.getMinSizeMB()));
		mpEntryMinSize->set_sensitive(group.doAutoAssignGroup());
		if(0 == group.getMaxSizeMB())
			mpEntryMaxSize->set_text("");
		else
			mpEntryMaxSize->set_text(Glib::ustring::compose("%1", group.getMaxSizeMB()));
		mpEntryMaxSize->set_sensitive(group.doAutoAssignGroup());
	}
}

void GroupsInterface::onNzbGroupNameEdited(const Glib::ustring& path, const Glib::ustring& new_text)
{
	Glib::RefPtr<GroupsListStore> ref_model
		= Glib::RefPtr<GroupsListStore>::cast_dynamic(mpGroupTreeView->get_model());

	// reference the target GNzbGroup and EditState by getting the
	// tuple index from the selected treeview item
	Gtk::TreeIter iter = ref_model->get_iter(path);
	int idx = (*iter)[ref_model->columns().col_entry_idx()];
	GNzbGroup& group = std::get<0>(m_group_tuples[idx]);
	EditState& state = std::get<1>(m_group_tuples[idx]);

	// set the group name to the user entered text and
	// mark the group as DIRTY if it is not maked NEW
	group.setName(new_text.c_str());
	if(EditState::NEW != state)
		state = EditState::DIRTY;

	// we must set the text in the treeview model since the
	// handling of the signal is being done here
	(*iter)[ref_model->columns().col_name()] = new_text;
}

void GroupsInterface::onAddNzbGroup()
{
	// add a new PrefsNntpServer instance
	const int idx = m_group_tuples.size();
	m_group_tuples.emplace_back(
		GNzbGroup(getMaxGroupMetric() + 1, "New group", "#000000", false, false, true), EditState::NEW);

	// add it to the TreeView
	Glib::RefPtr<GroupsListStore> ref_model
		= Glib::RefPtr<GroupsListStore>::cast_dynamic(mpGroupTreeView->get_model());
	Gtk::TreeIter iter = ref_model->add_group(idx, std::get<0>(m_group_tuples[idx]));

	// set the name to be in edit mode
	Gtk::TreeViewColumn *pColName = mpGroupTreeView->get_column(1);
	Gtk::CellRenderer *pRendererName = mpGroupTreeView->get_column_cell_renderer(1);
	mpGroupTreeView->grab_focus();
	mpGroupTreeView->set_cursor(Gtk::TreePath(iter), *pColName, *pRendererName, true);
}

void GroupsInterface::onRemoveNzbGroup()
{
	if(0 < mpGroupTreeView->get_selection()->count_selected_rows())
	{
		Glib::RefPtr<GroupsListStore> ref_model
			= Glib::RefPtr<GroupsListStore>::cast_dynamic(mpGroupTreeView->get_model());

		Gtk::TreeIter sel_iter = mpGroupTreeView->get_selection()->get_selected();
		int idx = (*sel_iter)[ref_model->columns().col_entry_idx()];

		Glib::ustring make_sure_msg = Glib::ustring::compose(
			"You are about to delete the group named %1.\n"
			"Are you sure you want to delete the selected group?",
			std::get<0>(m_group_tuples[idx]).getName());

		// make sure the user wants to delete the server
		const int response = get_yes_or_no(*mpGroupsDialog, "Delete Selected Server?", make_sure_msg);
		if(Gtk::RESPONSE_NO == response)
			return;

		// remove the treeview row and mark state as DELETED
		ref_model->erase(sel_iter);
		std::get<1>(m_group_tuples[idx]) = EditState::DELETED;
	}
}

void GroupsInterface::onNzbGroupColorCellData(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter)
{
	BulletPointRenderer *pBulletRenderer = (BulletPointRenderer*)cell;

	Glib::RefPtr<GroupsListStore> ref_model
		= Glib::RefPtr<GroupsListStore>::cast_dynamic(mpGroupTreeView->get_model());

	pBulletRenderer->set_color((*iter)[ref_model->columns().col_color()]);
}

void GroupsInterface::initNzbGroupsList()
{
	Glib::RefPtr<GroupsListStore> ref_model = Glib::RefPtr<GroupsListStore>(new GroupsListStore);

	// set up the tree view
	mpGroupTreeView->set_model(ref_model);
	mpBulletRenderer = new BulletPointRenderer;
	mpBulletRenderer->set_fixed_size(16, 16);
	mpGroupTreeView->append_column("", *mpBulletRenderer);
	mpGroupTreeView->append_column("Group Name", ref_model->columns().col_name());
	mpGroupTreeView->get_column(0)->set_cell_data_func(*mpBulletRenderer, sigc::mem_fun(*this, &GroupsInterface::onNzbGroupColorCellData));

	// monitor list selection changes
	mpGroupTreeView->get_selection()->signal_changed().connect(sigc::mem_fun(*this, &GroupsInterface::onNzbGroupsSelection));

	std::vector<GNzbGroup> groups =	m_groups_db.get_groups();
	for(unsigned int i = 0; i < groups.size(); ++i)
	{
		// don't insert the "No group" entry it is not supposed to be edited
		if(0 == groups[i].get_db_id()) continue;

		// add a group tuple and add to the treeview model
		m_group_tuples.emplace_back(groups[i], EditState::CLEAN);
		ref_model->add_group(m_group_tuples.size() - 1, std::get<0>(m_group_tuples.back()));
	}

	// let the server aliases be edited in the list
	Gtk::CellRendererText *pNameRenderer = (Gtk::CellRendererText*)mpGroupTreeView->get_column_cell_renderer(1);
	pNameRenderer->property_editable() = true;
	pNameRenderer->signal_edited().connect(sigc::mem_fun(*this, &GroupsInterface::onNzbGroupNameEdited));

	// connect the add/remove group button signals
	mpBtnAddGroup->signal_clicked().connect(sigc::mem_fun(*this, &GroupsInterface::onAddNzbGroup));
	mpBtnRemoveGroup->signal_clicked().connect(sigc::mem_fun(*this, &GroupsInterface::onRemoveNzbGroup));

	// select the first NNTP server listed
	mpGroupTreeView->get_selection()->select(ref_model->children().begin());

	// allow for the rows to be re-ordered so that the group search order can be set by the user
	mpGroupTreeView->set_reorderable();
}

void GroupsInterface::onGroupToggleChanged(Gtk::ToggleButton *pToggle)
{
	// ignoe if there is no group selected (should not happen)
	if(0 >= mpGroupTreeView->get_selection()->count_selected_rows())
		return;

	// get the tuple index from the selected item in the treeview
	Glib::RefPtr<GroupsListStore> ref_model
		= Glib::RefPtr<GroupsListStore>::cast_dynamic(mpGroupTreeView->get_model());
	Gtk::TreeIter sel_iter = mpGroupTreeView->get_selection()->get_selected();
	const int idx = (*sel_iter)[ref_model->columns().col_entry_idx()];

	// reference the group and its state
	GNzbGroup& group = std::get<0>(m_group_tuples[idx]);
	EditState& state = std::get<1>(m_group_tuples[idx]);

	const bool isActive = pToggle->get_active();

	// the cleanup option
	if(pToggle == mpToggleCleanup)
		group.setDoCleanup(isActive);
	// the move to Season X option
	else if(pToggle == mpToggleMoveTvSeries)
		group.setDoMoveToTvSeriesDir(isActive);
	// the "Move Completed To" option
	else if(pToggle == mpToggleMoveCompleted)
	{
		mpComboMoveLocation->set_sensitive(isActive);
		mpToggleMoveTvSeries->set_sensitive(isActive);
		group.setDoMoveToDir(isActive);
	}
	else if(pToggle == mpToggleAutoAssign)
	{
		mpEntryNameIncludes->set_sensitive(isActive);
		mpEntryNameExcludes->set_sensitive(isActive);
		mpEntryMinSize->set_sensitive(isActive);
		mpEntryMaxSize->set_sensitive(isActive);
		group.setAutoAssignGroup(isActive);
	}

	// set edit-dirty if not new
	if(EditState::NEW != state)
		state = EditState::DIRTY;
}

bool GroupsInterface::onKeyRelease(_GdkEventKey* pEventKey, Gtk::Widget *pWidget)
{
	if(0 < mpGroupTreeView->get_selection()->count_selected_rows())
	{
		Glib::RefPtr<GroupsListStore> ref_model
			= Glib::RefPtr<GroupsListStore>::cast_dynamic(mpGroupTreeView->get_model());

		// get the group tuple index from the treeview selected item
		Gtk::TreeIter sel_iter = mpGroupTreeView->get_selection()->get_selected();
		const int idx = (*sel_iter)[ref_model->columns().col_entry_idx()];

		// reference the group and its state
		GNzbGroup& group = std::get<0>(m_group_tuples[idx]);
		EditState& state = std::get<1>(m_group_tuples[idx]);

		if(pWidget == mpEntryNameIncludes)
			group.setNameIncludes(mpEntryNameIncludes->get_text().c_str());
		else if(pWidget == mpEntryNameExcludes)
			group.setNameExcludes(mpEntryNameExcludes->get_text().c_str());
		else if(pWidget == mpEntryMinSize)
			group.setMinSizeMB(std::stoul(mpEntryMinSize->get_text().c_str()));
		else if(pWidget == mpEntryMaxSize)
			group.setMaxSizeMB(std::stoul(mpEntryMaxSize->get_text().c_str()));

		if(EditState::NEW != state)
			state = EditState::DIRTY;
	}

	// allow futher processing
	return false;
}

void GroupsInterface::setMoveToLocation(const std::string& path)
{
	Glib::RefPtr<FolderSelectionListStore> ref_model
		= Glib::RefPtr<FolderSelectionListStore>::cast_dynamic(mpComboMoveLocation->get_model());
	ref_model->setDirectory(path);
	if(path.empty())
		ref_model->setDirectoryIcon(get_icon_for_path("/"));
	else
		ref_model->setDirectoryIcon(get_icon_for_path(path));
}

void GroupsInterface::onMoveToLocationChanged()
{
	Glib::RefPtr<FolderSelectionListStore> ref_location_model
		= Glib::RefPtr<FolderSelectionListStore>::cast_dynamic(mpComboMoveLocation->get_model());

	Gtk::TreeIter iter = mpComboMoveLocation->get_active();
	if((*iter)[ref_location_model->columns().type()] == FolderSelectionListStore::SELECT)
	{
		// get the index for the currently selected group
		Glib::RefPtr<GroupsListStore> ref_model
			= Glib::RefPtr<GroupsListStore>::cast_dynamic(mpGroupTreeView->get_model());
		Gtk::TreeIter selIter = mpGroupTreeView->get_selection()->get_selected();
		const int idx = (*selIter)[ref_model->columns().col_entry_idx()];

		// reference the group and its state
		GNzbGroup& group = std::get<0>(m_group_tuples[idx]);
		EditState& state = std::get<1>(m_group_tuples[idx]);

		// let the user choose the directory
		Gtk::FileChooserDialog dirChooser("Select Location...", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
		dirChooser.set_create_folders();
		dirChooser.add_button("Cancel", Gtk::RESPONSE_CANCEL);
		dirChooser.add_button("Select", Gtk::RESPONSE_ACCEPT);
		switch(dirChooser.run())
		{
			case Gtk::RESPONSE_ACCEPT:
				group.setMoveToDir(dirChooser.get_filename().c_str());
				ref_location_model->setDirectory(dirChooser.get_filename().c_str());
				ref_location_model->setDirectoryIcon(get_icon_for_path(dirChooser.get_filename()));
				if(EditState::NEW != state)
					state = EditState::DIRTY;
				// fall through, because the first item is to always be selected
			default:
				mpComboMoveLocation->set_active(ref_location_model->children().begin());
				break;
		}
	}
}

void GroupsInterface::initWidgets()
{
	// "move completed to" combo box selector
	Glib::RefPtr<FolderSelectionListStore> moveToLocationModel
		= Glib::RefPtr<FolderSelectionListStore>(new FolderSelectionListStore);
	mpComboMoveLocation->set_model(moveToLocationModel);
	mpComboMoveLocation->set_row_separator_func(sigc::mem_fun(*moveToLocationModel.operator->(), &FolderSelectionListStore::isRowSeparator));
	setMoveToLocation("");
	mpComboMoveLocation->set_active(0);
	mpComboMoveLocation->signal_changed().connect(sigc::mem_fun(*this, &GroupsInterface::onMoveToLocationChanged));

	Gtk::CellRendererPixbuf *pPixbufRenderer = new Gtk::CellRendererPixbuf;
	Gtk::CellRendererText *pTextRenderer = new Gtk::CellRendererText;

	mpComboMoveLocation->pack_start(*pPixbufRenderer, false);
	mpComboMoveLocation->pack_end(*pTextRenderer);
	mpComboMoveLocation->add_attribute(*pPixbufRenderer, "gicon", moveToLocationModel->columns().icon());
	mpComboMoveLocation->add_attribute(*pTextRenderer, "text", moveToLocationModel->columns().text());

	// listen for color selection changes
	mpBtnGroupColor->signal_color_set().connect(sigc::mem_fun(*this, &GroupsInterface::onNzbGroupColorSet));

	// listen for toggle changes
	mpToggleMoveCompleted->signal_toggled().connect(
		sigc::bind<Gtk::ToggleButton*>(sigc::mem_fun(*this, &GroupsInterface::onGroupToggleChanged), mpToggleMoveCompleted));
	mpToggleMoveTvSeries->signal_toggled().connect(
		sigc::bind<Gtk::ToggleButton*>(sigc::mem_fun(*this, &GroupsInterface::onGroupToggleChanged), mpToggleMoveTvSeries));
	mpToggleCleanup->signal_toggled().connect(
		sigc::bind<Gtk::ToggleButton*>(sigc::mem_fun(*this, &GroupsInterface::onGroupToggleChanged), mpToggleCleanup));
	mpToggleAutoAssign->signal_toggled().connect(
		sigc::bind<Gtk::ToggleButton*>(sigc::mem_fun(*this, &GroupsInterface::onGroupToggleChanged), mpToggleAutoAssign));

	// listen for entry typing
	mpEntryNameIncludes->signal_key_release_event().connect(
		sigc::bind<Gtk::Widget*>(sigc::mem_fun(*this, &GroupsInterface::onKeyRelease), mpEntryNameIncludes));
	mpEntryNameExcludes->signal_key_release_event().connect(
		sigc::bind<Gtk::Widget*>(sigc::mem_fun(*this, &GroupsInterface::onKeyRelease), mpEntryNameExcludes));
	mpEntryMinSize->signal_key_release_event().connect(
		sigc::bind<Gtk::Widget*>(sigc::mem_fun(*this, &GroupsInterface::onKeyRelease), mpEntryMinSize));
	mpEntryMaxSize->signal_key_release_event().connect(
		sigc::bind<Gtk::Widget*>(sigc::mem_fun(*this, &GroupsInterface::onKeyRelease), mpEntryMaxSize));
}