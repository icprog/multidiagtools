/****************************************************************************
 **
 ** Copyright (C) 2011-2014 Philippe Steinmann.
 **
 ** This file is part of multiDiagTools library.
 **
 ** multiDiagTools is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** multiDiagTools is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public License
 ** along with multiDiagTools.  If not, see <http://www.gnu.org/licenses/>.
 **
 ****************************************************************************/
#ifndef MDT_TT_TEST_NODE_UNIT_H
#define MDT_TT_TEST_NODE_UNIT_H


#include "mdtTtTestNodeUnitData.h"
#include "mdtTtBase.h"
#include <QVariant>
#include <QList>
#include <QSqlDatabase>
#include <QString>

class QObject;

/*! \brief Helper class for test node unit data manipulation
 */
class mdtTtTestNodeUnit : public mdtTtBase
{
 public:

  /*! \brief Constructor
   */
  mdtTtTestNodeUnit(QObject *parent, const QSqlDatabase & db);

  /*! \brief Get SQL statement for test connection selection
   */
  QString sqlForTestConnectionSelection(const QList<QVariant> connectionIdList) const;

  /*! \brief Get a list of unit connection IDs for given unit ID
   */
  QList<QVariant> getConnectionIdListOfUnitId(const QVariant & unitId, bool *ok);

  /*! \brief Get unit connection IDs that are linked to given unit connection IDs
   */
  QList<QVariant> getConnectionIdListLinkedToConnectionIdList(const QList<QVariant> connectionIdList, bool *ok);

  /*! \brief Get unit data
   */
  mdtSqlRecord getUnitData(const QVariant & unitId, bool *ok);

  /*! \brief Get test node unit data
   */
  mdtTtTestNodeUnitData getData(const QVariant & nodeUnitId, bool *ok, bool includeTestConnectionData);

  /*! \brief Add a test node unit
   */
  bool add(const mdtTtTestNodeUnitData & data);

  /*! \brief Edit a test node unit
   */
  bool edit(const QVariant & nodeUnitId, const mdtTtTestNodeUnitData & data);

  /*! \brief Remove a test node unit
   */
  bool remove(const QVariant & nodeUnitId);

 private:

  Q_DISABLE_COPY(mdtTtTestNodeUnit);
  
};

#endif // #ifndef MDT_TT_TEST_NODE_UNIT_H
