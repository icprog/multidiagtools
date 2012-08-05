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
#include "mdtPortManagerTest.h"
#include "mdtPort.h"
#include "mdtPortManager.h"
#include "mdtPortConfig.h"
#include "mdtFrame.h"
#include <QTemporaryFile>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include "mdtApplication.h"

#include "linux/mdtUsbtmcPort.h"
#include "linux/mdtUsbtmcPortManager.h"

//#include <QDebug>

void mdtPortManagerTest::portTest()
{
  mdtPortManager m, m2;
  QTemporaryFile file;
  mdtPortConfig *cfg, *cfg2;
  mdtPort *port, *port2;

  // Create a temporary file
  QVERIFY(file.open());
  file.close();

  // Port setup
  cfg = new mdtPortConfig;
  cfg->setFrameType(mdtFrame::FT_ASCII);
  port = new mdtPort;
  port->setConfig(cfg);

  // Init port manager
  m.setPort(port);
  QVERIFY(m.setPortName(file.fileName()));
  QVERIFY(m.openPort());

  // start threads
  QVERIFY(m.start());
  QVERIFY(m.isRunning());

  // Check the wait function without available data
  QVERIFY(!m.waitReadenFrame());

  // Send some data
  QVERIFY(m.writeData("Test\n"));

  // We must re-open the file, else nothing will be readen
  QTest::qWait(100);
  m.closePort();
  QVERIFY(m.openPort());
  QVERIFY(m.start());
  // Get readen data
  QVERIFY(m.waitReadenFrame());
  QVERIFY(m.lastReadenFrame() == "Test");

  // Setup a second port manager
  // Port setup
  cfg2 = new mdtPortConfig;
  port2 = new mdtPort;
  port2->setConfig(cfg2);
  m2.setPort(port2);

  // Setup same port than first manager, open must fail
  QVERIFY(m2.setPortName(file.fileName()));
  QVERIFY(!m2.openPort());

  // In read only, port must be open
  ///m2.config().setReadOnly(true);
  ///QVERIFY(m2.openPort());

  // Chack that start and running flag works
  ///QVERIFY(m2.start());
  ///QVERIFY(m2.isRunning());

  qDebug() << "About to close ...";
  m.closePort();
  ///m2.closePort();
  qDebug() << "About to delete port ...";
  delete port;
  delete cfg;
  qDebug() << "About to delete port2 ...";
  delete port2;
  qDebug() << "About to delete cfg2 ...";
  delete cfg2;
  qDebug() << "End";
}

void mdtPortManagerTest::usbTmcPortTest()
{
  mdtUsbtmcPortManager m;
  QStringList ports;

  qDebug() << "* A USBTMC compatible device must be attached, else test will fail *";

  // Find attached devices
  ports = m.scan();
  QVERIFY(ports.size() > 0);

  // Init port manager
  QVERIFY(m.setPortName(ports.at(0)));
  QVERIFY(m.openPort());

  // start threads
  QVERIFY(m.start());

  // Query without answer
  QVERIFY(m.writeData("*CLS\n", false));

  // Try to query device
  for(int i=0; i<5; i++){
    // Send a command
    QVERIFY(m.writeData("*IDN?\n", true));
    // Wait and read answer
    QVERIFY(m.lastReadenFrame().size() <= 0);
    QVERIFY(m.waitReadenFrame());
    QVERIFY(m.lastReadenFrame().size() > 0);
  }
}


int main(int argc, char **argv)
{
  mdtApplication app(argc, argv);
  mdtPortManagerTest test;

  if(!app.init()){
    return 1;
  }

  return QTest::qExec(&test, argc, argv);
}
