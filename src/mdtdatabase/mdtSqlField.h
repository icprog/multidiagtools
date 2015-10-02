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
#ifndef MDT_SQL_FIELD_H
#define MDT_SQL_FIELD_H

#include "mdtSqlFieldType.h"
#include <QString>
#include <QVariant>

/*! \brief Represents a field in a SQL database table
 *
 * This is a simple data storage class.
 *  Because of multiple possible usage,
 *  no coherence check is made.
 */
class mdtSqlField final
{
 public:

  /*! \brief Default constructor
   */
  mdtSqlField()
   : pvType(mdtSqlFieldType::UnknownType),
     pvIsAutoValue(false),
     pvIsRequired(false),
     pvLength(-1)
  {
  }

  /*! \brief Clear
   */
  void clear()
  {
    pvType = mdtSqlFieldType::UnknownType;
    pvName.clear();
    pvIsAutoValue = false;
    pvIsRequired = false;
    pvDefaultValue.clear();
    pvLength = -1;
  }

  /*! \brief Set field type
   */
  void setType(mdtSqlFieldType::Type t)
  {
    pvType = t;
  }

  /*! \brief Get field type
   */
  mdtSqlFieldType::Type type() const
  {
    return pvType;
  }

  /*! \brief Set field name
   */
  void setName(const QString & name)
  {
    pvName = name;
  }

  /*! \brief Get field name
   */
  QString name() const
  {
    return pvName;
  }

  /*! \brief Set auto value
   */
  void setAutoValue(bool autoValue)
  {
    pvIsAutoValue = autoValue;
  }

  /*! \brief Check if field is auto value
   */
  bool isAutoValue() const
  {
    return pvIsAutoValue;
  }

  /*! \brief Set field required
   */
  void setRequired(bool r)
  {
    pvIsRequired = r;
  }

  /*! \brief Check if field is required
   */
  bool isRequired() const
  {
    return pvIsRequired;
  }

  /*! \brief Set default value
   */
  void setDefaultValue(const QString & v)
  {
    pvDefaultValue = v;
  }

  /*! \brief Get default value
   */
  QVariant defaultValue() const
  {
    return pvDefaultValue;
  }

  /*! \brief Set length
   *
   * \sa length()
   */
  void setLength(int length)
  {
    pvLength = length;
  }

  /*! \brief Get length
   *
   * A value < 0 means that length is not set.
   */
  int length() const
  {
    return pvLength;
  }

//   static QMap<QVariant::Type, mdtSqlField::Type> essai()
//   {
//     return {
//       std::pair<QVariant::Type, mdtSqlField::Type>{QVariant::Int, mdtSqlField::Integer},
//       std::pair<QVariant::Type, mdtSqlField::Type>{QVariant::Bool, mdtSqlField::Boolean}
//     };
//   }
  
 private:

  mdtSqlFieldType::Type pvType;
  uint pvIsAutoValue : 1;
  uint pvIsRequired : 1;
  int pvLength;
  QString pvName;
  QVariant pvDefaultValue;
};

#endif // #ifndef MDT_SQL_FIELD_H
