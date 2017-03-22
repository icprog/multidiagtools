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
#include "AddressWidget.h"
#include "AddressModel.h"
#include "Mdt/ItemEditor/TableViewController.h"
#include "Mdt/ItemModel/FormatProxyModel.h"
#include <QLabel>

using namespace Mdt::ItemModel;
using namespace Mdt::ItemEditor;

AddressWidget::AddressWidget(QWidget* parent)
 : TableViewWidget(parent)
{
  auto *model = new AddressModel(this);
  setModel(model);
  addResizeToContentsActionToTopBar();
  addNavigationActionsToTopArea();
  addEditionActionsToBottomArea();
  addInsertActionToBottomArea();
  setInsertActionText(tr("Add address"));
  addRemoveActionToBottomArea();
  setRemoveActionText(tr("Remove addresse"));
  auto ctrl = controller();
  ctrl->setInsertLocation(TableViewController::InsertAtEnd);
  ctrl->setPrimaryKey({0});
//   ctrl->setPrimaryKeyEditable(false);
//   ctrl->setPrimaryKeyItemsEnabled(false);
  ctrl->setPrimaryKeyHidden(true);
  ctrl->setForeignKey({1});
//   ctrl->setForeignKeyEditable(false);
//   ctrl->setForeignKeyItemsEnabled(false);
  ctrl->setForeignKeyHidden(true);
  /*
   * Setup some formatting
   */
  auto formatModel = new FormatProxyModel(this);
  formatModel->setTextAlignmentForColumn(0, Qt::AlignCenter);
  formatModel->setTextAlignmentForColumn(1, Qt::AlignCenter);
  QFont font;
  font.setFamily("Times");
  font.setPointSize(12);
  font.setBold(true);
  formatModel->setTextFontForColumn(2, font);
  appendProxyModel(formatModel);

  addWidgetToTopArea(new QLabel("State: "));
  mStateLabel = new QLabel;
  addWidgetToTopArea(mStateLabel);
  connect(ctrl, &TableViewController::controllerStateChanged, this, &AddressWidget::onControllerStateChanged);
}

void AddressWidget::onControllerStateChanged(Mdt::ItemEditor::ControllerState state)
{
  mStateLabel->setText( controllerStateText(state) );
}
