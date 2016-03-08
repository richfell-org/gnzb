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
#include "toolbar.h"
#include <giomm/action.h>
#include <gtkmm/separatortoolitem.h>
#include <gtkmm/builder.h>
#include "toolbutton.h"
#include "toggletoolbutton.h"
#include "../../uiresource.h"

GNzbToolbar::GNzbToolbar(GtkToolbar *p_object, Glib::RefPtr<Gtk::Builder>& ref_builder)
:   Gtk::Toolbar(p_object),
	m_sig_conns(BTN_COUNT)
{
	set_toolbar_style(Gtk::TOOLBAR_ICONS);

	append(*(new GNzbToolButton(
		"Add NZB file", ImageResourcePath("icons/toolbar/add.png"), ImageResourcePath("icons/toolbar/add-selected.png"), false, 32)));

	add_separator(15);

	append(*(new GNzbToolButton(
		"Resume all downloads", ImageResourcePath("icons/toolbar/resume.png"), ImageResourcePath("icons/toolbar/resume-selected.png"), false, 32)));
	append(*(new GNzbToolButton(
		"Pause all downloads", ImageResourcePath("icons/toolbar/pause.png"), ImageResourcePath("icons/toolbar/pause-selected.png"), false, 32)));
	append(*(new GNzbToolButton(
		"Cancel all downloads", ImageResourcePath("icons/toolbar/cancel.png"), ImageResourcePath("icons/toolbar/cancel-selected.png"), false, 32)));

	add_separator(15);

	append(*(new GNzbToggleToolButton(
		"Show/Hide NZB details", ImageResourcePath("icons/toolbar/detailview.png"), ImageResourcePath("icons/toolbar/detailview-selected.png"), 32)));
	append(*(new GNzbToggleToolButton(
		"Show/Hide NZB summary", ImageResourcePath("icons/toolbar/summary.png"), ImageResourcePath("icons/toolbar/summary-selected.png"), 32)));

	add_separator(15);

	append(*(new GNzbToolButton(
		"Preferences", ImageResourcePath("icons/toolbar/preferences.png"), ImageResourcePath("icons/toolbar/preferences-selected.png"), false, 32)));

	append(*(new GNzbToolButton(
		"About", ImageResourcePath("icons/toolbar/info.png"), ImageResourcePath("icons/toolbar/info-selected.png"), false, 32)));
}

GNzbToolbar::~GNzbToolbar()
{
	register int n_items = get_n_items();
	for(register int i = 0; i < n_items; ++i)
	{
		Gtk::ToolItem *p_item = get_nth_item(i);
		remove(*p_item);
		delete p_item;
	}
}

Gtk::ToolButton *GNzbToolbar::get_button(ButtonId id)
{
	Gtk::ToolButton *result = (Gtk::ToolButton*)get_nth_item(id);
	return result;
}

void GNzbToolbar::set_clicked_handler(ButtonId id, const sigc::slot<void>& handler_slot)
{
	// check for existing signal connection(s)
	disconnect(id);

	// connect to the given slot
	std::get<SIG_ACTION>(m_sig_conns[id]) = get_button(id)->signal_clicked().connect(handler_slot);
}

void GNzbToolbar::set_toggled_handler(ButtonId id, const sigc::slot<void>& handler_slot)
{
	// check for existing signal connection(s)
	disconnect(id);

	// connect to the given slot
	Gtk::ToggleToolButton *p_tool_toggle = dynamic_cast<Gtk::ToggleToolButton*>(get_button(id));
	std::get<0>(m_sig_conns[id]) = p_tool_toggle->signal_toggled().connect(handler_slot);
}

void GNzbToolbar::set_clicked_action(ButtonId id, Glib::RefPtr<Gio::Action> ref_action)
{
	// check for existing signal connection(s)
	disconnect(id);

	// set the button sensitivity according to the enabled state of the given action
	Gtk::ToolButton *p_button = get_button(id);
	if(nullptr == p_button) return;
	p_button->set_sensitive(ref_action->property_enabled());

	// we can also track action "enabled" changes
	std::get<SIG_ENABLED>(m_sig_conns[id]) = ref_action->property_enabled().signal_changed().connect(
		[p_button, ref_action](){p_button->set_sensitive(ref_action->property_enabled());});

	// the button's signal_clicked will call the Gio::Action instance's activate method
	std::get<SIG_ACTION>(m_sig_conns[id]) = p_button->signal_clicked().connect([ref_action](){ref_action->activate();});
}

void GNzbToolbar::set_toggled_action(ButtonId id, Glib::RefPtr<Gio::Action> ref_action)
{
	// check for existing signal connection(s)
	disconnect(id);

	// set the toggle button's sensitivity and state according to the action
	Gtk::ToggleToolButton *p_tool_toggle = dynamic_cast<Gtk::ToggleToolButton*>(get_button(id));
	if(nullptr == p_tool_toggle) return;
	p_tool_toggle->set_sensitive(ref_action->property_enabled());
	p_tool_toggle->set_active(get_action_state(ref_action));

	// track the state of the action
	std::get<SIG_STATE>(m_sig_conns[id]) = ref_action->property_state().signal_changed().connect(
		[this, id, p_tool_toggle, ref_action]()
		{
			signal_block(id);
			p_tool_toggle->set_active(get_action_state(ref_action));
			signal_unblock(id);
		});

	// we can also track action "enabled" changes
	std::get<SIG_ENABLED>(m_sig_conns[id]) = ref_action->property_enabled().signal_changed().connect(
		[p_tool_toggle, ref_action](){p_tool_toggle->set_sensitive(ref_action->property_enabled());});

	// the button's signal_toggled will call the Gio::Action instance's activate method
	std::get<SIG_ACTION>(m_sig_conns[id])= p_tool_toggle->signal_toggled().connect([ref_action](){ref_action->activate();});
}

void GNzbToolbar::signal_block(ButtonId id)
{
	sigc::connection sigaction = std::get<SIG_ACTION>(m_sig_conns[id]);
	if(sigaction.connected())
		sigaction.block();
}

void GNzbToolbar::signal_unblock(ButtonId id)
{
	sigc::connection sigaction = std::get<SIG_ACTION>(m_sig_conns[id]);
	if(sigaction.connected())
		sigaction.unblock();
}

bool GNzbToolbar::is_signal_blocked(ButtonId id) const
{
	bool result = false;
	sigc::connection sigaction = std::get<SIG_ACTION>(m_sig_conns[id]);
	if(sigaction.connected())
		result = sigaction.blocked();
	return result;
}

void GNzbToolbar::disconnect(ButtonId id)
{
	// disconnect any connected signal
	sigc::connection sigaction, sigactive, sigstate;
	std::tie(sigaction, sigactive, sigstate) = m_sig_conns[id];
	if(sigaction.connected()) sigaction.disconnect();
	if(sigactive.connected()) sigactive.disconnect();
	if(sigstate.connected()) sigstate.disconnect();		
}

void GNzbToolbar::add_separator(int width, bool bDdraw/* = true*/)
{
	Gtk::SeparatorToolItem *pSep = new Gtk::SeparatorToolItem;
	pSep->set_size_request(width);
	pSep->set_draw(bDdraw);
	pSep->show();
	append(*pSep);
}

bool GNzbToolbar::get_action_state(const Glib::RefPtr<Gio::Action>& ref_action)
{
	static Glib::Variant<bool> bool_variant_true = Glib::Variant<bool>::create(true);

	Glib::VariantBase state_value = ref_action->get_state_variant();
	return state_value.equal(bool_variant_true);
}