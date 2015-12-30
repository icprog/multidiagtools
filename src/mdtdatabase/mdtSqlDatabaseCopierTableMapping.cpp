/****************************************************************************
 **
 ** Copyright (C) 2011-2015 Philippe Steinmann.
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
#include "mdtSqlDatabaseCopierTableMapping.h"
#include "mdtSqlField.h"
#include "mdtSqlFieldType.h"
#include "mdtSqlPrimaryKey.h"
#include <QSqlDriver>

//#include <QDebug>

// mdtSqlDatabaseCopierTableMapping::mdtSqlDatabaseCopierTableMapping(const mdtSqlDatabaseCopierTableMapping & other)
//  : mdtSqlCopierTableMapping(other),
//    pvSourceDatabase(other.pvSourceDatabase),
//    pvSourceTable(other.pvSourceTable),
//    pvDestinationDatabase(other.pvDestinationDatabase),
//    pvDestinationTable(other.pvDestinationTable)
// {
// }

// mdtSqlDatabaseCopierTableMapping& mdtSqlDatabaseCopierTableMapping::operator=(const mdtSqlDatabaseCopierTableMapping & other)
// {
//   pvSourceDatabase = other.pvSourceDatabase;
//   pvSourceTable = other.pvSourceTable;
//   pvDestinationDatabase = other.pvDestinationDatabase;
//   pvDestinationTable = other.pvDestinationTable;
// }

bool mdtSqlDatabaseCopierTableMapping::setSourceTable(const QString & tableName, const QSqlDatabase & db)
{
  clearFieldMapping();
  if(!pvSourceTable.setupFromTable(tableName, db)){
    pvLastError = pvSourceTable.lastError();
    return false;
  }
  pvSourceDatabase = db;
  resetFieldMapping();

  return true;
}

bool mdtSqlDatabaseCopierTableMapping::setDestinationTable(const QString & tableName, const QSqlDatabase & db)
{
  clearFieldMapping();
  if(!pvDestinationTable.setupFromTable(tableName, db)){
    pvLastError = pvDestinationTable.lastError();
    return false;
  }
  pvDestinationDatabase = db;
  resetFieldMapping();

  return true;
}

void mdtSqlDatabaseCopierTableMapping::generateFieldMappingByName()
{
  auto sourceDriverType = mdtSqlDriverType::typeFromName(pvSourceDatabase.driverName());
  auto destinationDriverType = mdtSqlDriverType::typeFromName(pvDestinationDatabase.driverName());

  resetFieldMapping();
  if(sourceDriverType == mdtSqlDriverType::Unknown){
    return;
  }
  if(destinationDriverType == mdtSqlDriverType::Unknown){
    return;
  }
  for(auto & fm : fieldMappingList()){
    // Get source field
    Q_ASSERT(fm.destinationFieldIndex >= 0);
    Q_ASSERT(fm.destinationFieldIndex < pvDestinationTable.fieldCount());
    mdtSqlField destinationField = pvDestinationTable.field(fm.destinationFieldIndex);
    // Get source field index that matches destination field name
    fm.sourceFieldIndex = pvSourceTable.fieldIndex(destinationField.name());
    updateFieldMappingState(fm, sourceDriverType, destinationDriverType);
  }
  // Update table mapping state
  updateTableMappingState();
}

// void mdtSqlDatabaseCopierTableMapping::setSourceField(int index, const QString & fieldName)
// {
//   Q_ASSERT(index >= 0);
//   Q_ASSERT(index < fieldCount());
// 
//   auto fm = fieldMappingAt(index);
//   auto sourceDriverType = mdtSqlDriverType::typeFromName(pvSourceDatabase.driverName());
//   auto destinationDriverType = mdtSqlDriverType::typeFromName(pvDestinationDatabase.driverName());
// 
//   if(sourceDriverType == mdtSqlDriverType::Unknown){
//     return;
//   }
//   if(destinationDriverType == mdtSqlDriverType::Unknown){
//     return;
//   }
//   if(fieldName.isEmpty()){
//     fm.sourceFieldIndex = -1;
//   }else{
//     fm.sourceFieldIndex = pvSourceTable.fieldIndex(fieldName);
//   }
//   updateFieldMappingState(fm, sourceDriverType, destinationDriverType);
//   updateFieldMappingAt(index, fm);
//   // Update table mapping state
//   updateTableMappingState();
// }

QString mdtSqlDatabaseCopierTableMapping::sourceFieldName(int index) const
{
  Q_ASSERT(index >= 0);
  Q_ASSERT(index < fieldCount());

  int sourceFieldIndex = fieldMappingAt(index).sourceFieldIndex;
  Q_ASSERT(sourceFieldIndex < pvSourceTable.fieldCount());
  if(sourceFieldIndex < 0){
    return QString();
  }
  return pvSourceTable.fieldName(sourceFieldIndex);
}

QString mdtSqlDatabaseCopierTableMapping::sourceFieldTypeName(int index) const
{
  Q_ASSERT(index >= 0);
  Q_ASSERT(index < fieldCount());

  int sourceFieldIndex = fieldMappingAt(index).sourceFieldIndex;
  Q_ASSERT(sourceFieldIndex < pvSourceTable.fieldCount());
  if(sourceFieldIndex < 0){
    return QString();
  }
  return pvSourceTable.fieldTypeName(sourceFieldIndex, mdtSqlDriverType::typeFromName(pvSourceDatabase.driverName()));
}

mdtSqlCopierTableMapping::FieldKeyType mdtSqlDatabaseCopierTableMapping::sourceFieldKeyType(int index) const
{
  Q_ASSERT(index >= 0);
  Q_ASSERT(index < fieldCount());

  const int sourceFieldIndex = fieldMappingAt(index).sourceFieldIndex;
  Q_ASSERT(sourceFieldIndex < pvSourceTable.fieldCount());
  if(sourceFieldIndex < 0){
    return NotAKey;
  }
  if(pvSourceTable.isFieldPartOfPrimaryKey(sourceFieldIndex)){
    return PrimaryKey;
  }
  return NotAKey;
}

QString mdtSqlDatabaseCopierTableMapping::destinationFieldName(int index) const
{
  Q_ASSERT(index >= 0);
  Q_ASSERT(index < fieldCount());

  int destinationFieldIndex = fieldMappingAt(index).destinationFieldIndex;
  Q_ASSERT(destinationFieldIndex < pvDestinationTable.fieldCount());
  if(destinationFieldIndex < 0){
    return QString();
  }
  return pvDestinationTable.fieldName(destinationFieldIndex);
}

QString mdtSqlDatabaseCopierTableMapping::destinationFieldTypeName(int index) const
{
  Q_ASSERT(index >= 0);
  Q_ASSERT(index < fieldCount());

  int destinationFieldIndex = fieldMappingAt(index).destinationFieldIndex;
  Q_ASSERT(destinationFieldIndex < pvDestinationTable.fieldCount());
  if(destinationFieldIndex < 0){
    return QString();
  }
  return pvDestinationTable.fieldTypeName(destinationFieldIndex, mdtSqlDriverType::typeFromName(pvDestinationDatabase.driverName()));
}

mdtSqlCopierTableMapping::FieldKeyType mdtSqlDatabaseCopierTableMapping::destinationFieldKeyType(int index) const
{
  Q_ASSERT(index >= 0);
  Q_ASSERT(index < fieldCount());

  const int destinationFieldIndex = fieldMappingAt(index).destinationFieldIndex;
  Q_ASSERT(destinationFieldIndex < pvDestinationTable.fieldCount());
  if(destinationFieldIndex < 0){
    return NotAKey;
  }
  if(pvDestinationTable.isFieldPartOfPrimaryKey(destinationFieldIndex)){
    return PrimaryKey;
  }
  return NotAKey;
}

void mdtSqlDatabaseCopierTableMapping::updateSourceField(mdtSqlCopierFieldMapping & fm, const QString & sourceFieldName)
{
  auto sourceDriverType = mdtSqlDriverType::typeFromName(pvSourceDatabase.driverName());
  auto destinationDriverType = mdtSqlDriverType::typeFromName(pvDestinationDatabase.driverName());

  if(sourceDriverType == mdtSqlDriverType::Unknown){
    return;
  }
  if(destinationDriverType == mdtSqlDriverType::Unknown){
    return;
  }
  if(sourceFieldName.isEmpty()){
    fm.sourceFieldIndex = -1;
  }else{
    fm.sourceFieldIndex = pvSourceTable.fieldIndex(sourceFieldName);
  }
  updateFieldMappingState(fm, sourceDriverType, destinationDriverType);
}

void mdtSqlDatabaseCopierTableMapping::updateFieldMappingState(mdtSqlCopierFieldMapping & fm, mdtSqlDriverType::Type sourceDriverType, mdtSqlDriverType::Type destinationDriverType)
{
  Q_ASSERT(sourceDriverType != mdtSqlDriverType::Unknown);
  Q_ASSERT(destinationDriverType != mdtSqlDriverType::Unknown);

  if(fm.isNull()){
    fm.mappingState = mdtSqlCopierFieldMapping::MappingNotSet;
    return;
  }
  Q_ASSERT(fm.sourceFieldIndex >= 0);
  Q_ASSERT(fm.sourceFieldIndex < pvSourceTable.fieldCount());
  Q_ASSERT(fm.destinationFieldIndex >= 0);
  Q_ASSERT(fm.destinationFieldIndex < pvDestinationTable.fieldCount());
  // Do checks regarding field types
  auto sourceFieldType = pvSourceTable.field(fm.sourceFieldIndex).getFieldType(sourceDriverType);
  auto destinationFieldType = pvDestinationTable.field(fm.destinationFieldIndex).getFieldType(destinationDriverType);
  if(sourceFieldType == destinationFieldType){
    fm.mappingState = mdtSqlCopierFieldMapping::MappingComplete;
  }else{
    fm.mappingState = mdtSqlCopierFieldMapping::MappingError;
  }
}
