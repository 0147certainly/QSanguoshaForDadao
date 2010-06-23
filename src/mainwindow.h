#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QSettings>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void closeEvent(QCloseEvent *);

private:    
    QGraphicsScene *scene;
    Ui::MainWindow *ui;

    void restoreFromConfig();

private slots:
    void on_actionStart_Server_triggered();
    void on_actionStart_Game_triggered();
    void gotoScene(QGraphicsScene *scene);

    void on_actionExit_triggered();
};

#endif // MAINWINDOW_H
