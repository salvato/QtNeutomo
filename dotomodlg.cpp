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
#include "dotomodlg.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QFileDialog>
#include <QSettings>

DoTomoDlg::DoTomoDlg(QWidget *parent)
  : QDialog(parent)
{
  createWidgets();
  buildLayout();
  // Restore previous section values
  QSettings settings("Gabriele Salvato", "QtNeuTomo");
  restoreGeometry(settings.value("DoTomoDlg/geometry").toByteArray());
  normalizedPathEdit->setText(settings.value("DoTomoDlg/normalizedPath", ".").toByteArray());
  slicesPathEdit->setText(settings.value("DoTomoDlg/slicesPath", ".").toByteArray());
  initSignals();
}


void
DoTomoDlg::createWidgets() {
  normalizedPathLabel = new QLabel(tr("Normalized Images Path"));
  slicesPathLabel     = new QLabel(tr("Output Slices Path"));

  normalizedPathEdit = new QLineEdit(tr("."));
  slicesPathEdit     = new QLineEdit(tr("."));

  normalizedPathButton = new QPushButton("...");
  slicesPathButton     = new QPushButton("...");
  okButton             = new QPushButton("Done");
  cancelButton         = new QPushButton("Cancel");

  // Tooltips:
  normalizedPathEdit->setToolTip(tr("Path containing the normalized projections"));
  slicesPathEdit->setToolTip(tr("Path for the output slices"));
  normalizedPathButton->setToolTip(tr("Change the path containing the normalized projections"));
  slicesPathButton->setToolTip(tr("Change the path for the output slices"));
}


void
DoTomoDlg::buildLayout() {
  QGridLayout* mainLayout = new QGridLayout();
  mainLayout->addWidget(normalizedPathLabel, 1, 1);
  mainLayout->addWidget(slicesPathLabel,     2, 1);

  mainLayout->addWidget(normalizedPathEdit, 1, 2, 1, 2);
  mainLayout->addWidget(slicesPathEdit,     2, 2, 1, 2);

  mainLayout->addWidget(normalizedPathButton, 1, 4);
  mainLayout->addWidget(slicesPathButton,     2, 4);

  mainLayout->addWidget(okButton,     6, 1, 1, 2, Qt::AlignHCenter);
  mainLayout->addWidget(cancelButton, 6, 3, 1, 3, Qt::AlignHCenter);

  setLayout(mainLayout);

  setWindowTitle(tr("Tomo Setup"));
}


void
DoTomoDlg::initSignals() {
  connect(normalizedPathButton, SIGNAL(clicked(bool)), this, SLOT(onNormalizedPathButtonPressed()));
  connect(slicesPathButton, SIGNAL(clicked(bool)), this, SLOT(onSlicesPathButtonPressed()));
  connect(okButton, SIGNAL(clicked(bool)), this, SLOT(onOkButtonPressed()));
  connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(onCancelButtonPressed()));
}


QString
DoTomoDlg::getSlicesPath() {
  return slicesPathEdit->text();
}


QString
DoTomoDlg::getNormalizedPath() {
  return normalizedPathEdit->text();
}


void
DoTomoDlg::onNormalizedPathButtonPressed() {
  QString dir = QFileDialog::getExistingDirectory(this, tr("Select Normalized Projection Directory"),
                                                  normalizedPathEdit->text(),
                                                  QFileDialog::DontResolveSymlinks |
                                                  QFileDialog::DontUseNativeDialog);
  if(dir != QString("")) {
    normalizedPathEdit->setText(dir);
  }
}


void
DoTomoDlg::onSlicesPathButtonPressed() {
  QString dir = QFileDialog::getExistingDirectory(this, tr("Select Output Slices Directory"),
                                                  slicesPathEdit->text(),
                                                  QFileDialog::DontResolveSymlinks |
                                                  QFileDialog::DontUseNativeDialog);
  if(dir != QString("")) {
    slicesPathEdit->setText(dir);
  }
}


void
DoTomoDlg::onOkButtonPressed() {
  QSettings settings("Gabriele Salvato", "QtNeuTomo");
  settings.setValue("DoTomoDlg/geometry",      saveGeometry());

  // Save File Paths
  settings.setValue("DoTomoDlg/normalizedPath",      normalizedPathEdit->text());
  settings.setValue("DoTomoDlg/slicesPath", slicesPathEdit->text());
  accept();
}


void
DoTomoDlg::onCancelButtonPressed() {
  QSettings settings("Gabriele Salvato", "QtNeuTomo");
  settings.setValue("DoTomoDlg/geometry", saveGeometry());
  reject();
}



