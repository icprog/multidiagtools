/****************************************************************************
 **
 ** Copyright (C) 2011-2014 Philippe Steinmann.
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
#ifndef MDT_CL_UNIT_H
#define MDT_CL_UNIT_H

#include "mdtClUnitConnectionData.h"
#include "mdtClUnitConnectorData.h"
#include "mdtClLinkData.h"
#include "mdtTtBase.h"
#include "mdtError.h"
#include "mdtSqlRecord.h"
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlError>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QList>
#include <QModelIndex>

/*! \brief Helper class to mana unit and related data
 */
class mdtClUnit : public mdtTtBase
{
 public:

  /*! \brief Constructor
   */
  mdtClUnit(QObject *parent, QSqlDatabase db);

  /*! \brief Destructor
   */
  ~mdtClUnit();

  /*! \brief Get SQL statement for component selection
   *
   * List all units, expected those that are allready component of given unitId
   */
  QString sqlForComponentSelection(const QVariant & unitId) const;

  /*! \brief Get SQL statement for connector contacts selection
   *
   * List all connector contacts for given connector ID (from Connector_tbl).
   */
  QString sqlForConnectorContactSelection(const QVariant & connectorId) const;

  /*! \brief Get SQL statement for article connector selection
   *
   * List all article connectors for given articleId wich are not
   *  used by given unitId .
   */
  QString sqlForArticleConnectorSelection(const QVariant & articleId, const QVariant & unitId) const;

  /*! \brief Get SQL statement free for article connection selection
   *
   * List all article connections that are not based on a article connector for given article ID,
   *  and that are not used by a given unit ID.
   */
  QString sqlForFreeArticleConnectionSelection(const QVariant & articleId, const QVariant & unitId) const;

  /*! \brief Get SQL statement for article connection selection
   *
   * List all article connections for a given article connector ID,
   *  and that are not used by a given unit ID .
   */
  QString sqlForArticleConnectionLinkedToArticleConnectorSelection(const QVariant & articleConnectorId, const QVariant & unitId) const;

  /*! \brief Get unit connection data for given unit connection ID
   *
   * If given unit connection is based on a article connection,
   *  the article connection data part will also be set.
   */
  mdtClUnitConnectionData getConnectionData(const QVariant & unitConnectionId, bool *ok);

  /*! \brief Get unit connector data
   *
   * \param unitConnectorId Unit connector ID
   * \param includeConnectionData If true, and SQL statement points to a unit connector
   *            that contains unit connections, result will be populated with these unit connection data.
   *            For each unit connection data that is based on a article connection,
   *             article connection data will be included.
   * \param includeArticleConnectorData If true, article connector data will be included.
   *            Note: article connection data related to article connector are not included.
   *                  To get article connection data, set the includeConnectionData flag to true,
   *                  so it's possible to get article connection data related to unit connection data.
   * \param includeBaseConnectorData If true, and SQL statement points to a unit connector
   *            that is based on a connector (from Connector_tbl), result will be populated with these connector data.
   *
   * Note: if both includeArticleConnectorData and includeBaseConnectorData are set,
   *       and UnitConnector_tbl.Connector_Id_FK is different from ArticleConnector_tbl.Connector_Id_FK,
   *       an error will be generated and this method will fail, because this is not coherent.
   *
   * \pre Given SQL statement must return 1 unit connector row
   * \pre ok must be valid
   */
  mdtClUnitConnectorData getConnectorData(const QVariant & unitConnectorId, bool *ok, bool includeConnectionData, bool includeArticleConnectorData, bool includeBaseConnectorData);

  /*! \brief Add unit connections into unit connector data based on a list of given connector contact ID list
   *
   * Will also get some values from data (UnitConnector_tbl) and set it to created connections (UnitConnection_tbl):
   *  - UnitConnector_tbl.Id_PK -> UnitConnection_tbl.UnitConnector_Id_FK
   *  - UnitConnector_tbl.Article_Id_FK -> UnitConnection_tbl.Unit_Id_FK
   *
   * Note: will only update given data, nothing is written to database.
   */
  bool addConnectionDataListFromConnectorContactIdList(mdtClUnitConnectorData & data, const QList<QVariant> & connectorContactIdList);

  /*! \brief Add article connector data part into unit connector
   *
   * Will update some value in data (UnitConnector_tbl):
   *  - Connector_Id_FK : will be copied from article connector data
   *  - ArticleConnector_Id_FK : will be copied from article connector data
   *
   * \param data Unit connector data to update
   * \param articleConnectorId ID of article connector
   * \param copyConnectorName If true, unit connector name will be copied from article connector name
   *
   * Note: will only update given data, nothing is written to database.
   */
  bool addArticleConnectorData(mdtClUnitConnectorData & data, const QVariant & articleConnectorId, bool copyConnectorName);

  /*! \brief Add unit connections into unit connector data based on given article connection ID list
   * \todo What is better ? 2 methods ?? Should coherence between article connection and article connector be done ? Or let addConnector() do it ?
   *
   * Will also set some values to newly created article connections (ArticleConnection_tbl):
   *  - Unit_Id_FK : will be copied from unit connector data
   *  - UnitConnector_Id_FK : will be copied from unit connector data
   *  - ArticleConnection_Id_FK : will be copied from articleConnectionIdList
   *  - UnitContactName : if copyContactName is true, it will be copied from article connection data
   *
   * \param data Unit connector data to update
   * \param articleConnectionIdList For each ID in list, a unit connection is created on base of article connection data.
   * \param copyContactName If true, unit contact name will be copied from article contact name.
   */
  bool addConnectionDataListFromArticleConnectionIdList(mdtClUnitConnectorData & data, const QList<QVariant> & articleConnectionIdList, bool copyContactName);

  /*! \brief Add a unit as component
   *
   * \return True on success, false else.
   *          To get reason of failure, use lastError() .
   */
  bool addComponent(const QVariant &unitId, const QVariant &componentId);

  /*! \brief Remove a single component
   */
  bool removeComponent(const QVariant &componentId);

  /*! \brief Remove a list of components
   */
  bool removeComponents(const QList<QVariant> &componentIdList);

  /*! \brief Remove each component that is contained in selection
   *
   * This is usefull used together with mdtSqlTableWidget .
   *
   * \return True on success, false else.
   *          To get reason of failure, use lastError() .
   */
  bool removeComponents(const QModelIndexList & indexListOfSelectedRows);

  /*! \brief Return the number unit connections that are related to article connections
   *
   * \return A number >= 0 on success, a value < 0 on error.
   *         On error, use lastError() to check what goes wrong.
   */
  int toUnitRelatedArticleConnectionCount(const QVariant & unitId);

  /*! \brief Get a model that contains links that are related to given unit ID and a list of unit connection IDs
   */
  QSqlQueryModel *toUnitRelatedLinksModel(const QVariant &unitId, const QList<QVariant> &unitConnectionIdList);

  /*! \brief Get a list of links that are related to given unit ID
   *
   * This is a helper method to display a message to the user in a simple way.
   *  Do not use the result for technical processing.
   */
  QStringList toUnitRelatedLinksList(const QVariant &unitId, const QList<QVariant> &unitConnectionIdList);

  /*! \brief Get a list of links that are related to given unit ID
   *
   * This is a helper method to display a message to the user in a simple way.
   *  Do not use the result for technical processing.
   */
  QString toUnitRelatedLinksListStr(const QVariant &unitId, const QList<QVariant> &unitConnectionIdList);

  /*! \brief Get a list of links that are related to given unit ID
   *
   * This is a helper method to display a message to the user in a simple way.
   *  Do not use the result for technical processing.
   */
  QString toUnitRelatedLinksListStr(const QVariant &unitId, const QModelIndexList & indexListOfSelectedRows);

  /*! \brief Add unit connection
   */
  bool addConnection(const mdtClUnitConnectionData & data);

  /*! \brief Edit a unit connection
   *
   * \return True on success, false else.
   *          To get reason of failure, use lastError() .
   */
  bool editConnection(const QVariant & connectionId, const mdtClUnitConnectionData & data);

  /*! \brief Remove a single unit connection
   */
  bool removeConnection(const QVariant & unitConnectionId);

  /*! \brief Remove each unit connection that is contained in selection
   *
   * This is usefull used together with mdtSqlTableWidget .
   *
   * \return True on success, false else.
   *          To get reason of failure, use lastError() .
   */
  bool removeConnections(const QModelIndexList & indexListOfSelectedRows);

  /*! \brief Add a unit connector
   *
   * If data contains connections,
   *  they will be added.
   *
   * Note: in connection data (contained in data),
   *  Unit_Id_FK and UnitConnector_Id_FK are not relevant,
   *  because they are token from data directly.
   */
  bool addConnector(const mdtClUnitConnectorData & data);

  /*! \brief Remove a unit connector and its contacts
   */
  bool removeConnector(const QVariant & unitConnectorId);

  /*! \brief Remove each unit connector that is contained in selection
   *
   * This is usefull used together with mdtSqlTableWidget .
   *
   * \return True on success, false else.
   *          To get reason of failure, use lastError() .
   */
  bool removeConnectors(const QModelIndexList & indexListOfSelectedRows);

  /*! \brief
   */
  bool addRange(const QVariant & baseUnitConnectionId, const mdtClUnitConnectionData & rangeData);

  /*! \brief Get a data object for a given link
   */
  mdtClLinkData getUnitLinkData(const QVariant &unitConnectionStartId, const QVariant &unitConnectionEndId, bool *ok);

  /*! \brief Add a unit link
   */
  bool addLink(const mdtClLinkData &data);

  /*! \brief Edit a unit link
   */
  bool editLink(const QVariant &unitConnectionStartId, const QVariant &unitConnectionEndId, const mdtClLinkData &data);

  /*! \brief Remove a unit link
   *
   * Will also remove all vehicle type related links
   */
  bool removeLink(const QVariant &unitConnectionStartId, const QVariant &unitConnectionEndId);

  /*! \brief Remove each unit link that is contained in selection
   */
  bool removeLinks(const QList<QModelIndexList> &indexListOfSelectedRowsByRows);

 private:

  /*! \brief Get unit connection data and article connection data if based on
   *
   * \pre Given SQL statement must return 1 unit connection row
   * \pre ok must be valid
   */
  mdtClUnitConnectionData getConnectionDataPv(const QString & sql, bool *ok);

  /*! \brief Get unit connector data
   *
   * \param includeConnectionData If true, and SQL statement points to a unit connector
   *            that contains unit connections, result will be populated with these unit connection data.
   *            For each unit connection data that is based on a article connection,
   *             article connection data will be included.
   * \param includeArticleConnectorData If true, article connector data will be included.
   *            Note: article connection data related to article connector are not included.
   *                  To get article connection data, set the includeConnectionData flag to true,
   *                  so it's possible to get article connection data related to unit connection data.
   * \param includeBaseConnectorData If true, and SQL statement points to a unit connector
   *            that is based on a connector (from Connector_tbl), result will be populated with these connector data.
   *
   * Note: if both includeArticleConnectorData and includeBaseConnectorData are set,
   *       and UnitConnector_tbl.Connector_Id_FK is different from ArticleConnector_tbl.Connector_Id_FK,
   *       an error will be generated and this method will fail, because this is not coherent.
   *
   * \pre Given SQL statement must return 1 unit connector row
   * \pre ok must be valid
   */
  mdtClUnitConnectorData getConnectorDataPv(const QString & sql, bool *ok, bool includeConnectionData, bool includeArticleConnectorData, bool includeBaseConnectorData);

  /*! \brief Do some coherence check on unit connector data
   */
  bool checkConnectorData(const mdtClUnitConnectorData & data);

  /*! \brief Add link to vehicle type table
   */
  bool addLinkToVehicleType(const QVariant &vehicleTypeStartId, const QVariant &vehicleTypeEndId, const QVariant &unitConnectionStartId, const QVariant &unitConnectionEndId);

  /*! \brief Remove a link from vehicle type table
   */
  bool removeLinkFromVehicleType(const QVariant &vehicleTypeStartId, const QVariant &vehicleTypeEndId, const QVariant &unitConnectionStartId, const QVariant &unitConnectionEndId, QSqlQuery &query);

  Q_DISABLE_COPY(mdtClUnit);

  QSqlQueryModel *pvUnitLinkModel;
};

#endif  // #ifndef MDT_CL_UNIT_H
