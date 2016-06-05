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
#ifndef MDT_SQL_SCHEMA_DRIVER_IMPLEMENTATION_INTERFACE_H
#define MDT_SQL_SCHEMA_DRIVER_IMPLEMENTATION_INTERFACE_H

#include "DriverType.h"
#include "FieldType.h"
#include "FieldTypeList.h"
#include "Field.h"
#include "Collation.h"
#include "AutoIncrementPrimaryKey.h"
#include "SingleFieldPrimaryKey.h"
#include "PrimaryKey.h"
#include "Index.h"
#include "Mdt/Error.h"
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QMetaType>

namespace Mdt{ namespace Sql{ namespace Schema{

  /*! \brief SQL Schema driver implementation interface
   */
  class DriverImplementationInterface
  {
   public:

    /*! \brief Constructor
     */
    DriverImplementationInterface(const QSqlDatabase & db)
     : pvDatabase(db)
    {
      Q_ASSERT(pvDatabase.isValid());
    }

    /*! \brief Get QSqlDriver instance
     *
     * \post Allways returns a non null pointer
     */
    QSqlDriver *qsqlDriver() const
    {
      Q_ASSERT(pvDatabase.driver() != nullptr);
      return pvDatabase.driver();
    }

    /*! \brief Get driver type
     */
    virtual DriverType type() const = 0;

    /*! \brief Get a list of avaliable field type
     *
     * Default implementation returns all type defined in FieldType
     */
    virtual FieldTypeList getAvailableFieldTypeList() const;

    /*! \brief Get SQL field type from QMetaType::Type
     *
     * Can be overloaded if default implementation does not match
     */
    virtual FieldType fieldTypeFromQMetaType(QMetaType::Type qmt) const;

    /*! \brief Get QMetaType::Type from SQL field type
     *
     * Can be overloaded if default implementation does not match
     */
    virtual QMetaType::Type fieldTypeToQMetaType(FieldType ft) const;

    /*! \brief Get collation definition
     */
    virtual QString getCollationDefinition(const Collation & collation) const = 0;

    /*! \brief Get field definition
     */
    virtual QString getFieldDefinition(const Field & field) const = 0;

    /*! \brief Get field definition of a auto increment primary key
     */
    virtual QString getPrimaryKeyFieldDefinition(const AutoIncrementPrimaryKey & pk) const = 0;

    /*! \brief Get field definition of a single field primary key
     */
    virtual QString getPrimaryKeyFieldDefinition(const SingleFieldPrimaryKey & pk) const = 0;

    /*! \brief Get primary key definition
     */
    virtual QString getPrimaryKeyDefinition(const PrimaryKey & pk) const;

    /*! \brief Get SQL statement to create a index
     */
    virtual QString getSqlToCreateIndex(const Index & index) const;

    /*! \brief Get SQL statement to drop a index
     */
    virtual QString getSqlToDropIndex(const Index & index) const;

    /*! \brief Get last error
     */
    Mdt::Error lastError() const
    {
      return pvLastError;
    }

   protected:

    /*! \brief Escape field name
     *
     * Returns the fieldName escaped according to the database rules.
     */
    QString escapeFieldName(const QString & fieldName) const;

    /*! \brief Escape table name
     *
     * Returns the tableName escaped according to the database rules.
     */
    QString escapeTableName(const QString & tableName) const;

    /*! \brief Set last error
     */
    void setLastError(const Mdt::Error & error)
    {
      pvLastError = error;
    }

    /*! \brief Call QObject::tr()
     */
    QString tr(const char *sourceText);

   private:

    QSqlDatabase pvDatabase;
    Mdt::Error pvLastError;
  };

}}} // namespace Mdt{ namespace Sql{ namespace Schema{

#endif // #ifndef MDT_SQL_SCHEMA_DRIVER_IMPLEMENTATION_INTERFACE_H
