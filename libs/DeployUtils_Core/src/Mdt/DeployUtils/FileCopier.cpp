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
#include "FileCopier.h"
#include "LibraryInfo.h"
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QLatin1String>

// #include <QDebug>

namespace Mdt{ namespace DeployUtils{

FileCopier::FileCopier(QObject* parent)
 : QObject(parent)
{
}

bool FileCopier::createDirectory(const QString& directoryPath)
{
  auto dir = QDir::current();
  QFileInfo fi(directoryPath);

  // Check if directoryPath is a file
  if(fi.isFile()){
    const QString msg = tr("Could not create directory '%1' because it allready exists and is a file.")
                        .arg(directoryPath);
    auto error = mdtErrorNewQ(msg, Mdt::Error::Critical, this);
    setLastError(error);
    return false;
  }
  // Create directory
  if(!dir.mkpath(directoryPath)){
    const QString msg = tr("Could not create directory '%1'")
                        .arg(directoryPath);
    auto error = mdtErrorNewQ(msg, Mdt::Error::Critical, this);
    setLastError(error);
    return false;
  }

  return true;
}

bool FileCopier::copyLibraries(const LibraryInfoList & libraries, const QString & destinationDirectoryPath)
{
  if(!createDirectory(destinationDirectoryPath)){
    return false;
  }
  for(const auto & sourceLibrary : libraries){
    QFileInfo sourceFileInfo(sourceLibrary.absoluteFilePath());
    const auto fileName = sourceFileInfo.fileName();
    const auto destinationFilePath = QDir::cleanPath( destinationDirectoryPath + QLatin1String("/") + fileName );
    QFile sourceFile(sourceFileInfo.absoluteFilePath());
    if( !sourceFile.copy(destinationFilePath) ){
      const QString msg = tr("Could not copy file '%1' to '%2'")
                          .arg(sourceFileInfo.absoluteFilePath(), destinationDirectoryPath);
      auto error = mdtErrorNewQ(msg, Mdt::Error::Critical, this);
      error.stackError( mdtErrorFromQFile(sourceFile, this) );
      setLastError(error);
      return false;
    }
  }

  return true;
}

void FileCopier::setLastError(const Error& error)
{
  mLastError = error;
  mLastError.commit();
}

}} // namespace Mdt{ namespace DeployUtils{
