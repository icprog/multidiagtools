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
#ifndef MDT_CSV_FILE_PARSER_SETTINGS_DIALOG_H
#define MDT_CSV_FILE_PARSER_SETTINGS_DIALOG_H

#include "mdtCsvSettings.h"
#include "ui_mdtCsvFileParserSettingsDialog.h"
#include <QDialog>
#include <QString>
#include <QByteArray>
// #include <QStateMachine>

class QWidget;
class mdtCsvFileParserModel;
namespace mdt{ namespace csv{
  class RecordFormatSetupWidget;
}}

/*! \brief CSV file generator settings dialog
 */
class mdtCsvFileParserSettingsDialog : public QDialog, Ui::mdtCsvFileParserSettingsDialog
{
 Q_OBJECT

 public:

  /*! \brief Constructor
   */
  mdtCsvFileParserSettingsDialog(QWidget *parent = nullptr);

  /*! \brief Copy disabled
   */
  mdtCsvFileParserSettingsDialog(const mdtCsvFileParserSettingsDialog &) = delete;

  /*! \brief Copy disabled
   */
  mdtCsvFileParserSettingsDialog & operator=(const mdtCsvFileParserSettingsDialog &) = delete;

  /*! \brief Set file settings
   */
  void setFileSettings(const QString & path, const QByteArray & encoding);

  /*! \brief Get file path
   */
  QString filePath() const;

  /*! \brief Get file encoding
   */
  QByteArray fileEncoding() const;

  /*! \brief Set CSV settings
   */
  void setCsvSettings(const mdtCsvParserSettings & settings);

  /*! \brief Get CSV settings
   */
  mdtCsvParserSettings getCsvSettings() const;

 private slots:

  /*! \brief Actions when file settings changed
   */
  void onFileSettingsChanged(const QString & path, const QByteArray & encoding);

  /*! \brief Actions when CSV settings changed
   */
  void onCsvSettingsChanged(const mdtCsvParserSettings & settings);

  /*! \brief Actions when format settings changed
   */
//   void onFieldTypeChanged(int fieldIndex, int type);

//   void onStateIdleEntered();
//   void onStateParsingFileEntered();

 private:

  /*! \brief Set controls enabled
   */
  void setControlsEnabled(bool enable);

  /*! \brief Resize view to contents
   */
  void resizeViewToContents();

  /*! \brief Setup state machine
   */
  void setupStateMachine();

//   mdt::csv::RecordFormatSetupWidget *pvRecordFormatWidget;
  mdtCsvFileParserModel *pvDataPreviewModel;
//   QStateMachine pvStateMachine;
};

#endif // #ifndef MDT_CSV_FILE_PARSER_SETTINGS_DIALOG_H
