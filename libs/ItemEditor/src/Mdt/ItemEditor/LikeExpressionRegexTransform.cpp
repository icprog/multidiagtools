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
#include "LikeExpressionRegexTransform.h"
#include "LikeExpression.h"
#include <QRegularExpression>
#include <QLatin1String>
#include <QLatin1Char>
#include <QChar>
#include <QStringBuilder>

#include <algorithm>

#include <QDebug>

namespace Mdt{ namespace ItemEditor{

// QString LikeExpressionRegexTransform::getRegexPattern(const LikeExpression & expr)
// {
//   QString resultPattern;
//   QString pattern = expr.expression();
//   QRegularExpression regex;
// 
//   // Replace regular expression metacharacters and abbreviations
//   pattern.replace(QLatin1String("{"), QLatin1String("\\{"));
//   pattern.replace(QLatin1String("}"), QLatin1String("\\}"));
//   pattern.replace(QLatin1String("["), QLatin1String("\\["));
//   pattern.replace(QLatin1String("]"), QLatin1String("\\]"));
//   pattern.replace(QLatin1String("("), QLatin1String("\\("));
//   pattern.replace(QLatin1String(")"), QLatin1String("\\)"));
//   pattern.replace(QLatin1String("+"), QLatin1String("\\+"));
//   pattern.replace(QLatin1String("|"), QLatin1String("\\|"));
//   pattern.replace(QLatin1String("^"), QLatin1String("\\^"));
//   pattern.replace(QLatin1String("$"), QLatin1String("\\$"));
//   pattern.replace(QLatin1String("."), QLatin1String("\\."));
//   pattern.replace(QLatin1String("\\d"), QLatin1String("\\\\d"));
//   pattern.replace(QLatin1String("\\D"), QLatin1String("\\\\D"));
//   pattern.replace(QLatin1String("\\s"), QLatin1String("\\\\s"));
//   pattern.replace(QLatin1String("\\S"), QLatin1String("\\\\S"));
//   pattern.replace(QLatin1String("\\w"), QLatin1String("\\\\w"));
//   pattern.replace(QLatin1String("\\W"), QLatin1String("\\\\W"));
//   pattern.replace(QLatin1String("\\N"), QLatin1String("\\\\N"));
// 
//   /// \todo Comment
//   //regex.setPattern("[^\\\\]\\?");
//   regex.setPattern("\\?");
// //   qDebug() << "Error: " << regex.errorString();
//   Q_ASSERT(regex.isValid());
//   pattern.replace(regex, QLatin1String("."));
// 
//   /// \todo Comment
//   regex.setPattern("\\_");
//   Q_ASSERT(regex.isValid());
//   pattern.replace(regex, QLatin1String("."));
// 
//   /// \todo Comment
//   regex.setPattern("\\*");
//   Q_ASSERT(regex.isValid());
//   pattern.replace(regex, QLatin1String(".*"));
// 
//   /// \todo Comment
//   regex.setPattern("\\%");
//   Q_ASSERT(regex.isValid());
//   pattern.replace(regex, QLatin1String(".*"));
// 
//   resultPattern = QLatin1Char('^') % pattern % QLatin1Char('$');
// 
//   return resultPattern;
// }

QString LikeExpressionRegexTransform::getRegexPattern(const LikeExpression & expr)
{
  QString pattern = expr.expression();

  escapeRegexMetacharacters(pattern);
  replaceWildcards(pattern);
//   replaceWildcard(pattern, '?', QLatin1String("."));
//   replaceWildcard(pattern, '_', QLatin1String("."));
//   replaceWildcard(pattern, '%', QLatin1String(".*"));
//   replaceWildcard(pattern, '*', QLatin1String(".*"));
  pattern = '^' % pattern % '$';

  return pattern;
}

void LikeExpressionRegexTransform::escapeRegexMetacharacters(QString & str)
{
  escapeBackslash(str);
  str.replace('{', QLatin1String("\\{"));
  str.replace('}', QLatin1String("\\}"));
  str.replace('[', QLatin1String("\\["));
  str.replace(']', QLatin1String("\\]"));
  str.replace('(', QLatin1String("\\("));
  str.replace(')', QLatin1String("\\)"));
  str.replace('^', QLatin1String("\\^"));
  str.replace('$', QLatin1String("\\$"));
  str.replace('.', QLatin1String("\\."));
  str.replace('+', QLatin1String("\\+"));
  str.replace('|', QLatin1String("\\|"));
}

void LikeExpressionRegexTransform::escapeBackslash(QString & str)
{
  if(str.isEmpty()){
    return;
  }
  int i = str.indexOf('\\');
  while( (i > -1) && (i < str.size()) ){
    /*
     * If we matched '\' at last char, we never have a wildcard after -> escape.
     * Else, we have to check if following char is a wildcard
     */
    if( i == (str.size()-1) ){
      str.insert(i, '\\');
    }else{
      Q_ASSERT( i < (str.size()-1) );
      if( !isTokenWildcard(str.at(i+1)) ){
        str.insert(i, '\\');
      }
    }
    /*
     * Increment and find next match
     */
    i += 2;
    if(i < str.size()){
      i = str.indexOf('\\', i);
    }
  }
}

// void LikeExpressionRegexTransform::replaceWildcard(QString & str, const QChar & wildcard, const QString & regexMeta)
// {
//   qDebug() << "str: " << str << " , wildcard: " << wildcard << " , regexMeta: " << regexMeta;
// 
//   if(str.isEmpty()){
//     return;
//   }
// //   if( (str.size() == 1) && (str.at(0) == wildcard) ){
// //     qDebug() << " - replace.";
// //     return;
// //   }
//   const int n = regexMeta.size();
//   Q_ASSERT(n > 0);
//   int i = str.indexOf(wildcard);
//   while( (i > -1) && (i < str.size()) ){
//     qDebug() << " - i: " << i;
//     /*
//      * If wildcard is found at first position in str, it is cannot be escaped.
//      * Else, we must check:
//      *  - If found wildcard is escaped, don't replace
//      *  - 
//      */
//     if(i == 0){
//       qDebug() << " - replace.... c[0]: " << str.at(0);
//       str.replace(0, 1, regexMeta);
//       i += n-1;
//     }else if( (str.at(i) == wildcard) && (str.at(i-1) != '\\') ){
//       qDebug() << " - replace.... i: " << i << "c[i]: " << str.at(i) << " , c[i-1]: " << str.at(i-1);
//       str.replace(i, 1, regexMeta);
//       i += n-1;
//     }
//     
//     
//     ++i;
//     if(i < str.size()){
//       i = str.indexOf(wildcard, i);
//     }
//   }
//   
// //   Q_ASSERT(isTokenWildcard(wildcard));
// // 
// //   
// // 
// //   QRegularExpression regex;
// //   QString pattern = QLatin1String("[^\\\\]\\") + QChar(wildcard);
// //   qDebug() << " - pattern: " << pattern;
// // //   regex.setPattern("\\{0,0}" + QChar(wildcard));
// //   regex.setPattern(pattern);
// //   Q_ASSERT(regex.isValid());
// //   str.replace(regex, regexMeta);
// 
//   qDebug() << " -> str: " << str;
// }

void LikeExpressionRegexTransform::replaceWildcards(QString & str)
{
//   qDebug() << "str: " << str;

  if(str.isEmpty()){
    return;
  }

  auto wc = findFirstWildcard(str, 0);
  int i = wc.index;
  while( (i > -1) && (i < str.size()) ){
    Q_ASSERT(!wc.wildcard.isNull());
//     qDebug() << "- replace " << wc.wildcard << " at " << i;
    i = replaceWildcard(str, i, wc.wildcard);
    ///++i;
    if(i < str.size()){
      wc = findFirstWildcard(str, i);
      i = wc.index;
    }
  }
//   if(wc.index > -1){
//     qDebug() << "- found " << wc.wildcard << " at index " << wc.index;
//   }
}

int LikeExpressionRegexTransform::replaceWildcard(QString & str, int pos, const QChar & wildcard)
{
  Q_ASSERT(pos >= 0);
  Q_ASSERT(pos < str.size());

  switch(wildcard.toLatin1()){
    case '?':
      str.replace(pos, 1, '.');
      return pos + 1;
    case '_':
      str.replace(pos, 1, '.');
      return pos + 1;
    case '%':
      str.replace(pos, 1, QLatin1String(".*"));
      return pos + 2;
    case '*':
      str.replace(pos, 1, QLatin1String(".*"));
      return pos + 2;
  }
  return -1;
}

LikeExpressionRegexTransformWildcardMatch LikeExpressionRegexTransform::findFirstWildcard(const QString & str, int from)
{
  Q_ASSERT(!str.isEmpty());
  Q_ASSERT(from >= 0);
  Q_ASSERT(from < str.size());

//   qDebug() << " str: " << str << " , from: " << from;
  
  LikeExpressionRegexTransformWildcardMatch wc;
  auto it = str.constBegin() + from;
  const auto last = str.constEnd();

  while(it != last){
    it = std::find_if(it, last, [](const QChar & c){ return isTokenWildcard(c); });
    // If we found a wildcard, see if it is not escaped
    if(it != last){
      /*
       * If wildcard is found at first position in str, it is cannot be escaped.
       * Else, we must check.
       */
//       qDebug() << " - potential match: " << *it;
      if(it == str.constBegin()){
//         qDebug() << " - match at 0: " << *it;
        wc.index = 0;
        wc.wildcard = *it;
        break;
      }else{
        if(*(it-1) != '\\'){
//           qDebug() << " - match: " << *it;
          wc.index = it - str.constBegin();
          wc.wildcard = *it;
          break;
        }
      }
      ++it;
    }
  }

//   auto it = std::find_if(str.constBegin()+from, str.constEnd(), [](const QChar & c){ return isTokenWildcard(c); });
//   // If we found a wildcard, see if it is not escaped
//   if(it != str.constEnd()){
//     /*
//      * If wildcard is found at first position in str, it is cannot be escaped.
//      * Else, we must check.
//      */
//     if(it == str.constBegin()){
//       qDebug() << " - match: " << *it;
//       wc.index = 0;
//       wc.wildcard = *it;
//     }else{
//       if(*(it-1) != '\\'){
//         qDebug() << " - match: " << *it;
//         wc.index = it - str.constBegin();
//         wc.wildcard = *it;
//       }
//     }
//   }
  Q_ASSERT( (wc.index < 0) || (!wc.wildcard.isNull()) );
  
//   i = str.indexOf('?', from);
  

  return wc;
}

bool LikeExpressionRegexTransform::isTokenWildcard(const QChar & c)
{
  return (c == '?' || c == '*' || c == '%' || c == '_');
}

}} // namespace Mdt{ namespace ItemEditor{
