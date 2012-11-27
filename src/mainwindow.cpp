
//  qnetwalk/mainwindow.cpp
//  Copyright (C) 2004-2008, Andi Peredri <andi@ukr.net>
//  Ported to BlackBerry 10 By Antti Pohjola <summeli@summeli.fi>

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QEventLoop>
#include <QFile>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QStringList>
#include <QSound>
#include <QUrl>
#include <QDebug>
#include <QSequentialAnimationGroup>
#include <QPropertyAnimation>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cell.h"
#include "mainwindow.h"
#include "mydialog.h"

#if defined(Q_OS_SYMBIAN)
#include <e32std.h>
#include <touchfeedback.h>
#endif

#define NO_SOUND
// Global variables
static QMap<Cell::Dirs, Cell::Dirs> contrdirs;


// Constants
static const char* kApplicationName = "FixTheNet";

MainWindow::~MainWindow()
{
    delete myDialog;
}

MainWindow::MainWindow() : QMainWindow(0, Qt::Dialog)
{
    contrdirs[Cell::U] = Cell::D;
    contrdirs[Cell::R] = Cell::L;
    contrdirs[Cell::D] = Cell::U;
    contrdirs[Cell::L] = Cell::R;

    const QString sndDir = "netwalksounds\\";

#ifndef NO_SOUND
    winSound     = new QSound(sndDir + "win.wav", this);
    turnSound    = new QSound(sndDir + "turn.wav", this);
    clickSound   = new QSound(sndDir + "click.wav", this);
    startSound   = new QSound(sndDir + "start.wav", this);
    connectSound = new QSound(sndDir + "connect.wav", this);
#endif
    QSettings settings(kApplicationName);

     const QString userEnv = tr("My Name");

#ifdef Q_OS_SYMBIAN
    touchFeedback = MTouchFeedback::Instance();
#endif

    user = settings.value("Username", userEnv).toString();
    bool isSound = settings.value("Sound", false).toBool();
    myDialog = NULL;

    highscores = settings.value("Highscores").toStringList();
    if(highscores.count() != NumHighscores * 8)
    {
        highscores.clear();
	for(int i = 1; i < 5; i++)
	{
            for(int scores = 100 * i; scores < 110 * i; scores += i)
	    {
		highscores.append("...");
		highscores.append(QString::number(scores));
	    }
	}
    }

    skill = settings.value("Skill", Novice).toInt();
    if((skill != Novice) && (skill != Normal) && (skill != Expert))
	skill = Master;

    setWindowTitle(tr(kApplicationName));
  //  setFixedSize(minimumSizeHint());
    //setGeometry(0,200,720,800);
    setFocusPolicy(Qt::StrongFocus);

    menu = new QMenu(this);
    connect (menu, SIGNAL(triggered(QAction*)), this, SLOT(touch()));

    menu->addAction(QPixmap(), tr("New game"), this, SLOT(newGame()));

    //No Sound
    /*
    soundAction = menu->addAction(QPixmap(), tr("Sound"));
    soundAction->setCheckable(true);
    soundAction->setChecked(isSound);
*/
    menu->addAction(QPixmap(), tr("Highscores"), this, SLOT(showHighscores()));
    menu->addAction(tr("How to play"), this, SLOT(help()));
    menu->addAction(QPixmap(), tr("About"), this, SLOT(about()));
    menu->addAction(QPixmap(), tr("Quit"), qApp, SLOT(closeAllWindows()));
   // menu->setStyleSheet("QMenu { font-size:40px;}");
    QActionGroup * actions = new QActionGroup(this);
    connect(actions, SIGNAL(triggered(QAction *)), SLOT(triggeredSkill(QAction *)));

    QAction * action = actions->addAction(tr("Novice"));
    action->setData(Novice);
        
    action = actions->addAction(tr("Normal"));
    action->setData(Normal);
    
    action = actions->addAction(tr("Expert"));
    action->setData(Expert);

    action = actions->addAction(tr("Master"));
    action->setData(Master);
    
    foreach(QAction * a, actions->actions())
    {
        a->setCheckable(true);
        if(a->data() == skill)
            a->setChecked(true);
    }

    skillMenu = new QMenu(tr("Skill"), this);
    //skillMenu->setStyleSheet("QMenu { font-size:40px;}");
    skillMenu->addActions(actions->actions());

    QVBoxLayout * vboxLayout = new QVBoxLayout;
    vboxLayout->setMargin(0);
    vboxLayout->setSpacing(0);
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(vboxLayout);

    QHBoxLayout* hboxLayout = new QHBoxLayout;
    hboxLayout->setMargin(5); hboxLayout->setAlignment(Qt::AlignCenter);
    QWidget* labelsWidget = new QWidget(this);
    labelsWidget->setLayout(hboxLayout);
    movesLabel = new QLabel(tr("Moves") + QString(": 0"), labelsWidget);
    hboxLayout->addWidget(movesLabel);
    hboxLayout->addStretch();
    timeLabel = new QLabel(tr("Time") + QString(": 00:00"), labelsWidget);
    hboxLayout->addWidget(timeLabel);

    vboxLayout->addWidget(labelsWidget);

    const int cellsize = QPixmap(":/pics/background.png").width();
    const int gridsize = cellsize * MasterBoardSize + 2;

    QGridLayout * grid = new QGridLayout;
    grid->setMargin(0);
    grid->setSpacing(0);

    QFrame* frame = new QFrame(centralWidget);
    frame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    frame->setFixedSize(gridsize, gridsize);
    frame->setLayout(grid);
    //set margins to set the game to the center of the screen
    frame->setContentsMargins(7,10,0,0);
    vboxLayout->addWidget(frame);


    infoLabel = new QLabel("", this);
    infoLabel->setAlignment(Qt::AlignHCenter);
    vboxLayout->addWidget(infoLabel);

    skillLabel = new QLabel("", this);
    skillLabel->setAlignment(Qt::AlignCenter);
    vboxLayout->addWidget(skillLabel);
    vboxLayout->addStretch();

    QWidget* buttonsContainer = new QWidget;
    QHBoxLayout* buttonsLayout = new QHBoxLayout(buttonsContainer);


    QPushButton* menuButton = new QPushButton(tr("Menu"), this);
    menuButton->setMinimumSize(150, 120);
    connect ( menuButton, SIGNAL(clicked()), this, SLOT(showMenu()));
    buttonsLayout->addWidget(menuButton);

    buttonsLayout->addStretch();

    newGameButton = new QPushButton("", this);
    newGameButton->setIcon(QIcon(":/pics/refresh.png"));
    newGameButton->setIconSize(QSize(70,70));
    newGameButton->setMinimumSize(150, 120);
    connect (newGameButton, SIGNAL(clicked()), this, SLOT(newGame()));
    connect (newGameButton, SIGNAL(clicked()), this, SLOT(touch()));
    buttonsLayout->addWidget(newGameButton);

    buttonsLayout->addStretch();

    QPushButton* skillButton = new QPushButton(tr("Skill"), this);
    skillButton->setMinimumSize(150, 120);
    connect (skillButton, SIGNAL(clicked()), this, SLOT(showMenu()));
    buttonsLayout->addWidget(skillButton);

    vboxLayout->addWidget(buttonsContainer);

    setCentralWidget(centralWidget);

    Cell::initPixmaps();
    for(int i = 0; i < MasterBoardSize * MasterBoardSize; i++)
    {
	board[i] = new Cell(frame, i);
	board[i]->setFixedSize(cellsize, cellsize);
#ifdef Q_OS_SYMBIAN
        board[i]->setTouchFeedback(touchFeedback);
#endif
        connect(board[i], SIGNAL(noRotation()), SLOT(noRotation()));
        connect(board[i], SIGNAL(startRotation()), SLOT(startRotation()));
        connect(board[i], SIGNAL(finishRotation()), SLOT(finishRotation()));
	grid->addWidget(board[i], i / MasterBoardSize, i % MasterBoardSize);
    }


    for(int y = 0; y < MasterBoardSize; y++)
    {
	for(int x = 0; x < MasterBoardSize; x++)
	{
            const int i = MasterBoardSize * y + x;
	    if(y == 0)
	    {
		if(x == 0)
		    board[i]->setShadow(Cell::TopLeftShadow);
                else if(x == MasterBoardSize - 1)
		    board[i]->setShadow(Cell::TopRightShadow);
		else
		    board[i]->setShadow(Cell::TopShadow);
	    }
	    else if(y == MasterBoardSize - 1)
	    {
		if(x == 0)
		    board[i]->setShadow(Cell::BottomLeftShadow);
                else if(x == MasterBoardSize - 1)
		    board[i]->setShadow(Cell::BottomRightShadow);
		else
		    board[i]->setShadow(Cell::BottomShadow);
	    }
            else if(x == 0)
		board[i]->setShadow(Cell::LeftShadow);
            else if(x == MasterBoardSize - 1)
		board[i]->setShadow(Cell::RightShadow);
            else
		board[i]->setShadow(Cell::NoShadow);
	}
    }
    srand(time(0));
    setSkill(skill);
    timerId = 0;
    menu->setMinimumWidth(300);
    menuBar()->addMenu(menu);
    menuBar()->addMenu(skillMenu);
    skillMenu->setMinimumWidth(300);

    qApp->setStyleSheet(" QMenu { border: 5px solid black; border-color: #c0c0c0; color: #000000; background-color: #f0f0f0; } QMenu::item { padding-right: 40px; padding-left: 40px; border:5px; height: 40px; background-color: transparent;} QMenu::item:selected { background-color: #c0c0c0;} QMenu::indicator::checked {image: url(:/pics/mark_30.png); }");

}

void  MainWindow::showMenu()
{
    touch();


    const QPushButton* button = static_cast<QPushButton*>(sender());
    if (button->text().compare(tr("Menu")) == 0)
    {
        const QPoint menuPos ((size().width() - menu->sizeHint().width())/2 , (size().height() - menu->sizeHint().height())/2);
        menu->popup(menuPos);
    }
    else
    {
        const QPoint menuPos ((size().width() - skillMenu->sizeHint().width())/2 , (size().height() - skillMenu->sizeHint().height())/2);
        skillMenu->popup(menuPos);
    }
}

QMenu * MainWindow::createPopupMenu()
{
    return 0;
}


void MainWindow::triggeredSkill(QAction * action)
{
    touch();
    setSkill(action->data().toInt());
}

void MainWindow::updateTimeElapsed(int seconds)
{
    const int m = seconds / 60;
    const int s = seconds % 60;
    const int h = m / 60;

    const QTime elapsedTime (h, m, s);

    const QString fString = (h <= 0 ? "mm:ss" : "hh:mm::ss");
    timeLabel->setText( tr("Time") + QString(": " + elapsedTime.toString(fString) ));
}

void MainWindow::updateNumberOfMoves( int num )
{
    numberOfMoves = num;
    movesLabel->setText( tr("Moves") + QString(": " + QString::number(num) ));
}

void MainWindow::newGame()
{
    updateNumberOfMoves(0);
    updateTimeElapsed(0);

    if (timerId)
        killTimer(timerId);

    timerId = 0;

    infoMessage(tr("Tap on a tile to start the game!"));

#ifndef NO_SOUND
    if(soundAction->isChecked())
    {
	startSound->play();
    }
#endif
    for(int i = 0; i < MasterBoardSize * MasterBoardSize; i++)
    {
	board[i]->setDirs(Cell::None);
	board[i]->setConnected(false);
        board[i]->setLocked(false);
	board[i]->setRoot(false);
    }

    const int size = (skill == Novice) ? NoviceBoardSize :
	(skill == Normal) ? NormalBoardSize :
        (skill == Expert) ? ExpertBoardSize : MasterBoardSize;

    const int start = (MasterBoardSize - size) / 2;
    const int rootrow = rand() % size;
    const int rootcol = rand() % size;

    root = board[(start + rootrow) * MasterBoardSize + start + rootcol];
    root->setConnected(true);
    root->setRoot(true);

    while(true)
    {
	for(int row = start; row < start + size; row++)
	    for(int col = start; col < start + size; col++)
		board[row * MasterBoardSize + col]->setDirs(Cell::Free);

	CellList list;
	list.append(root);
	if(rand() % 2) 
	    addRandomDir(list);

	while(!list.isEmpty())
	{
	    if(rand() % 2)
	    {
		addRandomDir(list);
		if(rand() % 2) 
		    addRandomDir(list);
		list.removeFirst();
	    }
	    else
	    {
		list.append(list.first());
		list.removeFirst();
	    }
	}

	int cells = 0;
	for(int i = 0; i < MasterBoardSize * MasterBoardSize; i++)
	{
	    Cell::Dirs d = board[i]->dirs();
	    if((d != Cell::Free) && (d != Cell::None)) 
		cells++;
	}
	if(cells >= MinimumNumCells) 
	    break;
    }

    Cell::setGameOver(false);
    Cell::setFocused(0);
    for(int i = 0; i < MasterBoardSize * MasterBoardSize; i++)
	board[i]->rotate((rand() % 4) * 90);
    updateConnections();
}


bool MainWindow::updateConnections()
{
    bool newconnection[MasterBoardSize * MasterBoardSize];
    for(int i = 0; i < MasterBoardSize * MasterBoardSize; i++)
	newconnection[i] = false;

    CellList list;
    if(!root->isRotated())
    {
        newconnection[root->index()] = true;
	list.append(root);
    }
    while(!list.isEmpty())
    {
	Cell* cell = list.first();
	Cell* ucell = uCell(cell, wrapped);
	Cell* rcell = rCell(cell, wrapped);
	Cell* dcell = dCell(cell, wrapped);
	Cell* lcell = lCell(cell, wrapped);

	if((cell->dirs() & Cell::U) && ucell && (ucell->dirs() & Cell::D) &&
	   !newconnection[ucell->index()] && !ucell->isRotated())
	{
	    newconnection[ucell->index()] = true;
            list.append(ucell);
	}
	if((cell->dirs() & Cell::R) && rcell && (rcell->dirs() & Cell::L) &&
	   !newconnection[rcell->index()] && !rcell->isRotated())
	{
	    newconnection[rcell->index()] = true;
            list.append(rcell);
	}
	if((cell->dirs() & Cell::D) && dcell && (dcell->dirs() & Cell::U) &&
	   !newconnection[dcell->index()] && !dcell->isRotated())
	{
	    newconnection[dcell->index()] = true;
            list.append(dcell);
	}
	if((cell->dirs() & Cell::L) && lcell && (lcell->dirs() & Cell::R) &&
	   !newconnection[lcell->index()] && !lcell->isRotated())
	{
	    newconnection[lcell->index()] = true;
            list.append(lcell);
	}
	list.removeFirst();
    }

    bool isnewconnection = false;
    for(int i = 0; i < MasterBoardSize * MasterBoardSize; i++)
    {
	if(!board[i]->isConnected() && newconnection[i])
	    isnewconnection = true;
	board[i]->setConnected(newconnection[i]);
    }
    return isnewconnection;
}


void MainWindow::addRandomDir(CellList& list)
{
    Cell * cell = list.first();
    Cell * ucell = uCell(cell, wrapped);
    Cell * rcell = rCell(cell, wrapped);
    Cell * dcell = dCell(cell, wrapped);
    Cell * lcell = lCell(cell, wrapped);

    typedef QMap<Cell::Dirs, Cell *> CellMap;
    CellMap freecells;

    if(ucell && ucell->dirs() == Cell::Free) 
	freecells[Cell::U] = ucell;
    if(rcell && rcell->dirs() == Cell::Free) 
	freecells[Cell::R] = rcell;
    if(dcell && dcell->dirs() == Cell::Free) 
	freecells[Cell::D] = dcell;
    if(lcell && lcell->dirs() == Cell::Free) 
	freecells[Cell::L] = lcell;
    if(freecells.isEmpty()) 
	return;

    CellMap::ConstIterator it = freecells.constBegin();
    for(int i = rand() % freecells.count(); i > 0; --i) 
	++it;

    cell->setDirs(Cell::Dirs(cell->dirs() | it.key()));
    it.value()->setDirs(contrdirs[it.key()]);
    list.append(it.value());
}


Cell * MainWindow::uCell(Cell * cell, bool wrap) const
{
    if(cell->index() >= MasterBoardSize)
	return board[cell->index() - MasterBoardSize];
    else if(wrap)
	return board[MasterBoardSize * (MasterBoardSize - 1) + cell->index()];
    else return 0;
}


Cell * MainWindow::dCell(Cell * cell, bool wrap) const
{
    if(cell->index() < MasterBoardSize * (MasterBoardSize - 1))
	return board[cell->index() + MasterBoardSize];
    else if(wrap)
	return board[cell->index() - MasterBoardSize * (MasterBoardSize - 1)];
    else return 0;
}


Cell * MainWindow::lCell(Cell * cell, bool wrap) const
{
    if(cell->index() % MasterBoardSize > 0)
	return board[cell->index() - 1];
    else if(wrap)
	return board[cell->index() - 1 + MasterBoardSize];
    else return 0;
}


Cell * MainWindow::rCell(Cell * cell, bool wrap) const
{
    if(cell->index() % MasterBoardSize < MasterBoardSize - 1)
	return board[cell->index() + 1];
    else if(wrap)
	return board[cell->index() + 1 - MasterBoardSize];
    else return 0;
}


void MainWindow::timerEvent (QTimerEvent * event)
{
    if (event->timerId() == timerId)
    {
        updateTimeElapsed(gameStartTime.elapsed()/1000);

        event->accept();
    }
}

void MainWindow::keyPressEvent(QKeyEvent * event)
{
    Cell * focused = Cell::focused();
    if(focused)
    {
	switch(event->key())
	{
	case Qt::Key_Up:
	    Cell::setFocused(uCell(focused, true));
	    break;
	case Qt::Key_Down:
	    Cell::setFocused(dCell(focused, true));
	    break;
	case Qt::Key_Left:
	    Cell::setFocused(lCell(focused, true));
	    break;
	case Qt::Key_Right:
	    Cell::setFocused(rCell(focused, true));
            break;
	case Qt::Key_PageUp:
	case Qt::Key_Home:
	case Qt::Key_Delete:
	case Qt::Key_Z:
	    focused->start(Cell::RotationToLeft);
            break;
	case Qt::Key_PageDown:
	case Qt::Key_End:
	case Qt::Key_X:
	    focused->start(Cell::RotationToRight);
            break;
	case Qt::Key_Return:
	case Qt::Key_Enter:
	case Qt::Key_Space:
	    focused->start(Cell::LockUnlock);
	}
    }
    else if(event->modifiers() == Qt::NoModifier)
	Cell::setFocused(board[MasterBoardSize * MasterBoardSize / 2]);
}


void MainWindow::noRotation()
{
#ifndef NO_SOUND
    if(soundAction->isChecked())
	clickSound->play();
#endif
}


void MainWindow::startRotation()
{
#ifndef NO_SOUND
    if(soundAction->isChecked())
    	turnSound->play();
#endif
    updateConnections();
}


void MainWindow::finishRotation()
{
#ifndef NO_SOUND
    if(updateConnections() && soundAction->isChecked())
    	connectSound->play();
#else
    updateConnections();
#endif
    if (numberOfMoves==0)
    {
        gameStartTime.start();
        timerId = startTimer(1000);
        infoMessage(QString());
    }
    updateNumberOfMoves(numberOfMoves+1);

    if(isGameOver())
    {
#ifndef NO_SOUND
        if(soundAction->isChecked())
        {
	    winSound->play();            
        }
#endif
        const QString score = highscores.at(2 * (skill + 1) * NumHighscores - 1);
        if(numberOfMoves <= score.toInt())
            addHighscore(numberOfMoves);

        killTimer(timerId);
        timerId = 0;
    }
}


bool MainWindow::isGameOver()
{
    for(int i = 0; i < MasterBoardSize * MasterBoardSize; i++)
    {
	const Cell::Dirs d = board[i]->dirs();
        if((d != Cell::Free) && (d != Cell::None) && !board[i]->isConnected())
            return false;
    }

    for(int i = 0; i < MasterBoardSize * MasterBoardSize; i++)
	board[i]->setLocked(true);
    Cell::setGameOver(true);
    Cell::setFocused(0);
    return true;
}


void MainWindow::showHighscores()
{
    addHighscore(0);
}


void MainWindow::addHighscore(int score)
{
    QDialog * dialog = new QDialog(this);
    dialog->setWindowTitle(tr("Highscores"));
    
    QGridLayout * grid = new QGridLayout(dialog);
    grid->setMargin(10);
    grid->setSpacing(5);
    grid->setColumnStretch(2, 1);
    grid->setRowStretch(NumHighscores + 3, 1);
    grid->setColumnMinimumWidth(0, 40);
    grid->setColumnMinimumWidth(2, 150);

    QLabel * label = new QLabel(dialog);
    label->setPixmap(QPixmap(":/pics/computer2.png"));
    grid->addWidget(label, 0, 0, 2, 1);
    label = new QLabel(QString("<h3>") + dialog->windowTitle(), dialog);
    grid->addWidget(label, 0, 1, 1, 3);

    const QString header = (skill == Novice) ? tr("Novices") :
	(skill == Normal) ? tr("Amateurs") :
	(skill == Expert) ? tr("Experts") : tr("Masters");

    grid->addWidget(new QLabel("<b>#", dialog), 1, 1);
    grid->addWidget(new QLabel("<b>" + header, dialog), 1, 2);
    grid->addWidget(new QLabel("<b>" + tr("Scores"), dialog), 1, 3);

    QFrame * frame = new QFrame(dialog);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    grid->addWidget(frame, 2, 1, 1, 3);

    QLineEdit * line = 0;
    int inserted = 0;
    int index = 2 * skill * NumHighscores;
    for(unsigned int i = 0; i < NumHighscores; i++)
    {
        label = new QLabel(QString::number(i + 1), dialog);
	grid->addWidget(label, i + 3, 1);

	int next = index;
	if((score > 0) && (score <= highscores.at(++next).toInt()) && !line)
	{
            inserted = index;
	    line = new QLineEdit(user, dialog);
	    grid->addWidget(line, i + 3, 2);
            label = new QLabel(QString::number(score), dialog);
	    grid->addWidget(label, i + 3, 3);
	}
	else
	{
	    grid->addWidget(new QLabel(highscores.at(index++), dialog), i + 3, 2);
	    grid->addWidget(new QLabel(highscores.at(index++), dialog), i + 3, 3);
	}
    }
    QPushButton * button = new QPushButton(tr("Ok"), dialog);
    connect(button, SIGNAL(clicked()), dialog, SLOT(accept()));
    const int pos = NumHighscores + 4;
    grid->addWidget(button, pos, 0, 1, 4, Qt::AlignHCenter);
    dialog->showFullScreen();
    dialog->exec();


    if(line)
    {
        user = line->text();
	highscores.insert(inserted, QString::number(score));
	highscores.insert(inserted, user);
	highscores.removeAt(index);
	highscores.removeAt(index);
    }
    delete dialog;
}


void MainWindow::setSkill(int s)
{
    skill = s;

    if(skill == Master)
	wrapped = true;
    else 
	wrapped = false;

    QString message = tr("Skill level") + ": ";
    switch (skill)
    {
    case Novice: message += tr("novice"); break;
    case Normal: message += tr("normal"); break;
    case Expert: message += tr("expert"); break;
    case Master: message += tr("master"); break;
    default: break;
    }
    skillLabel->setText(message + ".");

    newGame();
}


void MainWindow::closeEvent(QCloseEvent * event)
{
    QSettings settings(kApplicationName);
    settings.setValue("Skill", skill);
    settings.setValue("Username", user);
    settings.setValue("Highscores", highscores);
    settings.setValue("Sound", soundAction->isChecked());
    settings.sync();
    event->accept();
}


void MainWindow::openHomepage()
{
    static const QUrl url("http://qt.osdn.org.ua/qnetwalk.html");

    if(!QDesktopServices::openUrl(url))
	QMessageBox::warning(this, tr("Error"),
                             tr("Could not launch the web browser.\n"));
}


void MainWindow::help()
{
	if(myDialog==NULL){
		myDialog = new MyDialog;
		connect(myDialog, SIGNAL(dialogHidden()), SLOT(doHideDialog()));
	}

    const QString text = ("<h3>" + tr("Rules of Play") + "</h3><p>" +
                          tr("You are the network administrator and your goal "
                             "is to connect each computer to the central server.") +
                             "<p>" + tr("Tap to turn the cable in a clockwise direction."
                             "In master skill level the board edges wrap around,"
                             "cells on the right will connect to those on left, left to right and like wise for up and down.") +
                             "<p>" + tr("Start the LAN with as few turns as possible!"));
    myDialog->setText(text);
    myDialog->showFullScreen();
}


void MainWindow::about()
{
	if(myDialog==NULL){
		myDialog = new MyDialog;
		connect(myDialog, SIGNAL(dialogHidden()), SLOT(doHideDialog()));
	}

    const QString text = "<h3>About FixTheNet"  "1.0.3"  "</h3>"
                    "<p>NetwalkMobile is port of the QNetWalk game by <b>Andi Peredri</b> "
                    " http://qt.osdn.org.ua/qnetwalk.html</p>"
                    "<p>Ported to BlackBerry by Antti Pohjola, summeli@summeli.fi</p>"
                    "<p>This program is distributed under the terms of the "
                       "GNU General Public License.</p><p>The source code can be downloaded from: http://www.github.com/Summeli/NetwalkMobile</p>"
                       "Some of the graphics taken from www.openclipart.org</p>";

    myDialog->setText(text);
    myDialog->showFullScreen();
}

void MainWindow::pauseGame()
{
}

void MainWindow::resumeGame()
{
}

bool MainWindow::event(QEvent *event)
{
    switch (event->type()) {
    default:
        break;

    case QEvent::WindowActivate:
        resumeGame();
        break;

    case QEvent::WindowDeactivate:
        pauseGame();
        break;
    }

    return QMainWindow::event(event);
}

void MainWindow::touch()
{
#if defined(Q_OS_SYMBIAN)
    touchFeedback->InstantFeedback(ETouchFeedbackBasic);
#endif
}

void  MainWindow::infoMessage(const QString& message)
{
    infoLabel->setText(message);
}

void  MainWindow::doHideDialog()
{
	delete myDialog;
	myDialog = NULL;
}
