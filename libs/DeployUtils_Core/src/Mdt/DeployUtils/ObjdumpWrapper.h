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
#ifndef MDT_DEPLOY_UTILS_OBJDUMP_WRAPPER_H
#define MDT_DEPLOY_UTILS_OBJDUMP_WRAPPER_H

#include "ToolExecutableWrapper.h"
#include "MdtDeployUtils_CoreExport.h"
#include <QString>
#include <QStringList>

namespace Mdt{ namespace DeployUtils{

  /*! \brief Wrapps a objdump executable
   */
  class MDT_DEPLOYUTILS_CORE_EXPORT ObjdumpWrapper : public ToolExecutableWrapper
  {
   Q_OBJECT

   public:

    /*! \brief Constructor
     */
    explicit ObjdumpWrapper(QObject* parent = nullptr);

    /*! \brief Execute the command to find dependencies
     *
     * \param binaryFilePath Path to a executable or a library
     */
    bool execFindDependencies(const QString & binaryFilePath);

    /*! \brief Execute the command to read format
     *
     * \param binaryFilePath Path to a executable or a library
     */
    bool execReadFormat(const QString & binaryFilePath);

    /*! \brief Find objdump executable
     *
     * \note This method is called automatically by find*() methods.
     */
    QString findObjdump();

   private:

    /*! \brief Execute objdump command
     *
     * Will try to find objdump if not in exec path
     */
    bool execObjdump(const QStringList & arguments);

    bool checkStdError();
  };

}} // namespace Mdt{ namespace DeployUtils{

#endif // #ifndef MDT_DEPLOY_UTILS_OBJDUMP_WRAPPER_H
