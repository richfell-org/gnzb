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
#ifndef __NZB_TREEVIEW_HEADER__
#define __NZB_TREEVIEW_HEADER__

#include <glibmm/refptr.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treeviewcolumn.h>
#include "cellrenderer/gnzbcellrenderer.h"
#include "cellrenderer/gnzbactioncellrenderer.h"

namespace Gtk {
	class Builder;
}

class NzbListStore;

/**
 * State variable for tracking the NZB action button
 * which was selected on a key press.
 * 
 */
class ActionButtonPath
{
public:

	ActionButtonPath() {}
	ActionButtonPath(const Gtk::TreePath& p, int a) : m_path(p), m_action(a) {}
	ActionButtonPath(Gtk::TreePath&& p, int a) : m_path(std::move(p)), m_action(a) {}
	~ActionButtonPath() {}

// attrubutes
public:

	Gtk::TreePath& path() { return m_path; }
	const Gtk::TreePath& path() const { return m_path; }

	int& action() { return m_action; }
	const int& action() const { return m_action; }

// operations
public:

	ActionButtonPath& clear() { m_path.clear(); m_action = 0; return *this; }

	ActionButtonPath& operator =(ActionButtonPath&& that)
	{
		m_path = std::move(that.m_path);
		m_action = that.m_action;
		return *this;
	}

	ActionButtonPath& operator =(const ActionButtonPath& that)
	{
		m_path = that.m_path;
		m_action = that.m_action;
		return *this;
	}

	operator bool() const { return m_path && (m_action != 0); }

private:

	Gtk::TreePath m_path;
	int m_action;
};

inline bool operator ==(const ActionButtonPath& lhs, const ActionButtonPath& rhs)
{
	return (lhs.path() == rhs.path()) && (lhs.action() == rhs.action()); 
}

/**
 *
 *
 */
class NzbTreeView : public Gtk::TreeView
{
public:

	NzbTreeView();
	NzbTreeView(GtkTreeView *p_object, const Glib::RefPtr<Gtk::Builder>& ref_builder);
	~NzbTreeView();

// attributes
public:

	Glib::RefPtr<NzbListStore> get_nzb_model();
	Glib::RefPtr<const NzbListStore> get_nzb_model() const;

	// signal for action button clicks
	sigc::signal<void, const Gtk::TreePath&, GNzbAction>& signal_action() { return m_sig_action; }
	const sigc::signal<void, const Gtk::TreePath&, GNzbAction>& signal_action() const { return m_sig_action; }

	// signal for group button clicks
	sigc::signal<void, const Gtk::TreePath&, GdkEventButton*>& signal_group() { return m_sig_group; }
	const sigc::signal<void, const Gtk::TreePath&, GdkEventButton*>& signal_group() const { return m_sig_group; }

// operations
public:

	bool is_selected(const GNzb* p_nzb) const;
	bool is_selected(const GNzb& nzb) const    { return is_selected(&nzb); }

// implementation
private:

	// signals
	sigc::signal<void, const Gtk::TreePath&, GNzbAction> m_sig_action;
	sigc::signal<void, const Gtk::TreePath&, GdkEventButton*> m_sig_group;

	// overrides
	bool on_button_press_event(GdkEventButton *p_event);
	bool on_button_release_event(GdkEventButton *p_event);

	enum { ACTION_NONE, ACTION_ONE, ACTION_TWO, GROUP, };

	int get_action_for_event(const GdkEventButton& event, Gtk::TreePath& path);

	// initialization
	void init_columns();

	// state tracking
	ActionButtonPath m_action_pressed_path;

	// view columns and renderers
	Gtk::TreeViewColumn m_view_col;
	GNzbActionCellRenderer m_action1_renderer, m_action2_renderer;
	GNzbCellRenderer m_renderer_nzb;
};

#endif	/* __NZB_TREEVIEW_HEADER__ */