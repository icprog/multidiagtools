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
#ifndef MDT_ITEM_MODEL_FILTER_PROXY_MODEL_H
#define MDT_ITEM_MODEL_FILTER_PROXY_MODEL_H

namespace Mdt{ namespace ItemModel{

  /*! \brief Provides support for filtering data between another model and a view
   *
   * Example of usage:
   * \code
   * #include <Mdt/ItemModel/FilterProxyModel.h>
   * #include <QTableView>
   * #include "ClientTableModel.h"
   *
   * -> Update includes and using once fixed
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
   *
   * Example to filter data by matching data from a other model:
   * \todo seems to have no sense..
   * \code
   * #include <Mdt/ItemModel/FilterProxyModel.h>
   * #include <QTableView>
   * #include "ClientTableModel.h"
   * #include "AddressTableModel.h"
   *
   * namepsace ItemModel = Mdt::ItemModel;
   * using ItemModel::FilterProxyModel;
   * using ItemModel::FilterColumn;
   * using ItemModel::FilterModelColumn;
   *
   * auto *view = new QTableView;
   * auto *addressModel = new AddressTableModel(view);
   * auto *proxyModel = new FilterProxyModel(view);
   * auto *clientModel = new ClientTableModel;
   *
   * // Setup view act on addressModel
   * proxyModel->setSourceModel(addressModel);
   * view->setModel(proxyModel);
   *
   * // Setup a filter and apply it
   * FilterModelColumn
   * 
   * FilterColumn clientFirstName(1);
   * FilterColumn clientLastName(2);
   * proxyModel->setFilter( (clientFirstName == "A") && (clientLastName == "%B%") );
   * \endcode
   */
  class FilterProxyModel
  {
  };

}} // namespace Mdt{ namespace ItemModel{

#endif // #ifndef MDT_ITEM_MODEL_FILTER_PROXY_MODEL_H
