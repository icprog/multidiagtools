/****************************************************************************
 **
 ** Copyright (C) 2011-2012 Philippe Steinmann.
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
#include "mdtSerialPortManager.h"
#include "mdtSerialPortConfig.h"
#include "mdtError.h"
#include "mdtAlgorithms.h"
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <typeinfo>

#include <QDebug>

mdtSerialPortManager::mdtSerialPortManager(QObject *parent)
 : mdtPortManager(parent)
{
  // Port setup
  mdtSerialPortConfig *config = new mdtSerialPortConfig;
  mdtSerialPort *port = new mdtSerialPort;
  port->setConfig(config);
  setPort(port);

  // Threads setup
  pvWriteThread = new mdtPortWriteThread;
  pvReadThread = new mdtPortReadThread;
  pvCtlThread = new mdtSerialPortCtlThread;
  addThread(pvWriteThread);
  addThread(pvReadThread);
  addThread(pvCtlThread);
}

mdtSerialPortManager::~mdtSerialPortManager()
{
  Q_ASSERT(pvPort != 0);

  delete &pvPort->config();
  detachPort(true, true);
}

QStringList mdtSerialPortManager::scan()
{
  Q_ASSERT(!isRunning());

  mdtSerialPort *port;
  QStringList portNames;
  QStringList availablePorts;

#ifdef Q_OS_UNIX
  QFileInfoList filesInfo;
  QDir dir;
  QStringList extList;

  // We only need system files
  dir.setFilter(QDir::Files | QDir::System);
  // Default Linux interfaces
  extList << "ttyS*";
  // USB interfaces
  extList << "ttyUSB*";
  // Moxa USB interfaces
  extList << "ttyMXUSB*";
  // Moxa PCI interfaces
  extList << "ttyMI*";
  // CDC ACM (USB Modems)
  extList << "ttyACM*";
  // Common device directory
  dir.setPath("/dev");
  if(!dir.exists()){
    mdtError e(MDT_UNDEFINED_ERROR, "directory '" + dir.dirName() + "' not exists", mdtError::Error);
    MDT_ERROR_SET_SRC(e, "mdtSerialPortManagerPosix");
    e.commit();
    return availablePorts;
  }
  dir.setNameFilters(extList);
  // Gest list of found files
  filesInfo = dir.entryInfoList();
  // Construct the list of absolute file paths
  for(int i=0; i<filesInfo.size(); ++i){
    portNames << filesInfo.at(i).absoluteFilePath();
  }
  // Sort the list
  portNames = mdtAlgorithms::sortStringListWithNumericEnd(portNames);
#endif  // #ifdef Q_OS_UNIX

#ifdef Q_OS_WIN
  QString str;

  // I found nothing better than this ..
  for(int i=1; i<=20; i++){
    str = QString("COM%1").arg(i);
    portNames << str;
  }
#endif  // #ifdef Q_OS_WIN

  // For each port name, try to open the port and get some attributes (to see if it really exists)
  port = new mdtSerialPort;
  for(int i=0; i<portNames.size(); ++i){
    // Try to open port and get UART type
    Q_ASSERT(port != 0);
    port->setPortName(portNames.at(i));
    qDebug() << "Scan, trying " << portNames.at(i) << " ...";
    if(port->open() == mdtAbstractPort::NoError){
      qDebug() << "Scan, port " << portNames.at(i) << " is open";
      if(port->uartType() != mdtAbstractSerialPort::UT_UNKNOW){
        availablePorts.append(portNames.at(i));
        qDebug() << "mdtSerialPortManager::scan(): adding port " << port->portName() << " to list";
      }
      qDebug() << "Scan, closing port " << portNames.at(i) << " ...";
      port->close();
    }
  }
  delete port;

  return availablePorts;
}

mdtSerialPortConfig &mdtSerialPortManager::config()
{
  Q_ASSERT(pvPort != 0);

  // We must typecast to the mdtSerialPort* classes
  mdtSerialPort *p = dynamic_cast<mdtSerialPort*>(pvPort);
  Q_ASSERT(p != 0);

  return p->config();
}

mdtAbstractSerialPort &mdtSerialPortManager::port()
{
  Q_ASSERT(pvPort != 0);

  mdtAbstractSerialPort *sp = dynamic_cast<mdtAbstractSerialPort*>(pvPort);
  Q_ASSERT(sp != 0);

  return *sp;
}

mdtPortWriteThread *mdtSerialPortManager::writeThread()
{
  Q_ASSERT(pvWriteThread != 0);

  return pvWriteThread;
}

mdtPortReadThread *mdtSerialPortManager::readThread()
{
  Q_ASSERT(pvReadThread != 0);

  return pvReadThread;
}

mdtSerialPortCtlThread *mdtSerialPortManager::ctlThread()
{
  Q_ASSERT(pvCtlThread != 0);

  return pvCtlThread;
}
