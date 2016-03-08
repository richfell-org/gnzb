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
#ifndef __RECT_TOOLS_HEADER__
#define __RECT_TOOLS_HEADER__

namespace Gdk {
	class Rectangle;
}

// sets the rectangle 'centered' to centered within 'whole' with the given width and height
void rectGetCentered(const Gdk::Rectangle& whole, Gdk::Rectangle& centered, int width, int height);

// gets the bottom Y value
int rectGetBottom(const Gdk::Rectangle& rect);

// 
void rectAdjustArea(Gdk::Rectangle& rect, int offsetWidth, int offsetHeight);

// 
void rectAdjustOrigin(Gdk::Rectangle& rect, int offsetX, int offsetY);

// 
void rectAdjust(Gdk::Rectangle& rect, int offsetX, int offsetY, int offsetWidth, int offsetHeight);

// convience: moves a rectangle up/down or accoss
void rectMoveVertical(Gdk::Rectangle& rect, int delta);
void rectMoveHorizontal(Gdk::Rectangle& rect, int delta);

// divides one rectangle vertically into 2 equal sub-rectangular areas
void div2RectVertical(const Gdk::Rectangle& whole, Gdk::Rectangle& top, Gdk::Rectangle& bottom);

// divides one rectangle vertically into 3 equal sub-rectangular areas
void div3RectVertical(const Gdk::Rectangle& whole, Gdk::Rectangle& top, Gdk::Rectangle& middle, Gdk::Rectangle& bottom);

// determines if the X,Y point is within the are specified by the rectangle
bool isPointWithinRect(const Gdk::Rectangle& r, int x, int y);

#endif  /* __RECT_TOOLS_HEADER__ */