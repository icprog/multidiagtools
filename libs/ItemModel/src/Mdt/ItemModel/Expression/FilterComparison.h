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
#ifndef MDT_ITEM_MODEL_EXPRESSION_FILTER_COMPARISON_H
#define MDT_ITEM_MODEL_EXPRESSION_FILTER_COMPARISON_H

// #include "LeftTerminal.h"
// #include "RightTerminal.h"

#include "../FilterColumn.h"
#include "../LikeExpression.h"
#include "MdtItemModelExport.h"
#include "Mdt/FilterExpression/LiteralValue.h"
#include <boost/proto/traits.hpp>
#include <boost/proto/matches.hpp>

namespace Mdt{ namespace ItemModel{ namespace Expression{

  /*! \brief Equality or inequality comparison grammar
   */
  struct FilterCompareEquality : boost::proto::or_<
                                  boost::proto::equal_to< FilterColumn , FilterExpression::LiteralValue > ,
                                  boost::proto::equal_to< FilterColumn , LikeExpression > ,
                                  boost::proto::not_equal_to< FilterColumn , FilterExpression::LiteralValue >
                                >
  {
  };

  /*! \brief Less or less equal comparison grammar
   */
  struct FilterCompareLess : boost::proto::or_<
                              boost::proto::less< FilterColumn , FilterExpression::LiteralValue > ,
                              boost::proto::less_equal< FilterColumn , FilterExpression::LiteralValue >
                            >
  {
  };

  /*! \brief Greater or greater equal comparison grammar
   */
  struct FilterCompareGreater : boost::proto::or_<
                                  boost::proto::greater< FilterColumn , FilterExpression::LiteralValue > ,
                                  boost::proto::greater_equal< FilterColumn , FilterExpression::LiteralValue >
                                >
  {
  };

  /*! \brief Comparison grammar
   */
  struct FilterComparison : boost::proto::or_<
                          FilterCompareEquality ,
                          FilterCompareLess ,
                          FilterCompareGreater
                        >
  {
  };

}}} // namespace Mdt{ namespace ItemModel{ namespace Expression{

#endif // #ifndef MDT_ITEM_MODEL_EXPRESSION_FILTER_COMPARISON_H
