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
#include "mdtCsvGeneratorSettingsWidget.h"
#include <QComboBox>
#include <QCheckBox>
#include <QString>
#include <QChar>
#include <QVariant>

//#include <QDebug>

mdtCsvGeneratorSettingsWidget::mdtCsvGeneratorSettingsWidget(QWidget *parent)
 : mdtCsvSettingsWidget(parent)
{
  setParseExpVisible(false);
  // Update regarding default settings
  setSettings(mdtCsvGeneratorSettings());
}

void mdtCsvGeneratorSettingsWidget::setSettings(const mdtCsvGeneratorSettings& settings)
{
  selectFieldSeparator(settings.fieldSeparator);
  selectFieldProtection(settings.fieldProtection);
  selectEol(settings.eol);
  setAllwaysProtectTextFields(settings.allwaysProtectTextFields);
}

mdtCsvGeneratorSettings mdtCsvGeneratorSettingsWidget::getSettings() const
{
  mdtCsvGeneratorSettings settings;

  settings.fieldSeparator = getSelectedFieldSeparator();
  settings.fieldProtection = getSelectedFieldProtection();
  settings.eol = getSelectedEol();
  settings.allwaysProtectTextFields = allwaysProtectTextFields();

  return settings;
}
