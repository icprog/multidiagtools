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
#ifndef MDT_SQL_COPIER_UNIQUE_INSERT_EXPRESSION_MODEL_H
#define MDT_SQL_COPIER_UNIQUE_INSERT_EXPRESSION_MODEL_H

#include "UniqueInsertExpression.h"
#include "TableMapping.h"
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QVariant>

namespace mdt{ namespace sql{ namespace copier{

  /*! \brief Table model to access UniqueInsertExpression match items
   */
  class UniqueInsertExpressionModel : public QAbstractTableModel
  {
   Q_OBJECT

   public:

    /*! \brief Constructor
     */
    UniqueInsertExpressionModel(const TableMapping & tm, UniqueInsertExpression & exp, QObject *parent = nullptr);

    // Copy is disabled
    UniqueInsertExpressionModel(const UniqueInsertExpressionModel &) = delete;
    UniqueInsertExpressionModel & operator=(const UniqueInsertExpressionModel &) = delete;

    /*! \brief Get row count
     *
     * Returns the number of match items in expression
     */
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;

    /*! \brief Get column count
     */
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;

    /*! \brief Get header data
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /*! \brief Get data
     */
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

    /*! \brief Set data
     */
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;

    /*! \brief Get item flags
     */
    Qt::ItemFlags flags(const QModelIndex & index) const override;

    /*! \brief Insert row
     */
    bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex()) override;

    /*! \brief Remove rows
     */
    bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex()) override;

   private:

    /*! \brief Column index
     */
    enum ColumnIndex
    {
      OperatorWithPreviousIndex,  /*!< Column index of operator with previous match item */
      DestinationFieldNameIndex,  /*!< Column index of destination field name */
      MatchOperatorIndex,         /*!< Column index of operator between destination field and source value field */
      SourceValueFieldNameIndex   /*!< Column index of source value field name */
    };

    /*! \brief Get operator with previous text
     */
    QVariant operatorWithPreviousText(const UniqueInsertMatchExpressionItem & item) const;

    /*! \brief Set destination field
     */
    void setDestinationField(UniqueInsertMatchExpressionItem & item, const QString & fieldName);

    /*! \brief Get destination field name
     */
    QVariant destinationFieldName(const UniqueInsertMatchExpressionItem & item) const;

    /*! \brief Set source value field
     */
    void setSourceValueField(UniqueInsertMatchExpressionItem & item, const QString & fieldName);

    /*! \brief Get source value field name
     */
    QVariant sourceValueFieldName(const UniqueInsertMatchExpressionItem & item) const;

    const TableMapping & pvTableMapping;
    UniqueInsertExpression & pvExpression;
  };

}}} // namespace mdt{ namespace sql{ namespace copier{

#endif // #ifndef MDT_SQL_COPIER_UNIQUE_INSERT_EXPRESSION_MODEL_H
