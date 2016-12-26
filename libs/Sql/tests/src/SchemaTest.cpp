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
#include "SchemaTest.h"
#include "Mdt/Application.h"
#include "Mdt/Sql/Schema/FieldTypeList.h"
#include "Mdt/Sql/Schema/FieldTypeListModel.h"
#include "Mdt/Sql/Schema/FieldTypeName.h"
#include "Mdt/Sql/Schema/Driver.h"
#include "Mdt/Sql/Schema/Charset.h"
#include "Mdt/Sql/Schema/Field.h"
#include "Mdt/Sql/Schema/FieldList.h"
#include "Mdt/Sql/Schema/AutoIncrementPrimaryKey.h"
#include "Mdt/Sql/Schema/PrimaryKey.h"
#include "Mdt/Sql/Schema/PrimaryKeyContainer.h"
#include "Mdt/Sql/Schema/Index.h"
#include "Mdt/Sql/Schema/IndexList.h"
#include "Mdt/Sql/Schema/IndexListModel.h"
// #include "Mdt/Sql/Schema/ForeignKey.h"
// #include "Mdt/Sql/Schema/ForeignKeyList.h"
#include "Mdt/Sql/Schema/Table.h"
#include "Mdt/Sql/Schema/TableModel.h"
#include "Mdt/Sql/Schema/TableList.h"
// #include "Mdt/Sql/Schema/TablePopulation.h"
#include "Mdt/Sql/Schema/Schema.h"
#include "Schema/Client.h"
#include "Schema/Address.h"
#include "Schema/ClientAddressView.h"
#include "Schema/ClientPopulation.h"
#include <QSqlDatabase>
#include <QComboBox>
#include <QTableView>
#include <QTreeView>

namespace Sql = Mdt::Sql;

void SchemaTest::initTestCase()
{
  QSqlDatabase::addDatabase("QSQLITE", "SQLITE_1");
}

void SchemaTest::cleanupTestCase()
{
  QSqlDatabase::removeDatabase("SQLITE_1");
}

/*
 * Tests
 */

void SchemaTest::charsetTest()
{
  using Mdt::Sql::Schema::Charset;

  /*
   * Initial state
   */
  Charset cs;
  QVERIFY(cs.isNull());
  /*
   * Set/get
   */
  cs.setCharsetName("utf8");
  QVERIFY(!cs.isNull());
  QCOMPARE(cs.charsetName(), QString("utf8"));
  /*
   * Clear
   */
  cs.clear();
  QVERIFY(cs.charsetName().isEmpty());
  QVERIFY(cs.isNull());
}

void SchemaTest::localeTest()
{
  using Mdt::Sql::Schema::Locale;

  Locale locale;

  /*
   * Initial state
   */
  QVERIFY(locale.country() == QLocale::AnyCountry);
  QVERIFY(locale.language() == QLocale::AnyLanguage);
  QVERIFY(locale.isNull());
  /*
   * Set/get
   */
  locale.setCountry(QLocale::Switzerland);
  QVERIFY(!locale.isNull());
  locale.setLanguage(QLocale::French);
  QVERIFY(locale.country() == QLocale::Switzerland);
  QVERIFY(locale.language() == QLocale::French);
  QVERIFY(!locale.isNull());
  /*
   * Clear
   */
  locale.clear();
  QVERIFY(locale.country() == QLocale::AnyCountry);
  QVERIFY(locale.language() == QLocale::AnyLanguage);
  QVERIFY(locale.isNull());
}

void SchemaTest::collationTest()
{
  using Mdt::Sql::Schema::Collation;
  using Mdt::Sql::Schema::CaseSensitivity;

  Collation collation;

  /*
   * Initial state
   */
  QVERIFY(collation.caseSensitivity() == CaseSensitivity::NotDefined);
  QVERIFY(collation.locale().isNull());
  QVERIFY(collation.isNull());
  /*
   * Simple set/get
   */
  collation.setCaseSensitive(false);
  collation.setCountry(QLocale::Switzerland);
  collation.setLanguage(QLocale::French);
//   collation.setCharset("utf8");
  QVERIFY(collation.caseSensitivity() == CaseSensitivity::CaseInsensitive);
  QVERIFY(!collation.isCaseSensitive());
  QVERIFY(collation.locale().country() == QLocale::Switzerland);
  QVERIFY(collation.locale().language() == QLocale::French);
//   QCOMPARE(collation.charset(), QString("utf8"));
  QVERIFY(!collation.isNull());
  /*
   * Clear
   */
  collation.clear();
  QVERIFY(collation.caseSensitivity() == CaseSensitivity::NotDefined);
  QVERIFY(collation.locale().isNull());
//   QVERIFY(collation.charset().isEmpty());
  QVERIFY(collation.isNull());
  /*
   * Check null flag
   */
  collation.setCaseSensitive(true);
  QVERIFY(!collation.isNull());
  collation.clear();
  QVERIFY(collation.isNull());
  collation.setCountry(QLocale::Switzerland);
  collation.setLanguage(QLocale::French);
  QVERIFY(!collation.isNull());
  collation.clear();
  QVERIFY(collation.isNull());
}

void SchemaTest::indexTest()
{
  using Mdt::Sql::Schema::Index;
  using Mdt::Sql::Schema::Field;
  using Mdt::Sql::Schema::FieldType;
  using Mdt::Sql::Schema::Table;

  /*
   * Setup a field
   */
  Field Id_A;
  Id_A.setType(FieldType::Integer);
  Id_A.setName("Id_A");
  /*
   * Setup a table
   */
  Table Client_tbl;
  Client_tbl.setTableName("Client_tbl");
  Client_tbl.addField(Id_A);
  /*
   * Initial state
   */
  Index index;
  QVERIFY(!index.isUnique());
  QVERIFY(index.isNull());
  /*
   * Simple set/get
   */
  index.setTable(Client_tbl);
  QVERIFY(index.isNull());
  index.addField(Id_A);
  QVERIFY(index.isNull());
  index.generateName();
  QVERIFY(!index.isNull());
  index.setUnique(true);
  // Check
  QCOMPARE(index.name(), QString("Client_tbl_Id_A_index"));
  QCOMPARE(index.tableName(), QString("Client_tbl"));
  QCOMPARE(index.fieldCount(), 1);
  QCOMPARE(index.fieldName(0), QString("Id_A"));
  QCOMPARE(index.fieldNameList().size(), 1);
  QCOMPARE(index.fieldNameList().at(0), QString("Id_A"));
  QVERIFY(index.containsFieldName("Id_A"));
  QVERIFY(index.containsFieldName("id_a"));
  QVERIFY(!index.containsFieldName("Id_B"));
  QVERIFY(index.isUnique());
  /*
   * Clear
   */
  index.clear();
  QVERIFY(index.name().isEmpty());
  QVERIFY(index.tableName().isEmpty());
  QCOMPARE(index.fieldCount(), 0);
  QVERIFY(!index.isUnique());
  QVERIFY(index.isNull());
  /*
   * Check setting table name from user defined table
   */
  index.setTable(Schema::Address());
  QCOMPARE(index.tableName(), QString("Address_tbl"));
  /*
   * Clear
   */
  index.clear();
  QVERIFY(index.name().isEmpty());
  QVERIFY(index.tableName().isEmpty());
  QCOMPARE(index.fieldCount(), 0);
  QVERIFY(!index.isUnique());
  QVERIFY(index.isNull());
}

void SchemaTest::indexListTest()
{
  using Mdt::Sql::Schema::Index;
  using Mdt::Sql::Schema::IndexList;
  using Mdt::Sql::FieldName;
  using Mdt::Sql::TableName;

  /*
   * Setup index
   */
  Index index;
  index.setTableName("A_tbl");
  index.setUnique(true);
  index.addFieldName("Name");
  index.generateName();
  QVERIFY(!index.isNull());
  /*
   * Initial state
   */
  IndexList list;
  QCOMPARE(list.size(), 0);
  /*
   * Add 1 element
   */
  list.append(index);
  QCOMPARE(list.size(), 1);
  QCOMPARE(list.at(0).tableName(), QString("A_tbl"));
  QVERIFY(list.at(0).isUnique());
  for(const auto & idx : list){
    QCOMPARE(idx.tableName(), QString("A_tbl"));
  }
  /*
   * Check updating table name
   */
  list.updateTableName("B_tbl");
  QCOMPARE(list.size(), 1);
  QCOMPARE(list.at(0).tableName(), QString("B_tbl"));
  QVERIFY(list.at(0).isUnique());
  /*
   * Check find index
   */
  QVERIFY(list.findIndex(TableName("None_tbl"), FieldName("None")).isNull());
  QVERIFY(list.findIndex(TableName("B_tbl"), FieldName("None")).isNull());
  QVERIFY(list.findIndex(TableName("None_tbl"), FieldName("Name")).isNull());
  QVERIFY(!list.findIndex(TableName("B_tbl"), FieldName("Name")).isNull());
  QVERIFY(!list.findIndex(TableName("b_tbl"), FieldName("name")).isNull());
  QCOMPARE(list.findIndex(TableName("B_tbl"), FieldName("Name")).tableName(), QString("B_tbl"));
  QCOMPARE(list.findIndex(TableName("B_tbl"), FieldName("Name")).fieldCount(), 1);
  QCOMPARE(list.findIndex(TableName("B_tbl"), FieldName("Name")).fieldNameList().at(0), QString("Name"));
  /*
   * Clear
   */
  list.clear();
  QCOMPARE(list.size(), 0);
}

void SchemaTest::indexListModelTest()
{
  using Mdt::Sql::Schema::Index;
  using Mdt::Sql::Schema::IndexList;
  using Mdt::Sql::Schema::IndexListModel;

  Index index;
  IndexList indexList;
  IndexListModel model;
  QModelIndex modelIndex;
  /*
   * Setup views
   */
  QTableView tableView;
  tableView.setModel(&model);
  tableView.resize(400, 200);
  QTreeView treeView;
  treeView.setModel(&model);
  treeView.resize(400, 200);
  QComboBox comboBox;
  comboBox.setModel(&model);
  comboBox.setModelColumn(IndexListModel::IndexNameColumn);
  /*
   * Setup indexes
   */
  // Index on Name field
  index.clear();
  index.setTableName("Connector_tbl");
  index.setName("Name_idx");
  index.addFieldName("Name");
  index.setUnique(true);
  indexList.append(index);
  // Index on A and B fields
  index.clear();
  index.setTableName("Connector_tbl");
  index.setName("AB_idx");
  index.addFieldName("A");
  index.addFieldName("B");
  indexList.append(index);

  /*
   * Initial state
   */
  QCOMPARE(model.columnCount(), 4);
  QCOMPARE(model.rowCount(), 0);
  /*
   * Set index list
   */
  model.setIndexList(indexList);
  QCOMPARE(model.rowCount(), 2);
  // Check row 0
  modelIndex = model.index(0, IndexListModel::IndexNameColumn);
  QVERIFY(modelIndex.isValid());
  QCOMPARE(model.data(modelIndex), QVariant("Name_idx"));
  modelIndex = model.index(0, IndexListModel::TableNameColumn);
  QVERIFY(modelIndex.isValid());
  QCOMPARE(model.data(modelIndex), QVariant("Connector_tbl"));
  modelIndex = model.index(0, IndexListModel::FieldNameListColumn);
  QVERIFY(modelIndex.isValid());
  QCOMPARE(model.data(modelIndex), QVariant("Name"));
  modelIndex = model.index(0, IndexListModel::IsUniqueColumn);
  QVERIFY(modelIndex.isValid());
  QCOMPARE(model.data(modelIndex), QVariant(true));
  // Check row 1
  modelIndex = model.index(1, IndexListModel::IndexNameColumn);
  QVERIFY(modelIndex.isValid());
  QCOMPARE(model.data(modelIndex), QVariant("AB_idx"));
  modelIndex = model.index(1, IndexListModel::TableNameColumn);
  QVERIFY(modelIndex.isValid());
  QCOMPARE(model.data(modelIndex), QVariant("Connector_tbl"));
  modelIndex = model.index(1, IndexListModel::FieldNameListColumn);
  QVERIFY(modelIndex.isValid());
  QCOMPARE(model.data(modelIndex), QVariant("A, B"));
  modelIndex = model.index(1, IndexListModel::IsUniqueColumn);
  QVERIFY(modelIndex.isValid());
  QCOMPARE(model.data(modelIndex), QVariant(false));

  /*
   * Play
   */
  tableView.show();
  tableView.resizeColumnsToContents();
  tableView.resizeRowsToContents();
  treeView.show();
  comboBox.show();
  /*
  while(tableView.isVisible()){
    QTest::qWait(500);
  }
  */
}

void SchemaTest::triggerTest()
{
  using Mdt::Sql::Schema::Trigger;

  Schema::Client client;

  /*
   * Initial state
   */
  Trigger t;
  QVERIFY(!t.isTemporary());
  QVERIFY(t.event() == Trigger::Unknown);
  /*
   * Set
   */
  t.setName("TRG");
  t.setEvent(Trigger::AfterInsert);
  t.setTemporary(true);
  t.setTable(client);
  t.setScript("UPDATE Client_tbl SET Name = 'Some name';");
  // Check
  QCOMPARE(t.name(), QString("TRG"));
  QVERIFY(t.event() == Trigger::AfterInsert);
  QVERIFY(t.isTemporary());
  QCOMPARE(t.tableName(), client.tableName());
  QCOMPARE(t.script(), QString("UPDATE Client_tbl SET Name = 'Some name';"));
  /*
   * Clear
   */
  t.clear();
  QVERIFY(t.name().isEmpty());
  QVERIFY(t.event() == Trigger::Unknown);
  QVERIFY(!t.isTemporary());
  QVERIFY(t.tableName().isEmpty());
  QVERIFY(t.script().isEmpty());
}

void SchemaTest::triggerListTest()
{
  using Mdt::Sql::Schema::Trigger;
  using Mdt::Sql::Schema::TriggerList;

  /*
   * Initial state
   */
  TriggerList list;
  QVERIFY(list.isEmpty());
  /*
   * Add 1 element
   */
  Trigger t;
  t.setName("TRG");
  list.append(t);
  QCOMPARE(list.size(), 1);
  QVERIFY(!list.isEmpty());
  QCOMPARE(list.at(0).name(), QString("TRG"));
  for(const auto & trg : list){
    QCOMPARE(trg.name(), QString("TRG"));
  }
  /*
   * Clear
   */
  list.clear();
  QVERIFY(list.isEmpty());
}

void SchemaTest::schemaTest()
{
  using Mdt::Sql::Schema::TablePopulation;
  using Mdt::Sql::Schema::Trigger;

  Mdt::Sql::Schema::Schema schema;
  Schema::Client client;
  Schema::Address address;
  Schema::ClientAdrressView clientAddressView;
  Schema::ClientPopulation clientPopulation;
  TablePopulation tp;
  Trigger trigger;

  /*
   * Initial state
   */
  QCOMPARE(schema.tableCount(), 0);
  QCOMPARE(schema.viewCount(), 0);
  QCOMPARE(schema.tablePopulationCount(), 0);
  /*
   * Add tables
   */
  schema.addTable(client);
  schema.addTable(address);
  QCOMPARE(schema.tableCount(), 2);
  QCOMPARE(schema.tableName(0), client.tableName());
  QCOMPARE(schema.tableName(1), address.tableName());
  /*
   * Add views
   */
  schema.addView(clientAddressView);
  QCOMPARE(schema.viewCount(), 1);
  QCOMPARE(schema.viewName(0), clientAddressView.name());
  /*
   * Add populations
   */
  tp.clear();
  tp.setName("TP1");
  schema.addTablePopulation(tp);
  QCOMPARE(schema.tablePopulationCount(), 1);
  QCOMPARE(schema.tablePopulationName(0), QString("TP1"));
  // Edit table population
  schema.refTablePopulationAt(0).setName("TP11");
  QCOMPARE(schema.tablePopulationName(0), QString("TP11"));
  /*
   * Add table population template
   */
  schema.addTablePopulation(clientPopulation);
  QCOMPARE(schema.tablePopulationCount(), 2);
  QCOMPARE(schema.refTablePopulationAt(1).tableName(), QString("Client_tbl"));
  /*
   * Add triggers
   */
  trigger.clear();
  trigger.setName("TRG1");
  schema.addTrigger(trigger);
  QCOMPARE(schema.triggerCount(), 1);
  QCOMPARE(schema.triggerName(0), QString("TRG1"));
  /*
   * Clear
   */
  schema.clear();
  QCOMPARE(schema.tableCount(), 0);
  QCOMPARE(schema.viewCount(), 0);
  QCOMPARE(schema.tablePopulationCount(), 0);
  QCOMPARE(schema.triggerCount(), 0);
}

/*
 * Main
 */

int main(int argc, char **argv)
{
  Mdt::Application app(argc, argv);
  SchemaTest test;

  if(!app.init()){
    return 1;
  }

  return QTest::qExec(&test, argc, argv);
}
