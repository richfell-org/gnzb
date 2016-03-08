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
#include <gdkmm/rectangle.h>

void rectGetCentered(const Gdk::Rectangle& whole, Gdk::Rectangle& centered, int width, int height)
{
	centered.set_x(whole.get_x() + ((whole.get_width() - width) / 2) + 1);
	centered.set_y(whole.get_y() + ((whole.get_height() - height) / 2) + 1);
	centered.set_width(width);
	centered.set_height(height);
}

int rectGetBottom(const Gdk::Rectangle& rect)
{
	return rect.get_y() + rect.get_height() - 1;
}

void rectAdjustArea(Gdk::Rectangle& rect, int offsetWidth, int offsetHeight)
{
	if(0 != offsetWidth)
		rect.set_width(rect.get_width() + offsetWidth);
	if(0 != offsetHeight)
		rect.set_height(rect.get_height() + offsetHeight);
}

void rectAdjustOrigin(Gdk::Rectangle& rect, int offsetX, int offsetY)
{
	if(0 != offsetX)
		rect.set_x(rect.get_x() + offsetX);
	if(0 != offsetY)
		rect.set_y(rect.get_y() + offsetY);
}

void rectAdjust(Gdk::Rectangle& rect, int offsetX, int offsetY, int offsetWidth, int offsetHeight)
{
	rectAdjustOrigin(rect, offsetX, offsetY);
	rectAdjustArea(rect, offsetWidth, offsetHeight);
}

void rectMoveVertical(Gdk::Rectangle& rect, int delta)
{
	rectAdjustOrigin(rect, 0, delta);
}

void rectMoveHorizontal(Gdk::Rectangle& rect, int delta)
{
	rectAdjustOrigin(rect, delta, 0);
}

void div2RectVertical(const Gdk::Rectangle& whole, Gdk::Rectangle& top, Gdk::Rectangle& bottom)
{
	int newHeight = whole.get_height() / 2;

	// top
	top.set_x(whole.get_x());
	top.set_y(whole.get_y());
	top.set_width(whole.get_width());
	top.set_height(newHeight);

	// bottom
	bottom.set_x(whole.get_x());
	bottom.set_y(whole.get_y() + newHeight);
	bottom.set_width(whole.get_width());
	bottom.set_height(newHeight);
}

void div3RectVertical(
	const Gdk::Rectangle& whole,
	Gdk::Rectangle& top,
	Gdk::Rectangle& middle,
	Gdk::Rectangle& bottom)
{
	int curX = whole.get_x();
	int curY = whole.get_y();
	int curWidth = whole.get_width();

	// calculate the height for each sub-rectangle
	int subHeight = whole.get_height() / 3;

	// top
	top.set_x(curX);
	top.set_y(curY);
	top.set_width(curWidth);
	top.set_height(subHeight);

	curY += subHeight;

	// middle
	middle.set_x(curX);
	middle.set_y(curY);
	middle.set_width(curWidth);
	middle.set_height(subHeight);

	curY += subHeight;

	// bottom
	bottom.set_x(curX);
	bottom.set_y(curY);
	bottom.set_width(curWidth);
	bottom.set_height(subHeight);
}

bool isPointWithinRect(const Gdk::Rectangle& r, int x, int y)
{
	register int rectRight = r.get_x() + r.get_width();
	register int rectBot = r.get_y() + r.get_height();

	return x >= r.get_x() && x < rectRight && y >= r.get_y() && y < rectBot; 
}