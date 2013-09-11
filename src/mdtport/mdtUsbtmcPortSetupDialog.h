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
#ifndef MDT_USBTMC_PORT_SETUP_DIALOG_H
#define MDT_USBTMC_PORT_SETUP_DIALOG_H

#include "mdtAbstractPortSetupDialog.h"
#include "mdtPortConfigWidget.h"
#include "mdtPortInfoCbHandler.h"
#include "mdtPortInfo.h"
#include <QDialog>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

class mdtUsbtmcPortManager;
class QWidget;
class QAbstractButton;

class mdtUsbtmcPortSetupDialog : public mdtAbstractPortSetupDialog
{
 Q_OBJECT

 public:

  mdtUsbtmcPortSetupDialog(QWidget *parent = 0);
  ~mdtUsbtmcPortSetupDialog();

 private slots:

  // Read the configuration and update GUI
  void displayConfig();

  // Read options from GUI and update configuration
  void updateConfig();

  // Close port
  void closePort();

  // Rescan
  void rescan();

  // Open port
  void openPort();

 private:

  /*! \brief Called by setPortManager()
   */
  void portManagerSet();

  /*! \brief Set the PortClosed state
   */
  void setStatePortClosed();

  /*! \brief Set the Disconnected state
   */
  void setStateDisconnected();

  /*! \brief Set the Connecting state
   */
  void setStateConnecting();

  /*! \brief Set the PortReady state
   */
  void setStatePortReady();

  /*! \brief Set the Ready state
   */
  void setStateReady();

  /*! \brief Set the Busy state
   */
  void setStateBusy();

  /*! \brief Set the Error state
   */
  void setStateError();

  /*! \brief Apply setup
   */
  bool applySetup();

  mdtPortInfoCbHandler pvPortInfoCbHandler;
  mdtPortInfo pvCurrentPortInfo;
  QPushButton *pbOpen;
  QPushButton *pbClose;
  QPushButton *pbRescan;
  QComboBox *cbPort;
  QComboBox *cbInterface;
  QWidget *pvDeviceInfoWidget;
  QLabel *lbManufacturer;
  QLabel *lbModel;
  QLabel *lbSerial;
  QLabel *lbFirmware;
};

#endif  // #ifndef MDT_USBTMC_PORT_SETUP_DIALOG_H
