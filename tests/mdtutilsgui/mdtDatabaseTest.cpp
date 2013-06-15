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
#include "mdtDatabaseTest.h"
#include "mdtApplication.h"

#include "mdtSqlTableModel.h"
#include "mdtSqlDataWidgetMapper.h"
#include "mdtApplication.h"
#include "mdtSqlRelation.h"
#include "mdtSqlFieldHandler.h"
#include "mdtSqlFormWidget.h"
#include "mdtSqlWindow.h"
#include "ui_mdtSqlFormWidgetTestForm.h"
#include <QSqlDatabase>
#include <QTemporaryFile>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QWidget>
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateTimeEdit>
#include <QAbstractButton>
#include <QComboBox>
#include <QString>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QFile>
#include <QVariant>
#include <QTimer>
#include <QAbstractButton>

#include <QTableView>
#include <QItemSelectionModel>

#include <QDebug>

void mdtDatabaseTest::initTestCase()
{
  QTemporaryFile tempFile;
  QSqlDatabase db;
  QSqlField fld;
  QString sql;

  // Create a temp file that will be used for database
  QVERIFY(tempFile.open());
  tempFile.close();
  pvDbFileInfo.setFile(tempFile.fileName() + ".db");
  qDebug() << "Init, DB file: " << pvDbFileInfo.filePath();

  // Open database
  db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName(pvDbFileInfo.filePath());
  QVERIFY(db.open());

  // QSqlQuery must be created after db.open() was called.
  QSqlQuery q;

  /*
   * Create Client table
   */
  sql = "CREATE TABLE 'Client' (";
  sql += "'id_PK' INTEGER PRIMARY KEY AUTOINCREMENT, ";
  sql += "'first_name' VARCHAR(50) NOT NULL, ";
  sql += "'remarks' VARCHAR(500) )";
  QVERIFY(q.exec(sql));
  // Verify some attributes
  fld = db.record("Client").field("id_PK");
  QVERIFY(fld.isAutoValue());
  fld = db.record("Client").field("first_name");
  QVERIFY(fld.requiredStatus() == QSqlField::Required);
  // Inert some data
  QVERIFY(q.exec("INSERT INTO 'Client' ('first_name') VALUES('Andy')"));
  QVERIFY(q.exec("INSERT INTO 'Client' ('first_name', 'remarks') VALUES('Bety', 'Remark on Bety')"));
  QVERIFY(q.exec("SELECT * FROM 'Client'"));
  while(q.next()){
    QVERIFY(!q.record().isEmpty());
  }

  /*
   * Create Addresses table
   */
  sql = "CREATE TABLE 'Address' (";
  sql += "'id_PK' INTEGER PRIMARY KEY AUTOINCREMENT, ";
  sql += "'street_name' VARCHAR(50) NOT NULL, ";
  sql += "'street_number' INTEGER NOT NULL, ";
  sql += "'id_client_FK' INTEGER NOT NULL)";
  QVERIFY(q.exec(sql));
  // Verify some attributes
  fld = db.record("Address").field("id_PK");
  QVERIFY(fld.isAutoValue());
  fld = db.record("Address").field("street_name");
  QVERIFY(fld.requiredStatus() == QSqlField::Required);
  fld = db.record("Address").field("street_number");
  QVERIFY(fld.requiredStatus() == QSqlField::Required);
  fld = db.record("Address").field("id_client_FK");
  QVERIFY(fld.requiredStatus() == QSqlField::Required);

  // Inert some data
  QVERIFY(q.exec("INSERT INTO 'Address' ('street_name', 'street_number', 'id_client_FK') VALUES('Andy street 1', 56 , 1)"));
  QVERIFY(q.exec("INSERT INTO 'Address' ('street_name', 'street_number', 'id_client_FK') VALUES('Andy street 2', 87 , 1)"));
  QVERIFY(q.exec("INSERT INTO 'Address' ('street_name', 'street_number', 'id_client_FK') VALUES('Bety street 1', 45 , 2)"));
  QVERIFY(q.exec("SELECT * FROM 'Address'"));
  while(q.next()){
    QVERIFY(!q.record().isEmpty());
  }

}

/// \todo Make data checks - Nothing is finished !!
void mdtDatabaseTest::relationsTest()
{
  QSqlTableModel parentModel;
  QSqlTableModel childModel;
  mdtSqlRelation relation;

  // Setup parent model
  parentModel.setTable("Client");
  parentModel.select();
  // Setup child model
  childModel.setTable("Address");
  childModel.select();
  // Setup relation
  relation.setParentModel(&parentModel);
  relation.setChildModel(&childModel);
  QVERIFY(relation.addRelation("id_PK", "id_client_FK"));

  // Check data of each model
  
  // Setup parent view
  QTableView pv;
  pv.setModel(&parentModel);
  QVERIFY(pv.selectionModel() != 0);
  QObject::connect(pv.selectionModel(), SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), \
                    &relation, SLOT(setParentCurrentIndex(const QModelIndex&, const QModelIndex&)));
  // Setup child view
  QTableView cv;
  cv.setModel(&childModel);

  // Play...
  pv.show();
  cv.show();
  while(pv.isVisible()){
    QTest::qWait(1000);
  }
}

void mdtDatabaseTest::sqlFieldHandlerTest()
{
  mdtSqlFieldHandler fh;
  QSqlField field;

  // Check inital flags
  QVERIFY(fh.isNull());
  QVERIFY(!fh.dataWasEdited());
  QVERIFY(!fh.isReadOnly());

  // Setup field
  field.setAutoValue(false);
  field.setLength(10);
  field.setRequiredStatus(QSqlField::Required);
  field.setReadOnly(false);
  fh.setField(field);

  /*
   * Tests with QLineEdit
   */
  QLineEdit le;
  fh.setDataWidget(&le);
  le.show();
  // Check field handler flags
  QVERIFY(fh.isNull());
  QVERIFY(!fh.dataWasEdited());
  QVERIFY(!fh.isReadOnly());
  // Check widget flags
  QVERIFY(le.isEnabled());
  QVERIFY(le.text().isEmpty());
  QVERIFY(!le.isReadOnly());
  // User edit ...
  QTest::keyClicks(&le, "ABCD");
  // Check field handler flags
  QVERIFY(!fh.isNull());
  QVERIFY(fh.dataWasEdited());
  // User edit ...
  QTest::keyClick(&le, Qt::Key_Backspace);
  QTest::keyClick(&le, Qt::Key_Backspace);
  QTest::keyClick(&le, Qt::Key_Backspace);
  QTest::keyClick(&le, Qt::Key_Backspace);
  // Check field handler flags
  QVERIFY(fh.isNull());
  QVERIFY(fh.dataWasEdited());
  // User edit ...
  QTest::keyClicks(&le, "123");
  // Check field handler flags
  QVERIFY(!fh.isNull());
  QVERIFY(fh.dataWasEdited());
  // User edit ...
  le.clear();
  QTest::keyClicks(&le, " ");
  // Check field handler flags
  QVERIFY(fh.isNull());
  QVERIFY(fh.dataWasEdited());
  // Field is mandatory, so check must fail
  QVERIFY(!fh.checkBeforeSubmit());
  // Line edit must be empty now
  QVERIFY(le.text().isEmpty());
  // User edit ...
  le.clear();
  QTest::keyClicks(&le, "ZA");
  // Check field handler flags
  QVERIFY(!fh.isNull());
  QVERIFY(fh.dataWasEdited());
  // Check must be OK now
  QVERIFY(fh.checkBeforeSubmit());
  // User edit to much
  le.clear();
  QTest::keyClicks(&le, "123456789ABC");
  // Check field handler flags
  QVERIFY(!fh.isNull());
  QCOMPARE(le.text(), QString("123456789A"));
  // Set read only
  fh.setReadOnly(true);
  QVERIFY(fh.isReadOnly());
  QVERIFY(le.isReadOnly());
  // Set editable again
  fh.setReadOnly(false);
  QVERIFY(!fh.isReadOnly());
  QVERIFY(!le.isReadOnly());
  // Now we change the row
  le.setText("Row 2 from DB");
  fh.updateFlags();
  // Check field handler flags
  QVERIFY(!fh.isNull());
  QVERIFY(!fh.dataWasEdited());
  // User edit ...
  le.clear();
  QTest::keyClicks(&le, " ");
  // Check field handler flags
  QVERIFY(fh.isNull());
  QVERIFY(fh.dataWasEdited());
  // We change the row
  le.setText("Row 3 from DB");
  fh.updateFlags();
  // Check field handler flags
  QVERIFY(!fh.isNull());
  QVERIFY(!fh.dataWasEdited());
  // We change the row
  le.setText("");
  fh.updateFlags();
  // Check field handler flags
  QVERIFY(fh.isNull());
  QVERIFY(!fh.dataWasEdited());

  
  // Setup field as read only
  field.setAutoValue(false);
  field.setLength(10);
  field.setRequiredStatus(QSqlField::Required);
  field.setReadOnly(true);
  fh.setField(field);
  // Check field handler flags
  ///QVERIFY(fh.isReadOnly());
  // Check widget flags
  QVERIFY(le.isEnabled());
  QVERIFY(le.isReadOnly());


  
  
  /*
   * Play
   */
  /*
  while(le.isVisible()){
    QTest::qWait(1000);
  }
  */

}

void mdtDatabaseTest::sqlFormWidgetTest()
{
  mdtSqlFormWidget *sqlFormWidget;
  mdtSqlWindow window;
  Ui::mdtSqlFormWidgetTestForm form;
  QSqlTableModel model;
  QLineEdit *leFirstName = 0;
  QLineEdit *leRemarks = 0;
  QWidget *w;
  int rowCount;
  int row;
  QVariant data;

  // Setup model + form view
  model.setTable("Client");
  model.select();
  sqlFormWidget = new mdtSqlFormWidget;
  QTest::qWait(50);
  sqlFormWidget->setModel(&model);
  form.setupUi(sqlFormWidget);
  sqlFormWidget->mapFormWidgets("fld_first_name");
  // Setup window
  window.setSqlWidget(sqlFormWidget);
  window.enableNavigation();
  window.enableEdition();
  // We need access to form's line edits
  QVERIFY(sqlFormWidget->layout() != 0);
  for(int i=0; i<sqlFormWidget->layout()->count(); i++){
    QVERIFY(sqlFormWidget->layout()->itemAt(i) != 0);
    w = sqlFormWidget->layout()->itemAt(i)->widget();
    QVERIFY(w != 0);
    if(w->objectName() == "fld_first_name"){
      leFirstName = dynamic_cast<QLineEdit*>(w);
    }else if(w->objectName() == "fld_remarks"){
      leRemarks = dynamic_cast<QLineEdit*>(w);
    }
  }
  QVERIFY(leFirstName != 0);
  QVERIFY(leRemarks != 0);
  
  window.show();
  
  /*
   * Check insertion
   *  - Insert 2 records
   *  - Check in model
   *  - Check in form
   *
   * Note:
   *  Because of mdtSqlFormWidget's internall state machine,
   *  witch runs asynchronousliy, we must wait between each action.
   */
  rowCount = model.rowCount();
  // Insert a record
  QTest::qWait(50);
  sqlFormWidget->insert();
  QTest::qWait(50);
  QVERIFY(leFirstName->isEnabled());
  QVERIFY(leFirstName->text().isEmpty());
  QVERIFY(leRemarks->isEnabled());
  QVERIFY(leRemarks->text().isEmpty());
  leFirstName->clear();
  QTest::keyClicks(leFirstName, "New name 1");
  leRemarks->clear();
  QTest::keyClicks(leRemarks, "New remark 1");
  sqlFormWidget->submit();
  QTest::qWait(50);
  // Check that model was updated
  QVERIFY(model.rowCount() > rowCount);
  rowCount = model.rowCount();
  row = sqlFormWidget->currentRow();
  data = model.data(model.index(row, model.fieldIndex("first_name")));
  QCOMPARE(data.toString(), QString("New name 1"));
  data = model.data(model.index(row, model.fieldIndex("remarks")));
  QCOMPARE(data.toString(), QString("New remark 1"));
  // Check that widget displays the correct row
  QCOMPARE(leFirstName->text(), QString("New name 1"));
  QCOMPARE(leRemarks->text(), QString("New remark 1"));
  // Insert a record
  sqlFormWidget->insert();
  QTest::qWait(50);
  QVERIFY(leFirstName->isEnabled());
  QVERIFY(leFirstName->text().isEmpty());
  QVERIFY(leRemarks->isEnabled());
  QVERIFY(leRemarks->text().isEmpty());
  leFirstName->clear();
  QTest::keyClicks(leFirstName, "New name 2");
  leRemarks->clear();
  QTest::keyClicks(leRemarks, "New remark 2");
  sqlFormWidget->submit();
  QTest::qWait(50);
  // Check that model was updated
  QVERIFY(model.rowCount() > rowCount);
  rowCount = model.rowCount();
  row = sqlFormWidget->currentRow();
  data = model.data(model.index(row, model.fieldIndex("first_name")));
  QCOMPARE(data.toString(), QString("New name 2"));
  data = model.data(model.index(row, model.fieldIndex("remarks")));
  QCOMPARE(data.toString(), QString("New remark 2"));
  // Check that widget displays the correct row
  QCOMPARE(leFirstName->text(), QString("New name 2"));
  QCOMPARE(leRemarks->text(), QString("New remark 2"));
  // Try to insert a record with no name - must fail
  sqlFormWidget->insert();
  QTest::qWait(50);
  QCOMPARE(leFirstName->text(), QString(""));
  QCOMPARE(leRemarks->text(), QString(""));
  leRemarks->clear();
  QTest::keyClicks(leRemarks, "New remark ...");
  // Catch and accept the message box that will pop-up
  QTimer::singleShot(50, this, SLOT(acceptModalDialog()));
  sqlFormWidget->submit();
  QTest::qWait(100);
  /*
   * We cannot check now, beacuse new row was inserted in model
   * We will revert, then check that new inserted row is suppressed
   */
  // Catch the message box that will pop-up and click Yes button
  QTimer::singleShot(50, this, SLOT(clickMessageBoxButtonYes()));
  sqlFormWidget->revert();
  QTest::qWait(100);
  QVERIFY(model.rowCount() == rowCount);
  /*
   * Check edition
   *  - Edit current row
   *  - Check in model
   *  - Check in form
   */
  // Edit in form and submit
  leFirstName->clear();
  QTest::keyClicks(leFirstName, "Edit name A");
  leRemarks->clear();
  QTest::keyClicks(leRemarks, "Edit remark A");
  sqlFormWidget->submit();
  QTest::qWait(50);
  // Check that model was updated
  QVERIFY(model.rowCount() == rowCount);
  row = sqlFormWidget->currentRow();
  data = model.data(model.index(row, model.fieldIndex("first_name")));
  QCOMPARE(data.toString(), QString("Edit name A"));
  data = model.data(model.index(row, model.fieldIndex("remarks")));
  QCOMPARE(data.toString(), QString("Edit remark A"));
  // Check that widget displays the correct row
  QCOMPARE(leFirstName->text(), QString("Edit name A"));
  QCOMPARE(leRemarks->text(), QString("Edit remark A"));
  /*
   * Check delete:
   *  - One time on accepting warning message box
   *  - One time rejecting warning message box
   */
  // Catch the message box that will pop-up and click No button
  QTimer::singleShot(50, this, SLOT(clickMessageBoxButtonNo()));
  sqlFormWidget->remove();
  QTest::qWait(100);
  // Check that nothing was removed and that index did not change
  QVERIFY(model.rowCount() == rowCount);
  row = sqlFormWidget->currentRow();
  data = model.data(model.index(row, model.fieldIndex("first_name")));
  QCOMPARE(data.toString(), QString("Edit name A"));
  data = model.data(model.index(row, model.fieldIndex("remarks")));
  QCOMPARE(data.toString(), QString("Edit remark A"));
  // Check that widget displays the correct row
  QCOMPARE(leFirstName->text(), QString("Edit name A"));
  QCOMPARE(leRemarks->text(), QString("Edit remark A"));
  // Catch the message box that will pop-up and click Yes button
  QTimer::singleShot(50, this, SLOT(clickMessageBoxButtonYes()));
  sqlFormWidget->remove();
  QTest::qWait(100);
  // Check that one row was removed and that widget displays something different than before
  QCOMPARE(model.rowCount(), (rowCount-1));
  rowCount = model.rowCount();
  QVERIFY(leFirstName->text() != QString("Edit name A"));
  QVERIFY(leRemarks->text() != QString("Edit remark A"));
  /*
   * Remove first row
   */
  sqlFormWidget->toFirst();
  QTest::qWait(50);
  row = sqlFormWidget->currentRow();
  QCOMPARE(row, 0);
  data = model.data(model.index(row, model.fieldIndex("first_name")));
  // Catch the message box that will pop-up and click Yes button
  QTimer::singleShot(50, this, SLOT(clickMessageBoxButtonYes()));
  sqlFormWidget->remove();
  QTest::qWait(100);
  // Check that one row was removed and that widget displays something different than before
  QCOMPARE(model.rowCount(), (rowCount-1));
  rowCount = model.rowCount();
  QVERIFY(leFirstName->text() != data.toString());
  // Remove all rows - prepare for next tests
  while(model.rowCount() > 0){
    // Catch the message box that will pop-up and click Yes button
    QTimer::singleShot(50, this, SLOT(clickMessageBoxButtonYes()));
    sqlFormWidget->remove();
    QTest::qWait(100);
  }
  // Check that widgets are empty and disabled
  QVERIFY(leFirstName->text().isEmpty());
  QVERIFY(!leFirstName->isEnabled());
  QVERIFY(leRemarks->text().isEmpty());
  QVERIFY(!leRemarks->isEnabled());
  /*
   * Check index changing on unsaved data
   *  - One time on accepting warning message box (-> submit, ok, ...)
   *  - One time rejecting warning message box
   */
  // Insert a record
  QTest::qWait(50);
  sqlFormWidget->insert();
  QTest::qWait(50);
  QVERIFY(leFirstName->isEnabled());
  QVERIFY(leFirstName->text().isEmpty());
  QVERIFY(leRemarks->isEnabled());
  QVERIFY(leRemarks->text().isEmpty());
  leFirstName->clear();
  QTest::keyClicks(leFirstName, "ABCD");
  leRemarks->clear();
  QTest::keyClicks(leRemarks, "1234");
  sqlFormWidget->submit();
  QTest::qWait(50);
  // Insert a record
  QTest::qWait(50);
  sqlFormWidget->insert();
  QTest::qWait(50);
  QVERIFY(leFirstName->isEnabled());
  QVERIFY(leFirstName->text().isEmpty());
  QVERIFY(leRemarks->isEnabled());
  QVERIFY(leRemarks->text().isEmpty());
  leFirstName->clear();
  QTest::keyClicks(leFirstName, "EFGH");
  leRemarks->clear();
  QTest::keyClicks(leRemarks, "5678");
  // Try to go to previous row - Catch message box and click Ok
  QTimer::singleShot(50, this, SLOT(clickMessageBoxButtonOk()));
  sqlFormWidget->toPrevious();
  QTest::qWait(100);
  // Check that we are allways in second row and save data
  QCOMPARE(leFirstName->text(), QString("EFGH"));
  QCOMPARE(leRemarks->text(), QString("5678"));
  sqlFormWidget->submit();
  QTest::qWait(50);
  // Go back and check that we are back to first row
  sqlFormWidget->toPrevious();
  QTest::qWait(50);
  QCOMPARE(leFirstName->text(), QString("ABCD"));
  QCOMPARE(leRemarks->text(), QString("1234"));
  // Go forward and check that second row was saved
  sqlFormWidget->toNext();
  QTest::qWait(50);
  QCOMPARE(leFirstName->text(), QString("EFGH"));
  QCOMPARE(leRemarks->text(), QString("5678"));


  
  sqlFormWidget->submit();
  QTest::qWait(50);

  
  

  /*
   * Play
   */
  while(window.isVisible()){
    QTest::qWait(1000);
  }
}

void mdtDatabaseTest::clickMessageBoxButton(QMessageBox::StandardButton button)
{
  QMessageBox *mdgBox;
  QAbstractButton *btn;

  if(QApplication::activeModalWidget() != 0){
    mdgBox = dynamic_cast<QMessageBox*>(QApplication::activeModalWidget());
    if(mdgBox != 0){
      qDebug() << "Found a message box, name: " << mdgBox->objectName();
      // Get requested button
      btn = mdgBox->button(button);
      if(btn != 0){
        btn->click();
      }
    }
  }
}

void mdtDatabaseTest::clickMessageBoxButtonYes()
{
  clickMessageBoxButton(QMessageBox::Yes);
}

void mdtDatabaseTest::clickMessageBoxButtonNo()
{
  clickMessageBoxButton(QMessageBox::No);
}

void mdtDatabaseTest::clickMessageBoxButtonCancel()
{
  clickMessageBoxButton(QMessageBox::Cancel);
}

void mdtDatabaseTest::clickMessageBoxButtonOk()
{
  clickMessageBoxButton(QMessageBox::Ok);
}

void mdtDatabaseTest::acceptModalDialog()
{
  QDialog *dlg;

  if(QApplication::activeModalWidget() != 0){
    dlg = dynamic_cast<QDialog*>(QApplication::activeModalWidget());
    if(dlg != 0){
      qDebug() << "Found a dilaog, name: " << dlg->objectName();
      dlg->accept();
    }
  }
}

void mdtDatabaseTest::rejectModalDialog()
{
  QDialog *dlg;

  if(QApplication::activeModalWidget() != 0){
    dlg = dynamic_cast<QDialog*>(QApplication::activeModalWidget());
    if(dlg != 0){
      qDebug() << "Found a dilaog, name: " << dlg->objectName();
      dlg->reject ();
    }
  }
}

void mdtDatabaseTest::mdtSqlTableModelTest()
{
}

void mdtDatabaseTest::mdtSqlDataWidgetMapperTest()
{
  mdtSqlDataWidgetMapper mapper;
  mdtSqlTableModel model;
  QLineEdit id, first_name, remarks;
  
  // Init model
  model.setTable("Client");
  model.select();

  // Init mapper
  mapper.setModel(&model);
  mapper.addMapping(&id, 0);
  mapper.addMapping(&first_name, 1);
  mapper.addMapping(&remarks, 2);
  mapper.toFirst();

  // Check that data are available in widgets
  QVERIFY(!id.text().isEmpty());
  QVERIFY(!first_name.text().isEmpty());
}



void mdtDatabaseTest::cleanupTestCase()
{
  QFile::remove(pvDbFileInfo.filePath());
}

int main(int argc, char **argv)
{
  mdtApplication app(argc, argv);
  mdtDatabaseTest test;

  if(!app.init()){
    return 1;
  }

  return QTest::qExec(&test, argc, argv);
}
