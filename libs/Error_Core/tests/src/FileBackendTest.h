/****************************************************************************
 **
 ** Copyright (C) 2011-2017 Philippe Steinmann.
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
#ifndef MDT_ERROR_FILE_BACKEND_TEST_H
#define MDT_ERROR_FILE_BACKEND_TEST_H

#include <QString>
#include <QObject>
#include <QtTest/QtTest>

class FileBackendTest : public QObject
{
 Q_OBJECT

 private slots:

  void initTestCase();
  void cleanupTestCase();

  void basicPropertyTest();
  void logErrorTest();
  void logNoErrorTest();
  void backupTest();

 private:

  static QString readFile(const QString & filePath);
};

#endif // #ifndef MDT_ERROR_FILE_BACKEND_TEST_H
