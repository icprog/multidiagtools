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
#ifndef MDT_DEPLOY_UTILS_BINARY_DEPENDENCIES_H
#define MDT_DEPLOY_UTILS_BINARY_DEPENDENCIES_H

// #include "Platform.h"

// #include "OperatingSystem.h"
#include "LibraryInfoList.h"
#include "PathList.h"
#include "Mdt/Error.h"
#include <QObject>
#include <QString>
#include <QStringList>
// #include <memory>

namespace Mdt{ namespace DeployUtils{

//   class BinaryDependenciesImplementationInterface;

  /*! \brief Find dependencies for a executable or a library
   */
  class BinaryDependencies : public QObject
  {
   Q_OBJECT

   public:

    /*! \brief Option to include binary file's directory to library search paths
     *
     * \sa getLibrarySearchPathList()
     */
    enum BinaryFileDirectoryInclude
    {
      IncludeBinaryFileDirectory, /*!< Include binary file's directory to the libraries search paths */
      ExcludeBinaryFileDirectory  /*!< Do not include binary file's directory to the libraries search paths */
    };

    /*! \brief Constructor
     */
    BinaryDependencies(QObject* parent = nullptr);

    /*! \brief Destructor
     */
    ~BinaryDependencies();

//     /*! \brief Check if valid
//      *
//      * Returns true if a implementation for native platform could be loaded,
//      *  otherwise false.
//      */
//     bool isValid() const;

    /*! \brief Set a list of paths where to search dependencies first
     *
     * For target platforms which supports RPATH (like Linux),
     *  this has no effects, because the implementation will use ldd,
     *  which returns directly the full path for each dependency.
     *
     * For other target platforms, like Windows,
     *  use this method to tell where to find libraries
     *  that are not installed in a standard library location,
     *  or a alternative library version must be used.
     *
     * \sa setLibrarySearchFirstPathSuffixList()
     */
    void setLibrarySearchFirstPathList(const PathList & pathList);

    /*! \brief Specify addional subdirectories to check
     *
     * Specify addional subdirectories to check below each directory
     *  in those specified with setLibrarySearchFirstPathList().
     *  For example, if /opt/liba and /opt/libb have been set as
     *  paths to search first, and \a suffixList contains bin qt5/bin,
     *  libraries will be searched in
     *  /opt/liba, /opt/liba/bin, /opt/liba/qt5/bin, /opt/libb, /opt/libb/bin, /opt/libb/qt5/bin .
     *
     * \sa setLibrarySearchFirstPathList()
     */
    void setLibrarySearchFirstPathSuffixList(const QStringList & suffixList);

    /*! \brief Get a list of paths where to search dependencies first
     *
     * If a executable or a library have allready been set with setBinaryFile(),
     *  and \a binaryFileDirectoryInclude is IncludeBinaryFileDirectory,
     *  binary file's directory will be at first place in the list.
     *  If paths have been added with setLibrarySearchFirstPathList(),
     *  they will follow in the list.
     *  No system library paths will be added to this list.
     *  Each implementation will do it itself.
     *
     * \note The path list will be rebuilt at each call of this method.
     */
    PathList getLibrarySearchFirstPathList(BinaryFileDirectoryInclude binaryFileDirectoryInclude) const;

    /*! \brief Set library or executable
     */
    bool setBinaryFile(const QString & filePath);

    /*! \brief Find dependencies for executable or library
     */
    bool findDependencies();

    /*! \brief Find dependencies for a executable or a library
     *
     * This is the same as calling
     *  setBinaryFile() then findDependencies().
     */
    bool findDependencies(const QString & binaryFilePath);

    /*! \brief Get dependencies
     */
    LibraryInfoList dependencies() const
    {
      return mDependencies;
    }

    /*! \brief Get last error
     */
    Mdt::Error lastError() const
    {
      return mLastError;
    }

   private:

    void setLastError(const Mdt::Error & error);

    LibraryInfoList mDependencies;
    QString mBinaryFilePath;
    QString mBinaryFileDirectoryPath;
    PathList mLibrarySearchFirstPathList;
    QStringList mLibrarySearchFirstPathSuffixList;
    Mdt::Error mLastError;
  };

}} // namespace Mdt{ namespace DeployUtils{

#endif // #ifndef MDT_DEPLOY_UTILS_BINARY_DEPENDENCIES_H
