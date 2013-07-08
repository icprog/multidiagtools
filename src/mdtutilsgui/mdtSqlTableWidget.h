/****************************************************************************
 **
 ** Copyright (C) 2011-2013 Philippe Steinmann.
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
#ifndef MDT_SQL_TABLE_WIDGET_H
#define MDT_SQL_TABLE_WIDGET_H

#include "mdtAbstractSqlWidget.h"

#include <QModelIndex>
#include <QStyledItemDelegate>
#include <QString>

class QTableView;
class QHBoxLayout;
class QPushButton;
class QItemSelectionModel;
class QWidget;

/*
 * We use on manual submit edit strategy,so we can handle errors
 *  (display message to the user and let him the choice to revert, correct, wait, ...).
 * But, this way the user must allways click on save and insert button,
 *  which is not a fast solution for table based editor.
 * This class helps to catch key press events, and permits saving and inserting rows as expected.
 */
class mdtSqlTableWidgetKeyEventFilter : public QObject
{
 Q_OBJECT

 public:
  mdtSqlTableWidgetKeyEventFilter(QObject *parent);

 signals:
  void knownKeyPressed(int key);

 protected:
  bool eventFilter(QObject *obj, QEvent *event);
};

/*
 * To handle internal state machine correctly
 *  we need a signal that indicates that user begins to edit a item.
 *  We create a custom delegate that provide this signal.
 */
class mdtSqlTableWidgetItemDelegate : public QStyledItemDelegate
{
 Q_OBJECT

 public:
  mdtSqlTableWidgetItemDelegate(QObject *parent);
  QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;

 signals:
  void dataEditionBegins() const;
};

/*! \brief Table view based SQL widget
 *
 * Internally, QTableView is used.
 *  This class add some common needed methods,
 *  like insertion, removing, ...
 */
class mdtSqlTableWidget : public mdtAbstractSqlWidget
{
 Q_OBJECT

 public:

  /*! \brief Constructor
   */
  mdtSqlTableWidget(QWidget *parent = 0);

  /*! \brief Destructor
   */
  ~mdtSqlTableWidget();

  /*! \brief Get the current row
   */
  int currentRow() const;

  /*! \brief Set edition enabled/disabled
   */
  void setEditionEnabled(bool enable);

  /*! \brief Enable local edition
   *
   * Will add insert/revert/save/delete buttons.
   *
   * In normal way, when using mdtSqlWindow,
   *  local navigation is not needed.
   *  In master/detail forms, if this widget
   *  is a details (child) widget and edition
   *  is needed, then call this method.
   */
  void enableLocalEdition();

  /*! \brief Add a custom widget to local bar
   *
   * Typical usage can be to add a QPushButton
   *  for custom usage (edit, select, ...)
   *
   * Note: widget will be parented to layout,
   *  so Qt will delete it itself.
   *
   * \pre widget must be a valid pointer.
   */
  void addWidgetToLocalBar(QWidget *widget);

  /*! \brief Add a strech to local bar
   */
  void addStretchToLocalBar();

  /*! \brief Get internal table view's selection model
   */
  QItemSelectionModel *selectionModel();

  /*! \brief Set a user friendly name for a column
   *
   * \pre Model must be set with setModel() before using this method.
   */
  void setHeaderData(const QString &fieldName, const QString &data);

  /*! \brief Set a coulumn as hidden
   */
  void setColumnHidden(int column, bool hide);

  /*! \brief Set a coulumn as hidden
   *
   * \pre Model must be set with setModel() before using this method.
   */
  void setColumnHidden(const QString &fieldName, bool hide);

 public slots:

  /*! \brief Set row as current record
   */
  void setCurrentIndex(int row);

 private slots:

  /*! \internal Set internal delegateIsEditingData flag
   *
   * This slot is called by mdtSqlTableWidgetItemDelegate when editor was created.
   *  See remark in onTableViewKnownKeyPressed()
   *
   * This slot will emit dataEdited()
   */
  void onDataEditionBegins();

  /*! \internal Reset internal delegateIsEditingData
   *
   * See remark in onTableViewKnownKeyPressed()
   */
  void onDataChanged(const QModelIndex &, const QModelIndex &);

  /*! \brief Does some tasks when entering new row
   */
  void onCurrentRowChanged(const QModelIndex & current, const QModelIndex & previous);

  /*! \brief Execute a action after a known key was pressed in table view
   */
  void onTableViewKnownKeyPressed(int key);

 private:

  /*! \brief Set model
   *
   * Set the model that handle database.
   *  (See Qt's QSqlTableModel documentation for details).
   *
   * \pre model must be a valid pointer.
   */
  void doSetModel(QSqlTableModel *model);

  bool doSubmit();

  bool doRevert();

  bool doInsert();

  bool doSubmitNewRow();

  bool doRevertNewRow();

  bool doRemove();

  /*! \brief Set first record as current record
   */
  void toFirst();

  /*! \brief Set last record as current record
   */
  void toLast();

  /*! \brief Set previous record as current record
   */
  void toPrevious();

  /*! \brief Set next record as current record
   */
  void toNext();

  Q_DISABLE_COPY(mdtSqlTableWidget);

  QTableView *pvTableView;
  QHBoxLayout *pvNavigationLayout;
  // Navigation buttons
  QPushButton *pbNavToFirst;
  QPushButton *pbNavToPrevious;
  QPushButton *pbNavToNext;
  QPushButton *pbNavToLast;
  // Edition buttons
  QPushButton *pbInsert;
  QPushButton *pbSubmit;
  QPushButton *pbRevert;
  QPushButton *pbRemove;
  // Flags
  bool pvDelegateIsEditingData;   // See remark in onTableViewKnownKeyPressed()
};

#endif  // #ifndef MDT_SQL_TABLE_WIDGET_H
