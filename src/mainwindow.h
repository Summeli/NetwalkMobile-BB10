
//  qnetwalk/mainwindow.h
//  Copyright (C) 2004-2008, Andi Peredri <andi@ukr.net>
//  Ported to Symbian by Ahmad Mushtaq <ahmad.mushtaq@gmail.com>

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QTime>

class Cell;
class QAction;
class QSound;
class QMenu;
class QLabel;
class QPushButton;
class MyDialog;

#if defined(Q_OS_SYMBIAN)
class MTouchFeedback;
#endif

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    virtual ~MainWindow();
    virtual QMenu * createPopupMenu();
protected:
    virtual void closeEvent(QCloseEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void timerEvent (QTimerEvent *);
    virtual bool event(QEvent *event);
private:
    enum Skill { Novice, Normal, Expert, Master };
    enum BoardSize
    {
        NoviceBoardSize = 6,
        NormalBoardSize = 7,
        ExpertBoardSize = 8,
        MasterBoardSize = 8
    };
    enum
    {
	NumHighscores   = 10,
	MinimumNumCells = 20
    };
    typedef QList<Cell *> CellList;
private slots:
    void  help();
    void  about();
    void  newGame();
    void  noRotation();
    void  startRotation();
    void  finishRotation();
    void  openHomepage();
    void  showHighscores();
    void  triggeredSkill(QAction *);
    void  showMenu();
    void  pauseGame();
    void  resumeGame();
    void  touch();
    void  infoMessage(const QString&);

public slots:
	void doHideDialog();
private:
    Cell * uCell(Cell * cell, bool wrap) const;
    Cell * dCell(Cell * cell, bool wrap) const;
    Cell * lCell(Cell * cell, bool wrap) const;
    Cell * rCell(Cell * cell, bool wrap) const;
    bool   isGameOver();
    bool   updateConnections();
    void   setSkill(int s);
    void   addRandomDir(CellList & list);
    void   addHighscore(int score);
    void   dialog(const QString & caption, const QString & text);
    void   updateNumberOfMoves(int);
    void   updateTimeElapsed(int);
private:
    int          skill;
    bool         wrapped;
    Cell *       root;
    Cell *       board[MasterBoardSize * MasterBoardSize];
    QSound *     clickSound;
    QSound *     connectSound;
    QSound *     startSound;
    QSound *     turnSound;
    QSound *     winSound;
    QString      user;
    QStringList  highscores;
    QMenu*       menu;
    QAction*     soundAction;
    QMenu*       skillMenu;
    QLabel*      movesLabel;
    QLabel*      timeLabel;
    int          numberOfMoves;
    int          timerId;
    QTime        gameStartTime;
    MyDialog*    myDialog;
    QPushButton* newGameButton;
    QLabel*      infoLabel;
    QLabel*      skillLabel;

#ifdef Q_OS_SYMBIAN
    MTouchFeedback* touchFeedback;
#endif
};

#endif

