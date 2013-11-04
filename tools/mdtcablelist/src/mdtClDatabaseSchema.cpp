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
#include "mdtClDatabaseSchema.h"
#include "mdtSqlDatabaseManager.h"
#include "mdtDataTableManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlField>
#include <QFileInfo>
#include <QStringList>

#include <QDebug>

mdtClDatabaseSchema::mdtClDatabaseSchema(mdtSqlDatabaseManager *dbManager) 
{
  Q_ASSERT(dbManager != 0);

  ///pvDatabaseManager = new mdtSqlDatabaseManager;
  pvDatabaseManager = dbManager;
}

mdtClDatabaseSchema::~mdtClDatabaseSchema() 
{
  ///delete pvDatabaseManager;
}

bool mdtClDatabaseSchema::createSchemaSqlite(const QDir & startDirectory)
{
  if(!pvDatabaseManager->createDatabaseSqlite(startDirectory)){
    return false;
  }
  return createSchemaSqlite();
}

bool mdtClDatabaseSchema::createSchemaSqlite(const QFileInfo & dbFileInfo) 
{
  if(!pvDatabaseManager->createDatabaseSqlite(dbFileInfo, mdtSqlDatabaseManager::OverwriteExisting)){
    return false;
  }
  return createSchemaSqlite();
}

bool mdtClDatabaseSchema::createSchemaSqlite()
{
  Q_ASSERT(pvDatabaseManager->database().isOpen());

  pvTables.clear();
  setupTables();
  if(!createTablesSqlite()){
    return false;
  }

  qDebug() << "Table count: " << pvDatabaseManager->database().tables(QSql::Tables).size();
  qDebug() << "Tables: " << pvDatabaseManager->database().tables(QSql::Tables);

  if(!createViews()){
    return false;
  }

  qDebug() << "View count: " << pvDatabaseManager->database().tables(QSql::Views).size();
  qDebug() << "Views: " << pvDatabaseManager->database().tables(QSql::Views);

  return checkSchema();
}

bool mdtClDatabaseSchema::importDatabase(const QDir & startDirectory)
{
  Q_ASSERT(pvDatabaseManager->database().isOpen());

  QFileInfo dbFileInfo;

  // Let the user choose source database
  dbFileInfo = pvDatabaseManager->chooseDatabaseSqlite(startDirectory);
  if(dbFileInfo.fileName().isEmpty()){
    return false;
  }

  return importDatabase(dbFileInfo);
}

bool mdtClDatabaseSchema::importDatabase(const QFileInfo sourceDbFileInfo)
{
  Q_ASSERT(pvDatabaseManager->database().isOpen());

  mdtSqlDatabaseManager sourceDbManager;
  mdtDataTableManager tableManager;
  QStringList sourceTables, destinationTables, ignoredTables;
  int i;

  // Open source database
  if(!sourceDbManager.openDatabaseSqlite(sourceDbFileInfo, "import_db_connection")){
    pvLastError = sourceDbManager.lastError();
    return false;
  }
  // Check that it's not the current database
  if(sourceDbManager.database().databaseName() == pvDatabaseManager->database().databaseName()){
    pvLastError.setError("Selected source database is the same as current (destination) database.", mdtError::Error);
    MDT_ERROR_SET_SRC(pvLastError, "mdtClDatabaseSchema");
    pvLastError.commit();
    return false;
  }
  // Get the list of source tables and check if we have all in current database
  sourceTables = sourceDbManager.database().tables(QSql::Tables);
  destinationTables = pvDatabaseManager->database().tables(QSql::Tables);
  for(i = 0; i < sourceTables.size(); ++i){
    if(!destinationTables.contains(sourceTables.at(i))){
      pvLastError.setError("Table '" + sourceTables.at(i) + "' does not exists in current database - Will be ignored", mdtError::Warning);
      MDT_ERROR_SET_SRC(pvLastError, "mdtClDatabaseSchema");
      pvLastError.commit();
      ignoredTables.append(sourceTables.at(i));
    }
  }
  // Remove ignored tables
  for(i = 0; i < ignoredTables.size(); ++i){
    sourceTables.removeAll(ignoredTables.at(i));
  }
  // We also remove the sqlite_sequence table
  sourceTables.removeAll("sqlite_sequence");
  // Copy tables
  for(i = 0; i < sourceTables.size(); ++i){
    if(!tableManager.copyTable(sourceTables.at(i), sourceTables.at(i), mdtSqlDatabaseManager::KeepExisting, sourceDbManager.database(), pvDatabaseManager->database())){
      pvLastError = tableManager.lastError();
      return false;
    }
  }

  return true;
}

bool mdtClDatabaseSchema::checkSchema()
{
  Q_ASSERT(pvDatabaseManager->database().isOpen());

  QStringList dbTables;
  int i;

  // We call setup methods to build the list of expected tables
  pvTables.clear();
  setupTables();
  // Get database available tables and check
  dbTables = pvDatabaseManager->database().tables();
  for(i = 0; i < pvTables.size(); ++i){
    if(!dbTables.contains(pvTables.at(i).tableName())){
      pvLastError.setError("Table '" + pvTables.at(i).tableName() + "' is missing in database '" + pvDatabaseManager->database().databaseName() + "'.", mdtError::Error);
      MDT_ERROR_SET_SRC(pvLastError, "mdtClDatabaseSchema");
      pvLastError.commit();
      return false;
    }
  }

  return true;
}

void mdtClDatabaseSchema::setupTables() 
{
  // We must build tables list in correct order, regarding dependencies
  setupVehicleTypeTable();
  setupArticleTable();
  setupArticleComponentTable();
  setupArticleConnectionTable();
  setupUnitTable();
  setupUnitConnectionTable();
  setupVehicleTypeUnitTable();
  setupLinkDirectionTable();
  setupLinkTypeTable();
  setupArticleLinkTable();
  setupLinkTable();
  setupVehicleTypeLinkTable();
}

bool mdtClDatabaseSchema::createTablesSqlite() 
{
  int i;
  QSqlQuery query(pvDatabaseManager->database());

  for(i = 0; i < pvTables.size(); ++i){
    if(!pvDatabaseManager->createTable(pvTables[i], mdtSqlDatabaseManager::OverwriteExisting)){
      pvTables.clear();
      return false;
    }
  }

  return true;
}

bool mdtClDatabaseSchema::createViews() 
{
  if(!createArticleComponentView()){
    return false;
  }
  if(!createArticleComponentUsageView()){
    return false;
  }
  if(!createArticleLinkView()){
    return false;
  }
  if(!createUnitLinkView()){
    return false;
  }
  if(!createUnitVehicleTypeView()){
    return false;
  }
  if(!createUnitView()){
    return false;
  }
  if(!createUnitComponentView()){
    return false;
  }
  if(!createUnitConnectionView()){
    return false;
  }
  if(!createLinkListView()){
    return false;
  }
  if(!createVehicleTypeUnitView()){
    return false;
  }
  return true;
}

void mdtClDatabaseSchema::setupVehicleTypeTable() 
{
  mdtSqlSchemaTable table;
  QSqlField field;

  table.setTableName("VehicleType_tbl", "UTF8");
  // Id_PK
  field.setName("Id_PK");
  field.setType(QVariant::Int);
  field.setAutoValue(true);
  table.addField(field, true);
  // Type
  field = QSqlField();
  field.setName("Type");
  field.setType(QVariant::String);
  field.setLength(50);
  table.addField(field, false);
  // SubType
  field = QSqlField();
  field.setName("SubType");
  field.setType(QVariant::String);
  field.setLength(50);
  table.addField(field, false);
  // SubType
  field = QSqlField();
  field.setName("SeriesNumber");
  field.setType(QVariant::String);
  field.setLength(50);
  table.addField(field, false);
  field = QSqlField();

  pvTables.append(table);
}

void mdtClDatabaseSchema::setupVehicleTypeUnitTable() 
{
  mdtSqlSchemaTable table;
  QSqlField field;

  table.setTableName("VehicleType_Unit_tbl", "UTF8");
  // Id_PK
  field.setName("Id_PK");
  field.setType(QVariant::Int);
  field.setAutoValue(true);
  table.addField(field, true);
  // VehicleType_Id_FK
  field = QSqlField();
  field.setName("VehicleType_Id_FK");
  field.setType(QVariant::Int);
  field.setRequiredStatus(QSqlField::Required);
  table.addField(field, false);
  // Unit_Id_FK
  field = QSqlField();
  field.setName("Unit_Id_FK");
  field.setType(QVariant::Int);
  field.setRequiredStatus(QSqlField::Required);
  table.addField(field, false);
  // Indexes
  table.addIndex("VehicleType_Id_FK_idx", false);
  if(!table.addFieldToIndex("VehicleType_Id_FK_idx", "VehicleType_Id_FK")){
    return;/// false;
  }
  table.addIndex("Unit_Id_FK_idx", false);
  if(!table.addFieldToIndex("Unit_Id_FK_idx", "Unit_Id_FK")){
    return;/// false;
  }
  // Foreign keys
  table.addForeignKey("VehicleType_Id_FK_fk", "VehicleType_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("VehicleType_Id_FK_fk", "VehicleType_Id_FK", "Id_PK")){
    return;/// false
  }
  table.addForeignKey("Unit_Id_FK_fk2", "Unit_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("Unit_Id_FK_fk2", "Unit_Id_FK", "Id_PK")){
    return;/// false
  }

  pvTables.append(table);
}

void mdtClDatabaseSchema::setupVehicleTypeLinkTable() 
{
  mdtSqlSchemaTable table;
  QSqlField field;

  table.setTableName("VehicleType_Link_tbl", "UTF8");
  // VehicleTypeStart_Id_FK
  field.setName("VehicleTypeStart_Id_FK");
  field.setType(QVariant::Int);
  table.addField(field, true);
  // VehicleTypeEnd_Id_FK
  field.setName("VehicleTypeEnd_Id_FK");
  field.setType(QVariant::Int);
  table.addField(field, true);
  // UnitConnectionStart_Id_FK
  field.setName("UnitConnectionStart_Id_FK");
  field.setType(QVariant::Int);
  table.addField(field, true);
  // UnitConnectionEnd_Id_FK
  field.setName("UnitConnectionEnd_Id_FK");
  field.setType(QVariant::Int);
  table.addField(field, true);
  // Indexes
  table.addIndex("UnitConnectionIndex", false);
  if(!table.addFieldToIndex("UnitConnectionIndex", "UnitConnectionStart_Id_FK")){
    return;/// false;
  }
  if(!table.addFieldToIndex("UnitConnectionIndex", "UnitConnectionEnd_Id_FK")){
    return;/// false;
  }
  table.addIndex("VehicleTypeEnd_Id_FK_idx", false);
  if(!table.addFieldToIndex("VehicleTypeEnd_Id_FK_idx", "VehicleTypeEnd_Id_FK")){
    return;/// false;
  }
  // Foreign keys
  table.addForeignKey("UnitConnection_fk", "Link_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("UnitConnection_fk", "UnitConnectionStart_Id_FK", "UnitConnectionStart_Id_FK")){
    return;/// false
  }
  if(!table.addFieldToForeignKey("UnitConnection_fk", "UnitConnectionEnd_Id_FK", "UnitConnectionEnd_Id_FK")){
    return;/// false
  }
  table.addForeignKey("VehicleTypeStart_Id_FK_fk", "VehicleType_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("VehicleTypeStart_Id_FK_fk", "VehicleTypeStart_Id_FK", "Id_PK")){
    return;/// false
  }
  table.addForeignKey("VehicleTypeEnd_Id_FK_fk", "VehicleType_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("VehicleTypeEnd_Id_FK_fk", "VehicleTypeEnd_Id_FK", "Id_PK")){
    return;/// false
  }

  pvTables.append(table);
}

void mdtClDatabaseSchema::setupArticleTable() 
{
  mdtSqlSchemaTable table;
  QSqlField field;

  table.setTableName("Article_tbl", "UTF8");
  // Id_PK
  field.setName("Id_PK");
  field.setType(QVariant::Int);
  field.setAutoValue(true);
  table.addField(field, true);
  // Coordinate
  field = QSqlField();
  field.setName("ArticleCode");
  field.setType(QVariant::String);
  field.setLength(50);
  table.addField(field, false);
  // Designation EN
  field = QSqlField();
  field.setName("DesignationEN");
  field.setType(QVariant::String);
  field.setLength(150);
  table.addField(field, false);
  // Designation FR
  field = QSqlField();
  field.setName("DesignationFR");
  field.setType(QVariant::String);
  field.setLength(150);
  table.addField(field, false);
  // Designation DE
  field = QSqlField();
  field.setName("DesignationDE");
  field.setType(QVariant::String);
  field.setLength(150);
  table.addField(field, false);
  // Designation IT
  field = QSqlField();
  field.setName("DesignationIT");
  field.setType(QVariant::String);
  field.setLength(150);
  table.addField(field, false);
  // Unit
  field = QSqlField();
  field.setName("Unit");
  field.setType(QVariant::String);
  field.setLength(10);
  field.setRequiredStatus(QSqlField::Required);
  field.setDefaultValue("pce");
  table.addField(field, false);

  pvTables.append(table);
}

void mdtClDatabaseSchema::setupArticleComponentTable() 
{
  mdtSqlSchemaTable table;
  QSqlField field;

  table.setTableName("ArticleComponent_tbl", "UTF8");
  // Composite_Id_FK
  field.setName("Composite_Id_FK");
  field.setType(QVariant::Int);
  table.addField(field, true);
  // Component_Id_FK
  field = QSqlField();
  field.setName("Component_Id_FK");
  field.setType(QVariant::Int);
  table.addField(field, true);
  // ComponentQty
  field.setName("ComponentQty");
  field.setType(QVariant::Double);
  field.setRequiredStatus(QSqlField::Required);
  table.addField(field, false);
  // ComponentQtyUnit
  field = QSqlField();
  field.setName("ComponentQtyUnit");
  field.setType(QVariant::String);
  field.setLength(10);
  field.setRequiredStatus(QSqlField::Required);
  table.addField(field, false);
  // Foreign keys
  table.addForeignKey("Composite_Id_FK_fk2", "Article_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("Composite_Id_FK_fk2", "Composite_Id_FK", "Id_PK")){
    return;/// false
  }
  table.addForeignKey("Component_Id_FK_fk", "Article_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("Component_Id_FK_fk", "Component_Id_FK", "Id_PK")){
    return;/// false
  }

  pvTables.append(table);
}

void mdtClDatabaseSchema::setupArticleConnectionTable() 
{
  mdtSqlSchemaTable table;
  QSqlField field;

  table.setTableName("ArticleConnection_tbl", "UTF8");
  // Id_PK
  field.setName("Id_PK");
  field.setType(QVariant::Int);
  field.setAutoValue(true);
  table.addField(field, true);
  // Article_Id_FK
  field = QSqlField();
  field.setName("Article_Id_FK");
  field.setType(QVariant::Int);
  field.setRequiredStatus(QSqlField::Required);
  table.addField(field, false);
  // ArticleConnectorName
  field = QSqlField();
  field.setName("ArticleConnectorName");
  field.setType(QVariant::String);
  field.setLength(30);
  table.addField(field, false);
  // ArticleContactName
  field = QSqlField();
  field.setName("ArticleContactName");
  field.setType(QVariant::String);
  field.setLength(30);
  table.addField(field, false);
  // IoType
  field = QSqlField();
  field.setName("IoType");
  field.setType(QVariant::String);
  field.setLength(30);
  table.addField(field, false);
  // FunctionEN
  field = QSqlField();
  field.setName("FunctionEN");
  field.setType(QVariant::String);
  field.setLength(100);
  table.addField(field, false);
  // FunctionFR
  field = QSqlField();
  field.setName("FunctionFR");
  field.setType(QVariant::String);
  field.setLength(100);
  table.addField(field, false);
  // FunctionDE
  field = QSqlField();
  field.setName("FunctionDE");
  field.setType(QVariant::String);
  field.setLength(100);
  table.addField(field, false);
  // FunctionIT
  field = QSqlField();
  field.setName("FunctionIT");
  field.setType(QVariant::String);
  field.setLength(100);
  table.addField(field, false);
  // Indexes
  table.addIndex("Article_Id_FK_idx2", false);
  if(!table.addFieldToIndex("Article_Id_FK_idx2", "Article_Id_FK")){
    return;/// false;
  }
  // Foreign keys
  table.addForeignKey("Article_Id_FK_fk2", "Article_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("Article_Id_FK_fk2", "Article_Id_FK", "Id_PK")){
    return;/// false
  }

  pvTables.append(table);
}

void mdtClDatabaseSchema::setupArticleLinkTable() 
{
  mdtSqlSchemaTable table;
  QSqlField field;

  table.setTableName("ArticleLink_tbl", "UTF8");
  // Id_PK
  field.setName("Id_PK");
  field.setType(QVariant::Int);
  field.setAutoValue(true);
  table.addField(field, true);
  // ArticleConnectionStart_Id_FK
  field = QSqlField();
  field.setName("ArticleConnectionStart_Id_FK");
  field.setType(QVariant::Int);
  field.setRequiredStatus(QSqlField::Required);
  table.addField(field, false);
  // ArticleConnectionEnd_Id_FK
  field = QSqlField();
  field.setName("ArticleConnectionEnd_Id_FK");
  field.setType(QVariant::Int);
  field.setRequiredStatus(QSqlField::Required);
  table.addField(field, false);
  // Identification
  field = QSqlField();
  field.setName("Identification");
  field.setType(QVariant::String);
  field.setLength(50);
  table.addField(field, false);
  // LinkDirection_Code_FK
  field = QSqlField();
  field.setName("LinkDirection_Code_FK");
  field.setType(QVariant::String);
  field.setLength(10);
  field.setRequiredStatus(QSqlField::Required);
  table.addField(field, false);
  // LinkType_Code_FK
  field = QSqlField();
  field.setName("LinkType_Code_FK");
  field.setType(QVariant::String);
  field.setLength(10);
  field.setRequiredStatus(QSqlField::Required);
  table.addField(field, false);
  // Value
  field = QSqlField();
  field.setName("Value");
  field.setType(QVariant::Double);
  table.addField(field, false);
  // Indexes
  table.addIndex("LinkType_Code_FK_idx", false);
  if(!table.addFieldToIndex("LinkType_Code_FK_idx", "LinkType_Code_FK")){
    return;/// false;
  }
  table.addIndex("ArticleConnectionStart_Id_FK_idx", false);
  if(!table.addFieldToIndex("ArticleConnectionStart_Id_FK_idx", "ArticleConnectionStart_Id_FK")){
    return;/// false;
  }
  table.addIndex("ArticleConnectionEnd_Id_FK_idx", false);
  if(!table.addFieldToIndex("ArticleConnectionEnd_Id_FK_idx", "ArticleConnectionEnd_Id_FK")){
    return;/// false;
  }
  table.addIndex("LinkDirection_Code_FK_idx", false);
  if(!table.addFieldToIndex("LinkDirection_Code_FK_idx", "LinkDirection_Code_FK")){
    return;/// false;
  }
  // Foreign keys
  table.addForeignKey("ArticleConnectionStart_Id_FK_fk", "ArticleConnection_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("ArticleConnectionStart_Id_FK_fk", "ArticleConnectionStart_Id_FK", "Id_PK")){
    return;/// false
  }
  table.addForeignKey("ArticleConnectionEnd_Id_FK_fk", "ArticleConnection_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("ArticleConnectionEnd_Id_FK_fk", "ArticleConnectionEnd_Id_FK", "Id_PK")){
    return;/// false
  }
  table.addForeignKey("LinkDirection_Code_FK_fk", "LinkDirection_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("LinkDirection_Code_FK_fk", "LinkDirection_Code_FK", "Code_PK")){
    return;/// false
  }
  table.addForeignKey("LinkType_Code_FK_fk", "LinkType_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("LinkType_Code_FK_fk", "LinkType_Code_FK", "Code_PK")){
    return;/// false
  }

  pvTables.append(table);
}

void mdtClDatabaseSchema::setupUnitTable() 
{
  mdtSqlSchemaTable table;
  QSqlField field;

  table.setTableName("Unit_tbl", "UTF8");
  // Id_PK
  field.setName("Id_PK");
  field.setType(QVariant::Int);
  field.setAutoValue(true);
  table.addField(field, true);
  // Composite_Id_FK
  field = QSqlField();
  field.setName("Composite_Id_FK");
  field.setType(QVariant::Int);
  table.addField(field, false);
  // Article_Id_FK
  field = QSqlField();
  field.setName("Article_Id_FK");
  field.setType(QVariant::Int);
  table.addField(field, false);
  // Coordinate
  field = QSqlField();
  field.setName("Coordinate");
  field.setType(QVariant::String);
  field.setLength(10);
  table.addField(field, false);
  // Cabinet
  field = QSqlField();
  field.setName("Cabinet");
  field.setType(QVariant::String);
  field.setLength(30);
  table.addField(field, false);
  // SchemaPosition
  field = QSqlField();
  field.setName("SchemaPosition");
  field.setType(QVariant::String);
  field.setLength(50);
  table.addField(field, false);
  // Alias
  field = QSqlField();
  field.setName("Alias");
  field.setType(QVariant::String);
  field.setLength(50);
  table.addField(field, false);
  // Indexes
  table.addIndex("Article_Id_FK_idx", false);
  if(!table.addFieldToIndex("Article_Id_FK_idx", "Article_Id_FK")){
    return;/// false;
  }
  table.addIndex("Composite_Id_FK_idx", false);
  if(!table.addFieldToIndex("Composite_Id_FK_idx", "Composite_Id_FK")){
    return;/// false;
  }
  // Foreign keys
  table.addForeignKey("Composite_Id_FK_fk", "Unit_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("Composite_Id_FK_fk", "Composite_Id_FK", "Id_PK")){
    return;/// false
  }
  table.addForeignKey("Article_Id_FK_fk", "Article_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("Article_Id_FK_fk", "Article_Id_FK", "Id_PK")){
    return;/// false
  }

  pvTables.append(table);
}

void mdtClDatabaseSchema::setupUnitConnectionTable() 
{
  mdtSqlSchemaTable table;
  QSqlField field;

  table.setTableName("UnitConnection_tbl", "UTF8");
  // Id_PK
  field.setName("Id_PK");
  field.setType(QVariant::Int);
  field.setAutoValue(true);
  table.addField(field, true);
  // Unit_Id_FK
  field = QSqlField();
  field.setName("Unit_Id_FK");
  field.setType(QVariant::Int);
  field.setRequiredStatus(QSqlField::Required);
  table.addField(field, false);
  // ArticleConnection_Id_FK
  field = QSqlField();
  field.setName("ArticleConnection_Id_FK");
  field.setType(QVariant::Int);
  table.addField(field, false);
  // IsATestPoint
  field = QSqlField();
  field.setName("IsATestPoint");
  field.setType(QVariant::Bool);
  table.addField(field, false);
  // SchemaPage
  field = QSqlField();
  field.setName("SchemaPage");
  field.setType(QVariant::String);
  field.setLength(20);
  table.addField(field, false);
  // FunctionEN
  field = QSqlField();
  field.setName("FunctionEN");
  field.setType(QVariant::String);
  field.setLength(100);
  table.addField(field, false);
  // FunctionFR
  field = QSqlField();
  field.setName("FunctionFR");
  field.setType(QVariant::String);
  field.setLength(100);
  table.addField(field, false);
  // FunctionDE
  field = QSqlField();
  field.setName("FunctionDE");
  field.setType(QVariant::String);
  field.setLength(100);
  table.addField(field, false);
  // FunctionIT
  field = QSqlField();
  field.setName("FunctionIT");
  field.setType(QVariant::String);
  field.setLength(100);
  table.addField(field, false);
  // SignalName
  field = QSqlField();
  field.setName("SignalName");
  field.setType(QVariant::String);
  field.setLength(50);
  table.addField(field, false);
  // SwAddress
  field = QSqlField();
  field.setName("SwAddress");
  field.setType(QVariant::Int);
  table.addField(field, false);
  // UnitConnectorName
  field = QSqlField();
  field.setName("UnitConnectorName");
  field.setType(QVariant::String);
  field.setLength(30);
  table.addField(field, false);
  // UnitContactName
  field = QSqlField();
  field.setName("UnitContactName");
  field.setType(QVariant::String);
  field.setLength(30);
  table.addField(field, false);
  // Indexes
  table.addIndex("Unit_Id_FK_ArticleConnection_Id_FK_idx", true);
  if(!table.addFieldToIndex("Unit_Id_FK_ArticleConnection_Id_FK_idx", "Unit_Id_FK")){
    return;/// false;
  }
  if(!table.addFieldToIndex("Unit_Id_FK_ArticleConnection_Id_FK_idx", "ArticleConnection_Id_FK")){
    return;/// false;
  }
  table.addIndex("ArticleConnection_Id_FK_idx", false);
  if(!table.addFieldToIndex("ArticleConnection_Id_FK_idx", "ArticleConnection_Id_FK")){
    return;/// false;
  }
  // Foreign keys
  table.addForeignKey("Unit_Id_FK_fk", "Unit_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("Unit_Id_FK_fk", "Unit_Id_FK", "Id_PK")){
    return;/// false
  }
  table.addForeignKey("ArticleConnection_Id_FK_idx2", "ArticleConnection_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("ArticleConnection_Id_FK_idx2", "ArticleConnection_Id_FK", "Id_PK")){
    return;/// false
  }

  pvTables.append(table);
}

void mdtClDatabaseSchema::setupLinkTable() 
{
  mdtSqlSchemaTable table;
  QSqlField field;

  table.setTableName("Link_tbl", "UTF8");
  // UnitConnectionStart_Id_FK
  field.setName("UnitConnectionStart_Id_FK");
  field.setType(QVariant::Int);
  table.addField(field, true);
  // UnitConnectionEnd_Id_FK
  field = QSqlField();
  field.setName("UnitConnectionEnd_Id_FK");
  field.setType(QVariant::Int);
  table.addField(field, true);
  // ArticleLink_Id_FK
  field = QSqlField();
  field.setName("ArticleLink_Id_FK");
  field.setType(QVariant::Int);
  table.addField(field, false);
  // SinceVersion
  field = QSqlField();
  field.setName("SinceVersion");
  field.setType(QVariant::Double);
  table.addField(field, false);
  // Modification
  field = QSqlField();
  field.setName("Modification");
  field.setType(QVariant::String);
  field.setLength(20);
  table.addField(field, false);
  // Identification
  field = QSqlField();
  field.setName("Identification");
  field.setType(QVariant::String);
  field.setLength(50);
  table.addField(field, false);
  // LinkDirection_Code_FK
  field = QSqlField();
  field.setName("LinkDirection_Code_FK");
  field.setType(QVariant::String);
  field.setLength(10);
  field.setRequiredStatus(QSqlField::Required);
  table.addField(field, false);
  // LinkType_Code_FK
  field = QSqlField();
  field.setName("LinkType_Code_FK");
  field.setType(QVariant::String);
  field.setLength(10);
  field.setRequiredStatus(QSqlField::Required);
  table.addField(field, false);
  // Value
  field = QSqlField();
  field.setName("Value");
  field.setType(QVariant::Double);
  table.addField(field, false);
  // Indexes
  table.addIndex("Direction_Id_FK_idx", true);
  if(!table.addFieldToIndex("Direction_Id_FK_idx", "LinkDirection_Code_FK")){
    return;/// false;
  }
  table.addIndex("Type_Id_FK_idx", true);
  if(!table.addFieldToIndex("Type_Id_FK_idx", "LinkType_Code_FK")){
    return;/// false;
  }
  table.addIndex("ArticleLink_Id_FK_idx", true);
  if(!table.addFieldToIndex("ArticleLink_Id_FK_idx", "ArticleLink_Id_FK")){
    return;/// false;
  }
  // Foreign keys
  table.addForeignKey("UnitConnectionStart_Id_FK_fk", "UnitConnection_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("UnitConnectionStart_Id_FK_fk", "UnitConnectionStart_Id_FK", "Id_PK")){
    return;/// false
  }
  table.addForeignKey("UnitConnectionEnd_Id_FK_fk", "UnitConnection_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("UnitConnectionEnd_Id_FK_fk", "UnitConnectionEnd_Id_FK", "Id_PK")){
    return;/// false
  }
  table.addForeignKey("ArticleLink_Id_FK_fk", "ArticleLink_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("ArticleLink_Id_FK_fk", "ArticleLink_Id_FK", "Id_PK")){
    return;/// false
  }
  table.addForeignKey("LinkDirection_Code_FK_fk2", "LinkDirection_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("LinkDirection_Code_FK_fk2", "LinkDirection_Code_FK", "Code_PK")){
    return;/// false
  }
  table.addForeignKey("LinkType_Code_FK_fk2", "LinkType_tbl", mdtSqlSchemaTable::Restrict, mdtSqlSchemaTable::Cascade);
  if(!table.addFieldToForeignKey("LinkType_Code_FK_fk2", "LinkType_Code_FK", "Code_PK")){
    return;/// false
  }

  pvTables.append(table);
}

void mdtClDatabaseSchema::setupLinkDirectionTable() 
{
  mdtSqlSchemaTable table;
  QSqlField field;

  table.setTableName("LinkDirection_tbl", "UTF8");
  // Code_PK
  field.setName("Code_PK");
  field.setType(QVariant::String);
  field.setLength(10);
  table.addField(field, true);
  // PictureAscii
  field = QSqlField();
  field.setName("PictureAscii");
  field.setType(QVariant::String);
  field.setLength(50);
  table.addField(field, false);
  // NameEN
  field = QSqlField();
  field.setName("NameEN");
  field.setType(QVariant::String);
  field.setLength(50);
  table.addField(field, false);
  // NameFR
  field = QSqlField();
  field.setName("NameFR");
  field.setType(QVariant::String);
  field.setLength(50);
  table.addField(field, false);
  // NameDE
  field = QSqlField();
  field.setName("NameDE");
  field.setType(QVariant::String);
  field.setLength(50);
  table.addField(field, false);
  // NameIT
  field = QSqlField();
  field.setName("NameIT");
  field.setType(QVariant::String);
  field.setLength(50);
  table.addField(field, false);

  pvTables.append(table);
}

void mdtClDatabaseSchema::setupLinkTypeTable() 
{
  mdtSqlSchemaTable table;
  QSqlField field;

  table.setTableName("LinkType_tbl", "UTF8");
  // Code_PK
  field.setName("Code_PK");
  field.setType(QVariant::String);
  field.setLength(10);
  table.addField(field, true);
  // NameEN
  field = QSqlField();
  field.setName("NameEN");
  field.setType(QVariant::String);
  field.setLength(50);
  table.addField(field, false);
  // NameFR
  field = QSqlField();
  field.setName("NameFR");
  field.setType(QVariant::String);
  field.setLength(50);
  table.addField(field, false);
  // NameDE
  field = QSqlField();
  field.setName("NameDE");
  field.setType(QVariant::String);
  field.setLength(50);
  table.addField(field, false);
  // NameIT
  field = QSqlField();
  field.setName("NameIT");
  field.setType(QVariant::String);
  field.setLength(50);
  table.addField(field, false);
  // ValueUnit
  field = QSqlField();
  field.setName("ValueUnit");
  field.setType(QVariant::String);
  field.setLength(10);
  table.addField(field, false);

  pvTables.append(table);
}

bool mdtClDatabaseSchema::createView(const QString & viewName, const QString & sql) 
{
  QSqlQuery query(pvDatabaseManager->database());
  QSqlError sqlError;
  QString dropSql;

  qDebug() << "Creating view " << viewName << " ...";
  ///qDebug() << "SQL: " << sql;
  // Drop view
  dropSql = "DROP VIEW IF EXISTS '" + viewName +"'";
  if(!query.exec(dropSql)){
    sqlError = query.lastError();
    pvLastError.setError("Unable to drop view '" + viewName + "'", mdtError::Error);
    pvLastError.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(pvLastError, "mdtClDatabaseSchema");
    pvLastError.commit();
    return false;
  }
  // Create view
  if(!query.exec(sql)){
    sqlError = query.lastError();
    pvLastError.setError("Unable to create view '" + viewName + "'", mdtError::Error);
    pvLastError.setSystemError(sqlError.number(), sqlError.text());
    MDT_ERROR_SET_SRC(pvLastError, "mdtClDatabaseSchema");
    pvLastError.commit();
    return false;
  }
  
  ///qDebug() << "View " << viewName << " created (DONE)";

  return true;
}

bool mdtClDatabaseSchema::createVehicleTypeUnitView() 
{
  QString sql;

  sql = "CREATE VIEW VehicleType_Unit_view AS\n"\
        "SELECT\n"\
        "VehicleType_tbl.Type ,\n"\
        "VehicleType_tbl.SubType ,\n"\
        "VehicleType_tbl.SeriesNumber ,\n"\
        "Unit_tbl.Coordinate ,\n"\
        "Unit_tbl.Cabinet ,\n"\
        "Unit_tbl.SchemaPosition ,\n"\
        "VehicleType_Unit_tbl.Id_PK AS VehicleType_Unit_Id_PK ,\n"\
        "VehicleType_Unit_tbl.VehicleType_Id_FK ,\n"\
        "VehicleType_Unit_tbl.Unit_Id_FK\n"\
        "FROM VehicleType_Unit_tbl\n"\
        "JOIN VehicleType_tbl\n"\
        "  ON VehicleType_Unit_tbl.VehicleType_Id_FK = VehicleType_tbl.Id_PK\n"\
        "JOIN Unit_tbl\n"\
        "  ON VehicleType_Unit_tbl.Unit_Id_FK = Unit_tbl.Id_PK";

  return createView("VehicleType_Unit_view", sql);
}

bool mdtClDatabaseSchema::createArticleComponentUsageView() 
{
  QString sql;

  sql = "CREATE VIEW ArticleComponentUsage_view AS\n"\
        "SELECT\n"\
        " ArticleComponent_tbl.Component_Id_FK AS Component_Id_PK ,\n"\
        " ArticleComponent_tbl.Composite_Id_FK AS Article_Id_PK ,\n"\
        " Article_tbl.ArticleCode ,\n"\
        " Article_tbl.DesignationEN,\n"\
        " Article_tbl.DesignationFR,\n"\
        " Article_tbl.DesignationDE,\n"\
        " Article_tbl.DesignationIT\n"\
        "FROM ArticleComponent_tbl\n"\
        " JOIN Article_tbl\n"\
        "  ON Article_tbl.Id_PK = ArticleComponent_tbl.Composite_Id_FK";

  return createView("ArticleComponentUsage_view", sql);
}

bool mdtClDatabaseSchema::createArticleComponentView() 
{
  QString sql;

  sql = "CREATE VIEW ArticleComponent_view AS\n"\
        "SELECT\n"\
        " ArticleComponent_tbl.Composite_Id_FK AS Article_Id_PK ,\n"\
        " ArticleComponent_tbl.Component_Id_FK AS Component_Id_PK ,\n"\
        " ArticleComponent_tbl.ComponentQty ,\n"\
        " ArticleComponent_tbl.ComponentQtyUnit ,\n"\
        " Article_tbl.ArticleCode ,\n"\
        " Article_tbl.DesignationEN,\n"\
        " Article_tbl.DesignationFR,\n"\
        " Article_tbl.DesignationDE,\n"\
        " Article_tbl.DesignationIT\n"\
        "FROM ArticleComponent_tbl\n"\
        " JOIN Article_tbl\n"\
        "  ON Article_tbl.Id_PK = ArticleComponent_tbl.Component_Id_FK";

  return createView("ArticleComponent_view", sql);
}

bool mdtClDatabaseSchema::createArticleLinkView() 
{
  QString sql;

  sql = "CREATE VIEW ArticleLink_view AS\n"\
        "SELECT\n"\
        " ArticleLink_tbl.Id_PK ,\n"\
        " ArticleLink_tbl.LinkType_Code_FK ,\n"\
        " ArticleLink_tbl.LinkDirection_Code_FK ,\n"\
        " ArticleLink_tbl.ArticleConnectionStart_Id_FK ,\n"\
        " ArticleLink_tbl.ArticleConnectionEnd_Id_FK ,\n"\
        " ArticleLink_tbl.Identification ,\n"\
        " LinkType_tbl.NameEN AS LinkTypeNameEN ,\n"\
        " LinkType_tbl.NameEN AS LinkTypeNameFR ,\n"\
        " LinkType_tbl.NameEN AS LinkTypeNameDE ,\n"\
        " LinkType_tbl.NameEN AS LinkTypeNameIT ,\n"\
        " ArticleLink_tbl.Value ,\n"\
        " LinkType_tbl.ValueUnit ,\n"\
        " ACS.Article_Id_FK AS StartArticle_Id_FK ,\n"\
        " ACS.ArticleConnectorName AS StartArticleConnectorName ,\n"\
        " ACS.ArticleContactName AS StartArticleContactName ,\n"\
        " ACS.IoType AS StartIoType ,\n"\
        " ACS.FunctionEN AS StartFunctionEN ,\n"\
        " ACS.FunctionEN AS StartFunctionFR ,\n"\
        " ACS.FunctionEN AS StartFunctionDE ,\n"\
        " ACS.FunctionEN AS StartFunctionIT ,\n"\
        " LinkDirection_tbl.PictureAscii AS LinkDirectionPictureAscii ,\n"\
        " ACE.Article_Id_FK AS EndArticle_Id_FK ,\n"\
        " ACE.ArticleConnectorName AS EndArticleConnectorName ,\n"\
        " ACE.ArticleContactName AS EndArticleContactName ,\n"\
        " ACE.IoType AS EndIoType ,\n"\
        " ACE.FunctionEN AS EndFunctionEN,\n"\
        " ACE.FunctionEN AS EndFunctionFR,\n"\
        " ACE.FunctionEN AS EndFunctionDE,\n"\
        " ACE.FunctionEN AS EndFunctionIT\n"\
        "FROM ArticleLink_tbl\n"\
        " JOIN ArticleConnection_tbl ACS\n"\
        "  ON ArticleLink_tbl.ArticleConnectionStart_Id_FK = ACS.Id_PK\n"\
        " JOIN ArticleConnection_tbl ACE\n"\
        "  ON ArticleLink_tbl.ArticleConnectionEnd_Id_FK = ACE.Id_PK\n"\
        " JOIN LinkType_tbl\n"\
        "  ON LinkType_tbl.Code_PK = ArticleLink_tbl.LinkType_Code_FK\n"\
        " JOIN LinkDirection_tbl\n"\
        "  ON LinkDirection_tbl.Code_PK = ArticleLink_tbl.LinkDirection_Code_FK";

  return createView("ArticleLink_view", sql);
}

bool mdtClDatabaseSchema::createUnitView() 
{
  QString sql;

  sql = "CREATE VIEW Unit_view AS\n"\
        "SELECT\n"\
        " Unit_tbl.Id_PK AS Unit_Id_PK ,\n"\
        " Unit_tbl.Coordinate ,\n"\
        " Unit_tbl.Cabinet ,\n"\
        " Unit_tbl.SchemaPosition ,\n"\
        " Article_tbl.Id_PK AS Article_Id_PK ,\n"\
        " Article_tbl.ArticleCode ,\n"\
        " Article_tbl.DesignationEN,\n"\
        " Article_tbl.DesignationFR,\n"\
        " Article_tbl.DesignationDE,\n"\
        " Article_tbl.DesignationIT\n"\
        "FROM Unit_tbl\n"\
        " LEFT JOIN Article_tbl\n"\
        "  ON Unit_tbl.Article_Id_FK = Article_tbl.Id_PK";

  return createView("Unit_view", sql);
}

bool mdtClDatabaseSchema::createUnitComponentView() 
{
  QString sql;

  sql = "CREATE VIEW UnitComponent_view AS\n"\
        "SELECT\n"\
        " UnitComposite.Id_PK AS Unit_Id_PK ,\n"\
        " Unit_tbl.Id_PK AS UnitComponent_Id_PK ,\n"\
        " Unit_tbl.Article_Id_FK ,\n"\
        " Unit_tbl.Coordinate ,\n"\
        " Unit_tbl.Cabinet ,\n"\
        " Unit_tbl.SchemaPosition ,\n"\
        " Article_tbl.Id_PK AS Article_Id_PK ,\n"\
        " Article_tbl.ArticleCode ,\n"\
        " Article_tbl.DesignationEN,\n"\
        " Article_tbl.DesignationFR,\n"\
        " Article_tbl.DesignationDE,\n"\
        " Article_tbl.DesignationIT\n"\
        "FROM Unit_tbl AS UnitComposite\n"\
        " JOIN Unit_tbl\n"\
        "  ON Unit_tbl.Composite_Id_FK = UnitComposite.Id_PK\n"\
        " LEFT JOIN Article_tbl\n"\
        "  ON Unit_tbl.Article_Id_FK = Article_tbl.Id_PK";

  return createView("UnitComponent_view", sql);
}

bool mdtClDatabaseSchema::createUnitConnectionView() 
{
  QString sql;

  sql = "CREATE VIEW UnitConnection_view AS\n"\
        "SELECT\n"\
        " UnitConnection_tbl.Id_PK AS UnitConnection_Id_PK ,\n"\
        " UnitConnection_tbl.Unit_Id_FK ,\n"\
        " UnitConnection_tbl.ArticleConnection_Id_FK,\n"\
        " UnitConnection_tbl.UnitConnectorName ,\n"\
        " UnitConnection_tbl.UnitContactName ,\n"\
        " UnitConnection_tbl.SchemaPage ,\n"\
        " UnitConnection_tbl.FunctionEN AS UnitFunctionEN,\n"\
        " UnitConnection_tbl.FunctionFR AS UnitFunctionFR,\n"\
        " UnitConnection_tbl.FunctionDE AS UnitFunctionDE,\n"\
        " UnitConnection_tbl.FunctionIT AS UnitFunctionIT,\n"\
        " UnitConnection_tbl.SignalName ,\n"\
        " UnitConnection_tbl.SwAddress ,\n"\
        " ArticleConnection_tbl.ArticleConnectorName ,\n"\
        " ArticleConnection_tbl.ArticleContactName ,\n"\
        " ArticleConnection_tbl.IoType ,\n"\
        " ArticleConnection_tbl.FunctionEN AS ArticleFunctionEN,\n"\
        " ArticleConnection_tbl.FunctionFR AS ArticleFunctionFR,\n"\
        " ArticleConnection_tbl.FunctionDE AS ArticleFunctionDE,\n"\
        " ArticleConnection_tbl.FunctionIT AS ArticleFunctionIT\n"\
        "FROM UnitConnection_tbl\n"\
        " LEFT JOIN ArticleConnection_tbl\n"\
        "  ON UnitConnection_tbl.ArticleConnection_Id_FK = ArticleConnection_tbl.Id_PK";

  return createView("UnitConnection_view", sql);
}

bool mdtClDatabaseSchema::createUnitLinkView() 
{
  QString sql;

  sql = "CREATE VIEW UnitLink_view AS\n"\
        "SELECT\n"\
        " Link_tbl.Identification ,\n"\
        " US.SchemaPosition AS StartSchemaPosition ,\n"\
        " UCS.UnitConnectorName AS StartUnitConnectorName ,\n"\
        " UCS.UnitContactName AS StartUnitContactName ,\n"\
        " UE.SchemaPosition AS EndSchemaPosition ,\n"\
        " UCE.UnitConnectorName AS EndUnitConnectorName ,\n"\
        " UCE.UnitContactName AS EndUnitContactName ,\n"\
        " Link_tbl.SinceVersion ,\n"\
        " Link_tbl.Modification ,\n"\
        " LinkType_tbl.NameEN AS LinkTypeNameEN ,\n"\
        " Link_tbl.Value ,\n"\
        " LinkType_tbl.ValueUnit ,\n"\
        " LinkDirection_tbl.PictureAscii AS LinkDirectionPictureAscii ,\n"\
        " UCS.SchemaPage AS StartSchemaPage ,\n"\
        " UCS.FunctionEN AS StartFunctionEN ,\n"\
        " UCS.SignalName AS StartSignalName ,\n"\
        " UCS.SwAddress AS StartSwAddress ,\n"\
        " UCE.SchemaPage AS EndSchemaPage ,\n"\
        " UCE.FunctionEN AS EndFunctionEN ,\n"\
        " UCE.SignalName AS EndSignalName ,\n"\
        " UCE.SwAddress AS EndSwAddress ,\n"\
        " Link_tbl.UnitConnectionStart_Id_FK ,\n"\
        " Link_tbl.UnitConnectionEnd_Id_FK ,\n"\
        " Link_tbl.ArticleLink_Id_FK ,\n"\
        " UCS.Unit_Id_FK AS StartUnit_Id_FK ,\n"\
        " UCE.Unit_Id_FK AS EndUnit_Id_FK ,\n"\
        " Link_tbl.LinkType_Code_FK ,\n"\
        " Link_tbl.LinkDirection_Code_FK ,\n"\
        " ArticleLink_tbl.ArticleConnectionStart_Id_FK ,\n"\
        " ArticleLink_tbl.ArticleConnectionEnd_Id_FK\n"\
        "FROM Link_tbl\n"\
        " JOIN UnitConnection_tbl UCS\n"\
        "  ON Link_tbl.UnitConnectionStart_Id_FK = UCS.Id_PK\n"\
        " JOIN UnitConnection_tbl UCE\n"\
        "  ON Link_tbl.UnitConnectionEnd_Id_FK = UCE.Id_PK\n"\
        " JOIN Unit_tbl US\n"\
        "  ON US.Id_PK = UCS.Unit_Id_FK\n"\
        " JOIN Unit_tbl UE\n"\
        "  ON UE.Id_PK = UCE.Unit_Id_FK\n"\
        " JOIN LinkType_tbl\n"\
        "  ON LinkType_tbl.Code_PK = Link_tbl.LinkType_Code_FK\n"\
        " JOIN LinkDirection_tbl\n"\
        "  ON LinkDirection_tbl.Code_PK = Link_tbl.LinkDirection_Code_FK\n"\
        " LEFT JOIN ArticleLink_tbl\n"\
        "  ON ArticleLink_tbl.Id_PK = Link_tbl.ArticleLink_Id_FK";

  return createView("UnitLink_view", sql);
}

bool mdtClDatabaseSchema::createUnitVehicleTypeView() 
{
  QString sql;

  sql = "CREATE VIEW Unit_VehicleType_view AS\n"\
        "SELECT\n"\
        " VehicleType_tbl.Type ,\n"\
        " VehicleType_tbl.SubType ,\n"\
        " VehicleType_tbl.SeriesNumber ,\n"\
        " VehicleType_Unit_tbl.VehicleType_Id_FK ,\n"\
        " VehicleType_Unit_tbl.Unit_Id_FK\n"\
        "FROM VehicleType_tbl\n"\
        " JOIN VehicleType_Unit_tbl\n"\
        "  ON VehicleType_tbl.Id_PK = VehicleType_Unit_tbl.VehicleType_Id_FK";

  return createView("Unit_VehicleType_view", sql);
}

bool mdtClDatabaseSchema::createLinkListView() 
{
  QString sql;

  sql = "CREATE VIEW LinkList_view AS\n"\
        "SELECT\n"\
        " Link_tbl.UnitConnectionStart_Id_FK ,\n"\
        " Link_tbl.UnitConnectionEnd_Id_FK ,\n"\
        " VS.Type AS StartVehicleType ,\n"\
        " VS.SubType AS StartVehicleSubType ,\n"\
        " VS.SeriesNumber AS StartVehicleSerie,\n"\
        " Link_tbl.SinceVersion ,\n"\
        " Link_tbl.Modification ,\n"\
        " Link_tbl.Identification ,\n"\
        " Link_tbl.LinkDirection_Code_FK ,\n"\
        " Link_tbl.LinkType_Code_FK ,\n"\
        " US.Id_PK AS UnitStart_Id_PK ,\n"\
        " US.SchemaPosition AS StartSchemaPosition,\n"\
        " US.Cabinet AS StartCabinet,\n"\
        " US.Coordinate AS StartCoordinate ,\n"\
        " CS.UnitConnectorName AS StartUnitConnectorName ,\n"\
        " CS.UnitContactName AS StartUnitContactName ,\n"\
        " VE.Type AS EndVehicleType ,\n"\
        " VE.SubType AS EndVehicleSubType ,\n"\
        " VE.SeriesNumber AS EndVehicleSerie,\n"\
        " UE.Id_PK AS UnitEnd_Id_PK ,\n"\
        " UE.SchemaPosition AS EndSchemaPosition,\n"\
        " UE.Cabinet AS EndCabinet,\n"\
        " UE.Coordinate AS EndCoordinate ,\n"\
        " CE.UnitConnectorName AS EndUnitConnectorName ,\n"\
        " CE.UnitContactName AS EndUnitContactName\n"\
        "FROM Link_tbl\n"\
        " JOIN UnitConnection_tbl CS\n"\
        "  ON CS.Id_PK = Link_tbl.UnitConnectionStart_Id_FK\n"\
        " JOIN UnitConnection_tbl CE\n"\
        "  ON CE.Id_PK = Link_tbl.UnitConnectionEnd_Id_FK\n"\
        " JOIN Unit_tbl US\n"\
        "  ON US.Id_PK = CS.Unit_Id_FK\n"\
        " JOIN Unit_tbl UE\n"\
        "  ON UE.Id_PK = CE.Unit_Id_FK\n"\
        " JOIN VehicleType_Link_tbl\n"\
        "  ON VehicleType_Link_tbl.UnitConnectionStart_Id_FK = Link_tbl.UnitConnectionStart_Id_FK\n"\
        "  AND VehicleType_Link_tbl.UnitConnectionEnd_Id_FK = Link_tbl.UnitConnectionEnd_Id_FK\n"\
        " JOIN VehicleType_tbl VS\n"\
        "  ON VS.Id_PK = VehicleType_Link_tbl.VehicleTypeStart_Id_FK\n"\
        " JOIN VehicleType_tbl VE\n"\
        "  ON VE.Id_PK = VehicleType_Link_tbl.VehicleTypeEnd_Id_FK";

  return createView("LinkList_view", sql);
}
