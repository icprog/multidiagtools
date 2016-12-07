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
#ifndef MDT_ITEM_MODEL_VARIANT_TABLE_MODEL_ROW_H
#define MDT_ITEM_MODEL_VARIANT_TABLE_MODEL_ROW_H

#include "VariantTableModelItem.h"
#include "VariantTableModelStorageRule.h"
#include <vector>

namespace Mdt{ namespace ItemModel{

  /*! \brief Container for VariantTableModel
   */
  class VariantTableModelRow
  {
   public:

    /*! \brief Construct a row with colCount columns
     */
    VariantTableModelRow(VariantTableModelStorageRule storageRule, int colCount)
     : mRowData(colCount, VariantTableModelItem(storageRule))
    {
    }

    /*! \brief Get column count
     */
    int columnCount() const
    {
      return mRowData.size();
    }

    /*! \brief Get data at column
     *
     * If storage rule (passed in constructor) is SeparateDisplayAndEditRoleData,
     *  data for Qt::EditRole will be distinct from data for Qt::DisplayRole.
     *
     * \pre column must be in valid range
     *      ( 0 <= column < columnCount() )
     * \pre role must be Qt::DisplayRole or Qt::EditRole
     */
    QVariant data(int column, int role) const
    {
      Q_ASSERT(column >= 0);
      Q_ASSERT(column < columnCount());
      Q_ASSERT( (role == Qt::DisplayRole) || (role == Qt::EditRole) );
      return mRowData[column].data(role);
    }

    /*! \brief Set item enabled at column
     *
     * \pre column must be in valid range
     *      ( 0 <= column < columnCount() )
     */
    void setItemEnabled(int column, bool enable)
    {
      Q_ASSERT(column >= 0);
      Q_ASSERT(column < columnCount());
      mRowData[column].setEnabled(enable);
    }

    /*! \brief Set item editable at column
     *
     * \pre column must be in valid range
     *      ( 0 <= column < columnCount() )
     */
    void setItemEditable(int column, bool editable)
    {
      Q_ASSERT(column >= 0);
      Q_ASSERT(column < columnCount());
      mRowData[column].setEditable(editable);
    }

    /*! \brief Get flags at column
     *
     * Returns currentFlags with Qt::ItemIsEditable and ItemIsEnabled
     *  set or unset regarding what was set ba setItemEditable() and setItemEnabled().
     *
     * \pre column must be in valid range
     *      ( 0 <= column < columnCount() )
     */
    Qt::ItemFlags flags(int column, Qt::ItemFlags currentFlags) const
    {
      Q_ASSERT(column >= 0);
      Q_ASSERT(column < columnCount());
      return mRowData[column].flags(currentFlags);
    }

    /*! \brief Set data at column
     *
     * If storage rule (passed in constructor) is SeparateDisplayAndEditRoleData,
     *  data for Qt::EditRole will be distinct from data for Qt::DisplayRole.
     *
     * \pre column must be in valid range
     *      ( 0 <= column < columnCount() )
     * \pre role must be Qt::DisplayRole or Qt::EditRole
     */
    void setData(int column, const QVariant & value, int role = Qt::EditRole)
    {
      Q_ASSERT(column >= 0);
      Q_ASSERT(column < columnCount());
      Q_ASSERT( (role == Qt::DisplayRole) || (role == Qt::EditRole) );
      mRowData[column].setData(value, role);
    }

   private:

    std::vector<VariantTableModelItem> mRowData;
  };

}} // namespace Mdt{ namespace ItemModel{

#endif // #ifndef MDT_ITEM_MODEL_VARIANT_TABLE_MODEL_ROW_H
