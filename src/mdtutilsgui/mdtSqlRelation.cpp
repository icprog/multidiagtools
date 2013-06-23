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
#include "mdtSqlRelation.h"
#include "mdtSqlRelationItem.h"
#include "mdtError.h"
#include <QSqlField>

#include <QDebug>

mdtSqlRelation::mdtSqlRelation(QObject *parent)
 : QObject(parent)
{
  pvParentModel = 0;
  pvChildModel = 0;
  pvCurrentRow = -1;
}

mdtSqlRelation::~mdtSqlRelation()
{
  clear();
}

void mdtSqlRelation::setParentModel(QSqlTableModel *model)
{
  Q_ASSERT(model != 0);

  pvParentModel = model;
}

void mdtSqlRelation::setChildModel(QSqlTableModel *model)
{
  Q_ASSERT(model != 0);

  pvChildModel = model;
  connect(pvChildModel, SIGNAL(beforeInsert(QSqlRecord&)), this, SLOT(onChildBeforeInsert(QSqlRecord&)));
}

bool mdtSqlRelation::addRelation(const QString &parentFieldName, const QString &childFieldName)
{
  Q_ASSERT(pvParentModel != 0);
  Q_ASSERT(pvChildModel != 0);

  QSqlRecord record;
  int parentFieldIndex;
  QSqlField parentField;
  int childFieldIndex;
  QSqlField childField;

  // Get parent  field index, check that it exist
  record = pvParentModel->record();
  parentFieldIndex = record.indexOf(parentFieldName);
  if(parentFieldIndex < 0){
    mdtError e(MDT_DATABASE_ERROR, "Field '" + parentFieldName + "' not found in parent table", mdtError::Error);
    MDT_ERROR_SET_SRC(e, "mdtSqlRelation");
    e.commit();
    return false;
  }
  parentField = record.field(parentFieldIndex);
  // Get child field index, check that it exist
  record = pvChildModel->record();
  childFieldIndex = record.indexOf(childFieldName);
  if(childFieldIndex < 0){
    mdtError e(MDT_DATABASE_ERROR, "Field '" + childFieldName + "' not found in child table", mdtError::Error);
    MDT_ERROR_SET_SRC(e, "mdtSqlRelation");
    e.commit();
    return false;
  }
  childField = record.field(childFieldIndex);
  // Add relation informations
  mdtSqlRelationItem *item = new mdtSqlRelationItem;
  item->setParentField(parentField);
  item->setParentFieldIndex(parentFieldIndex);
  item->setChildField(childField);
  item->setChildFieldIndex(childFieldIndex);
  pvRelations.append(item);

  return true;
}

void mdtSqlRelation::clear()
{
  qDeleteAll(pvRelations);
  pvRelations.clear();
  pvParentModel = 0;
  pvChildModel = 0;
}

void mdtSqlRelation::setParentCurrentIndex(int index)
{
  Q_ASSERT(pvParentModel != 0);
  Q_ASSERT(pvChildModel != 0);

  pvCurrentRow = index;
  if(index < 0){
    return;
  }
  generateChildModelRelationFilter(index);
}

void mdtSqlRelation::setParentCurrentIndex(const QModelIndex &index)
{
  Q_ASSERT(pvParentModel != 0);
  Q_ASSERT(pvChildModel != 0);

  if(index.isValid()){
    setParentCurrentIndex(index.row());
  }else{
    setParentCurrentIndex(-1);
  }
}

void mdtSqlRelation::setParentCurrentIndex(const QModelIndex &current, const QModelIndex &previous)
{
  Q_ASSERT(pvParentModel != 0);
  Q_ASSERT(pvChildModel != 0);

  setParentCurrentIndex(current);
}

void mdtSqlRelation::onChildBeforeInsert(QSqlRecord &childRecord)
{
  Q_ASSERT(pvParentModel != 0);
  Q_ASSERT(pvChildModel != 0);

  QSqlRecord parentRecord;

  // On invalid index, we do nothing
  if(pvCurrentRow < 0){
    return;
  }
  // Get data record of the parent model
  parentRecord = pvParentModel->record(pvCurrentRow);
  // Update childs
  setChildForeingKeyValues(parentRecord, childRecord);
}

void mdtSqlRelation::setChildForeingKeyValues(QSqlRecord &parentRecord, QSqlRecord &childRecord)
{
  Q_ASSERT(pvParentModel != 0);
  Q_ASSERT(pvChildModel != 0);

  int i;
  mdtSqlRelationItem *item;
  QVariant data;

  // Copy parent PK -> child FK
  for(i=0; i<pvRelations.size(); ++i){
    item = pvRelations.at(i);
    Q_ASSERT(item != 0);
    // Get parent model's data
    data = parentRecord.value(item->parentFieldIndex());
    // Pust to child's field
    childRecord.setValue(item->childFieldIndex(), data);
  }
}

void mdtSqlRelation::generateChildModelRelationFilter(int row)
{
  Q_ASSERT(pvParentModel != 0);
  Q_ASSERT(pvChildModel != 0);
  Q_ASSERT(row >= 0);

  int i;
  mdtSqlRelationItem *item;
  QSqlRecord record;
  QVariant data;

  // Get data record of the parent model
  record = pvParentModel->record(row);
  // Build filter
  pvChildModelRelationFilter.clear();
  for(i=0; i<pvRelations.size(); ++i){
    item = pvRelations.at(i);
    Q_ASSERT(item != 0);
    // Get parent model's data
    data = record.value(item->parentFieldIndex());
    if(i>0){
      pvChildModelRelationFilter += " AND";
    }
    pvChildModelRelationFilter += " \"" + pvChildModel->tableName() + "\".\"" + item->childFieldName() + "\"=";
    if(data.isValid()){
      pvChildModelRelationFilter += item->dataProtection() + data.toString() + item->dataProtection();
    }else{
      pvChildModelRelationFilter += item->dataProtection() + item->valueForNoDataFilter().toString() + item->dataProtection();
    }
  }
  // Apply filter
  generateChildModelFilter();
  pvChildModel->setFilter(pvChildModelFilter);
}

void mdtSqlRelation::generateChildModelFilter()
{
  pvChildModelFilter = pvChildModelRelationFilter;
}
