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
#include "LikeExpressionTestSchema.h"
#include "Mdt/Sql/Schema/Table.h"
#include "Mdt/Sql/Schema/TablePopulation.h"
#include <QVariant>

namespace Sql = Mdt::Sql;
using Sql::Schema::Field;
using Sql::Schema::FieldType;
using Sql::Schema::Table;
using Sql::Schema::TablePopulation;

namespace Schema{

LikeExpressionTestSchema::LikeExpressionTestSchema()
{
  // Data field
  Field data;
  data.setName("Data");
  data.setType(FieldType::Varchar);
  data.setLength(50);
  // StrData table
  Table strData;
  strData.setTableName("StrData_tbl");
  strData.setAutoIncrementPrimaryKey("Id_PK");
  strData.addField(data);
  addTable(strData);
  // StrData table population
  TablePopulation strDataTp;
  strDataTp.setName("StrData pop");
  strDataTp.setTable(strData);
  strDataTp.addField(data);
  addTablePopulation(strDataTp);
  // MetaData table
  Table metaData;
  metaData.setTableName("MetaData_tbl");
  metaData.setAutoIncrementPrimaryKey("Id_PK");
  metaData.addField(data);
  addTable(metaData);
  // MetaData table population
  TablePopulation metaDataTp;
  metaDataTp.setName("MetaData pop");
  metaDataTp.setTable(metaData);
  metaDataTp.addField(data);
  addTablePopulation(metaDataTp);
}

void LikeExpressionTestSchema::addStrRow(const QString& str)
{
  refTablePopulationAt(0).addRow(str);
}

void LikeExpressionTestSchema::addMetaRow(const QString& meta)
{
  refTablePopulationAt(1).addRow(meta);
}

} // namespace Schema{
