/*
	gnzb python2 DSO plugin - gnzb python2 integration

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
#include "gnzbpyobject.h"
#include <gnzb.h>

#include <iostream>

void GnzbPyObject::init_type()
{
}

GnzbPyObject::GnzbPyObject(const std::shared_ptr<GNzb>& ptr_gnzb)
	m_ptr_gnzb(ptr_gnzb)
{
}

GnzbPyObject::~GnzbPyObject()
{
}