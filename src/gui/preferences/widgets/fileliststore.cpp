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
#include "fileliststore.h"
#include "../../guiutil.h"
#include <sys/types.h>
#include <dirent.h>

FileListStore::FileListStore(const std::forward_list<std::string>& dirs, const std::forward_list<std::string>& shellMatch)
{
	for(auto& dir : dirs)
	{
		for(auto& wildcard : shellMatch)
			add_matching_files(dir, wildcard);
	}
}

void FileListStore::add_matching_files(const std::string& path, const std::string& wildcard)
{
  std::string::size_type where;

	if((where = wildcard.find_first_of('*')) != std::string::npos
		|| (where = wildcard.find_first_of('?')) != std::string::npos)
	{
		std::string front = wildcard.substr(0, where);
		bool multiple = wildcard[where] == '*';
		std::string back = wildcard.substr(where+1);

		DIR *dirp = opendir(path.c_str());
		if(dirp != 0)
		{
			struct dirent *d;
			while((d = readdir(dirp)) != 0)
			{
				std::string name = d->d_name;

				if(name == "." || name == "..")
					continue;

				if(multiple)
				{
					if(name.size() >= wildcard.size()
						&& name.substr(0, where) == front
						&& name.substr(name.size() - back.size()) == back)
					{
						//matches->push_back(path + name);
						  std::string fqPath(path + name);
						  appendFileSystemItem(fqPath, get_icon_for_path(fqPath));
					}
				}
				else
				{
					if(name.size() == wildcard.size())
					{
						std::string::const_iterator pw = wildcard.begin();
						std::string::const_iterator pn = name.begin();
						while (pw != wildcard.end())
						{
							if(*pw != '?' && *pw != *pn)
								break;
							++pw;
							++pn;
						}

						if(pw == wildcard.end())
						{
							//matches->push_back(path + name);
							std::string fqPath(path + name);
							appendFileSystemItem(fqPath, get_icon_for_path(fqPath));
						}
					}
				}
			}

			closedir(dirp);
		}
	}
	else
	{
		struct stat st;
		std::string fn = path + wildcard;
		if (stat(fn.c_str(), &st) == 0)
		{
			//matches->push_back(path + wildcard);
			std::string fqPath(path + wildcard);
			appendFileSystemItem(fqPath, get_icon_for_path(fqPath));
		}
	}
}