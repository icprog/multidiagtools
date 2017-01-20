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
#ifndef MDT_ITEM_MODEL_EXPRESSION_GET_RELATION_KEY_FOR_EQUALITY_H
#define MDT_ITEM_MODEL_EXPRESSION_GET_RELATION_KEY_FOR_EQUALITY_H

#include "../FilterColumn.h"
#include "../ParentModelColumn.h"
#include "../RelationKey.h"
#include "ColumnTerminal.h"
#include <boost/proto/traits.hpp>
#include <boost/proto/matches.hpp>
#include <boost/proto/transform/when.hpp>
#include <boost/proto/transform.hpp>

namespace Mdt{ namespace ItemModel{ namespace Expression{

  /*! \brief Add a relation column pair
   */
  struct AddRelationColumnPair : boost::proto::callable
  {
    typedef void result_type;

    /*
     * When using boost::proto::deep_copy(),
     * terminal types can not be deduced.
     * This is why we use templates.
     */
    template<typename CMC, typename PMC>
    void operator()(const CMC & childModelColumn, const PMC & parentModelColumn, RelationKey & key) const
    {
      addColumnPair( boost::proto::value(childModelColumn), boost::proto::value(parentModelColumn), key);
    }

   private:

    static void addColumnPair(const FilterColumnData & childModelColumn, const ParentModelColumnData & parentModelColumn, RelationKey & key);
  };

  /*! \brief Get a RelationKey for equality
   */
  struct GetRelationKeyForEquality : boost::proto::or_<
                                      boost::proto::when<
                                        boost::proto::equal_to< FilterColumn, ParentModelColumn > ,
                                        boost::proto::call< AddRelationColumnPair(boost::proto::_left, boost::proto::_right, boost::proto::_data) >
                                      > ,
                                      boost::proto::when<
                                        boost::proto::less_equal< FilterColumn, ParentModelColumn > ,
                                        boost::proto::call< AddRelationColumnPair(boost::proto::_left, boost::proto::_right, boost::proto::_data) >
                                      > ,
                                      boost::proto::when<
                                        boost::proto::greater_equal< FilterColumn, ParentModelColumn > ,
                                        boost::proto::call< AddRelationColumnPair(boost::proto::_left, boost::proto::_right, boost::proto::_data) >
                                      > ,
                                      boost::proto::when<
                                        boost::proto::logical_and< GetRelationKeyForEquality , GetRelationKeyForEquality > ,
                                        boost::proto::_state
                                      > ,
                                      boost::proto::when<
                                        boost::proto::logical_or< GetRelationKeyForEquality , GetRelationKeyForEquality > ,
                                        boost::proto::_state
                                      >
                                     >
  {
  };

}}} // namespace Mdt{ namespace ItemModel{ namespace Expression{

#endif // #ifndef MDT_ITEM_MODEL_EXPRESSION_GET_RELATION_KEY_FOR_EQUALITY_H
