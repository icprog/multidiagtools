/****************************************************************************
 **
 ** Copyright (C) 2011-2012 Philippe Steinmann.
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
#include "mdtDigitalInWidget.h"
#include "mdtDigitalIo.h"
#include "mdtLed.h"
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

mdtDigitalInWidget::mdtDigitalInWidget(QWidget *parent)
 : mdtAbstractIoWidget(parent)
{
  QGridLayout *l = new QGridLayout;

  ///pvIo = 0;
  // Setup GUI
  l->addWidget(lbLabel, 0, 0);
  ldState = new mdtLed;
  ldState->setFixedSize(20, 20);
  l->addWidget(ldState, 1, 0);
  lbState = new QLabel;
  lbState->setAlignment(Qt::AlignHCenter);
  l->addWidget(lbState, 2, 0);
  l->addWidget(pbDetails, 3, 0);
  setLayout(l);
}

mdtDigitalInWidget::~mdtDigitalInWidget()
{
}

void mdtDigitalInWidget::setIo(mdtDigitalIo *io)
{
  Q_ASSERT(io != 0);

  ///pvIo = io;
  // Base Signals/slots connections
  mdtAbstractIoWidget::setIo(io);
  // Signals/slots from io to widget
  ///connect(io, SIGNAL(stateChangedForUi(bool)), this, SLOT(setOn(bool)));
  connect(io, SIGNAL(valueChangedForUi(const mdtValue&)), this, SLOT(setValue(const mdtValue&)));
  connect(io, SIGNAL(enabledStateChangedForUi(bool)), this, SLOT(setEnabled(bool)));
  // Set initial data
  ///setOn(io->isOn());
  setValue(io->value());
}

/**
void mdtDigitalInWidget::setOn(bool on)
{
  Q_ASSERT(pvIo != 0);

  if(pvIo->hasValidData()){
    ldState->setOn(on);
    if(on){
      lbState->setText(tr("On"));
    }else{
      lbState->setText(tr("Off"));
    }
  }else{
    ldState->setOn(false);
    lbState->setText("??");
  }
}
*/

void mdtDigitalInWidget::setEnabled(bool enabled)
{
  ldState->setEnabled(enabled);
}

void mdtDigitalInWidget::setValue(const mdtValue &value)
{
  bool on;

  if(value.isValid()){
    on = value.valueBool();
    ldState->setOn(on);
    if(on){
      lbState->setText(tr("On"));
    }else{
      lbState->setText(tr("Off"));
    }
  }else{
    ldState->setOn(false);
    lbState->setText("??");
  }
}
