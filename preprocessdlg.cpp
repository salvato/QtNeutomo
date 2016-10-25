/*
 *
Copyright (C) 2016  Gabriele Salvato

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
#include "preprocessdlg.h"
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>
#include <QFileDialog>
#include <QSettings>
#include <QDebug>


PreProcessDlg::PreProcessDlg(QWidget *parent)
  : QDialog(parent)
{
  createWidgets();
  buildLayout();
  // Restore previous section values
  QSettings settings("Gabriele Salvato", "QtNeuTomo");
  restoreGeometry(settings.value("PreProcessDlg/geometry", ".").toByteArray());
  // Restore File Paths
  darkPathEdit->setText(settings.value("PreProcessDlg/darkPath", ".").toByteArray());
  flatFieldPathEdit->setText(settings.value("PreProcessDlg/flatFieldPath", ".").toByteArray());
  inputPathEdit->setText(settings.value("PreProcessDlg/inputPath", ".").toByteArray());
  outputPathEdit->setText(settings.value("PreProcessDlg/outputPath", ".").toByteArray());
  // Restore Angular Step
  degPerStepEdit->setText(settings.value("PreProcessDlg/degPerStep", "1.0").toByteArray());
  // Restore Button Status ...use
  darkUseCheckBox->setChecked(settings.value("PreProcessDlg/darkUse", false).toBool());
  flatFieldUseCheckBox->setChecked(settings.value("PreProcessDlg/flatFieldUse", false).toBool());
  // Restore Button Status ...filter
  darkMedianFilterCheckBox->setChecked(settings.value("PreProcessDlg/darkMedian", false).toBool());
  flatFieldMedianFilterCheckBox->setChecked(settings.value("PreProcessDlg/flatFieldMedian", false).toBool());
  inputMedianFilterCheckBox->setChecked(settings.value("PreProcessDlg/inputMedian", false).toBool());
  outputdMedianFilterCheckBox->setChecked(settings.value("PreProcessDlg/outputdMedian", false).toBool());
  initStatus();
  initSignals();
}


void
PreProcessDlg::createWidgets() {
  darkPathLabel      = new QLabel(tr("Dark Images path"));
  flatFieldPathLabel = new QLabel(tr("Flat Field Images path"));
  inputPathLabel     = new QLabel(tr("Input Images path"));
  outputPathLabel    = new QLabel(tr("Output Images path"));
  degPerStepLabel    = new QLabel(tr("°/Step"));
  degPerStepLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

  darkPathEdit      = new QLineEdit(tr("."));
  flatFieldPathEdit = new QLineEdit(tr("."));
  inputPathEdit     = new QLineEdit(tr("."));
  outputPathEdit    = new QLineEdit(tr("."));
  degPerStepEdit    = new QLineEdit(tr("1.0"));
  degPerStepEdit->setAlignment(Qt::AlignRight);

  darkPathEdit->setMinimumSize(300, 0);
  flatFieldPathEdit->setMinimumSize(300, 0);
  inputPathEdit->setMinimumSize(300, 0);
  outputPathEdit->setMinimumSize(300, 0);

  darkUseCheckBox              = new QCheckBox(tr("Use"));
  flatFieldUseCheckBox         = new QCheckBox(tr("Use"));

  darkUseCheckBox->setToolTip(tr("Enable/Disable dark subtraction"));
  flatFieldUseCheckBox->setToolTip(tr("Enable/Disable Flat-Field normalization"));

  darkMedianFilterCheckBox      = new QCheckBox(tr("Filter"));
  flatFieldMedianFilterCheckBox = new QCheckBox(tr("Filter"));
  inputMedianFilterCheckBox     = new QCheckBox(tr("Filter"));
  outputdMedianFilterCheckBox   = new QCheckBox(tr("Filter"));

  darkMedianFilterCheckBox->setToolTip(tr("Median Filter the dark images"));
  flatFieldMedianFilterCheckBox->setToolTip(tr("Median Filter the flat-field images"));
  inputMedianFilterCheckBox->setToolTip(tr("Median Filter the input projections"));
  outputdMedianFilterCheckBox->setToolTip(tr("Median Filter the output projections"));

  darkPathButton      = new QPushButton("...");
  flatFieldPathButton = new QPushButton("...");
  inputPathButton     = new QPushButton("...");
  outputPathButton    = new QPushButton("...");
  okButton            = new QPushButton("Done");
  cancelButton        = new QPushButton("Cancel");

  darkPathButton->setToolTip(tr("Change dark images Path"));
  flatFieldPathButton->setToolTip(tr("Change flat-field images Path"));
  inputPathButton->setToolTip(tr("Change input projections Path"));
  outputPathButton->setToolTip(tr("Change output projections Path"));
}


void
PreProcessDlg::buildLayout() {

  QGridLayout* mainLayout = new QGridLayout();
  mainLayout->addWidget(darkPathLabel,      1, 1);
  mainLayout->addWidget(flatFieldPathLabel, 2, 1);
  mainLayout->addWidget(inputPathLabel,     3, 1);
  mainLayout->addWidget(outputPathLabel,    4, 1);

  mainLayout->addWidget(darkPathEdit,      1, 2);
  mainLayout->addWidget(flatFieldPathEdit, 2, 2);
  mainLayout->addWidget(inputPathEdit,     3, 2);
  mainLayout->addWidget(outputPathEdit,    4, 2);

  mainLayout->addWidget(darkUseCheckBox,      1, 3);
  mainLayout->addWidget(flatFieldUseCheckBox, 2, 3);
  mainLayout->addWidget(degPerStepLabel,      3, 3);
  mainLayout->addWidget(degPerStepEdit,       4, 3);

  mainLayout->addWidget(darkMedianFilterCheckBox,      1, 4);
  mainLayout->addWidget(flatFieldMedianFilterCheckBox, 2, 4);
  mainLayout->addWidget(inputMedianFilterCheckBox,     3, 4);
  mainLayout->addWidget(outputdMedianFilterCheckBox,   4, 4);

  mainLayout->addWidget(darkPathButton,      1, 5);
  mainLayout->addWidget(flatFieldPathButton, 2, 5);
  mainLayout->addWidget(inputPathButton,     3, 5);
  mainLayout->addWidget(outputPathButton,    4, 5);

  mainLayout->addWidget(okButton,            6, 1, 1, 2, Qt::AlignHCenter);
  mainLayout->addWidget(cancelButton,        6, 3, 1, 3, Qt::AlignHCenter);

  setLayout(mainLayout);

  setWindowTitle(tr("Projection Pre Processing"));
}


void
PreProcessDlg::initStatus() {
  if(darkUseCheckBox->isChecked()) {
    darkPathEdit->setEnabled(true);
    darkMedianFilterCheckBox->setEnabled(true);
    darkPathButton->setEnabled(true);
  } else {
    darkPathEdit->setEnabled(false);
    darkMedianFilterCheckBox->setEnabled(false);
    darkPathButton->setEnabled(false);
  }

  if(flatFieldUseCheckBox->isChecked()) {
    flatFieldPathEdit->setEnabled(true);
    flatFieldMedianFilterCheckBox->setEnabled(true);
    flatFieldPathButton->setEnabled(true);
  } else {
    flatFieldPathEdit->setEnabled(false);
    flatFieldMedianFilterCheckBox->setEnabled(false);
    flatFieldPathButton->setEnabled(false);
  }
}


void
PreProcessDlg::initSignals() {
  connect(darkUseCheckBox, SIGNAL(clicked()), this, SLOT(onDarkUseCheckBox()));
  connect(flatFieldUseCheckBox, SIGNAL(clicked()), this, SLOT(onFlatFieldUseCheckBox()));

  connect(darkPathButton, SIGNAL(clicked(bool)), this, SLOT(onDarkPathButtonPressed()));
  connect(flatFieldPathButton, SIGNAL(clicked(bool)), this, SLOT(onFlatFieldPathButtonPressed()));
  connect(inputPathButton, SIGNAL(clicked(bool)), this, SLOT(onInputPathButtonPressed()));
  connect(outputPathButton, SIGNAL(clicked(bool)), this, SLOT(onOutputPathButtonPressed()));
  connect(okButton, SIGNAL(clicked(bool)), this, SLOT(onOkButtonPressed()));
  connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(onCancelButtonPressed()));
}


QString
PreProcessDlg::getDarkPath() {
  return darkPathEdit->text();
}


QString
PreProcessDlg::getFlatFieldPath() {
  return flatFieldPathEdit->text();
}


QString
PreProcessDlg::getInputPath() {
  return inputPathEdit->text();
}


QString
PreProcessDlg::getOutputPath() {
  return outputPathEdit->text();
}


bool
PreProcessDlg::getDarkUse() {
  return darkUseCheckBox->isChecked();
}


bool
PreProcessDlg::getFlatFieldUse() {
  return flatFieldUseCheckBox->isChecked();
}


bool
PreProcessDlg::getDarkMedianFilter() {
  return darkMedianFilterCheckBox->isChecked();
}


bool
PreProcessDlg::getFlatFieldMedianFilter() {
  return flatFieldMedianFilterCheckBox->isChecked();
}


bool
PreProcessDlg::getInputMedianFilter() {
  return inputMedianFilterCheckBox->isChecked();
}


bool
PreProcessDlg::getOutputdMedianFilter() {
  return outputdMedianFilterCheckBox->isChecked();
}


float
PreProcessDlg::getDegPerStep() {
  return degPerStepEdit->text().toFloat();
}


void
PreProcessDlg::onDarkUseCheckBox() {
  bool bStatus = darkUseCheckBox->isChecked();
  darkUseCheckBox->setChecked(bStatus);
  darkPathEdit->setEnabled(bStatus);
  darkMedianFilterCheckBox->setEnabled(bStatus);
  darkPathButton->setEnabled(bStatus);
}


void
PreProcessDlg::onFlatFieldUseCheckBox() {
  bool bStatus = flatFieldUseCheckBox->isChecked();
  flatFieldUseCheckBox->setChecked(bStatus);
  flatFieldPathEdit->setEnabled(bStatus);
  flatFieldMedianFilterCheckBox->setEnabled(bStatus);
  flatFieldPathButton->setEnabled(bStatus);
}


void
PreProcessDlg::onDarkPathButtonPressed() {
  QString dir = QFileDialog::getExistingDirectory(this, tr("Select Dark Images Directory"),
                                                  darkPathEdit->text(),
                                                  QFileDialog::DontResolveSymlinks |
                                                  QFileDialog::DontUseNativeDialog);
  if(dir != QString("")) {
    darkPathEdit->setText(dir);
  }
}

void
PreProcessDlg::onFlatFieldPathButtonPressed() {
  QString dir = QFileDialog::getExistingDirectory(this, tr("Select Flat-Field Images Directory"),
                                                  flatFieldPathEdit->text(),
                                                  QFileDialog::DontResolveSymlinks |
                                                  QFileDialog::DontUseNativeDialog);
  if(dir != QString("")) {
    flatFieldPathEdit->setText(dir);
  }
}


void
PreProcessDlg::onInputPathButtonPressed() {
  QString dir = QFileDialog::getExistingDirectory(this, tr("Select Input Projection Directory"),
                                                  inputPathEdit->text(),
                                                  QFileDialog::DontResolveSymlinks |
                                                  QFileDialog::DontUseNativeDialog);
  if(dir != QString("")) {
    inputPathEdit->setText(dir);
  }
}

void
PreProcessDlg::onOutputPathButtonPressed() {
  QString dir = QFileDialog::getExistingDirectory(this, tr("Select Output Normalized Projections Directory"),
                                                  outputPathEdit->text(),
                                                  QFileDialog::DontResolveSymlinks |
                                                  QFileDialog::DontUseNativeDialog);
  if(dir != QString("")) {
    outputPathEdit->setText(dir);
  }
}


void
PreProcessDlg::onOkButtonPressed() {
  QSettings settings("Gabriele Salvato", "QtNeuTomo");
  settings.setValue("PreProcessDlg/geometry",      saveGeometry());

  // Save File Paths
  settings.setValue("PreProcessDlg/darkPath",      darkPathEdit->text());
  settings.setValue("PreProcessDlg/flatFieldPath", flatFieldPathEdit->text());
  settings.setValue("PreProcessDlg/inputPath",     inputPathEdit->text());
  settings.setValue("PreProcessDlg/outputPath",    outputPathEdit->text());

  // Save Angular Step
  settings.setValue("PreProcessDlg/degPerStep",    degPerStepEdit->text());

  // Save Button Status...use..
  settings.setValue("PreProcessDlg/darkUse",      darkUseCheckBox->isChecked());
  settings.setValue("PreProcessDlg/flatFieldUse", flatFieldUseCheckBox->isChecked());

  // Save Button Status...filter...
  settings.setValue("PreProcessDlg/darkMedian",      darkMedianFilterCheckBox->isChecked());
  settings.setValue("PreProcessDlg/flatFieldMedian", flatFieldMedianFilterCheckBox->isChecked());
  settings.setValue("PreProcessDlg/inputMedian",     inputMedianFilterCheckBox->isChecked());
  settings.setValue("PreProcessDlg/outputdMedian",   outputdMedianFilterCheckBox->isChecked());
  accept();
}


void
PreProcessDlg::onCancelButtonPressed() {
  QSettings settings("Gabriele Salvato", "QtNeuTomo");
  settings.setValue("PreProcessDlg/geometry", saveGeometry());
  reject();
}

