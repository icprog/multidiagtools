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
#include "PrimaryKeyContainer.h"

namespace Mdt{ namespace Sql{ namespace Schema{

class FieldNameVisitor : public boost::static_visitor<>
{
 public:

  void operator()(const AutoIncrementPrimaryKey & pk)
  {
    fieldName = pk.fieldName();
  }
//   void operator()(const SingleFieldPrimaryKey & pk)
//   {
//     fieldName = pk.fieldName();
//   }
  void operator()(const PrimaryKey &)
  {
  }
  QString fieldName;
};

class FieldTypeVisitor : public boost::static_visitor<>
{
 public:

  void operator()(const AutoIncrementPrimaryKey & pk)
  {
    fieldType = pk.fieldType();
  }
//   void operator()(const SingleFieldPrimaryKey & pk)
//   {
//     fieldType = pk.fieldType();
//   }
  void operator()(const PrimaryKey &)
  {
    fieldType = FieldType::UnknownType;
  }
  FieldType fieldType;
};

class FieldLengthVisitor : public boost::static_visitor<>
{
 public:

  void operator()(const AutoIncrementPrimaryKey &)
  {
    fieldLength = -1;
  }
//   void operator()(const SingleFieldPrimaryKey & pk)
//   {
//     fieldLength = pk.fieldLength();
//   }
  void operator()(const PrimaryKey &)
  {
    fieldLength = -1;
  }
  int fieldLength;
};

/*
 * PrimaryKeyContainer implementation
 */

QString PrimaryKeyContainer::fieldName() const
{
  FieldNameVisitor visitor;

  boost::apply_visitor(visitor, mPrimaryKey);

  return visitor.fieldName;
}

FieldType PrimaryKeyContainer::fieldType() const
{
  FieldTypeVisitor visitor;

  boost::apply_visitor(visitor, mPrimaryKey);

  return visitor.fieldType;
}

int PrimaryKeyContainer::fieldLength() const
{
  FieldLengthVisitor visitor;

  boost::apply_visitor(visitor, mPrimaryKey);

  return visitor.fieldLength;
}

AutoIncrementPrimaryKey PrimaryKeyContainer::autoIncrementPrimaryKey() const
{
  Q_ASSERT(mType == AutoIncrementPrimaryKeyType);
  return boost::get<AutoIncrementPrimaryKey>(mPrimaryKey);
}

// SingleFieldPrimaryKey PrimaryKeyContainer::singleFieldPrimaryKey() const
// {
//   Q_ASSERT(mType == SingleFieldPrimaryKeyType);
//   return boost::get<SingleFieldPrimaryKey>(mPrimaryKey);
// }

PrimaryKey PrimaryKeyContainer::primaryKey() const
{
  Q_ASSERT(mType == PrimaryKeyType);
  return boost::get<PrimaryKey>(mPrimaryKey);
}

void PrimaryKeyContainer::clear()
{
  mPrimaryKey = PrimaryKey();
  mType = PrimaryKeyType;
}

}}} // namespace Mdt{ namespace Sql{ namespace Schema{
