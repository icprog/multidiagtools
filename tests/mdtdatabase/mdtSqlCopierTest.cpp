/****************************************************************************
 **
 ** Copyright (C) 2011-2015 Philippe Steinmann.
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
#include "mdtSqlCopierTest.h"
#include "mdtSqlFieldSetupData.h"
#include "mdtApplication.h"
#include "mdtSqlDatabaseManager.h"
#include "mdtSqlSchemaTable.h"
#include "mdtSqlRecord.h"
#include "mdtSqlTransaction.h"
#include "mdtSqlCopierFieldMappingDialog.h"
#include "mdtSqlCopierTableMappingWidgetItem.h"
#include "mdtSqlCopierTableMappingWidget.h"
#include "mdtAbstractSqlCopierCodec.h"
#include "mdtSqlCopierCodec.h"
#include "mdtSqlCopierCodecSettings.h"
#include "mdtSqlCopierCodecSettingsWidget.h"

#include "mdtComboBoxItemDelegate.h"
#include "mdtSqlDatabaseCopierTableMapping.h"
#include "mdtSqlDatabaseCopierTableMappingModel.h"
#include "mdtSqlDatabaseCopierTableMappingDialog.h"
#include "mdtSqlDatabaseCopierMapping.h"
#include "mdtSqlDatabaseCopierMappingModel.h"
#include "mdtSqlDatabaseCopierDialog.h"

#include <QTemporaryFile>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QVariant>
#include <QTableView>
#include <QComboBox>
#include <QTreeView>
#include <memory>

#include <QDebug>

void mdtSqlCopierTest::initTestCase()
{
  createTestDatabase();
}

void mdtSqlCopierTest::cleanupTestCase()
{
  QFile::remove(pvDbFileInfo.filePath());
}

/*
 * Tests implemtations
 */

void mdtSqlCopierTest::codecSettingsTest()
{
  mdtSqlCopierCodecSettings cs;

  // Initial state
  QVERIFY(cs.isNull());
  QVERIFY(cs.type() == mdtSqlCopierCodecSettings::UnknownCodec);
  // SQLite database table codec settings
  cs.setCodecType(mdtSqlCopierCodecSettings::SqliteCodec);
  QVERIFY(cs.type() == mdtSqlCopierCodecSettings::SqliteCodec);
  QVERIFY(!cs.isNull());
  cs.setConnectionName("cnn1");
  QCOMPARE(cs.connectionName(), QString("cnn1"));

//   /*
//    * Base codec
//    */
//   mdtSqlCopierCodecSettings baseCs;
//   QVERIFY(baseCs.type() == mdtSqlCopierCodecSettings::UnknownCodec);
//   QVERIFY(baseCs.isNull());
//   baseCs.setConnectionName("FakeCn");
//   QVERIFY(baseCs.connectionName().isEmpty());
//   QVERIFY(baseCs.isNull());
//   /*
//    * SQLite database table codec settings
//    */
//   cs.reset(new mdtSqlCopierSqliteDatabaseTableCodecSettings);
//   QVERIFY(cs->type() == mdtSqlCopierCodecSettings::SqliteCodec);
//   QVERIFY(cs->isNull());
//   cs->setConnectionName("cnn1");
//   QCOMPARE(cs->connectionName(), QString("cnn1"));
}

void mdtSqlCopierTest::codecSettingsWidgetTest()
{
  mdtSqlCopierCodecSettingsWidget w;

  w.setCodecType(mdtSqlCopierCodecSettings::SqliteCodec);
  QVERIFY(w.codecSettings().type() == mdtSqlCopierCodecSettings::SqliteCodec);
  /*
   * Play
   */
  w.show();
  while(w.isVisible()){
    QTest::qWait(500);
  }
}

void mdtSqlCopierTest::codecTest()
{
  mdtSqlCopierCodec codec;
  mdtSqlCopierCodecSettings cs;

  
  // Sandbox
//   QString filePath = pvDatabaseManager.database().databaseName();
//   QSqlDatabase db1 = QSqlDatabase::addDatabase("QSQLITE", "cnn1");
//   db1.setDatabaseName(filePath);
//   db1.open();
//   qDebug() << "db1 (" << db1.connectionName() << ", " << db1.databaseName() << ") is open: " << db1.isOpen();
//   
//   QSqlDatabase db2 = QSqlDatabase::addDatabase("QSQLITE", "cnn2");
//   ///QSqlDatabase db2 = QSqlDatabase::database("cnn1", false);
//   db2.setDatabaseName(filePath);
//   db2.open();
//   qDebug() << "db2 (" << db2.connectionName() << ", " << db2.databaseName() << ") is open: " << db2.isOpen();
//   
//   qDebug() << "Closing db1 ...";
//   db1.close();
//   qDebug() << "db1 (" << db1.connectionName() << ", " << db1.databaseName() << ") is open: " << db1.isOpen();
//   qDebug() << "db2 (" << db2.connectionName() << ", " << db2.databaseName() << ") is open: " << db2.isOpen();
//   
//   qDebug() << "THD ID: " << QThread::currentThreadId();
//   
//   /// \todo provisoire !!
//   return;
  
  
  // Initial state
  QVERIFY(codec.settings().type() == mdtSqlCopierCodecSettings::UnknownCodec);
  
  /*
   * Check changing settings
   */
  // Unknown codec
  cs.clear();
  cs.setCodecType(mdtSqlCopierCodecSettings::UnknownCodec);
  codec.setSettings(cs);
  QVERIFY(codec.settings().type() == mdtSqlCopierCodecSettings::UnknownCodec);
  QVERIFY(!codec.openTarget());
  // Set SQLite codec
  cs.clear();
  cs.setCodecType(mdtSqlCopierCodecSettings::SqliteCodec);
  codec.setSettings(cs);
  QVERIFY(codec.settings().type() == mdtSqlCopierCodecSettings::SqliteCodec);
  // Change settings but keep SQLite codec
  cs.setConnectionName("cnn1");
  codec.setSettings(cs);
  QVERIFY(codec.settings().type() == mdtSqlCopierCodecSettings::SqliteCodec);
  QCOMPARE(codec.settings().connectionName(), QString("cnn1"));
  /*
   * Check opening a SQLite table
   */
 /// QVERIFY(pvDatabaseManager.database().isOpen());
  // Setup
  cs.clear();
  cs.setCodecType(mdtSqlCopierCodecSettings::SqliteCodec);
 /// cs.setConnectionName(pvDatabaseManager.database().connectionName());
  cs.setFilePath(pvDatabaseManager.database().databaseName());
  cs.setTableName("Client_tbl");
  codec.setSettings(cs);
  // Open target
  QVERIFY(codec.openTarget());
  // Check found attributes
  QCOMPARE(codec.fieldNameList().size(), 2);
  QCOMPARE(codec.fieldNameList().at(0), QString("Id_PK"));
  QCOMPARE(codec.fieldNameList().at(1), QString("Name"));
  
  // Close
  codec.close();
  QCOMPARE(codec.fieldNameList().size(), 0);
}

void mdtSqlCopierTest::codecSqliteBenchmark()
{
  mdtSqlCopierCodec codec;
  mdtSqlCopierCodecSettings cs;

  cs.setCodecType(mdtSqlCopierCodecSettings::SqliteCodec);
  cs.setFilePath(pvDatabaseManager.database().databaseName());
  cs.setTableName("Client_tbl");
  codec.setSettings(cs);
  QBENCHMARK{
    QVERIFY(codec.openTarget());
    QCOMPARE(codec.fieldNameList().size(), 2);
    codec.close();
    QCOMPARE(codec.fieldNameList().size(), 0);
  }
}


void mdtSqlCopierTest::sqlFieldSetupDataTest()
{
  mdtSqlFieldSetupData data;

  // Initial state
  QVERIFY(data.isNull());
//   QVERIFY(data.editionMode == mdtSqlFieldSetupEditionMode_t::Selection);
  // Set
//   data.tableName = "Client_tbl";
//   QVERIFY(data.isNull());
  data.field.setName("Id_PK");
  QVERIFY(data.isNull());
  data.field.setType(QVariant::Int);
  QVERIFY(!data.isNull());
  data.isPartOfPrimaryKey = true;
//   data.editionMode = mdtSqlFieldSetupEditionMode_t::Creation;
  // Clear
  data.clear();
//   QVERIFY(data.tableName.isEmpty());
  QVERIFY(data.isPartOfPrimaryKey == false);
  QVERIFY(data.field.name().isEmpty());
  QVERIFY(data.field.type() == QVariant::Invalid);
//   QVERIFY(data.editionMode == mdtSqlFieldSetupEditionMode_t::Selection);
  QVERIFY(data.isNull());
}

void mdtSqlCopierTest::fieldMappingDataTest()
{
  mdtSqlCopierFieldMapping data;

  // Initial state
  QCOMPARE(data.sourceFieldIndex, -1);
  QCOMPARE(data.destinationFieldIndex, -1);
  QVERIFY(data.isNull());
  // Set
  data.sourceFieldIndex = 0;
  QVERIFY(data.isNull());
  data.destinationFieldIndex = 0;
  QVERIFY(!data.isNull());
  // Clear
  data.clear();
  QCOMPARE(data.sourceFieldIndex, -1);
  QCOMPARE(data.destinationFieldIndex, -1);
  QVERIFY(data.isNull());
}

void mdtSqlCopierTest::fieldMappingDialogTest()
{
  mdtSqlCopierFieldMappingDialog dialog;
  mdtSqlCopierFieldMapping mapping;
  mdtSqlCopierCodecSettings cs;

  cs.setCodecType(mdtSqlCopierCodecSettings::SqliteCodec);
  cs.setConnectionName(pvDatabaseManager.database().connectionName());
  cs.setTableName("Client_tbl");
  QVERIFY(dialog.setSource(cs));
//   QVERIFY(dialog.setSourceTable("Client_tbl", pvDatabaseManager.database()));
//   QVERIFY(dialog.setDestinationTable("Client2_tbl", pvDatabaseManager.database()));
  cs.setTableName("Client2_tbl");
  QVERIFY(dialog.setDestination(cs));
  mapping.sourceFieldIndex = 1;
  mapping.destinationFieldIndex = 1;
  dialog.setMapping(mapping);
  dialog.exec();
  QCOMPARE(dialog.mapping().sourceFieldIndex, 1);
  QCOMPARE(dialog.mapping().destinationFieldIndex, 1);
}

// void mdtSqlCopierTest::tableMappingModelTest()
// {
//   mdtSqlTableMappingModel tmm;
//   QTableView tableView;
//   QTreeView treeView;
// 
//   tableView.setModel(&tmm);
//   treeView.setModel(&tmm);
//   
//   tableView.resize(300, 200);
//   tableView.show();
//   treeView.show();
// 
//   /*
//    * Play
//    */
//   while(tableView.isVisible()){
//     QTest::qWait(500);
//   }
// }

void mdtSqlCopierTest::tableMappingWidgetItemTest()
{
  mdtSqlCopierTableMappingWidget tmw;
  mdtSqlCopierTableMappingWidgetItem item(nullptr, &tmw);

  item.show();

  /*
   * Play
   */
  while(item.isVisible()){
    QTest::qWait(500);
  }
}

void mdtSqlCopierTest::tableMappingWidgetTest()
{
  mdtSqlCopierTableMappingWidget tmw;

  tmw.show();

  /*
   * Play
   */
  while(tmw.isVisible()){
    QTest::qWait(500);
  }
}

void mdtSqlCopierTest::sqlDatabaseCopierTableMappingTest()
{
  mdtSqlDatabaseCopierTableMapping mapping;
  mdtSqlCopierFieldMapping fm;

  /*
   * Initial state
   */
  QCOMPARE(mapping.fieldCount(), 0);
  QVERIFY(mapping.mappingState() == mdtSqlDatabaseCopierTableMapping::MappingNotSet);

  /*
   * Setup databases and tables
   */
//   QVERIFY(mapping.setSourceDatabase(pvDatabaseManager.database()));
//   QVERIFY(mapping.setDestinationDatabase(pvDatabaseManager.database()));
  QVERIFY(mapping.setSourceTable("Client_tbl", pvDatabaseManager.database()));
  QVERIFY(mapping.setDestinationTable("Client2_tbl", pvDatabaseManager.database()));
  QVERIFY(mapping.mappingState() == mdtSqlDatabaseCopierTableMapping::MappingNotSet);
  // Check attributes without any mapping set
  QCOMPARE(mapping.fieldCount(), 2);
  QCOMPARE(mapping.sourceFieldName(0), QString("Id_PK"));
  QCOMPARE(mapping.sourceFieldName(1), QString("Name"));
  QVERIFY(mapping.destinationFieldName(0).isNull());
  QVERIFY(mapping.destinationFieldName(1).isNull());
  /*
   * Set a field mapping:
   *  - Client_tbl.Id_PK -> Client2_tbl.Name
   */
  mapping.setDestinationField(0, "Name");
  // Check attributes
  QVERIFY(mapping.mappingState() == mdtSqlDatabaseCopierTableMapping::MappingPartial);
  QCOMPARE(mapping.fieldCount(), 2);
  QCOMPARE(mapping.sourceFieldName(0), QString("Id_PK"));
  QCOMPARE(mapping.sourceFieldName(1), QString("Name"));
  QCOMPARE(mapping.destinationFieldName(0), QString("Name"));
  QVERIFY(mapping.destinationFieldName(1).isNull());
  /*
   * Add a field mapping:
   *  - Client_tbl.Name -> Client2_tbl.Id_PK
   */
  mapping.setDestinationField(1, "Id_PK");
  // Check attributes
  QVERIFY(mapping.mappingState() == mdtSqlDatabaseCopierTableMapping::MappingComplete);
  QCOMPARE(mapping.fieldCount(), 2);
  QCOMPARE(mapping.sourceFieldName(0), QString("Id_PK"));
  QCOMPARE(mapping.sourceFieldName(1), QString("Name"));
  QCOMPARE(mapping.destinationFieldName(0), QString("Name"));
  QCOMPARE(mapping.destinationFieldName(1), QString("Id_PK"));
  /*
   * Check field mapping generation by field name
   */
  mapping.generateFieldMappingByName();
  QVERIFY(mapping.mappingState() == mdtSqlDatabaseCopierTableMapping::MappingComplete);
  QCOMPARE(mapping.fieldCount(), 2);
  QCOMPARE(mapping.sourceFieldName(0), QString("Id_PK"));
  QCOMPARE(mapping.sourceFieldName(1), QString("Name"));
  QCOMPARE(mapping.destinationFieldName(0), QString("Id_PK"));
  QCOMPARE(mapping.destinationFieldName(1), QString("Name"));
  /*
   * Clear
   */
  mapping.clearFieldMapping();
  QCOMPARE(mapping.fieldCount(), 0);
  QVERIFY(mapping.mappingState() == mdtSqlDatabaseCopierTableMapping::MappingNotSet);

}

void mdtSqlCopierTest::sqlDatabaseCopierTableMappingModelTest()
{
  QTableView tableView;
  QTreeView treeView;
  mdtSqlDatabaseCopierTableMappingModel model;
  mdtSqlDatabaseCopierTableMapping mapping;
  mdtComboBoxItemDelegate *delegate = new mdtComboBoxItemDelegate(&tableView);

  /*
   * Setup views
   */
  // Setup table view
  tableView.setModel(&model);
  tableView.setItemDelegateForColumn(2, delegate);
  tableView.resize(600, 150);
  tableView.show();
  // Setup tree view
  treeView.setModel(&model);
  treeView.show();

  /*
   * Setup model
   */
//   QVERIFY(model.setSourceDatabase(pvDatabaseManager.database()));
//   QVERIFY(model.setDestinationDatabase(pvDatabaseManager.database()));
  QVERIFY(model.setSourceTable("Client_tbl", pvDatabaseManager.database()));
  QVERIFY(model.setDestinationTable("Client2_tbl", pvDatabaseManager.database(), delegate));
  model.generateFieldMappingByName();
  tableView.resizeColumnsToContents();
  

  /*
   * Play
   */
  while(tableView.isVisible()){
    QTest::qWait(500);
  }
}

void mdtSqlCopierTest::sqlDatabaseCopierTableMappingDialogTest()
{
  mdtSqlDatabaseCopierTableMappingDialog dialog;
  mdtSqlDatabaseCopierTableMapping mapping;

  QVERIFY(mapping.setSourceTable("Client_tbl", pvDatabaseManager.database()));
  QVERIFY(mapping.setDestinationTable("Client2_tbl", pvDatabaseManager.database()));
  mapping.generateFieldMappingByName();

  dialog.setMapping(mapping);
  dialog.exec();
}

void mdtSqlCopierTest::sqlDatabaseCopierMappingTest()
{
  mdtSqlDatabaseCopierMapping mapping;

  /*
   * Initial state
   */
  QCOMPARE(mapping.tableMappingCount(), 0);
  /*
   * Set source database
   */
  QVERIFY(mapping.setSourceDatabase(pvDatabaseManager.database()));
  // Check attributes without any mapping set
  QCOMPARE(mapping.tableMappingCount(), 2);
  QCOMPARE(mapping.sourceTableName(0), QString("Client_tbl"));
  QCOMPARE(mapping.sourceTableName(1), QString("Client2_tbl"));
  
}

void mdtSqlCopierTest::sqlDatabaseCopierMappingModelTest()
{
  QTableView tableView;
  QTreeView treeView;
  mdtSqlDatabaseCopierMappingModel model;
  mdtSqlDatabaseCopierMapping mapping;
  mdtComboBoxItemDelegate *delegate = new mdtComboBoxItemDelegate(&tableView);

  /*
   * Setup views
   */
  // Setup table view
  tableView.setModel(&model);
  tableView.setItemDelegateForColumn(2, delegate);
  tableView.resize(600, 150);
  tableView.show();
  // Setup tree view
  treeView.setModel(&model);
  treeView.show();

  /*
   * Setup model
   */
  QVERIFY(model.setSourceDatabase(pvDatabaseManager.database()));
  QVERIFY(model.setDestinationDatabase(pvDatabaseManager.database()));
  QVERIFY(model.generateTableMappingByName());
  tableView.resizeColumnsToContents();
  

  /*
   * Play
   */
  while(tableView.isVisible()){
    QTest::qWait(500);
  }
}

void mdtSqlCopierTest::sqlDatabaseCopierDialogTest()
{
  mdtSqlDatabaseCopierDialog dialog;

  dialog.exec();
}


/*
 * Test data base manipulation methods
 */

void mdtSqlCopierTest::createTestDatabase()
{
  QTemporaryFile dbFile;
  mdtSqlSchemaTable table;
  QSqlField field;

  /*
   * Create and open database
   */
  QVERIFY(dbFile.open());
  dbFile.close();
  pvDbFileInfo.setFile(dbFile.fileName() + ".db");
  QVERIFY(pvDatabaseManager.createDatabaseSqlite(pvDbFileInfo, mdtSqlDatabaseManager::OverwriteExisting));
  QVERIFY(pvDatabaseManager.database().isOpen());
  /*
   * Create tables
   */
  // Create Client_tbl
  table.clear();
  table.setTableName("Client_tbl", "UTF8");
  // Id_PK
  field = QSqlField();
  field.setName("Id_PK");
  field.setType(QVariant::Int);
  field.setAutoValue(true);
  table.addField(field, true);
  // Name
  field = QSqlField();
  field.setName("Name");
  field.setType(QVariant::String);
  field.setLength(100);
  table.addField(field, false);
  QVERIFY(pvDatabaseManager.createTable(table, mdtSqlDatabaseManager::OverwriteExisting));
  // Create Client2_tbl
  table.clear();
  table.setTableName("Client2_tbl", "UTF8");
  // Id_PK
  field = QSqlField();
  field.setName("Id_PK");
  field.setType(QVariant::Int);
  field.setAutoValue(true);
  table.addField(field, true);
  // Name
  field = QSqlField();
  field.setName("Name");
  field.setType(QVariant::String);
  field.setLength(100);
  table.addField(field, false);
  QVERIFY(pvDatabaseManager.createTable(table, mdtSqlDatabaseManager::OverwriteExisting));

}

void mdtSqlCopierTest::populateTestDatabase()
{

}

void mdtSqlCopierTest::clearTestDatabaseData()
{

}

/*
 * Main
 */
int main(int argc, char **argv)
{
  mdtApplication app(argc, argv);
  mdtSqlCopierTest test;

  if(!app.init()){
    return 1;
  }

  return QTest::qExec(&test, argc, argv);
}
