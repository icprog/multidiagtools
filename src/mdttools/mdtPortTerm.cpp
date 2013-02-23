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
#include "mdtPortTerm.h"
#include "mdtApplication.h"
#include "mdtPortInfo.h"
#include "mdtUsbtmcPortSetupDialog.h"

#include "mdtDevice.h"
#include "mdtDeviceStatusWidget.h"
#include "mdtAbstractPort.h"

#include <QHBoxLayout>
#include <QAction>
#include <QList>
#include <QLabel>
#include <QTimer>
#include <QApplication>

#include <QDebug>

mdtPortTerm::mdtPortTerm(QWidget *parent)
 : QMainWindow(parent)
{
  setupUi(this);
  // Status bar
  pvStatusWidget = new mdtDeviceStatusWidget;
  statusBar()->addWidget(pvStatusWidget);
  // Serial port members
  pvSerialPortManager = 0;
  pvSerialPortCtlWidget = 0;
  // USBTMC port members
  pvUsbtmcPortManager = 0;
  // Current port manager
  pvCurrentPortManager = 0;
  // Flags
  pvRunning = false;

  connect(pbSendCmd, SIGNAL(clicked()), this, SLOT(sendCmd()));
  connect(leCmd, SIGNAL(returnPressed()), this, SLOT(sendCmd()));
  leCmd->setFocus();

  // Actions
  connect(action_Setup, SIGNAL(triggered()), this, SLOT(portSetup()));
  pvLanguageActionGroup = 0;
  // Port selection actions
  pvPortSelectActionGroup = new QActionGroup(this);
  pvPortSelectActionGroup->addAction(action_SerialPort);
  pvPortSelectActionGroup->addAction(action_UsbTmcPort);
  connect(pvPortSelectActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(selectPortType(QAction*)));
  action_SerialPort->setChecked(true);
  attachToSerialPort();
}

mdtPortTerm::~mdtPortTerm()
{
  detachFromPorts();
}

void mdtPortTerm::setAvailableTranslations(QMap<QString, QString> &avaliableTranslations, const QString &currentTranslationKey)
{
  QMap<QString, QString>::const_iterator it = avaliableTranslations.constBegin();

  // Create a action group
  if(pvLanguageActionGroup == 0){
    pvLanguageActionGroup = new QActionGroup(this);
    connect(pvLanguageActionGroup, SIGNAL(triggered(QAction*)), mdtApp, SLOT(changeLanguage(QAction*)));
  }
  // Travel available translation and add actions to menu + group
  while(it != avaliableTranslations.constEnd()){
    QAction *action = new QAction(it.value(), this);
    action->setCheckable(true);
    action->setData(it.key());
    if(it.key() == currentTranslationKey){
      action->setChecked(true);
    }
    menu_Language->addAction(action);
    pvLanguageActionGroup->addAction(action);
    it++;
  }
}

void mdtPortTerm::appendReadenData(QByteArray data)
{
  teTerm->append(data);
}

void mdtPortTerm::appendReadenData(mdtPortTransaction transaction)
{
  teTerm->append(transaction.data());
}

void mdtPortTerm::sendCmd()
{
  QString cmd;
  bool cmdIsQuery = false;

  if(!pvRunning){
    return;
  }
  if(pvCurrentPortManager == 0){
    qDebug() << "TERM: err, pvCurrentPortManager == 0";
    return;
  }
  if(leCmd->text().size() < 1){
    return;
  }
  cmd = leCmd->text();
  if(cmd.trimmed().right(1) == "?"){
    cmdIsQuery = true;
  }
  cmd.append('\n');
  //cmd.append((char)0x04);
  /// \todo Error handling !
  // Wait until write is possible and write
  if(!pvCurrentPortManager->waitOnWriteReady()){
    qDebug() << "TERM: cannot write for the moment";
    return;
  }
  if(pvCurrentPortManager->writeData(cmd.toAscii()) < 0){
    qDebug() << "TERM: write error";
    return;
  }
  // If we have a query for USBTMC port, send the read request
  if((pvCurrentPortManager == pvUsbtmcPortManager)&&(cmdIsQuery)){
    if(!pvCurrentPortManager->waitOnWriteReady()){
      qDebug() << "TERM: cannot write for the moment";
      return;
    }
    if(pvUsbtmcPortManager->sendReadRequest(false) < 0){
      qDebug() << "TERM: cannot send read request";
      return;
    }
  }
  leCmd->clear();
}

void mdtPortTerm::on_pbSendCmdAbort_clicked()
{
  if(pvCurrentPortManager != 0){
    pvCurrentPortManager->abort();
  }
  leCmd->clear();
}

void mdtPortTerm::on_pbClearTerm_clicked()
{
  teTerm->clear();
}

void mdtPortTerm::retranslate()
{
  retranslateUi(this);
}

void mdtPortTerm::attachToSerialPort()
{
  QList<mdtPortInfo*> ports;

  detachFromPorts();
  // Create objects
  pvSerialPortManager = new mdtSerialPortManager;
  pvSerialPortCtlWidget = new mdtSerialPortCtlWidget;
  pvCurrentPortManager = pvSerialPortManager;
  // Status widget
  pvStatusWidget->enableTxRxLeds(pvSerialPortManager->writeThread(), pvSerialPortManager->readThread());
  // Control widget
  pvStatusWidget->addCustomWidget(pvSerialPortCtlWidget);
  pvSerialPortCtlWidget->makeConnections(pvSerialPortManager);
  // Make connections
  connect(pvSerialPortManager, SIGNAL(errorStateChanged(int)), this, SLOT(setStateFromPortError(int)));
  connect(pvSerialPortManager, SIGNAL(newReadenFrame(QByteArray)), this, SLOT(appendReadenData(QByteArray)));
  // Try to open first port
  ports = pvSerialPortManager->scan();
  if(ports.size() < 1){
    setStateError("No free serial port found");
    return;
  }
  pvSerialPortManager->setPortInfo(*ports.at(0));
  if(!pvSerialPortManager->openPort()){
    setStateError(tr("Cannot open port ") + ports.at(0)->portName());
    qDeleteAll(ports);
    return;
  }
  if(pvSerialPortManager->start()){
    setStateRunning(tr("Port open: ") + pvCurrentPortManager->portInfo().displayText());
    QTimer::singleShot(5000, this, SLOT(setStateRunning()));
  }
  qDeleteAll(ports);
}

void mdtPortTerm::detachFromSerialPort()
{
  if(pvSerialPortManager != 0){
    disconnect(pvSerialPortManager, SIGNAL(newReadenFrame(QByteArray)), this, SLOT(appendReadenData(QByteArray)));
    disconnect(pvSerialPortManager, SIGNAL(errorStateChanged(int)), this, SLOT(setStateFromPortError(int)));
    // Ctl widget
    pvStatusWidget->removeCustomWidget();
    delete pvSerialPortCtlWidget;
    pvSerialPortCtlWidget = 0;
    // Status widget
    pvStatusWidget->disableTxRxLeds();
    // Free ..
    delete pvSerialPortManager;
    pvSerialPortManager = 0;
  }
  pvCurrentPortManager = 0;
  setStateStopped();
}

void mdtPortTerm::portSetup()
{
  Q_ASSERT(pvCurrentPortManager != 0);

  // Show the correct setup dialog
  if(pvSerialPortManager != 0){
    mdtSerialPortSetupDialog d(this);
    d.setPortManager(pvSerialPortManager);
    d.exec();
  }else if(pvUsbtmcPortManager != 0){
    mdtUsbtmcPortSetupDialog d(this);
    d.setPortManager(pvUsbtmcPortManager);
    d.exec();
  }
  // If port is running, enable terminal
  if(pvCurrentPortManager->isRunning()){
    setStateRunning(tr("Port open: ") + pvCurrentPortManager->portInfo().displayText());
    QTimer::singleShot(5000, this, SLOT(setStateRunning()));
  }else{
    setStateStopped();
  }
}

void mdtPortTerm::attachToUsbtmcPort()
{
  QList<mdtPortInfo*> ports;

  detachFromPorts();
  // Create objects
  pvUsbtmcPortManager = new mdtUsbtmcPortManager;
  pvCurrentPortManager = pvUsbtmcPortManager;
  // Make connections
  connect(pvUsbtmcPortManager, SIGNAL(errorStateChanged(int)), this, SLOT(setStateFromPortError(int)));
  connect(pvUsbtmcPortManager, SIGNAL(newReadenFrame(mdtPortTransaction)), this, SLOT(appendReadenData(mdtPortTransaction)));
  // Try to open first port
  ports = pvUsbtmcPortManager->scan();
  if(ports.size() < 1){
    setStateError(tr("No USBTMC device found"));
    return;
  }
  pvUsbtmcPortManager->setPortInfo(*ports.at(0));
  if(!pvUsbtmcPortManager->openPort()){
    setStateError(tr("Cannot open port ") + ports.at(0)->portName());
    qDeleteAll(ports);
    return;
  }
  if(pvUsbtmcPortManager->start()){
    setStateRunning(tr("Port open: ") + pvCurrentPortManager->portInfo().displayText());
    QTimer::singleShot(5000, this, SLOT(setStateRunning()));
  }
  qDeleteAll(ports);
}

void mdtPortTerm::detachFromUsbtmcPort()
{
  if(pvUsbtmcPortManager != 0){
    disconnect(pvUsbtmcPortManager, SIGNAL(newReadenFrame(mdtPortTransaction)), this, SLOT(appendReadenData(mdtPortTransaction)));
    pvUsbtmcPortManager->closePort();
    delete pvUsbtmcPortManager;
    pvUsbtmcPortManager = 0;
  }
  pvCurrentPortManager = 0;
}

void mdtPortTerm::selectPortType(QAction*)
{
  if(action_SerialPort->isChecked()){
    if(pvSerialPortManager == 0){
      attachToSerialPort();
    }
  }else if(action_UsbTmcPort->isChecked()){
    if(pvUsbtmcPortManager == 0){
      attachToUsbtmcPort();
    }
  }
}

void mdtPortTerm::detachFromPorts()
{
  detachFromSerialPort();
  detachFromUsbtmcPort();
  setStateStopped();
}

void mdtPortTerm::setStateFromPortError(int error)
{
  // It can happen that some errors are queued
  if(pvCurrentPortManager == 0){
    qApp->processEvents();
  }

  switch(error){
    case mdtAbstractPort::NoError:
      setStateRunning();
      break;
    case mdtAbstractPort::ReadTimeout:
      pvStatusWidget->setState(mdtDevice::Busy, tr("Read timeout occured"), "");
      QTimer::singleShot(5000, this, SLOT(setStateRunning()));
      break;
    case mdtAbstractPort::ReadCanceled:
      pvStatusWidget->setState(mdtDevice::Busy, tr("Read error occured"), "");
      QTimer::singleShot(5000, this, SLOT(setStateRunning()));
      break;
    case mdtAbstractPort::Disconnected:
      setStateStopped();
      ///QTimer::singleShot(5000, this, SLOT(setStateStopped()));  /// \todo bricolage ...
      break;
    case mdtAbstractPort::Connecting:
      setStateStopped(tr("Connecting ..."));
      break;
    case mdtAbstractPort::MessageInTimeout:
      qDebug() << "mdtPortTerm::setStateFromPortError(): message IN timeout , ignoring ...";
      break;
    default:
      ///pvStatusWidget->setState(mdtDevice::Unknown, "Received unknown error, number " + QString::number(error), "");
      pvStatusWidget->setState(mdtDevice::Error, "Received unknown error, number " + QString::number(error), "");
  }
}

void mdtPortTerm::setStateRunning(const QString &msg)
{
  pvRunning = true;
  pbSendCmd->setEnabled(true);
  pvStatusWidget->setState(mdtDevice::Ready, msg, "");
}

void mdtPortTerm::setStateStopped(const QString &msg)
{
  pvRunning = false;
  pbSendCmd->setEnabled(false);
  pvStatusWidget->setState(mdtDevice::Disconnected, msg, "");
}

void mdtPortTerm::setStateError(const QString &msg)
{
  pvRunning = false;
  pbSendCmd->setEnabled(false);
  ///pvStatusWidget->setState(mdtDevice::Unknown, msg, "");
  pvStatusWidget->setState(mdtDevice::Error, msg, "");
}
