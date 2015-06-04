/****************************************************************************
 **
 ** Copyright (C) 2011-2015 Philippe Steinmann.
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
#ifndef MDT_CL_ARTICLE_CONNECTION_DATA_H
#define MDT_CL_ARTICLE_CONNECTION_DATA_H

#include "mdtClArticleConnectionKeyData.h"

#include "mdtSqlRecord.h"
#include <QList>
#include <QVariant>
#include <QSqlDatabase>

/*! \brief Data container for article connection data
 *
 * Refers to ArticleConnection_tbl
 */
struct mdtClArticleConnectionData : public mdtSqlRecord  /// \todo When all is adapted, remove this inheritance
{
 public:

  /*! \brief Construct a empty mdtClArticleConnectionData
   */
  mdtClArticleConnectionData();

  /*! \brief Contruct a mdtClArticleConnectionData from a QSqlRecord
   *
   * Note: if this method is used, setup is not relevant.
   *
   * \pre All fields from ArticleConnection_tbl must exist in record
   */
  mdtClArticleConnectionData(const QSqlRecord & record);

  /*! \brief Setup fields from ArticleConnection_tbl
   */
  bool setup(QSqlDatabase db);
};

#endif // #ifndef MDT_CL_ARTICLE_CONNECTION_DATA_H
