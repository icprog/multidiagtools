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
#include "mdtCsvFileParser.h"
#include "mdtCsvParserTemplate.h"
#include "mdtFileError.h"
#include <boost/spirit/include/support_multi_pass.hpp>
#include <QObject>
#include <QDir>

/*! \todo Flexibility/usage simplicity
 *
 * Currently, parser must be built with scvSettings given,
 *  and is immutable.
 *  This is a constraint.
 * F.ex. mdtCsvFileParserModel must itself manage a dynamic object for
 *  this case, and recreate the object each time user wants to open another file.
 *  And, this create 2 pointer inderections for each function call.
 * Given that mdtCsvFileParser has allready a pointer (to hide parser template),
 *  it should be responsible to manage this !
 *
 * Other solution: currently, parser grammar (in mdtCsvParserTemplate)
 *  is done in constructor, witch requires CSV settings.
 *  But, it could be done in a ueparate function ?u
 */

mdtCsvFileParser::mdtCsvFileParser()
 : pvParser(new mdtCsvParserTemplate<mdtCsvFileParserMultiPassIterator>())
{
}

mdtCsvFileParser::mdtCsvFileParser(const mdtCsvParserSettings & csvSettings)
 : mdtCsvFileParser()
//  : pvParser(new mdtCsvParserTemplate<mdtCsvFileParserMultiPassIterator>())
{
  Q_ASSERT(csvSettings.isValid());
  pvParser->setupParser(csvSettings);
}

mdtCsvFileParser::~mdtCsvFileParser()
{
}

void mdtCsvFileParser::setCsvSettings(const mdtCsvParserSettings & csvSettings)
{
  Q_ASSERT(csvSettings.isValid());
  Q_ASSERT(!pvFile.isOpen());

  pvParser->setupParser(csvSettings);
}

bool mdtCsvFileParser::openFile(const QFileInfo & fileInfo, const QByteArray & encoding)
{
  // Close possibly previously open file
  if(pvFile.isOpen()){
    closeFile();
  }
  // Open file
  pvFile.setFileName(fileInfo.absoluteFilePath());
  if(!pvFile.open(QIODevice::ReadOnly)){
    QString msg = tr("Could not open file '") + fileInfo.fileName() + tr("'\n") \
                  + tr("Directory: '") + fileInfo.dir().absolutePath() + tr("'");
    pvLastError.setError(msg, mdtError::Error);
    MDT_ERROR_SET_SRC(pvLastError, "mdtCsvFileParserIteratorSharedData");
    pvLastError.stackError(mdtFileError::fromQFileDeviceError(pvFile));
    pvLastError.commit();
    return false;
  }
  // Assign file iterator
  if(!pvFileIterator.setSource(&pvFile, encoding)){
    pvLastError = pvFileIterator.lastError();
    return false;
  }

  return true;
}

void mdtCsvFileParser::closeFile()
{
  pvFileIterator.clear();
  pvFile.close();
}

bool mdtCsvFileParser::atEnd() const
{
  return pvFileIterator.isEof();
}

mdtExpected<mdtCsvRecord> mdtCsvFileParser::readLine()
{
  Q_ASSERT_X(pvParser->isValid(), "mdtCsvFileParser", "No CSV settings set");

  mdtExpected<mdtCsvRecord> record;

  // Create multi pass iterators
  auto first = mdtCsvFileParserMultiPassIterator(pvFileIterator);
  auto last = mdtCsvFileParserMultiPassIterator(mdtCsvFileParserIterator());
  // Parse a line
  record = pvParser->readLine(first, last);
  if(!record){
    // Maybe a error on file
    if(pvFileIterator.errorOccured()){
      record.error().stackError(pvFileIterator.lastError());
    }
  }

  return record;
}

mdtExpected<mdtCsvData> mdtCsvFileParser::readAll()
{
  Q_ASSERT_X(pvParser->isValid(), "mdtCsvFileParser", "No CSV settings set");

  mdtCsvData data;

  while(!atEnd()){
    auto record = readLine();
    if(!record){
      return record.error();
    }
    data.addRecord(record.value());
  }

  return data;
}

QString mdtCsvFileParser::tr(const char* sourceText)
{
  return QObject::tr(sourceText);
}
