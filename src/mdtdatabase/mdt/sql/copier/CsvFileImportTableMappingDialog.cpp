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
#include "CsvFileImportTableMappingDialog.h"
#include "CsvFileImportTableMappingModel.h"
#include "mdtCsvFileParserSettingsDialog.h"
#include "mdtErrorDialog.h"
#include "mdt/sql/Database.h"
#include <QLabel>
#include <QComboBox>
#include <QSignalBlocker>
#include <QToolButton>
#include <QHBoxLayout>

namespace mdt{ namespace sql{ namespace copier{

CsvFileImportTableMappingDialog::CsvFileImportTableMappingDialog(QWidget *parent)
 : TableMappingDialog(parent),
   pvMappingModel(new CsvFileImportTableMappingModel(this))
{
  setModel(pvMappingModel);
  // Setup source table widgets
  QHBoxLayout *l = new QHBoxLayout;
  lbSourceTable = new QLabel;
  QToolButton *tb = new QToolButton;
  tb->setText("...");
  l->addWidget(tb);
  l->addWidget(lbSourceTable);
  setSourceTableLayout(l);
  connect(tb, &QToolButton::clicked, this, &CsvFileImportTableMappingDialog::setupSourceFile);
  // Setup destination table widgets
  l = new QHBoxLayout;
  cbDestinationTable = new QComboBox;
  l->addWidget(cbDestinationTable);
  setDestinationTableLayout(l);
  connect(cbDestinationTable, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &CsvFileImportTableMappingDialog::setDestinationTable);
}

void CsvFileImportTableMappingDialog::setDestinationDatabase(const QSqlDatabase & db)
{
  Q_ASSERT(db.isOpen());

  pvMappingModel->setDestinationDatabase(db);
  populateDestinationTableCombobox(db);
}

void CsvFileImportTableMappingDialog::setMapping(const std::shared_ptr<CsvFileImportTableMapping> & m)
{
  Q_ASSERT(m);
//   Q_ASSERT(m->sourceDatabase().isOpen());
  Q_ASSERT(m->destinationDatabase().isOpen());

  // Prevent clearing mapping
  QSignalBlocker cbSignalBlocker(cbDestinationTable);
  // Update mapping
  pvMappingModel->setMapping(m);
  updateMapping();
  displaySourceTable();
  populateDestinationTableCombobox(m->destinationDatabase());
  cbDestinationTable->setCurrentText(m->destinationTableName());
}

std::shared_ptr<CsvFileImportTableMapping> CsvFileImportTableMappingDialog::mapping() const
{
  return pvMappingModel->mapping();
}

void CsvFileImportTableMappingDialog::setupSourceFile()
{
  mdtCsvFileParserSettingsDialog dialog(this);
  auto tm = pvMappingModel->mapping();
  Q_ASSERT(tm);

  // Setup and show dialog
  dialog.setFileSettings(tm->sourceFileInfo().absoluteFilePath(), tm->sourceFileEncoding());
  dialog.setCsvSettings(tm->sourceCsvSettings());
  dialog.setRecordFormat(pvMappingModel->sourceRecordFormat());
  if(dialog.exec() != QDialog::Accepted){
    return;
  }
  // Update source CSV settings
  if(!pvMappingModel->setSourceCsvSettings(dialog.getCsvSettings())){
    auto error = pvMappingModel->lastError();
    mdtErrorDialog errorDialog(error);
    errorDialog.exec();
    return;
  }
  // Update source CSV file
  if(!pvMappingModel->setSourceFile(dialog.filePath(), dialog.fileEncoding())){
    auto error = pvMappingModel->lastError();
    mdtErrorDialog errorDialog(error);
    errorDialog.exec();
    return;
  }
  // Update source CSV record format
  pvMappingModel->setSourceRecordFormat(dialog.recordFormat());
  // Update dialog
  displaySourceTable();
  updateSourceTableFieldSelectionDelegate();
}

void CsvFileImportTableMappingDialog::setDestinationTable(int cbIndex)
{
  if(cbIndex < 0){
    return;
  }
  if(!pvMappingModel->mapping()->destinationDatabase().isOpen()){
    return;
  }
  QString tableName = cbDestinationTable->itemText(cbIndex);
  if(!pvMappingModel->setDestinationTable(tableName)){
    displayError(pvMappingModel->lastError());
    return;
  }
  updateSourceTableFieldSelectionDelegate();
}

void CsvFileImportTableMappingDialog::displaySourceTable()
{
  lbSourceTable->setText(pvMappingModel->sourceTableName());
}

void CsvFileImportTableMappingDialog::populateDestinationTableCombobox(const QSqlDatabase & db)
{
  using mdt::sql::Database;

  auto tables = Database::getTables(db, Database::Tables);
  tables.sort();
  cbDestinationTable->clear();
  cbDestinationTable->addItems(tables);
}

void CsvFileImportTableMappingDialog::displayError(const mdtError &error)
{
  mdtErrorDialog dialog(error, this);
  dialog.exec();
}

const std::shared_ptr<const TableMapping> CsvFileImportTableMappingDialog::mappingBase() const
{
  return pvMappingModel->mapping();
}

const std::shared_ptr<TableMapping> CsvFileImportTableMappingDialog::mappingBase()
{
  return pvMappingModel->mapping();
}

TableMappingModel *CsvFileImportTableMappingDialog::mappingModelBase()
{
  return pvMappingModel;
}

}}} // namespace mdt{ namespace sql{ namespace copier{
