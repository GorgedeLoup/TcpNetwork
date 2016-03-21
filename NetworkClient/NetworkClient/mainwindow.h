#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "CTcpNetwork.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_u_listenButton_clicked();
    void on_u_sendButton_clicked();

    void on_u_checkButton_clicked();

private:
    Ui::MainWindow *ui;

    CTcpNetwork client;
};

#endif // MAINWINDOW_H
