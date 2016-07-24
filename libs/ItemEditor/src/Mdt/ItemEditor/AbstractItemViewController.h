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
#ifndef MDT_ITEM_EDITOR_ABSTRACT_ITEM_VIEW_CONTROLLER_H
#define MDT_ITEM_EDITOR_ABSTRACT_ITEM_VIEW_CONTROLLER_H

#include "AbstractController.h"
#include <QPointer>

class QAbstractItemModel;
class QAbstractItemView;

namespace Mdt{ namespace ItemEditor{

  class ItemSelectionModel;
  class EventCatchItemDelegate;

  /*! \brief Common base class for controllers that act on a QAbstractItemModel and a QAbstractItemView
   */
  class AbstractItemViewController : public AbstractController
  {
   Q_OBJECT

   public:

    /*! \brief Constructor
     */
    explicit AbstractItemViewController(QObject* parent = nullptr);

    // Copy disabled
    AbstractItemViewController(const AbstractItemViewController &) = delete;
    AbstractItemViewController & operator=(const AbstractItemViewController &) = delete;
    // Move disabled
    AbstractItemViewController(AbstractItemViewController &&) = delete;
    AbstractItemViewController & operator=(AbstractItemViewController &&) = delete;

    /*! \brief Attach view to this controller
     *
     * Once view is attached, this controller
     *  take control of it.
     *
     * \note A controller will only control 1 view.
     *       Also, controller does not take ownership
     *       of the view (it will not delete it).
     * \pre view must be a valid pointer.
     */
    void setView(QAbstractItemView *view);

    /*! \brief Get view attached to this controller
     *
     * Will also return a nullptr if no view was set
     */
    QAbstractItemView *view() const;

    /*! \brief Set model
     *
     * \note Because model can be shared with several objects (f.ex. other views),
     *        the controller does not take ownership of it (it will not delete it).
     * \pre model must be a valid pointer
     */
    void setModel(QAbstractItemModel *model) override;

   protected:

    /*! \brief Submit data to model
     */
    bool submitDataToModel() override;

    /*! \brief Revert data from model
     */
    void revertDataFromModel() override;

   private slots:

    /*! \brief Set model to view
     */
    void setModelToView(QAbstractItemModel *model);

   private:

    QPointer<QAbstractItemView> pvView;
    EventCatchItemDelegate *pvDelegate;
  };

}} // namespace Mdt{ namespace ItemEditor{

#endif // #ifndef MDT_ITEM_EDITOR_ABSTRACT_ITEM_VIEW_CONTROLLER_H
