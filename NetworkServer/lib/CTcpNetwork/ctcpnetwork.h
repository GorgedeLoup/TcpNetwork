// ************************************* //
// File: ctcpnetwork.h
// Author: Bofan ZHOU
// Create date: Feb. 29, 2016
// Last modify date: Mar. 2, 2016
// Description:
// ************************************* //

#ifndef CTCPNETWORK_H
#define CTCPNETWORK_H

#include <QObject>
#include <QtNetwork>
#include <QFile>

#include "ctcpnetwork_global.h"

#define SETTINGS_PATH "../lib/config/config.ini"
#define SEND_FILE "../SendFile.HIFU"
#define RECV_FILE "../RecvFile.HIFU"

class CTCPNETWORKSHARED_EXPORT CTcpNetwork : public QObject
{
    Q_OBJECT

public:
    CTcpNetwork(QObject *parent = 0);

private:
    QHostAddress m_localAddress;
    QString m_localIPAddress, m_remoteIPAddress;
    quint16 m_receivePort, m_sendPort;

    QTcpServer *m_server;
    QTcpSocket *m_sendSocket, *m_receiveSocket;

    qint16 m_totalBytesSend, m_bytesWritten, m_bytesToWrite, m_loadSize;
    qint16 m_totalBytesRecv, m_bytesReceived, m_recvFileNameSize;
    QByteArray m_baOut, m_baIn;

    QString m_sendFileName, m_recvFileName;
    QFile *m_sendFile, *m_receiveFile;
    QFile *m_sendTempFile, *m_recvTempFile;

public slots:
    void listen();  // Listen to the remote host, wait for connections

    void sendFile();    // Send file to the remote host

private slots:
    void updateSettings();  // Write IP settings to config file
    void readSettings();    // Read IP settings from config file
    QHostAddress getLocalIP();  // Return the available local host address

    void displayError(QAbstractSocket::SocketError);    // Display build-in error information

    void connectServer();   // Connect to the server remote host
    void acceptConnection();    // Build new socket connection

    void encodeFile();  // Create the file to send, for the TEST, REMOVE after
    void startSendFile(QString fileName);   // Send the signal and start the transfer
    void sendFileProg(qint64 numBytes);  // Transfer the file by block and judge the progress in time

    void receiveFileProg(); // Receive file from remote host

signals:
    socketBlockError();
};

#endif // CTCPNETWORK_H
