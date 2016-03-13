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
#ifndef MDT_SQL_COPIER_MAPPING_MODEL_H
#define MDT_SQL_COPIER_MAPPING_MODEL_H

#include "TableMapping.h"
#include "mdtError.h"
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QVariant>
#include <QString>
#include <vector>

namespace mdt{ namespace sql{ namespace copier{

  /*! \brief Model to fetch informations of a copier mapping
   */
  class MappingModel : public QAbstractTableModel
  {
   Q_OBJECT

   public:

    /*! \brief Copy status
     */
    enum CopyStatus
    {
      CopyStatusUnknown,  /*!< Copy status unknown */
      CopyStatusOk,       /*!< Copy Ok */
      CopyStatusError     /*!< Error occured */
    };

    /*! \brief Constructor
     */
    MappingModel(QObject *parent = nullptr);

    // Copy disabled
    MappingModel(const MappingModel &) = delete;
    MappingModel & operator=(const MappingModel &) = delete;

    // Move disabled
    MappingModel(MappingModel &&) = delete;
    MappingModel & operator=(MappingModel &&) = delete;

    /*! \brief Set table copy progress for given row
     *
     * \pre row must be in a valid range
     */
    void setTableCopyProgress(int row, int progress);

    /*! \brief Set table copy status
     *
     * \pre row must be in a valid range
     */
    void setTableCopyStatus(int row, int status);

    
    /*! \brief Clear copy status and progress for all rows
     */
    void clearCopyStatusAndProgress();

    /*! \brief Get column count
     */
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;

    /*! \brief Get header data
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /*! \brief Get data
     */
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

    /*! \brief Get source table name at given row
     *
     * \pre row must be in a valid range
     */
    virtual QString sourceTableName(int row) const = 0;

    /*! \brief Get destination table name at given row
     *
     * \pre row must be in a valid range
     */
    virtual QString destinationTableName(int row) const = 0;

    /*! \brief Get table mapping state at given row
     *
     * \pre row must be in a valid range
     */
    virtual TableMapping::MappingState tableMappingState(int row) const = 0;

   protected:

    /*! \brief Column index
     */
    enum ColumnIndex
    {
      SourceTableNameIndex = 0,       /*!< Column index of source table name */
      DestinationTableNameIndex = 1,  /*!< Column index of destination table name */
      TableMappingStateIndex = 2,     /*!< Column index of table mapping state */
      TableCopyProgressIndex = 3,     /*!< Column index of table copy progress */
      TableCopyStatusIndex = 4        /*!< Column index of table copy status */
    };

   private slots:

    /*! \brief Reset copy progress and status after model reset
     */
    void resetCopyStatusAndProgress();

   private:

    /*! \brief Get data about source table column
     */
    QVariant sourceTableData(int row, int role) const;

    /*! \brief Get data about destination table column
     */
    QVariant destinationTableData(int row, int role) const;

    /*! \brief Get mapping state data
     */
    QVariant tableMappingStateData(int row, int role) const;

    /*! \brief Get table mapping state text
     */
    QString tableMappingStateText(TableMapping::MappingState state) const;

    /*! \brief Get table mapping state decoration
     */
    QVariant tableMappingStateDecoration(TableMapping::MappingState state) const;

    /*! \brief Get table copy progress
     */
    QVariant tableCopyProgress(int row, int role) const;

    /*! \brief Get table copy status data
     */
    QVariant tableCopyStatusData(int row, int role) const;

    /*! \brief Get table copy status text
     */
    QString tableCopyStatusText(CopyStatus status) const;

    /*! \brief Get table copy status decoration
     */
    QVariant tableCopyStatusDecoration(CopyStatus status) const;

    std::vector<int> pvRowCopyProgress;
    std::vector<CopyStatus> pvRowCopyStatus;
  };

}}} // namespace mdt{ namespace sql{ namespace copier{

#endif // #ifndef MDT_SQL_COPIER_MAPPING_MODEL_H
