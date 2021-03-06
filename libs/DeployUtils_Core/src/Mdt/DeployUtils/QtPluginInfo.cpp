/****************************************************************************
 **
 ** Copyright (C) 2011-2017 Philippe Steinmann.
 **
 ** This file is part of Mdt library.
 **
 ** Mdt is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** Mdt is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public License
 ** along with Mdt.  If not, see <http://www.gnu.org/licenses/>.
 **
 ****************************************************************************/
#include "QtPluginInfo.h"

namespace Mdt{ namespace DeployUtils{

void QtPluginInfo::setAbsoluteFilePath(const QString& path)
{
  mLibraryInfo.setAbsoluteFilePath(path);
}

void QtPluginInfo::setLibraryPlatformName(const QString& name)
{
  mLibraryInfo.setLibraryPlatformName(name);
}

void QtPluginInfo::setLibraryName(const LibraryName& libraryName)
{
  mLibraryInfo.setLibraryName(libraryName);
}

void QtPluginInfo::setDirectoryName(const QString& name)
{
  mDirectoryName = name;
}

}} // namespace Mdt{ namespace DeployUtils{
