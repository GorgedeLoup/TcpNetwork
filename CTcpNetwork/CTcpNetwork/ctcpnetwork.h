// ************************************* //
// File: ctcpnetwork.h
// Author: Bofan ZHOU
// Create date: Feb. 29, 2016
// Last modify date: Mar. 21, 2016
// Description:
// ************************************* //

#ifndef CTCPNETWORK_H
#define CTCPNETWORK_H

#include <QObject>
#include <QtNetwork>
#include <thread>
#include <chrono>
#include <QLoggingCategory>
#include "ctcpnetwork_global.h"

#define SETTINGS_PATH "../lib/config/config.ini"
#define SEND_FILE "../SendFile.HIFU"
#define RECV_FILE "../RecvFile.HIFU"
#define SEPARATION "*******************"

Q_DECLARE_LOGGING_CATEGORY(TCPNETWORK)

class CTCPNETWORKSHARED_EXPORT CTcpNetwork : public QObject
{
    Q_OBJECT

public:
    CTcpNetwork(QObject *parent = 0);

private:
    QHostAddress m_localAddress;
    QString m_localIPAddress, m_remoteIPAddress;
    quint16 m_receivePort, m_sendPort, m_checkPort;

    QTcpServer * m_server;
    QList<QTcpSocket *> m_socketList;
    QTcpSocket * m_sendSocket, * m_checkSocket;

    qint64 m_totalBytesSend, m_bytesWritten, m_bytesToWrite, m_loadSize;
    qint64 m_totalBytesRecv, m_bytesReceived, m_recvFileNameSize;
    QByteArray m_baOut, m_baIn;
    QString m_receipt;

    QString m_sendFileName, m_recvFileName;
    QFile * m_sendFile, * m_receiveFile;
    QFile * m_sendTempFile, * m_recvTempFile;

public slots:
    void listen();  // Listen to the remote host, wait for connections
    void sendFile();    // Send file to the remote host
    void checkSend();   // Check the status of connection by sending and checking specific information

    void readHeader();  // Read the header of receive pack and determine the branch


private slots:
    void updateSettings();  // Write IP settings to config file
    void readSettings();    // Read IP settings from config file
    QHostAddress getLocalIP();  // Return the available local host address
    QString genReceipt();   // Generate the log information of treatment plan sending

    void displayError(QAbstractSocket::SocketError);    // Display build-in error information

    void connectServer(int port);   // Connect to the server remote host
    void acceptConnection();    // Build new socket connection

    void encodeFile();  // Create the file to send, for the TEST
    void startSendFile(QString fileName);   // Send the signal and start the transfer
    void sendFileProg(qint64 numBytes);  // Transfer the file by block and judge the progress in time
    void sendFileS(QString fileName);

    void receiveFileProg(QTcpSocket * recvSocket); // Receive file from remote host
    void receiveFileS(QTcpSocket * recvSocket);
    void readFile();    // Read the content of a binary file, just for the TEST
    void checkBack(QTcpSocket * recvSocket);   // Send back the information of checkSend()
    void checkRecp();   // Check the send-back data to confirm the validity of connection

signals:
    socketBlockError(); // Emit a error signal when the socket is already in transmission
    fileRecvDone(); // Emit a signal when a file is well received
};

#endif // CTCPNETWORK_H
