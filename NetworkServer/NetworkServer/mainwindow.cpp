#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_u_listenButton_clicked()
{
    server.listen();
}

void MainWindow::on_u_sendButton_clicked()
{
    server.sendFile();
}
