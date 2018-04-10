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
#ifndef MDT_SQL_JOIN_CONSTRAINT_FIELD_PAIR_LIST_H
#define MDT_SQL_JOIN_CONSTRAINT_FIELD_PAIR_LIST_H

#include "JoinConstraintFieldPair.h"
#include <QString>
#include <vector>

namespace Mdt{ namespace Sql{

  class SelectTable;

  /*! \brief List of field name pairs
   */
  class JoinConstraintFieldPairList
  {
   public:

    /*! \brief Construct a empty field par list
     *
     * \pre leftTable and right table must not be empty
     */
    JoinConstraintFieldPairList(const QString & leftTable, const QString & rightTable)
     : mLeftTable(leftTable),
       mRightTable(rightTable)
    {
      Q_ASSERT(!mLeftTable.isEmpty());
      Q_ASSERT(!mRightTable.isEmpty());
    }

    /*! \brief Get left table
     */
    QString leftTable() const
    {
      return mLeftTable;
    }

    /*! \brief Get right table
     */
    QString rightTable() const
    {
      return mRightTable;
    }

    /*! \brief Add a pair of field names
     *
     * \pre left and right must not be empty
     */
    void addFieldPair(const QString & left, const QString & right)
    {
      mList.emplace_back(left, right);
    }

    /*! \brief Get count of pairs
     */
    int size() const
    {
      return mList.size();
    }

    /*! \brief Check if pair list is empty
     */
    bool isEmpty() const
    {
      return mList.empty();
    }

    /*! \brief Check if null
     *
     * Returns true if isEmpty() is true
     *  or leftTable is empty or right is empty.
     */
    bool isNull() const
    {
      return ( isEmpty() || mLeftTable.isEmpty() || mRightTable.isEmpty() );
    }

    /*! \brief Get field par at index
     *
     * \pre index must be in valid range
     */
    const JoinConstraintFieldPair & at(int index) const
    {
      Q_ASSERT(index >= 0);
      Q_ASSERT(index < size());
      return mList[index];
    }

    /*! \brief Get a JoinConstraintFieldPairList from tables
     *
     * \pre At least left must have a foreign key that refers to right,
     *      or right must have one that refers to left.
     * \pre The foreign key that links left and right must have at least 1 field,
     *      and its parent and child fields count must be the same.
     */
    static JoinConstraintFieldPairList fromTables(const SelectTable & left, const SelectTable & right);

   private:

    QString mLeftTable;
    QString mRightTable;
    std::vector<JoinConstraintFieldPair> mList;
  };

}} // namespace Mdt{ namespace Sql{

#endif // #ifndef MDT_SQL_JOIN_CONSTRAINT_FIELD_PAIR_LIST_H
