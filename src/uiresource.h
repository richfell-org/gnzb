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
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/
#ifndef __UI_RESOURCE_HEADER__
#define __UI_RESOURCE_HEADER__

#include <string>

class ResourcePath
{
public:

	virtual ~ResourcePath() {}

	operator const std::string&() const { return m_fqpath; }

protected:

	ResourcePath(const char *base, const char *rel)
		: m_fqpath(base)
	{
		m_fqpath.append(rel);
	}
	ResourcePath(ResourcePath&& that) : m_fqpath(std::move(that.m_fqpath)) {}
	ResourcePath(const ResourcePath& that) : m_fqpath(that.m_fqpath) {}

	std::string m_fqpath;
};

/**
 * Generates a full path to UI definition file resources given a
 * relative path to a specific resource.
 */
class UiResourcePath : public ResourcePath
{
public:

	UiResourcePath(const char *rel_path) : ResourcePath(base_path, rel_path) {}
	UiResourcePath(UiResourcePath&& that) : ResourcePath(std::move(that)) {}
	UiResourcePath(const UiResourcePath& that) : ResourcePath(that) {}
	~UiResourcePath() {}

private:

	// UI resources base path
	static constexpr const char *base_path{"/org/richfell/gnzb/"};
};

/**
 * Generates a full path to image resources given a relative path
 * to a specific resource.
 */
class ImageResourcePath : public ResourcePath
{
public:

	ImageResourcePath(const char *rel_path) : ResourcePath(base_path, rel_path) {}
	ImageResourcePath(ImageResourcePath&& that) : ResourcePath(std::move(that)) {}
	ImageResourcePath(const ImageResourcePath& that) : ResourcePath(that) {}
	~ImageResourcePath() {}

private:

	// image resources base path
	static constexpr const char *base_path{"/org/richfell/gnzb/images/"};
};

#endif	/* __UI_RESOURCE_HEADER__ */
