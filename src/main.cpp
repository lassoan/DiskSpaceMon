#include <stdio.h>
#include <stdlib.h>
#include "MyMainWindow.h"
#include <QApplication>
#include "QIcon"

#include <QMessageBox>

int main(int argc, char * argv[]) 
{
	QApplication myApp(argc,argv);

  if (!QSystemTrayIcon::isSystemTrayAvailable()) 
  {
    QMessageBox::critical(0,
      QObject::tr("DiskSpaceMon"),
      QObject::tr("I couldn't detect any system tray on this system."));
    return 1;
  }

  QCoreApplication::setOrganizationName("PerkLab");
  QCoreApplication::setOrganizationDomain("perk.cs.queensu.ca");
  QCoreApplication::setApplicationName("DiskSpaceMon");

  QApplication::setQuitOnLastWindowClosed(false);

	MyMainWindow wnd;

	return myApp.exec();

}
