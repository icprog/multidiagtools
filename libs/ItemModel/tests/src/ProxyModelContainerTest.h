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
#ifndef MDT_ITEM_MODEL_PROXY_MODEL_CONTAINER_TEST_H
#define MDT_ITEM_MODEL_PROXY_MODEL_CONTAINER_TEST_H

#include <QObject>
#include <QtTest/QtTest>

class ProxyModelContainerTest : public QObject
{
 Q_OBJECT

 private slots:

  void initTestCase();
  void cleanupTestCase();

  void initialStateTest();
  void appendTest();
  void appendThenSetSourceModelTest();
  void prependTest();
  void prependThenSetSourceModelTest();
  void searchTest();
  void removeAtTest();
  /*
   * Bug:
   * crash when removing last proxy model
   * F.example: removeAt(1) when container has 2 proxy models
   */
  void removeAtIndexBugTest();
  void searchRemoveTest();
  void searchPointerRemoveTest();
  void customContainerTest();
};

#endif // #ifndef MDT_ITEM_MODEL_PROXY_MODEL_CONTAINER_TEST_H
