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
#ifndef MDT_ITEM_MODEL_EXPRESSION_CONTAINER_INTERFACE_H
#define MDT_ITEM_MODEL_EXPRESSION_CONTAINER_INTERFACE_H

#include <Qt>

class QAbstractItemModel;

namespace Mdt{ namespace ItemModel{ namespace Expression{

  /*! \brief Interface for container implementing a expression acting on a item model
   */
  class ContainerInterface
  {
   public:

    // Constructors / destructors
    ContainerInterface() = default;
    virtual ~ContainerInterface() = default;
    // Copy disabled
    ContainerInterface(const ContainerInterface &) = delete;
    ContainerInterface & operator=(const ContainerInterface &) = delete;
    // Move disabled
    ContainerInterface(ContainerInterface &&) = delete;
    ContainerInterface & operator=(ContainerInterface &&) = delete;

    /*! \brief Evaluate if row matches stored expression in model
     *
     * \pre model must be a valid pointer (not null)
     * \pre must be in valid range ( 0 <= row < model->rowCount() )
     */
    virtual bool eval(const QAbstractItemModel * const model, int row, Qt::CaseSensitivity caseSensitivity) const = 0;
  };

}}} // namespace Mdt{ namespace ItemModel{ namespace Expression{

#endif // #ifndef MDT_ITEM_MODEL_EXPRESSION_CONTAINER_INTERFACE_H
