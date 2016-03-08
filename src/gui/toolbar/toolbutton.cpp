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
#include "toolbutton.h"
#include <sigc++/sigc++.h>
#include <gdkmm/pixbuf.h>

static void create_pixbuf_for_insensitive(Gtk::Image& src_img, Gtk::Image& dest_img);

GNzbToolButton::GNzbToolButton(
	const Glib::ustring& tooltip,
    const std::string& image_resource_path,
	const std::string& image_slected_resource_path,
    bool create_insensitive_image/* = false*/,
	int minWidth/* = -1*/)
:   m_image(Gdk::Pixbuf::create_from_resource(image_resource_path)),
	m_image_sel(Gdk::Pixbuf::create_from_resource(image_slected_resource_path)),
	m_image_insensitive()
{
	// create an image for when the button is disabled?
	if(create_insensitive_image)
		create_pixbuf_for_insensitive(m_image, m_image_insensitive);

	init_instance(tooltip, minWidth);
}

GNzbToolButton::GNzbToolButton(
	const Glib::ustring& tooltip,
    const std::string& image_resource_path,
	const std::string& image_slected_resource_path,
	const std::string&  image_insensitive_resource_path,
	int minWidth/* = -1*/)
:   m_image(Gdk::Pixbuf::create_from_resource(image_resource_path)),
	m_image_sel(Gdk::Pixbuf::create_from_resource(image_slected_resource_path)),
	m_image_insensitive(Gdk::Pixbuf::create_from_resource(image_insensitive_resource_path))
{
	init_instance(tooltip, minWidth);
}

void GNzbToolButton::on_button_press(GdkEventButton *event)
{
	if(event->button == 1)
		set_icon_widget(m_image_sel);
}

void GNzbToolButton::on_button_release(GdkEventButton *event)
{
	if(event->button == 1)
		set_icon_widget(m_image);
}

void GNzbToolButton::on_state_flags_changed(Gtk::StateFlags previous_state_flags)
{
	if(m_image_insensitive.get_storage_type() == Gtk::IMAGE_EMPTY)
		return;

	Gtk::StateFlags cur_flags = get_state_flags();

	// going to sensitive?
	if((previous_state_flags & Gtk::STATE_FLAG_INSENSITIVE) && !(cur_flags & Gtk::STATE_FLAG_INSENSITIVE))
		set_icon_widget(m_image);

	// going to insensitive?
	else if(!(previous_state_flags & Gtk::STATE_FLAG_INSENSITIVE) && (cur_flags & Gtk::STATE_FLAG_INSENSITIVE))
		set_icon_widget(m_image_insensitive);
}

void GNzbToolButton::init_instance(const Glib::ustring& tooltip, int min_width/* = -1*/)
{
	set_icon_widget(m_image);
	set_tooltip_text(tooltip);
	set_size_request(min_width);
	set_homogeneous();

	// get notified of button pressed events
	Gtk::Widget *pChild = get_children()[0];
	pChild->signal_button_press_event().connect_notify(sigc::mem_fun(*this, &GNzbToolButton::on_button_press));
	pChild->signal_button_release_event().connect_notify(sigc::mem_fun(*this, &GNzbToolButton::on_button_release));

	// responsd to sensitivity changes if we have an insensitive image
	if(m_image_insensitive.get_storage_type() != Gtk::IMAGE_EMPTY)
	{
		signal_state_flags_changed().connect(sigc::mem_fun(*this, &GNzbToolButton::on_state_flags_changed));
		m_image_insensitive.show();
	}

	m_image.show();
	m_image_sel.show();
	show();
}

#include <memory>
#include <gdkmm/general.h>
#include <cairomm/surface.h>

static void create_pixbuf_for_insensitive(Gtk::Image& src_img, Gtk::Image& dest_img)
{
	Glib::RefPtr<Gdk::Pixbuf> ref_src_pixbuf = src_img.get_pixbuf();
	register int width = ref_src_pixbuf->get_width();
	register int height = ref_src_pixbuf->get_height();

	Cairo::RefPtr<Cairo::ImageSurface> ref_surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, width, height);
	ref_surface->reference();

	Cairo::RefPtr<Cairo::Context> ref_ctx = Cairo::Context::create(ref_surface);
	Gdk::Cairo::set_source_pixbuf(ref_ctx, ref_src_pixbuf);
	ref_ctx->paint_with_alpha(0.3);

	Glib::RefPtr<Gdk::Pixbuf> ref_img_copy = Gdk::Pixbuf::create(ref_ctx->get_target(), 0, 0, width, height);
	dest_img.set(ref_img_copy);
}