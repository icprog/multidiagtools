/****************************************************************************
 **
 ** Copyright (C) 2011-2013 Philippe Steinmann.
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
#include "mdtSqlFormWidget.h"
#include "mdtSqlDataWidgetMapper.h"
#include "mdtSqlFieldHandler.h"
#include "mdtError.h"
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QString>
#include <QLayout>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QMessageBox>

#include <QDebug>

mdtSqlFormWidget::mdtSqlFormWidget(QWidget *parent)
 : mdtAbstractSqlWidget(parent)
{
  pvWidgetMapper = new mdtSqlDataWidgetMapper(this);
  pvWidgetMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
  pvFirstDataWidget = 0;
}

mdtSqlFormWidget::~mdtSqlFormWidget()
{
  qDeleteAll(pvFieldHandlers);
}

void mdtSqlFormWidget::mapFormWidgets(const QString &firstWidgetInTabOrder)
{
  Q_ASSERT(layout() != 0);
  Q_ASSERT(pvWidgetMapper->model() != 0);

  int i, fieldIndex;
  QString fieldName;
  QWidget *w;
  mdtSqlFieldHandler *fieldHandler;
  QSqlRecord record;

  // Clear previous mapping
  disconnect(pvWidgetMapper, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
  qDeleteAll(pvFieldHandlers);
  pvFieldHandlers.clear();
  pvWidgetMapper->clearMapping();
  pvFirstDataWidget = 0;
  // Search widgets in layout's childs
  for(i=0; i<layout()->count(); ++i){
    w = layout()->itemAt(i)->widget();
    if(w != 0){
      fieldName = w->objectName();
      // If name begins with fld_ , search matching field in model
      if(fieldName.left(4) == "fld_"){
        fieldName = fieldName.right(fieldName.length()-4);
        fieldIndex = pvWidgetMapper->model()->record().indexOf(fieldName);
        // If field was found, map it
        if(fieldIndex >= 0){
          pvWidgetMapper->addMapping(w, fieldIndex);
          fieldHandler = new mdtSqlFieldHandler;
          // If we want informations about fields, we must get record from database instance
          record = pvWidgetMapper->model()->database().record(pvWidgetMapper->model()->tableName());
          fieldHandler->setField(record.field(fieldIndex));
          fieldHandler->setDataWidget(w);
          ///connect(pvWidgetMapper, SIGNAL(currentIndexChanged(int)), fieldHandler, SLOT(onCurrentIndexChanged(int)));
          ///connect(pvWidgetMapper, SIGNAL(currentIndexChanged(int)), fieldHandler, SLOT(updateFlags()));
          connect(fieldHandler, SIGNAL(dataEdited()), this, SIGNAL(dataEdited()));
          /*
           * When we go out from Editing state, we must update flags
           */
          /// \todo Check redoundances with onCurrentIndexChanged()
          connect(this, SIGNAL(stateEditingExited()), fieldHandler, SLOT(updateFlags()));
          connect(this, SIGNAL(stateEditingNewRowExited()), fieldHandler, SLOT(updateFlags()));
          pvFieldHandlers.append(fieldHandler);
          // If this widget is the first in focus chain, ref it
          qDebug() << "Mapping widget " << w->objectName() << " ...";
          if(w->objectName() == firstWidgetInTabOrder){
            qDebug() << "-> First widget : " << w->objectName();
            pvFirstDataWidget = w;
          }
        }else{
          w->setEnabled(false);
          mdtError e(MDT_DATABASE_ERROR, "Cannot find field for widget " + w->objectName(), mdtError::Warning);
          MDT_ERROR_SET_SRC(e, "mdtSqlFormWidget");
          e.commit();
        }
      }
    }
  }
  connect(pvWidgetMapper, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
  pvWidgetMapper->toFirst();
}

int mdtSqlFormWidget::currentRow() const
{
  return pvWidgetMapper->currentIndex();
}

void mdtSqlFormWidget::toFirst()
{
  if(currentState() != Visualizing){
    warnUserAboutUnsavedRow();
    return;
  }
  pvWidgetMapper->toFirst();
}

void mdtSqlFormWidget::toLast()
{
  if(currentState() != Visualizing){
    warnUserAboutUnsavedRow();
    return;
  }
  pvWidgetMapper->toLast();
}

void mdtSqlFormWidget::toPrevious()
{
  if(currentState() != Visualizing){
    warnUserAboutUnsavedRow();
    return;
  }
  pvWidgetMapper->toPrevious();
}

void mdtSqlFormWidget::toNext()
{
  if(currentState() != Visualizing){
    warnUserAboutUnsavedRow();
    return;
  }
  pvWidgetMapper->toNext();
}

void mdtSqlFormWidget::doSetModel(QSqlTableModel *model)
{
  Q_ASSERT(model != 0);

  model->setEditStrategy(QSqlTableModel::OnManualSubmit);
  pvWidgetMapper->setModel(model);
  pvInsertionPending = false;
}

void mdtSqlFormWidget::onCurrentIndexChanged(int row)
{
  Q_ASSERT(pvWidgetMapper->model() != 0);

  bool widgetsEnabled;
  mdtSqlFieldHandler *fieldHandler;
  int i;

  // Check if we have to enable/disable widgets regarding current row
  if(row < 0){
    widgetsEnabled = false;
  }else{
    widgetsEnabled = true;
  }
  qDebug() << "ROW changed, row: " << row << " , will enable widgets: " << widgetsEnabled;
  // Update widgets
  for(i=0; i<pvFieldHandlers.size(); ++i){
    fieldHandler = pvFieldHandlers.at(i);
    Q_ASSERT(fieldHandler != 0);
    if(fieldHandler->dataWidget() != 0){
      fieldHandler->dataWidget()->setEnabled(widgetsEnabled);
      if(!widgetsEnabled){
        fieldHandler->clearWidgetData();
      }
      fieldHandler->updateFlags();
    }
  }
  // Update navigation states
  if(row < 1){
    emit toFirstEnabledStateChanged(false);
    emit toPreviousEnabledStateChanged(false);
  }else{
    emit toFirstEnabledStateChanged(true);
    emit toPreviousEnabledStateChanged(true);
  }
  if(row < (pvWidgetMapper->model()->rowCount()-1)){
    emit toLastEnabledStateChanged(true);
    emit toNextEnabledStateChanged(true);
  }else{
    emit toLastEnabledStateChanged(false);
    emit toNextEnabledStateChanged(false);
  }
}

bool mdtSqlFormWidget::doSubmit()
{
  Q_ASSERT(pvWidgetMapper->model() != 0);

  int row;

  // Remember current row (will be lost during submit)
  row = pvWidgetMapper->currentIndex();
  qDebug() << "SAVE - row: " << row;

  // Do some check before real submit
  if(!checkBeforeSubmit()){
    QMessageBox msgBox;
    msgBox.setText(tr("There are some errors in edited data."));
    msgBox.setInformativeText( \
      tr("Fields that are not correct should be highlighted.\nMoving cursor over field with error should display the reason\nPlease correct errors, or cancel modifications, and try again."));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();    return false;
  }
  // Call widget mapper submit() (will commit data from widgets to model)
  if(!pvWidgetMapper->submit()){
    return false;
  }
  /*
   * We use QDataWidgetMapper::ManualSubmit submit policy and QSqlTableModel::OnManualSubmit edit strategy.
   * Widget mapper calls submit() on model, but this has no effect with OnManualSubmit edit strategy,
   * so we have to call submitAll() on model.
   */
  if(!pvWidgetMapper->model()->submitAll()){
    qDebug() << "mdtSqlFormWidget::submit(): Error handling not implemented !!!!!!!!!!!!!!!!!!!!";
    qDebug() << "-> Error: " << pvWidgetMapper->model()->lastError();
    return false;
  }
  // Go back to current row
  pvWidgetMapper->setCurrentIndex(row);

  return true;
}

bool mdtSqlFormWidget::doRevert()
{
  pvWidgetMapper->revert();

  return true;
}

bool mdtSqlFormWidget::doInsert()
{
  Q_ASSERT(pvWidgetMapper->model() != 0);

  int row;

  if(currentState() != Inserting){
    warnUserAboutUnsavedRow();
    return false;
  }
  // Insert new row at end
  row = pvWidgetMapper->model()->rowCount();
  pvWidgetMapper->model()->insertRow(row);
  pvWidgetMapper->setCurrentIndex(row);
  clearWidgets();
  setFocusOnFirstDataWidget();

  return true;
}

bool mdtSqlFormWidget::doSubmitNewRow()
{
  Q_ASSERT(pvWidgetMapper->model() != 0);

  return doSubmit();
}

bool mdtSqlFormWidget::doRevertNewRow()
{

  Q_ASSERT(pvWidgetMapper->model() != 0);

  int row;

  // Remeber current row and remove it
  row = pvWidgetMapper->currentIndex();
  if(!pvWidgetMapper->model()->removeRow(row)){
    qDebug() << "mdtSqlFormWidget::remove(): doRevertNewRow() failed Error handling not implemented !!!!!!!!!!!!!!!!!!!!";
    return false;
  }
  // Data was never submit to model, we simply go to last row
  clearWidgets();
  pvWidgetMapper->setCurrentIndex(qMin(row, pvWidgetMapper->model()->rowCount()-1));

  return true;
}

bool mdtSqlFormWidget::doRemove()
{
  Q_ASSERT(pvWidgetMapper->model() != 0);

  int row;

  // Remeber current row (will be lost during submit)
  row = pvWidgetMapper->currentIndex();
  qDebug() << "DEL - Current row: " << row;
  if(!pvWidgetMapper->model()->removeRow(row)){
    qDebug() << "mdtSqlFormWidget::doRemove(): removeRow() failed Error handling not implemented !!!!!!!!!!!!!!!!!!!!";
    return false;
  }
  // Call widget mapper submit() (will commit data from widgets to model)
  if(!pvWidgetMapper->submit()){
    qDebug() << "mdtSqlFormWidget::doRemove(): submit() failed Error handling not implemented !!!!!!!!!!!!!!!!!!!!";
    return false;
  }
  /*
   * We use QDataWidgetMapper::ManualSubmit submit policy and QSqlTableModel::OnManualSubmit edit strategy.
   * Widget mapper calls submit() on model, but this has no effect with OnManualSubmit edit strategy,
   * so we have to call submitAll() on model.
   */
  if(!pvWidgetMapper->model()->submitAll()){
    qDebug() << "mdtSqlFormWidget::doRemove(): Error handling not implemented !!!!!!!!!!!!!!!!!!!!";
    qDebug() << "-> Error: " << pvWidgetMapper->model()->lastError();
    return false;
  }
  // Go back to current row
  row = qMin(row, pvWidgetMapper->model()->rowCount()-1);
  if(row < 0){
    onCurrentIndexChanged(-1);
  }else{
    pvWidgetMapper->setCurrentIndex(row);
  }
  /**
  if(row < 0){
    clearWidgets();
    setWidgetsEnabled(false);
  }else{
    pvWidgetMapper->setCurrentIndex(row);
  }
  */
  qDebug() << "DEL done - Current row: " << row;
  

  return true;
}


void mdtSqlFormWidget::clearWidgets()
{
  int i;

  for(i=0; i<pvFieldHandlers.size(); ++i){
    Q_ASSERT(pvFieldHandlers.at(i) != 0);
    pvFieldHandlers.at(i)->clearWidgetData();
  }
}

void mdtSqlFormWidget::setFocusOnFirstDataWidget()
{
  if(pvFirstDataWidget != 0){
    qDebug() << "Set focus to " << pvFirstDataWidget->objectName();
    pvFirstDataWidget->setFocus();
  }
}

void mdtSqlFormWidget::setWidgetsEnabled(bool enabled)
{
  int i;
  QWidget *w;

  for(i=0; i<pvFieldHandlers.size(); ++i){
    Q_ASSERT(pvFieldHandlers.at(i) != 0);
    w = pvFieldHandlers.at(i)->dataWidget();
    if(w != 0){
      w->setEnabled(enabled);
    }
  }
}

bool mdtSqlFormWidget::checkBeforeSubmit()
{
  int i;
  bool allOk = true;
  QWidget *firstNokWidget = 0;

  // We call checkBeforeSubmit() for all items
  for(i=0; i<pvFieldHandlers.size(); ++i){
    Q_ASSERT(pvFieldHandlers.at(i) != 0);
    if(!pvFieldHandlers.at(i)->checkBeforeSubmit()){
      allOk = false;
      if(firstNokWidget == 0){
        firstNokWidget = pvFieldHandlers.at(i)->dataWidget();
      }
    }
  }
  // We set the focus on first Nok widget
  if(firstNokWidget != 0){
    firstNokWidget->setFocus();
  }

  return allOk;
}

void mdtSqlFormWidget::warnUserAboutUnsavedRow()
{
  QMessageBox msgBox;

  msgBox.setText(tr("Current record was modified."));
  msgBox.setInformativeText(tr("Please save or cancel your modifications and try again."));
  msgBox.setIcon(QMessageBox::Warning);
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.exec();
}
