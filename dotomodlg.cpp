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
#include <QSettings>

DoTomoDlg::DoTomoDlg(QWidget *parent)
  : QDialog(parent)
{
  createWidgets();
  buildLayout();
  // Restore previous section values
  QSettings settings("Gabriele Salvato", "QtNeuTomo");
  restoreGeometry(settings.value("ChooseRoiDlg/DoTomoDlg").toByteArray());
  normalizedPathEdit->setText(settings.value("ChooseRoiDlg/normalizedPath", ".").toByteArray());
  slicesPathEdit->setText(settings.value("ChooseRoiDlg/slicesPath", ".").toByteArray());
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
  slicesPathButton->setToolTip(tr("Change Path for the output slices"));
}


void
DoTomoDlg::buildLayout() {

}


void
DoTomoDlg::initSignals() {

}
