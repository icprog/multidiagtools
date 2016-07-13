/****************************************************************************
 **
 ** Copyright (C) 2011-2016 Philippe Steinmann.
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
#include "AbstractController.h"
#include "ItemSelectionModel.h"
#include "RowChangeEventMapper.h"
#include "RowChangeEventDispatcher.h"
#include <QAbstractItemModel>
#include <QItemSelectionModel>

#include <QDebug>

namespace Mdt{ namespace ItemEditor{

AbstractController::AbstractController(QObject* parent)
 : QObject(parent),
   pvCurrentRow(-1),
   pvRowChangeEventMapper(new RowChangeEventMapper(this)),
   pvRowChangeEventDispatcher(new RowChangeEventDispatcher(this))
{
  connect(pvRowChangeEventMapper, &RowChangeEventMapper::selectionModelChanged, pvRowChangeEventDispatcher, &RowChangeEventDispatcher::setSelectionModel);
  connect(pvRowChangeEventMapper, &RowChangeEventMapper::rowStateChanged, pvRowChangeEventDispatcher, &RowChangeEventDispatcher::setRowState);
  connect(pvRowChangeEventDispatcher, &RowChangeEventDispatcher::modelReset, this, &AbstractController::toFirst);
  connect(pvRowChangeEventDispatcher, &RowChangeEventDispatcher::rowStateChanged, this, &AbstractController::rowStateChanged);
  connect(pvRowChangeEventDispatcher, &RowChangeEventDispatcher::currentRowChanged, this, &AbstractController::setCurrentRow);
}

void AbstractController::setModel(QAbstractItemModel* model)
{
  Q_ASSERT(model != nullptr);

  pvModel = model;
  pvRowChangeEventMapper->setModel(model);
  emit modelChanged(model);
}

void AbstractController::setSelectionModel(QItemSelectionModel* model)
{
  Q_ASSERT(model != nullptr);

  pvSelectionModel = qobject_cast<ItemSelectionModel*>(model);
  Q_ASSERT(!pvSelectionModel.isNull());
  pvRowChangeEventMapper->setSelectionModel(model);
}

int AbstractController::rowCount() const
{
  if(pvModel.isNull()){
    return 0;
  }
  return pvModel->rowCount();
}

bool AbstractController::setCurrentRow(int row)
{
  Q_ASSERT(row >= -1);

  /**
   * \todo If row >= rowCount()
   *       we must try to fetch more data
   *       until we found row, or no more data is available,
   *       and consider this value as current row.
   */
  pvCurrentRow = row;
  pvRowChangeEventDispatcher->setCurrentRow(row);

  return true;
}

void AbstractController::toFirst()
{
  if(rowCount() > 0){
    setCurrentRow(0);
  }else{
    setCurrentRow(-1);
  }
}

void AbstractController::toPrevious()
{
  /// \todo checks..
  setCurrentRow(pvCurrentRow-1);
}

void AbstractController::toNext()
{
  /// \todo checks..
  setCurrentRow(pvCurrentRow+1);
}

void AbstractController::toLast()
{
  setCurrentRow(rowCount()-1);
}

// void AbstractController::setRowState(RowState rs)
// {
//   qDebug() << "AbstractController::setRowState() ...";
//   emit rowStateChanged(rs);
// }


}} // namespace Mdt{ namespace ItemEditor{
