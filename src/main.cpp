
//  qnetwalk/main.cpp
//  Copyright (C) 2004-2008, Andi Peredri <andi@ukr.net>
//  Ported to Symbian by Ahmad Mushtaq <ahmad.mushtaq@gmail.com>

#include <QApplication>
#include "mainwindow.h"


#ifdef Q_OS_SYMBIAN
#include <eikenv.h>
#include <eikappui.h>
#include <aknenv.h>
#include <aknappui.h>
#endif

int main(int argc, char ** argv)
{
#ifdef Q_OS_SYMBIAN
   // QApplication::setAttribute (Qt::AA_S60DontConstructApplicationPanes);
#endif

    QApplication app(argc, argv);

    MainWindow window;

#if defined(Q_OS_SYMBIAN) || defined(Q_WS_MAEMO_5)
    window.showFullScreen();
#else
   // window.show();
    window.show();
#endif

#ifdef Q_OS_SYMBIAN
    CAknAppUi* appUi = dynamic_cast<CAknAppUi*> (CEikonEnv::Static()->AppUi());
    if (appUi) {
        QT_TRAP_THROWING(appUi->SetOrientationL(CAknAppUi::EAppUiOrientationPortrait));
    }
#endif

    return app.exec();
}

