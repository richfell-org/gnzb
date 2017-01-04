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
#include "nzbtreeview.h"
#include "nzbliststore.h"
#include "../util/recttools.h"

NzbTreeView::NzbTreeView()
:	Glib::ObjectBase("nzbtreeview"),
	Gtk::TreeView(),
	m_view_col("NZB"),
	m_renderer_nzb(14)
{
	// configure the view
	set_model(Glib::RefPtr<Gtk::TreeModel>(new NzbListStore()));
	set_headers_visible(false);

	// configure columns and renderers
	m_view_col.property_sizing() = Gtk::TREE_VIEW_COLUMN_FIXED ;
	m_renderer_nzb.property_margin() = 7;

	init_columns();
}

NzbTreeView::NzbTreeView(GtkTreeView *p_object, const Glib::RefPtr<Gtk::Builder>& ref_builder)
:	Glib::ObjectBase("nzbtreeview"),
	Gtk::TreeView(p_object),
	m_view_col("NZB"),
	m_renderer_nzb(14)
{
	// configure the view
	set_model(Glib::RefPtr<Gtk::TreeModel>(new NzbListStore()));
	set_headers_visible(false);

	// configure columns and renderers
	m_view_col.property_sizing() = Gtk::TREE_VIEW_COLUMN_FIXED ;
	m_renderer_nzb.property_margin() = 7;

	init_columns();
}

NzbTreeView::~NzbTreeView()
{
}

Glib::RefPtr<NzbListStore> NzbTreeView::get_nzb_model()
{
	return Glib::RefPtr<NzbListStore>::cast_dynamic(get_model());
}

Glib::RefPtr<const NzbListStore> NzbTreeView::get_nzb_model() const
{
	return Glib::RefPtr<const NzbListStore>::cast_dynamic(get_model());
}

/*
 * This is the definition of GdkEventButton
 *
struct _GdkEventButton
{
  GdkEventType type;
  GdkWindow *window;
  gint8 send_event;
  guint32 time;
  gdouble x;
  gdouble y;
  gdouble pressure;
  gdouble xtilt;
  gdouble ytilt;
  guint state;
  guint button;
  GdkInputSource source;
  guint32 deviceid;
  gdouble x_root, y_root;
};
*/

/**
 * If the x,y coords for the event are within a GNzbAction button
 * image then the path to the row is set in "path" and the action
 * enum value for action one or two is returned.  Else, ACTION_NONE
 * is returned and "path" is unchanged.
 * 
 */
int NzbTreeView::get_action_for_event(const GdkEventButton& event, Gtk::TreePath& path)
{
	int cell_x, cell_y;
	Gtk::TreeViewColumn *p_column{nullptr};
	if(get_path_at_pos(event.x, event.y, path, p_column, cell_x, cell_y))
	{
		Gdk::Rectangle cell_area;
		get_cell_area(path, *p_column, cell_area);

		GdkRectangle *p_cell_rect = cell_area.gobj();

		// check for x,y being on the action1 button
		if(p_column->get_cell_position(m_action1_renderer, p_cell_rect->x, p_cell_rect->width)
		   && isPointWithinRect(cell_area, int(event.x), int(event.y)))
		{
			Gdk::Rectangle btn_rect;
			rectGetCentered(
        		cell_area, btn_rect,
                m_action1_renderer.get_image_width(),
                m_action1_renderer.get_image_height());
			if(isPointWithinRect(btn_rect, int(event.x), int(event.y)))
				return ACTION_ONE;
		}

		// else check for x,y being on the action2 button
		else if(p_column->get_cell_position(m_action2_renderer, p_cell_rect->x, p_cell_rect->width)
		   && isPointWithinRect(cell_area, int(event.x), int(event.y)))
		{
			Gdk::Rectangle btn_rect;
			rectGetCentered(
        		cell_area, btn_rect,
                m_action2_renderer.get_image_width(),
                m_action2_renderer.get_image_height());
			if(isPointWithinRect(btn_rect, int(event.x), int(event.y)))
				return ACTION_TWO;
		}

		// else check for group button
		else
		{
			// adjust rect for the group button area
			rectAdjustOrigin(
				cell_area,
				cell_area.get_width() + m_renderer_nzb.bullet_renderer().property_xpad(),
				m_renderer_nzb.bullet_renderer().property_ypad());
			cell_area.set_width(m_renderer_nzb.bullet_renderer().property_width());
			cell_area.set_height((cell_area.get_height() / 3) - m_renderer_nzb.bullet_renderer().property_ypad());

			// check for x,y within this area
			if(isPointWithinRect(cell_area, int(event.x), int(event.y)))
				return GROUP;
		}
	}

	// return from here means no action button was at the event point
	return ACTION_NONE;
}

/**
 * Determines if an GNzbAction button was pressed for this
 * button press event.  If so, then the path and which
 * action button is remembered for the button unpress event
 * 
 * event handler...return false to allow other handlers
 * in the chain to be called for the event
 */
bool NzbTreeView::on_button_press_event(GdkEventButton *p_event)
{
	// only want left button presses
	if(p_event->button != 1)
		return Gtk::TreeView::on_button_press_event(p_event);

	Glib::RefPtr<NzbListStore> ref_store = get_nzb_model();

	const int action = get_action_for_event(*p_event, m_action_pressed_path.path());
	switch(action)
	{
		case ACTION_ONE:
			m_action_pressed_path.action() = action;
			(*ref_store->get_iter(m_action_pressed_path.path()))[ref_store->columns().action1_pressed()] = true;
			return true;
			break;
		case ACTION_TWO:
			m_action_pressed_path.action() = action;
			(*ref_store->get_iter(m_action_pressed_path.path()))[ref_store->columns().action2_pressed()] = true;
			return true;
			break;
		case GROUP:
			m_action_pressed_path.action() = action;
			return true;
			break;
		case ACTION_NONE:
		default:
			break;
	}

	return Gtk::TreeView::on_button_press_event(p_event);
}

/**
 * Determines if a GNzbAction button was "clicked" as a result
 * of this button release event.  If the button release is over
 * the same action button (if any) as was remembered in the
 * button pressed event then the signal for that action will be
 * emitted.
 * 
 * event handler...return false to allow other handlers
 * in the chain to be called for the event
 */
bool NzbTreeView::on_button_release_event(GdkEventButton *p_event)
{
	// only want left button presses
	if((p_event->button != 1) || !m_action_pressed_path)
		return Gtk::TreeView::on_button_press_event(p_event);

	// mark buttons at original pressed path as un-pressed
	Glib::RefPtr<NzbListStore> ref_store = get_nzb_model();
	Gtk::TreeIter iter = ref_store->get_iter(m_action_pressed_path.path());
	(*iter)[ref_store->columns().action1_pressed()] = false;
	(*iter)[ref_store->columns().action2_pressed()] = false;

	// check for release over an action button
	Gtk::TreePath release_path;
	const int action = get_action_for_event(*p_event, release_path);

	// if the release path is the same as the pressed path then emit
	// a signal for the action that was clicked
	if(release_path && (release_path == m_action_pressed_path.path())
	   && (action == m_action_pressed_path.action()))
	{
		switch(action)
		{
			case ACTION_ONE:
				m_sig_action.emit(m_action_pressed_path.path(), (*iter)[ref_store->columns().action1()]);
				break;
			case ACTION_TWO:
				m_sig_action.emit(m_action_pressed_path.path(), (*iter)[ref_store->columns().action2()]);
				break;
			case GROUP:
				m_sig_group.emit(m_action_pressed_path.path(), p_event);
				break;
			case ACTION_NONE:
			default:
				break;
		}
	}

	// clear the action pressed path
	m_action_pressed_path.clear();

	// no further processing
	return true;
}

void NzbTreeView::init_columns()
{
	remove_all_columns();

	Glib::RefPtr<NzbListStore> ref_store = get_nzb_model();

	// add the NZB action renderers first 
	m_view_col.pack_start(m_action1_renderer, false);
	m_view_col.pack_start(m_action2_renderer, false);

	// add the GNzb renderer
	m_view_col.pack_start(m_renderer_nzb, true);

	// map renderer properties to model values
	// the NZB action renderers
	m_view_col.add_attribute(
		m_action1_renderer,
		m_action1_renderer.property_action().get_name(),
		ref_store->columns().action1());
	m_view_col.add_attribute(
		m_action2_renderer,
		m_action2_renderer.property_action().get_name(),
		ref_store->columns().action2());
	m_view_col.add_attribute(
		m_action1_renderer,
		m_action1_renderer.property_pressed().get_name(),
		ref_store->columns().action1_pressed());
	m_view_col.add_attribute(
		m_action2_renderer,
		m_action2_renderer.property_pressed().get_name(),
		ref_store->columns().action2_pressed());

	// the GNzb renderer
	m_view_col.add_attribute(m_renderer_nzb, m_renderer_nzb.property_nzb().get_name(), ref_store->columns().gnzb());
	m_view_col.add_attribute(m_renderer_nzb, m_renderer_nzb.property_nzb_name().get_name(), ref_store->columns().name());
	m_view_col.add_attribute(
		m_renderer_nzb,
		m_renderer_nzb.property_bullet_rgba_text().get_name(),
		ref_store->columns().group_color_text());

	append_column(m_view_col);
	m_view_col.set_visible();
}
