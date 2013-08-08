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
#include "mdtSqlSelectionDialog.h"
#include <QSqlQueryModel>
#include <QTableView>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSqlRecord>
#include <QItemSelectionModel>
#include <QLabel>

mdtSqlSelectionDialog::mdtSqlSelectionDialog(QWidget *parent)
 : QDialog(parent)
{
  QVBoxLayout *layout;
  QDialogButtonBox *buttonBox;

  // Set defaults
  pvDialogRejected = false;
  pvModel = 0;

  layout = new QVBoxLayout;

  // Setup message label
  pvMessageLabel = new QLabel;
  layout->addWidget(pvMessageLabel);
  // Setup table view
  pvTableView = new QTableView;
  pvTableView->setSortingEnabled(true);
  pvTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  pvTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  layout->addWidget(pvTableView);
  // Setup buttons
  buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  Q_ASSERT(buttonBox->button(QDialogButtonBox::Ok) != 0);
  connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
  Q_ASSERT(buttonBox->button(QDialogButtonBox::Cancel) != 0);
  connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
  layout->addWidget(buttonBox);
  setLayout(layout);
}

mdtSqlSelectionDialog::~mdtSqlSelectionDialog()
{
}

void mdtSqlSelectionDialog::setMessage(const QString &message)
{
  pvMessageLabel->setText(message);
}

void mdtSqlSelectionDialog::setModel(QSqlQueryModel *model, bool allowMultiSelection)
{
  Q_ASSERT(model != 0);

  pvModel = model;
  pvTableView->setModel(model);
  pvTableView->resizeColumnsToContents();
  pvTableView->resizeRowsToContents();
  if(allowMultiSelection){
    pvTableView->setSelectionMode(QAbstractItemView::MultiSelection);
  }else{
    pvTableView->setSelectionMode(QAbstractItemView::SingleSelection);
  }
}

void mdtSqlSelectionDialog::setHeaderData(const QString &fieldName, const QString &data)
{
  Q_ASSERT(pvModel != 0);

  int column;

  column = pvModel->record().indexOf(fieldName);
  if(column < 0){
    return;
  }
  pvModel->setHeaderData(column, Qt::Horizontal, data);
  pvTableView->resizeColumnToContents(column);
}

void mdtSqlSelectionDialog::setColumnHidden(int column, bool hide)
{
  pvTableView->setColumnHidden(column, hide);
}

void mdtSqlSelectionDialog::setColumnHidden(const QString &fieldName, bool hide)
{
  Q_ASSERT(pvModel != 0);

  int column;

  column = pvModel->record().indexOf(fieldName);
  if(column < 0){
    return;
  }
  setColumnHidden(column, hide);
}

void mdtSqlSelectionDialog::setSelectionResultColumns(const QList<int> &columns)
{
  pvSelectionResultColumns = columns;
}

void mdtSqlSelectionDialog::setSelectionResultColumns(const QStringList &fields)
{
  Q_ASSERT(pvModel != 0);
  int column;
  int i;

  pvSelectionResultColumns.clear();
  for(i = 0; i < fields.size(); ++i){
    column = pvModel->record().indexOf(fields.at(i));
    if(column >= 0){
      pvSelectionResultColumns.append(column);
    }
  }
}

void mdtSqlSelectionDialog::addSelectionResultColumn(const QString &field)
{
  Q_ASSERT(pvModel != 0);
  int column;

  column = pvModel->record().indexOf(field);
  if(column >= 0){
    pvSelectionResultColumns.append(column);
  }
}

QList<QVariant> mdtSqlSelectionDialog::selectionResult()
{
  Q_ASSERT(pvModel != 0);
  Q_ASSERT(pvTableView->selectionModel() != 0);

  QList<QVariant> result;
  int row, i;
  QModelIndex index;
  QModelIndexList selectedRows;

  /**
  if(pvDialogRejected){
    return result;
  }
  selectedRows = pvTableView->selectionModel()->selectedRows();
  if(selectedRows.size() != 1){
    return result;
  }
  index = selectedRows.at(0);
  if(!index.isValid()){
    return result;
  }
  row = index.row();
  for(i = 0; i < pvSelectionResultColumns.size(); ++i){
    index = pvModel->index(row, pvSelectionResultColumns.at(i));
    result.append(pvModel->data(index));
  }
  */
  if(pvSelectionResults.isEmpty()){
    return result;
  }
  // We take only first row of selection results
  index = pvSelectionResults.at(0);
  if(!index.isValid()){
    return result;
  }
  row = index.row();
  for(i = 0; i < pvSelectionResultColumns.size(); ++i){
    index = pvModel->index(row, pvSelectionResultColumns.at(i));
    result.append(pvModel->data(index));
  }

  return result;
}

QModelIndexList mdtSqlSelectionDialog::selectionResults()
{
  return pvSelectionResults;
}

void mdtSqlSelectionDialog::reject()
{
  pvDialogRejected = true;
  QDialog::reject();
}

void mdtSqlSelectionDialog::accept()
{
  buildSelectionResults();
  if(pvSelectionResults.isEmpty()){
    return;
  }
  pvDialogRejected = false;
  QDialog::accept();
}

void mdtSqlSelectionDialog::buildSelectionResults()
{
  Q_ASSERT(pvModel != 0);
  Q_ASSERT(pvTableView->selectionModel() != 0);

  int i;
  QModelIndex index;
  QModelIndexList selectedIndexes;

  pvSelectionResults.clear();
  selectedIndexes = pvTableView->selectionModel()->selectedRows();
  for(i = 0; i < selectedIndexes.size(); ++i){
    index = pvModel->index(selectedIndexes.at(i).row(), selectedIndexes.at(i).column());
    if((index.isValid()) && (pvSelectionResultColumns.contains(index.column()))){
      pvSelectionResults.append(index);
    }
  }
}
