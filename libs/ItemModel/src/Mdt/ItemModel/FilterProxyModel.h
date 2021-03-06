/****************************************************************************
 **
 ** Copyright (C) 2011-2017 Philippe Steinmann.
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
#ifndef MDT_ITEM_MODEL_FILTER_PROXY_MODEL_H
#define MDT_ITEM_MODEL_FILTER_PROXY_MODEL_H

#include "FilterColumn.h"
#include "LikeExpression.h"
#include "FilterExpression.h"
#include "MdtItemModelExport.h"
#include <QSortFilterProxyModel>

namespace Mdt{ namespace ItemModel{

  class FilterExpression;

  /*! \brief Provides support for filtering data between another model and a view
   *
   * Example to use compile time dfined filter:
   * \code
   * #include <Mdt/ItemModel/FilterProxyModel.h>
   * #include <QTableView>
   * #include "ClientTableModel.h"
   *
   * namepsace ItemModel = Mdt::ItemModel;
   * using ItemModel::FilterProxyModel;
   * using ItemModel::FilterColumn;
   * using Like = ItemModel::LikeExpression;
   *
   * auto *view = new QTableView;
   * auto *model = new ClientTableModel(view);
   * auto *proxyModel = new FilterProxyModel(view);
   *
   * proxyModel->setSourceModel(model);
   * view->setModel(proxyModel);
   *
   * // Setup a filter and apply it
   * FilterColumn clientFirstName(1);
   * FilterColumn clientLastName(2);
   * proxyModel->setFilter( (clientFirstName == "A") && (clientLastName == Like("A?B*\\?*")) );
   * \endcode
   */
  class MDT_ITEMMODEL_EXPORT FilterProxyModel : public QSortFilterProxyModel
  {
   Q_OBJECT

   public:

    /*! \brief Construct a model
     */
    explicit FilterProxyModel(QObject* parent = nullptr);

    // Disable copy
    FilterProxyModel(const FilterProxyModel &) = delete;
    FilterProxyModel & operator=(const FilterProxyModel &) = delete;
    // Disable move
    FilterProxyModel(FilterProxyModel &&) = delete;
    FilterProxyModel & operator=(FilterProxyModel &&) = delete;

    /*! \brief Set filter
     *
     * \param expression Expression to apply as filter.
     * \pre \a expression must be a filter expression type.
     *       A filter expression is based on FilterColumn,
     *       comparison operators, logical AND, logical OR.
     *       For example (see example code above for details):
     *       \code
     *       // Example of valid filter expression
     *       cliendId == 25
     *       (cliendId == 25) && (clientName != "A")
     *
     *       // Example of invalid filter expression
     *       cliendId + 5
     *       \endcode
     */
    template<typename Expr>
    void setFilter(const Expr & expression)
    {
      mFilterExpression.setExpression(expression);
      invalidateFilter();
    }

   private:

    /*! \brief Return true if filter expression was set and evaluates true
     */
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    FilterExpression mFilterExpression;
  };

}} // namespace Mdt{ namespace ItemModel{

#endif // #ifndef MDT_ITEM_MODEL_FILTER_PROXY_MODEL_H
