/****************************************************************************
 **
 ** Copyright (C) 2011-2013 Philippe Steinmann.
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
#include "mdtSqlDataWidgetMapper.h"
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlTableModel>

//#include <QDebug>

mdtSqlDataWidgetMapper::mdtSqlDataWidgetMapper(QObject *parent)
 : QDataWidgetMapper(parent)
{
}

mdtSqlDataWidgetMapper::~mdtSqlDataWidgetMapper()
{
}

void mdtSqlDataWidgetMapper::setModel(QSqlTableModel *model)
{
  Q_ASSERT(model != 0);

  QDataWidgetMapper::setModel(model);
}

QSqlTableModel *mdtSqlDataWidgetMapper::model()
{
  /*
   * Because setModel(QAbstractItemModel*) is private,
   *  it's not possible to set something other than QSqlTableModel.
   *  So we not have to check returned pointer
   */
  return static_cast<QSqlTableModel*>(QDataWidgetMapper::model());
}

void mdtSqlDataWidgetMapper::setModel(QAbstractItemModel *model)
{
}
