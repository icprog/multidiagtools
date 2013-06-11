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
#ifndef MDT_SQL_FORM_WIDGET_H
#define MDT_SQL_FORM_WIDGET_H

#include "mdtAbstractSqlWidget.h"
#include <QWidget>
#include <QList>

class mdtSqlDataWidgetMapper;
class mdtSqlFieldHandler;
class QSqlTableModel;

/*! \brief Widget that accept a user designed form and helps dealing with table model
 *
 * To create a database table form, Qt offers several classes.
 *  But, if we have to create many forms, we have to deal with
 *  common problems (insertion, feild mapping, etc..).
 *
 * It's possible to create a form, for example with Qt Designer,
 *  and add needed Widgets (QLineEdit, QSpinBox, ...).
 *  It's important to respect a convention here:
 *   For each Widget that must display/edit data from
 *   database (i.e. model), set the name of the field
 *   with fld_ prefix.
 *   For example, if we have a field called name,
 *   create a QLineEdit with fld_name as object name.
 *
 * Once your form was created, call his setupUi() method passing
 *  mdtSqlFormWidget object as parameter.
 *  setupUi() will also create layouts and widgets with mdtSqlFormWidget object as parent.
 *  As result, mdtSqlFormWidget object will have a layout
 *  with all widgets. Finally, use mapFormWidgets() to do the
 *  mapping between widgets and fields in database.
 *
 * To use the resulting form, set it, f.ex. as central widget
 *  of a QMainWindow. See the list of slots to know how to
 *  use integrated actions (navigation, insertion, ...).
 *
 * Internally, mdtSqlDataWidgetMapper is used together with
 *  mdtSqlFieldHandler.
 *  Errors and user interactions are handled by this class.
 */
class mdtSqlFormWidget : public mdtAbstractSqlWidget
{
 Q_OBJECT

 public:

  /*! \brief Constructor
   */
  mdtSqlFormWidget(QWidget *parent = 0);

  /*! \brief Destructor
   */
  ~mdtSqlFormWidget();

  /*! \brief Map widgets to matching database fields
   *
   * Will parse all child widgets found in layout,
   *  extract widgets that have a fld_ prefix as object name.
   *  For each of them, corresponding field will be searched in model (database).
   *
   * \pre The custom form's setupUi() must be called on this object before using this method.
   *       Once setupUi() is done, mdtSqlFormWidget object (this) will contain a layout
   *       with all widgets (QLineEdit, QSpinBox, ...).
   * \pre Model must be set with setModel() before using this method.
   */
  void mapFormWidgets();

  /*! \brief Get the current row
   *
   * Current row is the row that is displayed 
   *  in the form view.
   */
  int currentRow() const;

  ///void setForm(QWidget *form);

 public slots:

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

 private:

  /*! \brief Set model
   *
   * Set the model that handle database.
   *  (See Qt's QSqlTableModel documentation for details).
   *
   * \pre model must be a valid pointer.
   */
  void doSetModel(QSqlTableModel *model);

  /*! \brief Submit current row to model
   *
   * \pre Model must be set with setModel() before using this method.
   */
  bool doSubmit();

  /*! \brief Revert current row from model
   *
   * \pre Model must be set with setModel() before using this method.
   */
  bool doRevert();

  /*! \brief Insert a row row to model
   *
   * \pre Model must be set with setModel() before using this method.
   */
  bool doInsert();

  /*! \brief Submit new row to model
   *
   * \pre Model must be set with setModel() before using this method.
   */
  bool doSubmitNewRow();

  /*! \brief Revert new row
   *
   * \pre Model must be set with setModel() before using this method.
   */
  bool doRevertNewRow();

  /*! \brief Remove current row from model
   *
   * \pre Model must be set with setModel() before using this method.
   */
  bool doRemove();

  /*! \brief Clear content of all edit/view widgets
   */
  void clearWidgets();

  /*! \brief Call mdtSqlFieldHandler::checkBeforeSubmit() for each mapped widgets
   */
  bool checkBeforeSubmit();

  Q_DISABLE_COPY(mdtSqlFormWidget);

  mdtSqlDataWidgetMapper *pvWidgetMapper;
  QList<mdtSqlFieldHandler*> pvFieldHandlers;
  bool pvInsertionPending;  // Used by insert() , remove() and revert()
};

#endif  // #ifndef MDT_SQL_FORM_WIDGET_H
