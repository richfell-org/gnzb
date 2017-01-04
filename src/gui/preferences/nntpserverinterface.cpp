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
#include "nntpserverinterface.h"
#include <memory>
#include <future>
#include <atomic>
#include <exception>
#include <glibmm/ustring.h>
#include <glibmm/main.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/image.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/treeview.h>
#include <gtkmm/messagedialog.h>
#include <libusenet/options.h>
#include <libusenet/nntpclient.h>
#include "widgets/nntpserverliststore.h"
#include "../guiutil.h"
#include "../../uiresource.h"

#define SERVER_CHANGE_TO	2000
#define ATTR_CHANGE_TO		3000

static std::atomic_bool s_cancel_check{false};

/**
 * Funtion which can run in a thread to check NNTP server
 * settings and credentials.
 */
static void check_credentials(
	NntpClient::ServerAddr nntpserver,
	bool use_ssl,
	std::promise<bool> barrier,
	Glib::Dispatcher& dispatcher)
{
	NntpClient::Response response;
	try
	{
		std::unique_ptr<NntpClient::Connection> ptr_conn = use_ssl
			? std::make_unique<NntpClient::SslConnection>()
			: std::make_unique<NntpClient::Connection>();
		ptr_conn->set_timeout(20);
		ptr_conn->open(nntpserver, response);
		barrier.set_value(response.get_status() == NntpClient::ResponseStatus::CMD_OK);
	}
	catch(const std::exception& e)
	{
		barrier.set_value(false);
	}
	catch(...)
	{
		barrier.set_value(false);
	}

	if(!s_cancel_check.load())
		dispatcher();
}

NntpServerInterface::NntpServerInterface()
:	p_server_treeview(nullptr),
	p_entry_url(nullptr),
	p_entry_port(nullptr),
	p_entry_username(nullptr),
	p_entry_password(nullptr),
	p_entry_conncount(nullptr),
	p_entry_retention(nullptr),
	p_entry_volume(nullptr),
	p_entry_used_volume(nullptr),
	p_image_status(nullptr),
	p_toggle_usessl(nullptr),
	p_toggle_volume_warn(nullptr),
	p_toggle_volume_reset_monthly(nullptr),
	p_btn_addserver(nullptr),
	p_btn_removeserver(nullptr),
	p_btn_volume_reset(nullptr)
{
	s_cancel_check.store(false);
}

NntpServerInterface::~NntpServerInterface()
{
	m_to_conn.disconnect();
	s_cancel_check.store(true);
}

void NntpServerInterface::init(const AppPreferences& app_prefs, Glib::RefPtr<Gtk::Builder>& ref_builder)
{
	// reference our widgets, all of these are GTK managed, we do not delete them ourselves
	p_server_treeview = get_managed_widget<Gtk::TreeView>(ref_builder, "preferences.nntpservers.treeview");
	p_entry_url = get_managed_widget<Gtk::Entry>(ref_builder, "preferences.nntpserverurl.entry");
	p_entry_port = get_managed_widget<Gtk::Entry>(ref_builder, "preferences.nntpserverport.entry");
	p_entry_username = get_managed_widget<Gtk::Entry>(ref_builder, "preferences.nntpserverusername.entry");
	p_entry_password = get_managed_widget<Gtk::Entry>(ref_builder, "preferences.nntpserverpassword.entry");
	p_entry_conncount = get_managed_widget<Gtk::Entry>(ref_builder, "preferences.nntpserverconncount.entry");
	p_image_status = get_managed_widget<Gtk::Image>(ref_builder, "preferences.nntpserverstatus.img");
	p_toggle_usessl = get_managed_widget<Gtk::ToggleButton>(ref_builder, "preferences.nntpserverusessl.chk");
	p_entry_retention = get_managed_widget<Gtk::Entry>(ref_builder, "preferences.nntpserverretention.entry");
	p_entry_volume = get_managed_widget<Gtk::Entry>(ref_builder, "preferences.nntpservervolume.entry");
	p_entry_used_volume = get_managed_widget<Gtk::Entry>(ref_builder, "preferences.nntpserverusedvolume.entry");
	p_toggle_volume_warn = get_managed_widget<Gtk::ToggleButton>(ref_builder, "preferences.nntpservervolumewarning.chk");
	p_toggle_volume_reset_monthly = get_managed_widget<Gtk::ToggleButton>(ref_builder, "preferences.nntpserverresetvolumemonthly.chk");
	p_btn_addserver = get_managed_widget<Gtk::Button>(ref_builder, "preferences.nntpserveradd.btn");
	p_btn_removeserver = get_managed_widget<Gtk::Button>(ref_builder, "preferences.nntpserverremove.btn");
	p_btn_volume_reset = get_managed_widget<Gtk::Button>(ref_builder, "preferences.nntpserverusedvolumereset.btn");

#ifndef LIBUSENET_USE_SSL
	p_toggle_usessl->set_visible(false);
	p_toggle_usessl->set_active(false);
#endif  /* LIBUSENET_USE_SSL */

	// allocate a model for the NNTP server TreeView
	Glib::RefPtr<NntpServerListStore> ref_serverstore = Glib::RefPtr<NntpServerListStore>(new NntpServerListStore);

	// set up the tree view
	p_server_treeview->set_model(ref_serverstore);
	p_server_treeview->append_column("", ref_serverstore->columns().col_enabled());
	p_server_treeview->append_column("Server Name", ref_serverstore->columns().col_name());

	// load up the NNTP servers
	m_servers.clear();
	m_server_editstate.clear();
	app_prefs.getNntpServers(m_servers);
	for(unsigned int i = 0; i < m_servers.size(); ++i)
	{
		ref_serverstore->add_server(i, m_servers[i]);
		m_server_editstate.push_back(EditState::CLEAN);
	}

	// if there are no servers disable entry widgets
	if(m_servers.empty())
		on_server_selection();

	// let the server aliases be edited in the list
	Gtk::CellRendererText *pNameRenderer = (Gtk::CellRendererText*)p_server_treeview->get_column_cell_renderer(1);
	pNameRenderer->property_editable() = true;
	pNameRenderer->signal_edited().connect(sigc::mem_fun(*this, &NntpServerInterface::on_name_edited));

	// let the enabled state be change in the list
	Gtk::CellRendererToggle *pToggle = (Gtk::CellRendererToggle*)p_server_treeview->get_column_cell_renderer(0);
	pToggle->set_activatable();
	pToggle->signal_toggled().connect(sigc::mem_fun(*this, &NntpServerInterface::on_enable_toggled));

	// set the selection changed signal handler
	p_server_treeview->get_selection()->signal_changed().connect(
		sigc::mem_fun(*this, &NntpServerInterface::on_server_selection));

	// set the entry area signal handlers
	p_entry_url->signal_key_release_event().connect(
		sigc::bind<Gtk::Widget*>(sigc::mem_fun(*this, &NntpServerInterface::on_key_release), p_entry_url));
	p_entry_port->signal_key_release_event().connect(
		sigc::bind<Gtk::Widget*>(sigc::mem_fun(*this, &NntpServerInterface::on_key_release), p_entry_port));
	p_entry_username->signal_key_release_event().connect(
		sigc::bind<Gtk::Widget*>(sigc::mem_fun(*this, &NntpServerInterface::on_key_release), p_entry_username));
	p_entry_password->signal_key_release_event().connect(
		sigc::bind<Gtk::Widget*>(sigc::mem_fun(*this, &NntpServerInterface::on_key_release), p_entry_password));
	p_entry_conncount->signal_key_release_event().connect(
		sigc::bind<Gtk::Widget*>(sigc::mem_fun(*this, &NntpServerInterface::on_key_release), p_entry_conncount));

#ifdef LIBUSENET_USE_SSL
	p_toggle_usessl->signal_toggled().connect(
		sigc::bind<Gtk::ToggleButton*>(sigc::mem_fun(*this, &NntpServerInterface::on_option_changed), p_toggle_usessl));
#endif  /* LIBUSENET_USE_SSL */

	p_btn_removeserver->signal_clicked().connect(sigc::mem_fun(*this, &NntpServerInterface::on_remove));
	p_btn_addserver->signal_clicked().connect(sigc::mem_fun(*this, &NntpServerInterface::on_add));

	// select the first NNTP server listed
	Gtk::TreeIter iter_top = ref_serverstore->children().begin();
	if(iter_top)
		p_server_treeview->get_selection()->select(iter_top);

	// set server test failed image as default
	p_image_status->set(Gdk::Pixbuf::create_from_resource(ImageResourcePath("icons/preferences/credentials-failed.png")));

	// start in unmodified state
	set_modified(false);

	m_check_dispatch.connect(sigc::mem_fun(*this, &NntpServerInterface::on_check_complete));
}

void NntpServerInterface::save(AppPreferences& app_prefs)
{
	// update/add NNTP servers
	for(unsigned int i = 0; i < m_servers.size(); ++i)
	{
		switch(int(m_server_editstate[i]))
		{
			case int(EditState::DIRTY):
				app_prefs.updateNntpServer(m_servers[i]);
				break;
			case int(EditState::NEW):
				app_prefs.insertNntpServer(m_servers[i]);
				break;
			case int(EditState::DELETED):
				if(m_servers[i].isDbRecord())
					app_prefs.deleteNntpServer(m_servers[i]);
				break;
			case int(EditState::CLEAN):
			default:
				break;
		}
	}
}

void NntpServerInterface::on_name_edited(const Glib::ustring& path, const Glib::ustring& new_text)
{
	Glib::RefPtr<NntpServerListStore> ref_model
		= Glib::RefPtr<NntpServerListStore>::cast_dynamic(p_server_treeview->get_model());

	Gtk::TreeIter iter = ref_model->get_iter(path);
	int server_idx = (*iter)[ref_model->columns().col_entry_idx()];
	PrefsNntpServer *p_server_entry = &m_servers[server_idx];

	p_server_entry->setName(new_text.c_str());
	if(EditState::NEW != m_server_editstate[server_idx])
		m_server_editstate[server_idx] = EditState::DIRTY;

	(*iter)[ref_model->columns().col_name()] = new_text;

	set_modified();
}

void NntpServerInterface::on_enable_toggled(const Glib::ustring& path)
{
	Glib::RefPtr<NntpServerListStore> ref_model
		= Glib::RefPtr<NntpServerListStore>::cast_dynamic(p_server_treeview->get_model());

	Gtk::TreeIter iter = ref_model->get_iter(path);
	int server_idx = (*iter)[ref_model->columns().col_entry_idx()];
	PrefsNntpServer *p_server_entry = &m_servers[server_idx];

	// set the NNTP server's enable to match the toggle state
	p_server_entry->setEnabled(!(*iter)[ref_model->columns().col_enabled()]);

	// mark as dirty if needed
	if(EditState::NEW != m_server_editstate[server_idx])
		m_server_editstate[server_idx] = EditState::DIRTY;

	(*iter)[ref_model->columns().col_enabled()] = p_server_entry->isEnabled();

	set_modified();
}

void NntpServerInterface::on_option_changed(Gtk::ToggleButton *pToggle)
{
	Glib::RefPtr<NntpServerListStore> ref_model
		= Glib::RefPtr<NntpServerListStore>::cast_dynamic(p_server_treeview->get_model());

	// cancel any current timeout
	if(m_to_conn.connected())
		m_to_conn.disconnect();

	if(0 < p_server_treeview->get_selection()->count_selected_rows())
	{
		Gtk::TreeIter sel_iter = p_server_treeview->get_selection()->get_selected();
		int server_idx = (*sel_iter)[ref_model->columns().col_entry_idx()];
		m_servers[server_idx].setSsl(pToggle->get_active());

		// if this is not a new entry then mark as dirty
		if(EditState::NEW != m_server_editstate[server_idx])
			m_server_editstate[server_idx] = EditState::DIRTY;

		set_modified();

		// set a timeout for running the server check
		m_to_conn = Glib::signal_timeout().connect(
			sigc::mem_fun(*this, &NntpServerInterface::on_settings_changed_timeout), ATTR_CHANGE_TO);
	}
}

void NntpServerInterface::on_server_selection()
{
	Glib::RefPtr<NntpServerListStore> ref_model
		= Glib::RefPtr<NntpServerListStore>::cast_dynamic(p_server_treeview->get_model());

	// cancel any current timeout
	if(m_to_conn.connected())
		m_to_conn.disconnect();

	// no selection?
	if(0 == p_server_treeview->get_selection()->count_selected_rows())
	{
		p_entry_url->set_sensitive(false);
		p_entry_port->set_sensitive(false);
#ifdef LIBUSENET_USE_SSL
		p_toggle_usessl->set_active(false);
		p_toggle_usessl->set_sensitive(false);
#endif  /* LIBUSENET_USE_SSL */
		p_entry_username->set_sensitive(false);
		p_entry_password->set_sensitive(false);
		p_entry_conncount->set_sensitive(false);
	}
	// server is selected
	else
	{
		Gtk::TreeIter selIter = p_server_treeview->get_selection()->get_selected();
		int serverIdx = (*selIter)[ref_model->columns().col_entry_idx()];
		PrefsNntpServer *pServerEntry = &m_servers[serverIdx];

		p_entry_url->set_text(pServerEntry->getUrl());
		p_entry_url->set_sensitive();
		p_entry_port->set_text(pServerEntry->getPort());
		p_entry_port->set_sensitive();
#ifdef LIBUSENET_USE_SSL
		p_toggle_usessl->set_active(pServerEntry->useSsl());
		p_toggle_usessl->set_sensitive();
#endif  /* LIBUSENET_USE_SSL */
		p_entry_username->set_text(pServerEntry->getUsername());
		p_entry_username->set_sensitive();
		p_entry_password->set_text(pServerEntry->getPassword());
		p_entry_password->set_sensitive();
		p_entry_conncount->set_text(Glib::ustring::compose("%1", pServerEntry->getConnectionCount()));
		p_entry_conncount->set_sensitive();

		// set a timeout for running the server check
		m_to_conn = Glib::signal_timeout().connect(
			sigc::mem_fun(*this, &NntpServerInterface::on_settings_changed_timeout), SERVER_CHANGE_TO);
	}
}

void NntpServerInterface::on_remove()
{
	// cancel any current timeout
	const bool was_checking = m_to_conn.connected();
	if(was_checking)
		m_to_conn.disconnect();

	if(0 < p_server_treeview->get_selection()->count_selected_rows())
	{
		Glib::RefPtr<NntpServerListStore> ref_model
			= Glib::RefPtr<NntpServerListStore>::cast_dynamic(p_server_treeview->get_model());
		Gtk::TreeIter sel_iter = p_server_treeview->get_selection()->get_selected();
		int server_idx = (*sel_iter)[ref_model->columns().col_entry_idx()];

		Glib::ustring make_sure_msg = Glib::ustring::compose(
			"You are about to delete the server named %1.\n"
			"Are you sure you want to delete the selected server?",
			m_servers[server_idx].getName());

		// make sure the user want to delete the server
		if(Gtk::RESPONSE_NO == get_yes_or_no(*(Gtk::Window*)p_server_treeview->get_toplevel(), "Delete Selected Server?", make_sure_msg))
		{
			// re-issue server check if needed
			if(was_checking)
			{
				m_to_conn = Glib::signal_timeout().connect(
					sigc::mem_fun(*this, &NntpServerInterface::on_settings_changed_timeout), SERVER_CHANGE_TO);
			}
			return;
		}

		ref_model->erase(sel_iter);
		m_server_editstate[server_idx] = EditState::DELETED;

		set_modified();
	}
}

void NntpServerInterface::on_add()
{
	// add a new PrefsNntpServer instance
	int server_idx = m_servers.size();
	m_servers.push_back(PrefsNntpServer("New Server", "news.example.com", "119", 0, 0, 1, false, true));
	m_server_editstate.push_back(EditState::NEW);

	// add it to the TreeView
	Glib::RefPtr<NntpServerListStore> ref_model
		= Glib::RefPtr<NntpServerListStore>::cast_dynamic(p_server_treeview->get_model());
	Gtk::TreeIter iter = ref_model->add_server(server_idx, m_servers[server_idx]);

	// set the name to be in edit mode
	Gtk::TreeViewColumn *p_col_name = p_server_treeview->get_column(1);
	Gtk::CellRenderer *p_renderer_name = p_server_treeview->get_column_cell_renderer(1);
	p_server_treeview->grab_focus();
	p_server_treeview->set_cursor(Gtk::TreePath(iter), *p_col_name, *p_renderer_name, true);

	set_modified();
}

bool NntpServerInterface::on_key_release(_GdkEventKey* p_event, Gtk::Widget *pWidget)
{
	if(0 < p_server_treeview->get_selection()->count_selected_rows())
	{
		Glib::RefPtr<NntpServerListStore> ref_model
			= Glib::RefPtr<NntpServerListStore>::cast_dynamic(p_server_treeview->get_model());
		Gtk::TreeIter selIter = p_server_treeview->get_selection()->get_selected();
		int server_idx = (*selIter)[ref_model->columns().col_entry_idx()];
		PrefsNntpServer *p_server_entry = &m_servers[server_idx];

		if(pWidget == p_entry_url)
			p_server_entry->setUrl(p_entry_url->get_text().c_str());
		else if(pWidget == p_entry_port)
			p_server_entry->setPort(p_entry_port->get_text().c_str());
		else if(pWidget == p_entry_username)
			p_server_entry->setUsername(p_entry_username->get_text().c_str());
		else if(pWidget == p_entry_password)
			p_server_entry->setPassword(p_entry_password->get_text().c_str());
		else if(pWidget == p_entry_conncount)
			p_server_entry->setConnectionCount(atoi(p_entry_conncount->get_text().c_str()));

		if(EditState::NEW != m_server_editstate[server_idx])
			m_server_editstate[server_idx] = EditState::DIRTY;

		set_modified();

		// set the timeout for server/credential checking
		if(pWidget != p_entry_conncount)
		{
			if(m_to_conn.connected()) m_to_conn.disconnect();

			m_to_conn = Glib::signal_timeout().connect(
				sigc::mem_fun(*this, &NntpServerInterface::on_settings_changed_timeout), ATTR_CHANGE_TO);
		}
	}

	return false;
}

PrefsNntpServer const *NntpServerInterface::get_selected_server()
{
	// check for a selection in the NNTP server list box
	if(0 == p_server_treeview->get_selection()->count_selected_rows())
		throw std::runtime_error("No NNTP server selected");

	Glib::RefPtr<NntpServerListStore> ref_model
		= Glib::RefPtr<NntpServerListStore>::cast_dynamic(p_server_treeview->get_model());
	Gtk::TreeIter selIter = p_server_treeview->get_selection()->get_selected();
	int server_idx = (*selIter)[ref_model->columns().col_entry_idx()];
	return &m_servers[server_idx];
}

static NntpClient::ServerAddr to_serveraddr(PrefsNntpServer const *p_prefs_server)
{
	return NntpClient::ServerAddr(
		p_prefs_server->getUrl().c_str(),
		p_prefs_server->getUsername().c_str(),
		p_prefs_server->getPassword().c_str(),
		p_prefs_server->getPort().c_str());
}

void NntpServerInterface::on_check_complete()
{
	m_server_check_future.wait();
	if(m_server_check_future.get())
		p_image_status->set_from_resource(ImageResourcePath("icons/preferences/credentials-passed.png"));
	else
		p_image_status->set_from_resource(ImageResourcePath("icons/preferences/credentials-failed.png"));
}

/**
 * The handler for timeouts on settings changes.  This will
 * attempt to connect to the currently selected NNTP server,
 * using any given credentials, and the results will be indicated
 * using the status icon.
 */
bool NntpServerInterface::on_settings_changed_timeout()
{
	m_to_conn.disconnect();

	try
	{
		PrefsNntpServer const *p_prefs_server = get_selected_server();
		NntpClient::ServerAddr server = to_serveraddr(p_prefs_server);

		std::promise<bool> barrier;
		m_server_check_future = barrier.get_future();
		p_image_status->set_from_resource(ImageResourcePath("icons/preferences/wait-spinner-sm.gif"));
		m_server_check_thread = std::thread(check_credentials, server, p_prefs_server->useSsl(), std::move(barrier), std::ref(m_check_dispatch));
		m_server_check_thread.detach();
	}
	catch(const std::exception& e)
	{
		p_image_status->set_from_resource(ImageResourcePath("icons/preferences/credentials-failed.png"));
	}

	return false;
}
