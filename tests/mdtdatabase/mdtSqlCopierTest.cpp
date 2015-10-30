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
#include "mdtSqlDatabaseSchema.h"
#include "mdtSqlTablePopulationSchema.h"
#include "mdtSqlSchemaTable.h"
#include "mdtSqlRecord.h"
#include "mdtSqlTransaction.h"

#include "mdtComboBoxItemDelegate.h"

#include "mdtSqlDatabaseCopierTableMapping.h"
#include "mdtSqlDatabaseCopierTableMappingModel.h"
#include "mdtSqlDatabaseCopierTableMappingDialog.h"
#include "mdtSqlDatabaseCopierThread.h"
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

/*
 * Populate Client_tbl with test data
 */
class clientTableTestDataSet
{
 public:

  clientTableTestDataSet(const QSqlDatabase & db)
   : pvDatabase(db)
  {
    Q_ASSERT(db.isValid());
  }
  ~clientTableTestDataSet()
  {
    clear();
  }
  bool populate();
  void clear();

 private:

  mdtSqlTablePopulationSchema pvPopulationSchema;
  QSqlDatabase pvDatabase;
};

bool clientTableTestDataSet::populate()
{
  QSqlQuery query(pvDatabase);
  QString sql;

  pvPopulationSchema.clear();
  pvPopulationSchema.setName("clientTableTestDataSet");
  pvPopulationSchema.setTableName("Client_tbl");
  pvPopulationSchema.addFieldName("Id_PK");
  pvPopulationSchema.addFieldName("Name");
  pvPopulationSchema.addFieldName("FieldA");
  pvPopulationSchema.addFieldName("FieldB");
  // Add data
  pvPopulationSchema.currentRowData() << 1 << "Name 1" << "FieldA 1" << "FieldB 1";
  pvPopulationSchema.commitCurrentRowData();
  pvPopulationSchema.currentRowData() << 2 << "Name 2" << "FieldA 2" << "FieldB 2";
  pvPopulationSchema.commitCurrentRowData();
  // Insert to Client_tbl
  for(int row = 0; row < pvPopulationSchema.rowDataCount(); ++row){
    sql = pvPopulationSchema.sqlForInsert(pvDatabase.driver());
    if(!query.prepare(sql)){
      qDebug() << "Prepare for insertion into Client_tbl failed, error: " << query.lastError();
      return false;
    }
    for(const auto & data : pvPopulationSchema.rowData(row)){
      query.addBindValue(data);
    }
    if(!query.exec()){
      qDebug() << "Insertion into Client_tbl failed, error: " << query.lastError();
      return false;
    }
  }

  return true;
}

void clientTableTestDataSet::clear()
{
  QSqlQuery query(pvDatabase);
  QString sql;
  int lastRow = pvPopulationSchema.rowDataCount() - 1;

  // Build SQL
  sql = "DELETE FROM Client_tbl WHERE Id_PK IN(";
  for(int row = 0; row < lastRow; ++row){
    sql += pvPopulationSchema.rowData(row).at(0).toString() + ",";
  }
  sql += pvPopulationSchema.rowData(lastRow).at(0).toString() + ")";
  // Remove data
  if(!query.exec(sql)){
    qDebug() << "Removing test data from Client_tbl failed, error: " << query.lastError();
  }
}

/*
 * Test implementation
 */

void mdtSqlCopierTest::initTestCase()
{
  createTestDatabase();
}

void mdtSqlCopierTest::cleanupTestCase()
{
  ///QFile::remove(pvDbFileInfo.filePath());
}

/*
 * Tests implemtations
 */


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

void mdtSqlCopierTest::sqlDatabaseCopierTableMappingTest()
{
  mdtSqlDatabaseCopierTableMapping mapping;
  ///mdtSqlCopierFieldMapping fm;

  /*
   * Initial state
   */
  QCOMPARE(mapping.fieldCount(), 0);
  QVERIFY(mapping.mappingState() == mdtSqlDatabaseCopierTableMapping::MappingNotSet);

  /*
   * Setup databases and tables
   */
//   QVERIFY(mapping.setSourceDatabase(pvDatabase));
//   QVERIFY(mapping.setDestinationDatabase(pvDatabase));
  QVERIFY(mapping.setSourceTable("Client_tbl", pvDatabase));
  QVERIFY(mapping.setDestinationTable("Client2_tbl", pvDatabase));
  QVERIFY(mapping.mappingState() == mdtSqlDatabaseCopierTableMapping::MappingNotSet);
  // Check attributes without any mapping set
  QCOMPARE(mapping.fieldCount(), 4);
  QCOMPARE(mapping.sourceFieldName(0), QString("Id_PK"));
  QCOMPARE(mapping.sourceFieldName(1), QString("Name"));
  QCOMPARE(mapping.sourceFieldName(2), QString("FieldA"));
  QCOMPARE(mapping.sourceFieldName(3), QString("FieldB"));
  QVERIFY(mapping.destinationFieldName(0).isNull());
  QVERIFY(mapping.destinationFieldName(1).isNull());
  QVERIFY(mapping.destinationFieldName(2).isNull());
  QVERIFY(mapping.destinationFieldName(3).isNull());
  /*
   * Set a field mapping:
   *  - Client_tbl.Id_PK -> Client2_tbl.Id_PK
   */
  mapping.setDestinationField(0, "Id_PK");
  // Check attributes
  QVERIFY(mapping.mappingState() == mdtSqlDatabaseCopierTableMapping::MappingPartial);
  QCOMPARE(mapping.fieldCount(), 4);
  QCOMPARE(mapping.sourceFieldName(0), QString("Id_PK"));
  QCOMPARE(mapping.sourceFieldName(1), QString("Name"));
  QCOMPARE(mapping.sourceFieldName(2), QString("FieldA"));
  QCOMPARE(mapping.sourceFieldName(3), QString("FieldB"));
  QCOMPARE(mapping.destinationFieldName(0), QString("Id_PK"));
  QVERIFY(mapping.destinationFieldName(1).isNull());
  QVERIFY(mapping.destinationFieldName(2).isNull());
  QVERIFY(mapping.destinationFieldName(3).isNull());
  /*
   * Set a field mapping:
   *  - Client_tbl.Name -> Client2_tbl.Name
   */
  mapping.setDestinationField(1, "Name");
  // Check attributes
  QVERIFY(mapping.mappingState() == mdtSqlDatabaseCopierTableMapping::MappingPartial);
  QCOMPARE(mapping.fieldCount(), 4);
  QCOMPARE(mapping.sourceFieldName(0), QString("Id_PK"));
  QCOMPARE(mapping.sourceFieldName(1), QString("Name"));
  QCOMPARE(mapping.sourceFieldName(2), QString("FieldA"));
  QCOMPARE(mapping.sourceFieldName(3), QString("FieldB"));
  QCOMPARE(mapping.destinationFieldName(0), QString("Id_PK"));
  QCOMPARE(mapping.destinationFieldName(1), QString("Name"));
  QVERIFY(mapping.destinationFieldName(2).isNull());
  QVERIFY(mapping.destinationFieldName(3).isNull());
  /*
   * Set a field mapping:
   *  - Client_tbl.FieldA -> Client2_tbl.FieldB
   */
  mapping.setDestinationField(2, "FieldB");
  // Check attributes
  QVERIFY(mapping.mappingState() == mdtSqlDatabaseCopierTableMapping::MappingPartial);
  QCOMPARE(mapping.fieldCount(), 4);
  QCOMPARE(mapping.sourceFieldName(0), QString("Id_PK"));
  QCOMPARE(mapping.sourceFieldName(1), QString("Name"));
  QCOMPARE(mapping.sourceFieldName(2), QString("FieldA"));
  QCOMPARE(mapping.sourceFieldName(3), QString("FieldB"));
  QCOMPARE(mapping.destinationFieldName(0), QString("Id_PK"));
  QCOMPARE(mapping.destinationFieldName(1), QString("Name"));
  QCOMPARE(mapping.destinationFieldName(2), QString("FieldB"));
  QVERIFY(mapping.destinationFieldName(3).isNull());
  /*
   * Set a field mapping:
   *  - Client_tbl.FieldB -> Client2_tbl.FieldA
   */
  mapping.setDestinationField(3, "FieldA");
  // Check attributes
  QVERIFY(mapping.mappingState() == mdtSqlDatabaseCopierTableMapping::MappingComplete);
  QCOMPARE(mapping.fieldCount(), 4);
  QCOMPARE(mapping.sourceFieldName(0), QString("Id_PK"));
  QCOMPARE(mapping.sourceFieldName(1), QString("Name"));
  QCOMPARE(mapping.sourceFieldName(2), QString("FieldA"));
  QCOMPARE(mapping.sourceFieldName(3), QString("FieldB"));
  QCOMPARE(mapping.destinationFieldName(0), QString("Id_PK"));
  QCOMPARE(mapping.destinationFieldName(1), QString("Name"));
  QCOMPARE(mapping.destinationFieldName(2), QString("FieldB"));
  QCOMPARE(mapping.destinationFieldName(3), QString("FieldA"));
  /*
   * Reset
   */
  mapping.resetFieldMapping();
  QCOMPARE(mapping.fieldCount(), 4);
  QCOMPARE(mapping.sourceFieldName(0), QString("Id_PK"));
  QCOMPARE(mapping.sourceFieldName(1), QString("Name"));
  QCOMPARE(mapping.sourceFieldName(2), QString("FieldA"));
  QCOMPARE(mapping.sourceFieldName(3), QString("FieldB"));
  QVERIFY(mapping.destinationFieldName(0).isNull());
  QVERIFY(mapping.destinationFieldName(1).isNull());
  QVERIFY(mapping.destinationFieldName(2).isNull());
  QVERIFY(mapping.destinationFieldName(3).isNull());
  /*
   * Check field mapping generation by field name
   */
  mapping.generateFieldMappingByName();
  QVERIFY(mapping.mappingState() == mdtSqlDatabaseCopierTableMapping::MappingComplete);
  QCOMPARE(mapping.fieldCount(), 4);
  QCOMPARE(mapping.sourceFieldName(0), QString("Id_PK"));
  QCOMPARE(mapping.sourceFieldName(1), QString("Name"));
  QCOMPARE(mapping.sourceFieldName(2), QString("FieldA"));
  QCOMPARE(mapping.sourceFieldName(3), QString("FieldB"));
  QCOMPARE(mapping.destinationFieldName(0), QString("Id_PK"));
  QCOMPARE(mapping.destinationFieldName(1), QString("Name"));
  QCOMPARE(mapping.destinationFieldName(2), QString("FieldA"));
  QCOMPARE(mapping.destinationFieldName(3), QString("FieldB"));
  /*
   * Clear
   */
  mapping.clearFieldMapping();
  QCOMPARE(mapping.fieldCount(), 0);
  QVERIFY(mapping.mappingState() == mdtSqlDatabaseCopierTableMapping::MappingNotSet);

}

void mdtSqlCopierTest::sqlDatabaseCopierTableMappingSqliteTest()
{
  mdtSqlDatabaseCopierTableMapping mapping;
  QSqlDatabase db = pvDatabase;
  QString expectedSql;

  QCOMPARE(db.driverName(), QString("QSQLITE"));
  /*
   * Setup databases and tables
   */
  QVERIFY(mapping.setSourceTable("Client_tbl", pvDatabase));
  QVERIFY(mapping.setDestinationTable("Client2_tbl", pvDatabase));
  /*
   * Add field mapping:
   * - Client_tbl.Id_PK -> Client2_tbl.Id_PK
   */
  mapping.setDestinationField(0, "Id_PK");
  // Check SQL for count in source table
  expectedSql = "SELECT COUNT(*) FROM \"Client_tbl\"";
  QCOMPARE(mapping.getSqlForSourceTableCount(db), expectedSql);
  // Check SQL select data in source table
  expectedSql = "SELECT \"Id_PK\" FROM \"Client_tbl\"";
  QCOMPARE(mapping.getSqlForSourceTableSelect(db), expectedSql);
  // Check SQL to insert into destination table
  expectedSql = "INSERT INTO \"Client2_tbl\" (\"Id_PK\") VALUES (?)";
  QCOMPARE(mapping.getSqlForDestinationTableInsert(db), expectedSql);
  /*
   * Add field mapping:
   * - Client_tbl.Name -> Client2_tbl.Name
   */
  mapping.setDestinationField(1, "Name");
  // Check SQL for count in source table
  expectedSql = "SELECT COUNT(*) FROM \"Client_tbl\"";
  QCOMPARE(mapping.getSqlForSourceTableCount(db), expectedSql);
  // Check SQL select data in source table
  expectedSql = "SELECT \"Id_PK\",\"Name\" FROM \"Client_tbl\"";
  QCOMPARE(mapping.getSqlForSourceTableSelect(db), expectedSql);
  // Check SQL to insert into destination table
  expectedSql = "INSERT INTO \"Client2_tbl\" (\"Id_PK\",\"Name\") VALUES (?,?)";
  QCOMPARE(mapping.getSqlForDestinationTableInsert(db), expectedSql);
  /*
   * Add field mapping:
   * - Client_tbl.FieldA -> Client2_tbl.FieldB
   */
  mapping.setDestinationField(2, "FieldB");
  // Check SQL select data in source table
  expectedSql = "SELECT \"Id_PK\",\"Name\",\"FieldA\" FROM \"Client_tbl\"";
  QCOMPARE(mapping.getSqlForSourceTableSelect(db), expectedSql);
  // Check SQL to insert into destination table
  expectedSql = "INSERT INTO \"Client2_tbl\" (\"Id_PK\",\"Name\",\"FieldB\") VALUES (?,?,?)";
  QCOMPARE(mapping.getSqlForDestinationTableInsert(db), expectedSql);
  /*
   * Add field mapping:
   * - Client_tbl.FieldB -> Client2_tbl.FieldA
   */
  mapping.setDestinationField(3, "FieldA");
  // Check SQL select data in source table
  expectedSql = "SELECT \"Id_PK\",\"Name\",\"FieldA\",\"FieldB\" FROM \"Client_tbl\"";
  QCOMPARE(mapping.getSqlForSourceTableSelect(db), expectedSql);
  // Check SQL to insert into destination table
  expectedSql = "INSERT INTO \"Client2_tbl\" (\"Id_PK\",\"Name\",\"FieldB\",\"FieldA\") VALUES (?,?,?,?)";
  QCOMPARE(mapping.getSqlForDestinationTableInsert(db), expectedSql);

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
//   QVERIFY(model.setSourceDatabase(pvDatabase));
//   QVERIFY(model.setDestinationDatabase(pvDatabase));
  QVERIFY(model.setSourceTable("Client_tbl", pvDatabase));
  QVERIFY(model.setDestinationTable("Client2_tbl", pvDatabase, delegate));
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

  QVERIFY(mapping.setSourceTable("Client_tbl", pvDatabase));
  QVERIFY(mapping.setDestinationTable("Client2_tbl", pvDatabase));
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
  QCOMPARE(mapping.tableMappingList().size(), 0);
  /*
   * Set source database
   */
  QVERIFY(mapping.setSourceDatabase(pvDatabase));
  // Check attributes without any mapping set
  QCOMPARE(mapping.tableMappingCount(), 2);
  QCOMPARE(mapping.tableMappingList().size(), 2);
  // Note: tables are sorted, and '_' is after '2' in ascii
  QCOMPARE(mapping.sourceTableName(0), QString("Client2_tbl"));
  QCOMPARE(mapping.sourceTableName(1), QString("Client_tbl"));
  /*
   * Edit table mapping:
   *  Table Client_tbl -> Client2_tbl
   *  Fields:
   *   Client_tbl.Id_PK -> Client2_tbl.Id_PK
   *   Client_tbl.Name -> Client2_tbl.Name
   *   Client_tbl.FieldA -> Client2_tbl.FieldB
   *   Client_tbl.FieldB -> Client2_tbl.FieldA
   */
  auto tm = mapping.tableMapping(1);
  QVERIFY(tm.setDestinationTable("Client2_tbl", pvDatabase));
  tm.setDestinationField(0, "Id_PK");
  tm.setDestinationField(1, "Name");
  tm.setDestinationField(2, "FieldB");
  tm.setDestinationField(3, "FieldA");
  QVERIFY(tm.mappingState() == mdtSqlDatabaseCopierTableMapping::MappingComplete);
  mapping.setTableMapping(1, tm);
  QCOMPARE(mapping.tableMappingCount(), 2);
  QCOMPARE(mapping.tableMappingList().size(), 2);

}

void mdtSqlCopierTest::sqlDatabaseCopierMappingModelTest()
{
  QTableView tableView;
  QTreeView treeView;
  mdtSqlDatabaseCopierMappingModel model;
  QModelIndex index;
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
  QVERIFY(model.setSourceDatabase(pvDatabase));
  QVERIFY(model.setDestinationDatabase(pvDatabase));
  QVERIFY(model.generateTableMappingByName());
  tableView.resizeColumnsToContents();
  
  /*
   * Check updating table copy progress and status
   */
  QCOMPARE(model.rowCount(), 2);
  // Progress of row 0
  model.setTableCopyProgress(0, 15);
  index = model.index(0, 3);
  QVERIFY(index.isValid());
  QCOMPARE(model.data(index), QVariant(15));

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


void mdtSqlCopierTest::sqlDatabaseCopierThreadTest()
{
  mdtSqlDatabaseCopierThread thread;
  mdtSqlDatabaseCopierMapping mapping;
  QSqlDatabase db = pvDatabase;
  clientTableTestDataSet dataset(db);
  QSqlQuery query(db);

  QVERIFY(db.isValid());
  /*
   * Insert some test data into Client_tbl
   */
  QVERIFY(dataset.populate());
  /*
   * Check that we have no data in Client2_tbl
   */
  QVERIFY(query.exec("SELECT * FROM Client2_tbl"));
  QVERIFY(!query.next());
  /*
   * Setup database mapping
   */
  QVERIFY(mapping.setSourceDatabase(db));
  QVERIFY(mapping.setDestinationDatabase(db));
  QVERIFY(mapping.tableMappingCount() > 0);
  /*
   * Edit table mapping:
   *  Table Client_tbl -> Client2_tbl
   *  Fields:
   *   Client_tbl.Id_PK -> Client2_tbl.Id_PK
   *   Client_tbl.Name -> Client2_tbl.Name
   *   Client_tbl.FieldA -> Client2_tbl.FieldB
   *   Client_tbl.FieldB -> Client2_tbl.FieldA
   */
  auto tm = mapping.tableMapping(1);
  QVERIFY(tm.setDestinationTable("Client2_tbl", pvDatabase));
  tm.setDestinationField(0, "Id_PK");
  tm.setDestinationField(1, "Name");
  tm.setDestinationField(2, "FieldB");
  tm.setDestinationField(3, "FieldA");
  QVERIFY(tm.mappingState() == mdtSqlDatabaseCopierTableMapping::MappingComplete);
  mapping.setTableMapping(1, tm);
  QCOMPARE(mapping.tableMappingCount(), 2);
  QCOMPARE(mapping.tableMappingList().size(), 2);
  /*
   * Run copy
   */
  thread.copyData(mapping);
  thread.wait();
  /*
   * Check that copy was done
   */
  QVERIFY(query.exec("SELECT * FROM Client2_tbl"));
  // Row for Id_PK = 1
  QVERIFY(query.next());
  QCOMPARE(query.value(0), QVariant(1));
  QCOMPARE(query.value(1), QVariant("Name 1"));
  QCOMPARE(query.value(2), QVariant("FieldB 1"));
  QCOMPARE(query.value(3), QVariant("FieldA 1"));
  // Row for Id_PK = 2
  QVERIFY(query.next());
  QCOMPARE(query.value(0), QVariant(2));
  QCOMPARE(query.value(1), QVariant("Name 2"));
  QCOMPARE(query.value(2), QVariant("FieldB 2"));
  QCOMPARE(query.value(3), QVariant("FieldA 2"));
}



/*
 * Test data base manipulation methods
 */

void mdtSqlCopierTest::createTestDatabase()
{
  mdtSqlSchemaTable table;
  mdtSqlDatabaseSchema s;
  ///mdtSqlForeignKey fk;
  mdtSqlField field;

  /*
   * Init and open database
   */
  QVERIFY(pvTempFile.open());
  pvDatabase = QSqlDatabase::addDatabase("QSQLITE");
  pvDatabase.setDatabaseName(pvTempFile.fileName());
  QVERIFY(pvDatabase.open());

  /*
   * Create tables
   */
  // Create Client_tbl
  table.clear();
  table.setTableName("Client_tbl", "UTF8");
  // Id_PK
  field.clear();
  field.setName("Id_PK");
  field.setType(mdtSqlFieldType::Integer);
  field.setAutoValue(true);
  table.addField(field, true);
  // Name
  field.clear();
  field.setName("Name");
  field.setType(mdtSqlFieldType::Varchar);
  field.setLength(100);
  table.addField(field, false);
  // FieldA
  field.clear();
  field.setName("FieldA");
  field.setType(mdtSqlFieldType::Varchar);
  field.setLength(50);
  table.addField(field, false);
  // FieldB
  field.clear();
  field.setName("FieldB");
  field.setType(mdtSqlFieldType::Varchar);
  field.setLength(50);
  table.addField(field, false);
  s.addTable(table);
  // Create Client2_tbl
  table.clear();
  table.setTableName("Client2_tbl", "UTF8");
  // Id_PK
  field.clear();
  field.setName("Id_PK");
  field.setType(mdtSqlFieldType::Integer);
  field.setAutoValue(true);
  table.addField(field, true);
  // Name
  field.clear();
  field.setName("Name");
  field.setType(mdtSqlFieldType::Varchar);
  field.setLength(100);
  table.addField(field, false);
  // FieldA
  field.clear();
  field.setName("FieldA");
  field.setType(mdtSqlFieldType::Varchar);
  field.setLength(50);
  table.addField(field, false);
  // FieldB
  field.clear();
  field.setName("FieldB");
  field.setType(mdtSqlFieldType::Varchar);
  field.setLength(50);
  table.addField(field, false);
  s.addTable(table);
  // Create schema
  QVERIFY(s.createSchema(pvDatabase));
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
