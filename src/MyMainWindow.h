#ifndef MYMAINWINDOW_H
#define MYMAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QDateTime>
#include "ui_MyMainWindow.h"

class MyMainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MyMainWindow(QWidget *parent = 0);
  ~MyMainWindow();

public slots:
  void on_alertLevelGbSpinBox_valueChanged(double levelGb);
  void messageAcknowledged();
  void checkSpaceNow();
  void setVisible(bool visible);
  void closeEvent(QCloseEvent *event);
  void iconActivated(QSystemTrayIcon::ActivationReason reason);
  void messageClicked();
  void createActions();
  void createTrayIcon();
  void writeSettings();
  void readSettings();

signals: 
  void alertLevelChangedKb(float level);
  
private:
  void getFreeTotalSpace(const QString& sDirPath,double& fTotal, double& fFree) const;
  QString MyMainWindow::diskSpaceSummary();

  Ui::MyMainWindowClass ui;

  QAction *m_minimizeAction;
  QAction *m_maximizeAction;
  QAction *m_restoreAction;
  QAction *m_quitAction;

  QSystemTrayIcon *m_trayIcon;
  QMenu *m_trayIconMenu;

  QDateTime m_LastMessageAcknowledgeTime;
  QTimer *m_Timer;
  int m_checkIntervalSec;     
  QMessageBox *m_alertMessageBox;

};

#endif // MYMAINWINDOW_H
