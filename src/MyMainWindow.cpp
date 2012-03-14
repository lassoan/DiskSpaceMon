#ifdef _WIN32
#include "Windows.h" // for getting disk size on Windows
#endif _WIN32

#include "MyMainWindow.h"

#include <QDir>
#include <QTimer>
#include <QMessageBox>
#include <QSettings>

MyMainWindow::MyMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	ui.setupUi(this);

  m_checkIntervalSec=5;

  m_Timer = new QTimer(this);
  m_alertMessageBox = new QMessageBox(0); // have 0 parent so that the messagebox wil be centered on screen
  m_alertMessageBox->setWindowTitle("Disk space alert");
  m_alertMessageBox->setIcon(QMessageBox::Critical);

  // Keep warning popup on top
  Qt::WindowFlags flags = m_alertMessageBox->windowFlags();
  flags |= Qt::WindowStaysOnTopHint;
  m_alertMessageBox->setWindowFlags(flags);

  connect(m_alertMessageBox, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(messageAcknowledged()));

  connect(m_Timer, SIGNAL(timeout()), this, SLOT(checkSpaceNow()));

  m_Timer->setSingleShot(false);  
  m_Timer->start(m_checkIntervalSec*1000);

  // Don't initialize m_LastMessageAcknowledgeTime, then at startup it'll be invalid,
  // which will trigger immediate message display on application start.
  //m_LastMessageAcknowledgeTime=QDateTime::currentDateTime();

  createActions();
  createTrayIcon();

  readSettings();

  connect(m_trayIcon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));
  connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

  m_trayIcon->show();

  setVisible(false);
}

MyMainWindow::~MyMainWindow()
{
  delete m_alertMessageBox;
  m_alertMessageBox=NULL;
}

void MyMainWindow::writeSettings()
{
  QSettings settings;

  settings.beginGroup("AlertSettings");
  settings.setValue("monitoredPath", ui.monitoredPathEdit->text());
  settings.setValue("alertLevelGB", ui.alertLevelGbSpinBox->value());
  settings.setValue("alertMessage", ui.alertMessageEdit->toPlainText());
  settings.setValue("alertMessageDisplayFrequencySec", ui.alertMessageDisplayFrequencySecSpinBox->value());
  settings.setValue("alertMessagePopup", ui.displayAlertInPopupCheckBox->isChecked());
  settings.endGroup();
}

void MyMainWindow::readSettings()
{
  QSettings settings;

  settings.beginGroup("AlertSettings");
  ui.monitoredPathEdit->setText(settings.value("monitoredPath", ui.monitoredPathEdit->text()).toString());
  ui.alertLevelGbSpinBox->setValue(settings.value("alertLevelGB", ui.alertLevelGbSpinBox->value()).toDouble());
  ui.alertMessageEdit->setPlainText(settings.value("alertMessage", ui.alertMessageEdit->toPlainText()).toString());
  ui.alertMessageDisplayFrequencySecSpinBox->setValue(settings.value("alertMessageDisplayFrequencySec", ui.alertMessageDisplayFrequencySecSpinBox->value()).toInt());
  ui.displayAlertInPopupCheckBox->setChecked(settings.value("alertMessagePopup", ui.displayAlertInPopupCheckBox->isChecked()).toBool());
  settings.endGroup();
}

void MyMainWindow::on_alertLevelGbSpinBox_valueChanged(double levelGb)
{
  emit alertLevelChangedKb(levelGb*1024*1024);
}

void MyMainWindow::setVisible(bool visible)
{
    m_minimizeAction->setEnabled(visible);
    m_restoreAction->setEnabled(isMaximized() || !visible);
    QMainWindow::setVisible(visible);
}

void MyMainWindow::closeEvent(QCloseEvent *event)
{
    if (m_trayIcon->isVisible()) 
    {
        writeSettings();
        hide();
        event->ignore();
    }
}

QString MyMainWindow::diskSpaceSummary()
{
  QString dirPath=ui.monitoredPathEdit->text();
  double totalKb=0.0;
  double freeSpaceKb=0.0;
  getFreeTotalSpace(dirPath, totalKb, freeSpaceKb);
  QString freeGbString=freeSpaceKb<0?"?":QString("%1 GB").arg(freeSpaceKb/1024.0/1024.0, 0, 'g', 6);
  QString totalGbString=totalKb<0?"?":QString("%1 GB").arg(totalKb/1024.0/1024.0, 0, 'g', 6);
  ui.availableSpaceGbLabel->setText(freeGbString);
  QString msg=QString("Monitored directory: %1\nTotal space: %2\nFree space: %3")
    .arg(ui.monitoredPathEdit->text())
    .arg(totalGbString)
    .arg(freeGbString);
  return msg;
}

void MyMainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
  switch (reason) 
  {
  case QSystemTrayIcon::Trigger: // left click
    {
      m_trayIcon->showMessage("Disk space information", diskSpaceSummary(), QSystemTrayIcon::MessageIcon(QSystemTrayIcon::Information));
      m_LastMessageAcknowledgeTime=QDateTime::currentDateTime(); 
    }
    break;
  case QSystemTrayIcon::DoubleClick:
    show();
    raise();
    break;
  case QSystemTrayIcon::MiddleClick:
    //MyMainWindow::messageAcknowledged();
    break;
  default:
    ;
  }
}

void MyMainWindow::messageClicked()
{
    MyMainWindow::messageAcknowledged();
}

void MyMainWindow::createActions()
{
    m_restoreAction = new QAction(tr("&Show configuration"), this);
    connect(m_restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    m_minimizeAction = new QAction(tr("&Hide configuration"), this);
    connect(m_minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));
    connect(m_minimizeAction, SIGNAL(triggered()), this, SLOT(writeSettings()));

    m_quitAction = new QAction(tr("&Quit"), this);
    connect(m_quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(m_quitAction, SIGNAL(triggered()), this, SLOT(writeSettings()));
}

void MyMainWindow::createTrayIcon()
{
    m_trayIconMenu = new QMenu(this);    
    m_trayIconMenu->addAction(m_restoreAction);
    m_trayIconMenu->addAction(m_minimizeAction);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_quitAction);

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setContextMenu(m_trayIconMenu);

    m_trayIcon->setIcon(QIcon(":/database"));
    m_trayIcon->setToolTip("Disk space monitor");

    m_trayIcon->setVisible(true);
}

void MyMainWindow::checkSpaceNow()
{
  QString dirPath=ui.monitoredPathEdit->text();
  double totalKb=0.0;
  double freeSpaceKb=0.0;
  getFreeTotalSpace(dirPath, totalKb, freeSpaceKb);

  QString freeGbString("?");
  if (freeSpaceKb>0)
  {
    freeGbString=QString("%1 GB").arg(freeSpaceKb/1024.0/1024.0, 0, 'g', 6);
  }
  ui.availableSpaceGbLabel->setText(freeGbString);

  if (freeSpaceKb<0)
  {
    m_alertMessageBox->setText("Warning: Free disk space is unknown. Continue with care.");
  }
  else
  {
    int minimumSpaceKb=ui.alertLevelGbSpinBox->value()*1024*1024;
    if (freeSpaceKb<minimumSpaceKb)
    {
      m_trayIcon->setIcon(QIcon(":/database-error"));
      int timePassedSinceAcknowledgeSec=m_LastMessageAcknowledgeTime.msecsTo(QDateTime::currentDateTime())/1000.0;
      if (!m_LastMessageAcknowledgeTime.isValid() || timePassedSinceAcknowledgeSec>ui.alertMessageDisplayFrequencySecSpinBox->value())
      {
        // enough time passed since the last acknowledgement
        if (ui.displayAlertInPopupCheckBox->isChecked())
        {
          m_alertMessageBox->setText(ui.alertMessageEdit->toPlainText()+"\n\n"+diskSpaceSummary());
          m_alertMessageBox->show();
          m_alertMessageBox->raise();
        }
        else
        {
          m_trayIcon->showMessage("Disk space alert", ui.alertMessageEdit->toPlainText()+"\n\n"+diskSpaceSummary(), QSystemTrayIcon::MessageIcon(QSystemTrayIcon::Critical), 60000 * 1000);          
        }
        m_LastMessageAcknowledgeTime=QDateTime::currentDateTime();  
      }
    }
    else
    {
      m_alertMessageBox->hide();
      m_trayIcon->setIcon(QIcon(":/database-ok"));
    }
  }

}

void MyMainWindow::getFreeTotalSpace(const QString& sDirPath,double& fTotalKb, double& fFreeKb) const
{
  const float fKB=1024.0;

  fTotalKb=-1.0;
  fFreeKb=-1.0;

#ifdef _WIN32

  QString sCurDir = QDir::current().absolutePath();
  QDir::setCurrent( sDirPath );

  ULARGE_INTEGER free,total;
  if ( !::GetDiskFreeSpaceExA( 0 , &free , &total , NULL ) )
  {
    // failed to get free space
    return;
  }

  QDir::setCurrent( sCurDir );

  fFreeKb = static_cast<double>( static_cast<__int64>(free.QuadPart) ) / fKB;
  fTotalKb = static_cast<double>( static_cast<__int64>(total.QuadPart) ) / fKB;

#else //linux

  struct stat stst;
  struct statfs stfs;

  if ( ::stat(sDirPath.local8Bit(),&stst) == -1 ) return false;
  if ( ::statfs(sDirPath.local8Bit(),&stfs) == -1 ) return false;

  fFreeKb = stfs.f_bavail * ( stst.st_blksize / fKB );
  fTotalKb = stfs.f_blocks * ( stst.st_blksize / fKB );

#endif // _WIN32

}

void MyMainWindow::messageAcknowledged()
{
  m_LastMessageAcknowledgeTime=QDateTime::currentDateTime();  
}
