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
#include "LibraryTest.h"
#include "Mdt/DeployUtils/LibraryVersion.h"
#include "Mdt/DeployUtils/LibraryName.h"
#include "Mdt/DeployUtils/LibraryInfo.h"
#include "Mdt/DeployUtils/Library.h"
#include <QtGlobal>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QFileInfo>

using namespace Mdt::DeployUtils;

void LibraryTest::initTestCase()
{
}

void LibraryTest::cleanupTestCase()
{
}

/*
 * Tests
 */

void LibraryTest::libraryVersionTest()
{
  /*
   * Default constructed
   */
  LibraryVersion v0;
  QCOMPARE(v0.versionMajor(), (qint8)-1);
  QCOMPARE(v0.versionMinor(), (qint8)-1);
  QCOMPARE(v0.versionPatch(), (qint8)-1);
  QVERIFY(v0.isNull());
  QVERIFY(v0.toString().isEmpty());
  /*
   * Some non null versions
   */
  LibraryVersion v1(1);
  QCOMPARE(v1.versionMajor(), (qint8)1);
  QCOMPARE(v1.versionMinor(), (qint8)-1);
  QCOMPARE(v1.versionPatch(), (qint8)-1);
  QVERIFY(!v1.isNull());
  QCOMPARE(v1.toString(), QString("1"));
  LibraryVersion v12(1,2);
  QCOMPARE(v12.versionMajor(), (qint8)1);
  QCOMPARE(v12.versionMinor(), (qint8)2);
  QCOMPARE(v12.versionPatch(), (qint8)-1);
  QVERIFY(!v12.isNull());
  QCOMPARE(v12.toString(), QString("1.2"));
  LibraryVersion v123(1,2,3);
  QCOMPARE(v123.versionMajor(), (qint8)1);
  QCOMPARE(v123.versionMinor(), (qint8)2);
  QCOMPARE(v123.versionPatch(), (qint8)3);
  QVERIFY(!v123.isNull());
  QCOMPARE(v123.toString(), QString("1.2.3"));
}

void LibraryTest::libraryVersionSetStrTest()
{
  QFETCH(QString, versionString);
  QFETCH(QString, expectedVersionString);
  QFETCH(bool, expectedIsNull);

  LibraryVersion v(versionString);
  QCOMPARE(v.isNull(), expectedIsNull);
  QCOMPARE(v.toString(), expectedVersionString);
}

void LibraryTest::libraryVersionSetStrTest_data()
{
  QTest::addColumn<QString>("versionString");
  QTest::addColumn<QString>("expectedVersionString");
  QTest::addColumn<bool>("expectedIsNull");

  const bool Null = true;
  const bool NotNull = false;

  QTest::newRow("") << "" << "" << Null;
  QTest::newRow("1") << "1" << "1" << NotNull;
  QTest::newRow("1.2") << "1.2" << "1.2" << NotNull;
  QTest::newRow("1.2.3") << "1.2.3" << "1.2.3" << NotNull;
  QTest::newRow("0.0.0") << "0.0.0" << "0.0.0" << NotNull;
  QTest::newRow("a") << "a" << "" << Null;
  QTest::newRow("a.b") << "a.b" << "" << Null;
  QTest::newRow("a.b.c") << "a.b.c" << "" << Null;
}

void LibraryTest::libraryVersionBenchmark()
{
  QFETCH(QString, versionString);
  QFETCH(QString, expectedVersionString);

  QString outVersionString;
  QBENCHMARK{
    LibraryVersion v(versionString);
    outVersionString = v.toString();
  }
  QCOMPARE(outVersionString, expectedVersionString);
}

void LibraryTest::libraryVersionBenchmark_data()
{
  QTest::addColumn<QString>("versionString");
  QTest::addColumn<QString>("expectedVersionString");

  QTest::newRow("1") << "1" << "1";
  QTest::newRow("1.2") << "1.2" << "1.2";
  QTest::newRow("1.2.3") << "1.2.3" << "1.2.3";
}

void LibraryTest::libraryNameFromStrTest()
{
  QFETCH(QString, inputName);
  QFETCH(QString, expectedName);
  QFETCH(QString, expectedVersion);
  QFETCH(bool, expectedIsNull);

  LibraryName libName(inputName);
  QCOMPARE(libName.isNull(), expectedIsNull);
  QCOMPARE(libName.name(), expectedName);
  QCOMPARE(libName.version().toString(), expectedVersion);
}

void LibraryTest::libraryNameFromStrTest_data()
{
  QTest::addColumn<QString>("inputName");
  QTest::addColumn<QString>("expectedName");
  QTest::addColumn<QString>("expectedVersion");
  QTest::addColumn<bool>("expectedIsNull");

  const bool Null = true;
  const bool NotNull = false;

  QTest::newRow("") << "" << "" << "" << Null;
  QTest::newRow("Qt5Core") << "Qt5Core" << "Qt5Core" << "" << NotNull;
  QTest::newRow("libQt5Core") << "libQt5Core" << "Qt5Core" << "" << NotNull;
  QTest::newRow("libQt5Core.so") << "libQt5Core.so" << "Qt5Core" << "" << NotNull;
  QTest::newRow("libQt5Core.so.5") << "libQt5Core.so.5" << "Qt5Core" << "5" << NotNull;
  QTest::newRow("libQt5Core.so.5.6") << "libQt5Core.so.5.6" << "Qt5Core" << "5.6" << NotNull;
  QTest::newRow("libQt5Core.so.5.6.2") << "libQt5Core.so.5.6.2" << "Qt5Core" << "5.6.2" << NotNull;
  QTest::newRow("Qt5Core.dll") << "Qt5Core.dll" << "Qt5Core" << "" << NotNull;
  QTest::newRow("libpng12.so") << "libpng12.so" << "png12" << "" << NotNull;
  QTest::newRow("libpng12.so.0") << "libpng12.so.0" << "png12" << "0" << NotNull;
  QTest::newRow("libpng12.so.0") << "libpng12.so.0" << "png12" << "0" << NotNull;
}

void LibraryTest::libraryNameFromStrBenchmark()
{
  QFETCH(QString, inputName);
  QFETCH(QString, expectedName);
  QFETCH(QString, expectedVersion);

  QString libNameString;
  LibraryVersion libVersion;
  QBENCHMARK{
    LibraryName libName(inputName);
    libNameString = libName.name();
    libVersion = libName.version();
  }
  QCOMPARE(libNameString, expectedName);
  QCOMPARE(libVersion.toString(), expectedVersion);
}

void LibraryTest::libraryNameFromStrBenchmark_data()
{
  QTest::addColumn<QString>("inputName");
  QTest::addColumn<QString>("expectedName");
  QTest::addColumn<QString>("expectedVersion");

  QTest::newRow("Qt5Core") << "Qt5Core" << "Qt5Core" << "";
  QTest::newRow("libQt5Core") << "libQt5Core" << "Qt5Core" << "";
  QTest::newRow("libQt5Core.so") << "libQt5Core.so" << "Qt5Core" << "";
  QTest::newRow("libQt5Core.so.5.6.2") << "libQt5Core.so.5.6.2" << "Qt5Core" << "5.6.2";
}

void LibraryTest::libraryNameToFullNameLinuxTest()
{
  /*
   * No version
   */
  LibraryName name0("Qt5Core");
  QCOMPARE(name0.toFullNameLinux(), QString("libQt5Core.so"));
  /*
   * With version
   */
  LibraryName name123("libQt5Core.so.1.2.3");
  QCOMPARE(name123.toFullNameLinux(), QString("libQt5Core.so.1.2.3"));
}

void LibraryTest::libraryInfoTest()
{
  LibraryInfo li;
  QVERIFY(li.isNull());
  li.setAbsoluteFilePath("/tmp/libA.so");
  QVERIFY(!li.isNull());
  QCOMPARE(li.absoluteFilePath(), QString("/tmp/libA.so"));
}

void LibraryTest::searchLibraryTest()
{
  QFETCH(QString, libraryFilePathToCreate);
  QFETCH(QString, name);
  QFETCH(PathList, pathList);
  QFETCH(int, searchInSystemPaths);
  QFETCH(bool, expectedOk);
  /*
   * Create a file if it not exists on the system
   */
  QTemporaryDir testRootDirectory;
  if(!libraryFilePathToCreate.isEmpty()){
    QVERIFY(createFileInTemporaryDirectory(testRootDirectory, libraryFilePathToCreate));
    const PathList pathListCopy = pathList;
    pathList = PathList();
    for(const auto path : pathListCopy){
      pathList.appendPath(testRootDirectory.path() + path);
    }
  }
  /*
   * Check
   */
  Library library;
  QCOMPARE(library.findLibrary(name, pathList, static_cast<Library::SearchInSystemPaths>(searchInSystemPaths)), expectedOk);
}

void LibraryTest::searchLibraryTest_data()
{
  QTest::addColumn<QString>("libraryFilePathToCreate");
  QTest::addColumn<QString>("name");
  QTest::addColumn<PathList>("pathList");
  QTest::addColumn<int>("searchInSystemPaths");
  QTest::addColumn<bool>("expectedOk");

  const int IncludeSystemPaths = Library::IncludeSystemPaths;
  const int ExcludeSystemPaths = Library::ExcludeSystemPaths;
  const bool Exists = true;
  const bool NotExists = false;

#ifdef Q_OS_UNIX

  QTest::newRow("libc.so|c||")
    << "" << "c" << PathList{} << IncludeSystemPaths << Exists;

  QTest::newRow("libc.so|c|/var|")
    << "" << "c" << PathList{"/var"} << ExcludeSystemPaths << NotExists;

  QTest::newRow("libc.so|c|/non/existing/path|")
    << "" << "c" << PathList{"/non/existing/path"} << ExcludeSystemPaths << NotExists;

  QTest::newRow("libNoExistingLib|NoExistingLib||")
    << "" << "NoExistingLib" << PathList{} << IncludeSystemPaths << NotExists;

  QTest::newRow("libMyLib.so|MyLib|/opt/MyProject/lib|")
    << "/opt/MyProject/lib/libMyLib.so" << "c" << PathList{"/opt/MyProject/lib"} << IncludeSystemPaths << Exists;


#endif // #ifdef Q_OS_UNIX
}

void LibraryTest::searchLibraryBenchmark()
{
  QFETCH(QString, name);
  QFETCH(PathList, pathList);
  QFETCH(int, searchInSystemPaths);
  QFETCH(bool, expectedOk);

  Library library;
  QBENCHMARK{
    QCOMPARE(library.findLibrary(name, pathList, static_cast<Library::SearchInSystemPaths>(searchInSystemPaths)), expectedOk);
  }
}

void LibraryTest::searchLibraryBenchmark_data()
{
  QTest::addColumn<QString>("name");
  QTest::addColumn<PathList>("pathList");
  QTest::addColumn<int>("searchInSystemPaths");
  QTest::addColumn<bool>("expectedOk");

  const int IncludeSystemPaths = Library::IncludeSystemPaths;
  const int ExcludeSystemPaths = Library::ExcludeSystemPaths;
  const bool Exists = true;
  const bool NotExists = false;

#ifdef Q_OS_UNIX

  QTest::newRow("libc.so|c||")
    << "c" << PathList{} << IncludeSystemPaths << Exists;

  QTest::newRow("libc.so|c|/opt/lib44|")
    << "c" << PathList{"/opt/lib44"} << ExcludeSystemPaths << NotExists;

#endif // #ifdef Q_OS_UNIX

}


/*
 * Main
 */

int main(int argc, char **argv)
{
  Mdt::Application app(argc, argv);
  LibraryTest test;

  if(!app.init()){
    return 1;
  }
//   app.debugEnvironnement();

  return QTest::qExec(&test, argc, argv);
}
