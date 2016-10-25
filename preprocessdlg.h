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
#ifndef PREPROCESSDLG_H
#define PREPROCESSDLG_H

#include <QDialog>

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QPushButton)

class PreProcessDlg : public QDialog
{
  Q_OBJECT

public:
  PreProcessDlg(QWidget *parent = 0);

private:
  void createWidgets();
  void buildLayout();
  void initStatus();
  void initSignals();

private:
  QLabel* darkPathLabel;
  QLabel* flatFieldPathLabel;
  QLabel* inputPathLabel;
  QLabel* outputPathLabel;
  QLabel* degPerStepLabel;

  QLineEdit* darkPathEdit;
  QLineEdit* flatFieldPathEdit;
  QLineEdit* inputPathEdit;
  QLineEdit* outputPathEdit;
  QLineEdit* degPerStepEdit;

  QCheckBox* darkUseCheckBox;
  QCheckBox* flatFieldUseCheckBox;

  QCheckBox* darkMedianFilterCheckBox;
  QCheckBox* flatFieldMedianFilterCheckBox;
  QCheckBox* inputMedianFilterCheckBox;
  QCheckBox* outputdMedianFilterCheckBox;

  QPushButton* darkPathButton;
  QPushButton* flatFieldPathButton;
  QPushButton* inputPathButton;
  QPushButton* outputPathButton;
  QPushButton* okButton;
  QPushButton* cancelButton;

public slots:
  void onDarkUseCheckBox();
  void onFlatFieldUseCheckBox();
  void onDarkPathButtonPressed();
  void onFlatFieldPathButtonPressed();
  void onInputPathButtonPressed();
  void onOutputPathButtonPressed();
  void onOkButtonPressed();
  void onCancelButtonPressed();

public:
  QString getDarkPath();
  QString getFlatFieldPath();
  QString getInputPath();
  QString getOutputPath();

  bool getDarkUse();
  bool getFlatFieldUse();
  bool getDarkMedianFilter();
  bool getFlatFieldMedianFilter();
  bool getInputMedianFilter();
  bool getOutputdMedianFilter();

  float getDegPerStep();
};

#endif // PREPROCESSDLG_H
