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
#ifndef MDT_SQL_DATABASE_COPIER_TABLE_MAPPING_H
#define MDT_SQL_DATABASE_COPIER_TABLE_MAPPING_H

#include "mdt/sql/copier/TableMapping.h"
#include "mdt/sql/copier/mdtSqlCopierFieldMapping.h"
#include "mdtSqlSchemaTable.h"
#include "mdtSqlDriverType.h"
#include <QString>
#include <QStringList>
#include <QSqlDatabase>

class QSqlRecord;

/*! \brief Mapping used to copy 2 SQL tables
 */
class mdtSqlDatabaseCopierTableMapping : public mdt::sql::copier::TableMapping
{
 public:

  /*! \brief Constructor
   */
  mdtSqlDatabaseCopierTableMapping() = default;

  /*! \brief Copy constructor
   */
//    mdtSqlDatabaseCopierTableMapping(const mdtSqlDatabaseCopierTableMapping & other) = default;

  /*! \brief Copy assignation
   */
//    mdtSqlDatabaseCopierTableMapping & operator=(const mdtSqlDatabaseCopierTableMapping & other) = default;

  /*! \brief Set source table
   *
   * Will also reset field mapping.
   *
   * \sa resetFieldMapping()
   */
  bool setSourceTable(const QString & tableName, const QSqlDatabase & db);

  /*! \brief Set destination table
   *
   * Will also reset field mapping.
   *
   * \sa resetFieldMapping()
   */
  bool setDestinationTable(const QString & tableName, const QSqlDatabase & db);

  /*! \brief Get source table name
   */
  QString sourceTableName() const
  {
    return pvSourceTable.tableName();
  }

  /*! \brief Get destination table name
   */
  QString destinationTableName() const
  {
    return pvDestinationTable.tableName();
  }

  /*! \brief Get field count in source table
    */
  int sourceTableFieldCount() const override
  {
    return pvSourceTable.fieldCount();
  }

  /*! \brief Get field count in destination table
    */
  int destinationTableFieldCount() const override
  {
    return pvDestinationTable.fieldCount();
  }

  /*! \brief Generate field mapping by name
   *
   * Will first reset field mapping.
   *  Then, for each field in destination table,
   *  source field is defined by destination field name.
   */
  void generateFieldMappingByName();

  /*! \brief Get list of field names of source table
   */
  QStringList getSourceFieldNameList() const
  {
    return pvSourceTable.getFieldNameList();
  }

  /*! \brief Get list of field names of destination table
   */
  QStringList getDestinationFieldNameList() const
  {
    return pvDestinationTable.getFieldNameList();
  }

  /*! \brief Get destination field name for given field mapping index
   *
   * \pre index must be in valid range.
   */
  QString destinationFieldName(int index) const;

  /*! \brief Get destination field type name for given field mapping index
   *
   * \pre index must be in valid range.
   */
  QString destinationFieldTypeName(int index) const;

  /*! \brief Check if destination field is part of a key
   *
   * \pre index must be in valid range.
   */
  FieldKeyType destinationFieldKeyType(int index) const;

 private:

  /*! \brief Get field count of destination table
   *
   * Mainly used by resetFieldMapping()
   */
//   int destinationTableFieldCount() const
//   {
//     return pvDestinationTable.fieldCount();
//   }

  /*! \brief Set source field index for given field mapping
   */
  void setSourceFieldIndex(mdtSqlCopierFieldMapping & fm, const QString & sourceFieldName) override;

  /*! \brief Get field name for given fieldIndex in source table
   */
  QString fetchSourceTableFieldNameAt(int fieldIndex) const override
  {
    return pvSourceTable.fieldName(fieldIndex);
  }

  /*! \brief Get field type name for given fieldIndex in source table
   */
  QString fetchSourceTableFieldTypeNameAt(int fieldIndex) const override;

  /*! \brief Check if field is part of a key for given field index in source table
   */
  FieldKeyType fetchSourceTableFieldKeyType(int fieldIndex) const override;

  /*! \brief Get source field name for given source field index
   */
//   QString fetchSourceFieldName(int sourceFieldIndex) const;

  /*! \brief Get source field type name for given source field index
   */
//   QString fetchSourceFieldTypeName(int sourceFieldIndex) const;

  /*! \brief Check if source field is part of a key
   */
//   FieldKeyType fetchSourceFieldKeyType(int sourceFieldIndex) const;

  /*! \brief Get field name for given fieldIndex in destination table
   */
  QString fetchDestinationTableFieldNameAt(int fieldIndex) const override
  {
    return pvDestinationTable.fieldName(fieldIndex);
  }

  /*! \brief Check if source field is compatible with destination field
   */
  bool areFieldsCompatible(int sourceFieldIndex, int destinationFieldIndex) const;

  QSqlDatabase pvSourceDatabase;
  mdtSqlSchemaTable pvSourceTable;
  QSqlDatabase pvDestinationDatabase;
  mdtSqlSchemaTable pvDestinationTable;
};

#endif // #ifndef MDT_SQL_DATABASE_COPIER_TABLE_MAPPING_H
