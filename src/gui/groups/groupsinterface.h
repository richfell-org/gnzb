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
#ifndef __GROUPS_INTERFACE_HEADER__
#define __GROUPS_INTERFACE_HEADER__

#include <tuple>
#include <vector>
#include <string>
#include <sigc++/trackable.h>
#include "../../db/gnzbgroups.h"

struct _GdkEventKey;

namespace Glib {
	class ustring;
}   // namespace Glib

namespace Gtk {
	class Builder;
	class Widget;
	class Window;
	class Dialog;
	class TreeView;
	class Entry;
	class Button;
	class ColorButton;
	class ToggleButton;
	class ComboBox;
	class CellRenderer;
	class TreeModel;
	class TreeIter;
	class TreePath;
}   // namespace Gtk

/*
 * 
 */
class GroupsInterface : public sigc::trackable
{
// construction
public:

	GroupsInterface();
	~GroupsInterface();

// operations
public:

	int run(Gtk::Window *pParentWin = 0);

private:

	void onSaveGroups();
	void onCancelGroups();

	int getMaxGroupMetric();
	int getMetricValueFromListPosition(int iGroup);

	void initNzbGroupsList();
	void onAddNzbGroup();
	void onRemoveNzbGroup();
	void onNzbGroupNameEdited(const Glib::ustring& path, const Glib::ustring& new_text);
	void onNzbGroupColorCellData(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter);
	void onNzbGroupsSelection();
	void onNzbGroupColorSet();

	void initWidgets();
	void onGroupToggleChanged(Gtk::ToggleButton *pToggle);
	bool onKeyRelease(_GdkEventKey* pEventKey, Gtk::Widget *pWidget);
	void setMoveToLocation(const std::string& path);
	void onMoveToLocationChanged();

	enum class EditState { CLEAN, DIRTY, NEW, DELETED, };

	// NZB group data
	GNzbGroups m_groups_db;
	std::vector<std::tuple<GNzbGroup, EditState>> m_group_tuples; 
	Gtk::CellRenderer *mpBulletRenderer;

	// dialog box widgets
	Gtk::Dialog *mpGroupsDialog;
	Gtk::TreeView *mpGroupTreeView;
	Gtk::Button *mpBtnAddGroup;
	Gtk::Button *mpBtnRemoveGroup;
	Gtk::ColorButton *mpBtnGroupColor;
	Gtk::ToggleButton *mpToggleMoveCompleted;
	Gtk::ComboBox *mpComboMoveLocation;
	Gtk::ToggleButton *mpToggleMoveTvSeries;
	Gtk::ToggleButton *mpToggleCleanup;
	Gtk::ToggleButton *mpToggleAutoAssign;
	Gtk::Entry *mpEntryNameIncludes;
	Gtk::Entry *mpEntryNameExcludes;
	Gtk::Entry *mpEntryMinSize;
	Gtk::Entry *mpEntryMaxSize;
};

#endif  /* __GROUPS_INTERFACE_HEADER__ */