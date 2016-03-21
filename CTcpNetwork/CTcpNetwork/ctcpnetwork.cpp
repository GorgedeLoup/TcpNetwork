// ************************************* //
// File: ctcpnetwork.cpp
// Author: Bofan ZHOU
// Create date: Feb. 29, 2016
// Last modify date: Mar. 21, 2016
// Description:
// ************************************* //

#include <QDebug>
#include "ctcpnetwork.h"

Q_LOGGING_CATEGORY(TCPNETWORK, "TCPNETWORK")


CTcpNetwork::CTcpNetwork(QObject *parent) : QObject(parent),
    m_totalBytesSend(0), m_bytesWritten(0), m_bytesToWrite(0), m_loadSize(2*1024),
    m_totalBytesRecv(0), m_bytesReceived(0), m_recvFileNameSize(0)
{
    m_server = new QTcpServer(this);
    m_sendSocket = new QTcpSocket(this);
    m_checkSocket = new QTcpSocket(this);
    //m_recvSocketTemp = new QTcpSocket(this);

    readSettings();

    connect(m_checkSocket, SIGNAL(readyRead()), this, SLOT(checkRecp()));
    connect(m_sendSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
}


// Read IP settings from config file
void CTcpNetwork::readSettings()
{
    QSettings * settings = new QSettings(SETTINGS_PATH, QSettings::IniFormat);
    m_localIPAddress = settings->value("Receive/IpAddress").toString();
    m_receivePort = settings->value("Receive/Port").toString().toUShort(0,10);
    m_remoteIPAddress = settings->value("Send/IpAddress").toString();
    m_sendPort = settings->value("Send/Port").toString().toUShort(0,10);
    m_checkPort = settings->value("Check/Port").toString().toUShort(0,10);
    delete settings;
}


// Write IP settings to config file
void CTcpNetwork::updateSettings()
{
    QSettings * settings = new QSettings(SETTINGS_PATH, QSettings::IniFormat);
    settings->setValue("Receive/IpAddress", m_localIPAddress);
    settings->setValue("Receive/Port", m_receivePort);
    settings->setValue("Send/IpAddress", m_remoteIPAddress);
    settings->setValue("Send/Port", m_sendPort);
    settings->setValue("Check/Port", m_checkPort);
    delete settings;
}


// Return the available local host address
QHostAddress CTcpNetwork::getLocalIP()
{
    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
    QFlags<QNetworkInterface::InterfaceFlag> interfaceFlags;
    QString addressStr;
    QHostAddress localAddress;

    for (int i=0; i<interfaceList.size(); ++i)
    {
        bool isRunning = 0;
        bool isUp = 0;
        bool isNotLoop = 0;
        if (interfaceList[i].isValid())
        {
            interfaceFlags = interfaceList[i].flags();
            if (interfaceFlags.testFlag(QNetworkInterface::IsRunning)) isRunning = 1;
            if (interfaceFlags.testFlag(QNetworkInterface::IsUp)) isUp = 1;
            if (!interfaceFlags.testFlag(QNetworkInterface::IsLoopBack)) isNotLoop = 1;
            if ((isRunning == 1) && (isUp == 1) && (isNotLoop == 1))
            {
                QList<QNetworkAddressEntry> ipList = interfaceList[i].addressEntries();
                for (int i=0; i<ipList.size(); ++i)
                {
                    if (ipList[i].ip().toIPv4Address())
                    {
                        addressStr = ipList[i].ip().toString();
                        localAddress = QHostAddress(addressStr);
                    }
                }
            }
        }
    }

    return localAddress;
}


// Listen to the remote host, wait for connections
void CTcpNetwork::listen()
{
    if (m_localIPAddress.isEmpty())
    {
        m_localIPAddress = getLocalIP().toString();
        updateSettings();
    }

    if (!m_server->isListening())
    {
        QHostAddress ipAddress(m_localIPAddress);
        if(!m_server->listen(ipAddress, m_receivePort))
        {
            //qCWarning(SERVER()) << SERVER().categoryName() << m_server->errorString();
            qDebug() << m_server->errorString();
            m_server->close();
            return;
        }
    }
    qDebug() << "Listen IP:" << m_localIPAddress << "Port:" << m_receivePort << "OK";
    connect(m_server, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
}


// Build new socket connection
void CTcpNetwork::acceptConnection()
{
    std::thread threadTemp(readHeader, this);
    threadTemp.detach();
}


// Connect to the server remote host
void CTcpNetwork::connectServer(int port)
{
    QHostAddress ipAddress(m_remoteIPAddress);
    switch (port) {
    case 1:
        m_sendSocket->connectToHost(ipAddress, m_sendPort);
        break;
    case 2:
        m_checkSocket->connectToHost(ipAddress, m_sendPort);
        //m_checkSocket->connectToHost(ipAddress, m_checkPort);
        break;
    default:
        break;
    }

// Display the connection information
    //    qDebug() << "&&&&&&&&&&&&&&&&&&&&&&";
    //    qDebug() << "peerName:" << m_sendSocket->peerName();
    //    qDebug() << "peerAddress:" << m_sendSocket->peerAddress();
    //    qDebug() << "peerPort:" << m_sendSocket->peerPort();
    //    qDebug() << endl;
    //    qDebug() << "localAddress:" << m_sendSocket->localAddress();
    //    qDebug() << "localPort:" << m_sendSocket->localPort();
    //    qDebug() << "&&&&&&&&&&&&&&&&&&&&&&";
}


// Create the file to send, for the TEST, REMOVE after
void CTcpNetwork::encodeFile()
{
    // Generate file name
    QLocale curLocale(QLocale("C"));
    QLocale::setDefault(curLocale);
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeString = QLocale().toString(dateTime, "dMMMyyyy_hhmmss");

    m_sendFileName = "SendFile_" + dateTimeString + ".HIFU";
    qDebug() << m_sendFileName;

    // QDir::setCurrent("D:/");
    m_sendFile = new QFile(m_sendFileName);
    if (!m_sendFile->open(QIODevice::WriteOnly))  // EMIT ERROR
        qDebug() << "File write error !";

    QDataStream outData(m_sendFile);
    //    outData << (quint32)0xA0B0C0D0;
    //    outData << (qint32)460;
    outData.setVersion(QDataStream::Qt_4_6);
    outData << m_sendFileName;
    qDebug() << "Encode file finished.";

    m_sendFile->close();
}


// Send file to the remote host
void CTcpNetwork::sendFile()
{
    if (m_sendSocket->state() == m_sendSocket->ConnectingState)
    {
        emit socketBlockError();
        //qCWarning(SERVER()) << SERVER().categoryName() << "Socket block !";
        qDebug() << "Socket block !";
    }
    else
    {
        encodeFile();
        m_bytesWritten = 0;
        connectServer(1);
        connect(m_sendSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(sendFileProg(qint64)));
        startSendFile(m_sendFileName);
    }
}


// Send the signal and start the transfer
void CTcpNetwork::startSendFile(QString fileName)
{
    m_sendTempFile = new QFile(fileName);
    if(!m_sendTempFile->open(QFile::ReadOnly))
    {
        qDebug() << "Open file failed !";
        return;
    }
    m_totalBytesSend = m_sendTempFile->size();

    QDataStream outStream(&m_baOut, QIODevice::WriteOnly);
    outStream.setVersion(QDataStream::Qt_4_6);
    outStream << qint64(1) << qint64(0) << qint64(0) << fileName;    // Write total bytes space, filename space, filename in order
    qDebug() << "File name:" << fileName;

    m_totalBytesSend += m_baOut.size();    // m_totalBytesSend is the bytes information of filename space and the actual bytes of the file

    outStream.device()->seek(8);
    outStream << m_totalBytesSend << qint64((m_baOut.size() - sizeof(qint64)*3));   // Return the start of m_outBlock,
                                                                                    // replace two qint64 spaces by actual length information

    qDebug() << "Total bytes to send:" << m_totalBytesSend;
    m_bytesToWrite = m_totalBytesSend - m_sendSocket->write(m_baOut);        // The rest data length after the head information

    qDebug() << "Write file head finished...";
    qDebug() << "BytesToWrite:" << m_bytesToWrite;

    m_baOut.clear();
}


// Transfer the file by block and judge the progress in time
void CTcpNetwork::sendFileProg(qint64 numBytes)
{
    m_bytesWritten += (int)numBytes;    // Length of sent data

    if(m_bytesToWrite > 0)    // If any data has already been sent
    {
        m_baOut = m_sendTempFile->read(qMin(m_bytesToWrite, m_loadSize));   // The data length of every send progress, here 4KB as default,
                                                                            // if less than it, send the rest
        m_bytesToWrite -= (int)m_sendSocket->write(m_baOut);    // Length of remaining data

        qDebug() << "Block wirte finished...";
        qDebug() << "BytesWritten:" << m_bytesWritten;
        qDebug() << "BytesToWrite:" << m_bytesToWrite;

        m_baOut.resize(0);    // Clear the data buffer
    }
    else
    {
        m_sendTempFile->close();    // If no data is sent, close file
    }

    if(m_bytesWritten == m_totalBytesSend)    // Send over
    {
        m_sendTempFile->close();
        m_sendSocket->close();
        qDebug() << "Write file finished.";
        qDebug() << SEPARATION;
        disconnect(m_sendSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(sendFileProg(qint64)));
        m_bytesToWrite = 0;
        m_bytesWritten = 0;
        m_totalBytesSend = 0;
    }
}


// Read the header of receive pack and determine the branch
void CTcpNetwork::readHeader()
{
    QTcpSocket * recvSocket;
    recvSocket = m_server->nextPendingConnection();
    qDebug() << "Accept connection OK";

    m_socketList.append(recvSocket);

    //QByteArray baTemp;
    QDataStream in(recvSocket);
    in.setVersion(QDataStream::Qt_4_6);

    qint64 bytesAval = 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    while(recvSocket->bytesAvailable() != 0)
    {
        if (bytesAval != recvSocket->bytesAvailable())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            bytesAval = recvSocket->bytesAvailable();
        }
        else
            break;
        qDebug() << "BytesAvailable:" << recvSocket->bytesAvailable();
    }

    qint64 header;
    //qCDebug(CLIENT()) << CLIENT().categoryName() << "Reading header...";
    qDebug() << "Reading header...";

    in >> header;
    qDebug() << "Header code:" << header;

    switch (header) {
    case 1:
        receiveFileS(recvSocket);
        break;
    case 2:
        checkBack(recvSocket);
        break;
    default:
        recvSocket->close();
        recvSocket->deleteLater();
        break;
    }
}


// Receive file from remote host
void CTcpNetwork::receiveFileProg(QTcpSocket *recvSocket)
{
    qDebug() << "Receiving plan...";

    QDataStream inStream(recvSocket);
    inStream.setVersion(QDataStream::Qt_4_6);
    if(m_bytesReceived <= sizeof(qint64)*2)
    {
        qDebug() << "m_bytesReceived:" << m_bytesReceived;
        qDebug() << "bytesAvailable:" << recvSocket->bytesAvailable()
                 << endl << "fileNameSize:" << m_recvFileNameSize;
        // If received data length is less than 16 bytes, then it has just started, save incoming head information
        if((recvSocket->bytesAvailable() >= sizeof(qint64)*2) && (m_recvFileNameSize == 0))
        {
            // Receive the total data length and the length of filename
            inStream >> m_totalBytesRecv >> m_recvFileNameSize;
            qDebug() << "m_totalBytesRecv:" << m_totalBytesRecv << endl << "m_fileNameSize:" << m_recvFileNameSize;
            m_bytesReceived += sizeof(qint64) * 2;
        }
        if((recvSocket->bytesAvailable() >= m_recvFileNameSize) && (m_recvFileNameSize != 0))
        {
            // Receive the filename, and build the file
            inStream >> m_recvFileName;

            m_bytesReceived += m_recvFileNameSize;
            m_receiveFile = new QFile(m_recvFileName);
            qDebug() << "m_fileName:" << m_recvFileName;
            qDebug() << "BytesReceived:" << m_bytesReceived;
            if(!m_receiveFile->open(QFile::WriteOnly))
            {
                qDebug() << "Open file error !";
                return;
            }
        }
        else return;
    }

    if(m_bytesReceived < m_totalBytesRecv)
    {
        // If received data length is less than total length, then write the file
        m_bytesReceived += recvSocket->bytesAvailable();
        m_baIn = recvSocket->readAll();
        m_receiveFile->write(m_baIn);
        m_baIn.resize(0);
    }

    if(m_bytesReceived == m_totalBytesRecv)
    {
        // When receiving process is done
        m_receiveFile->close();
        recvSocket->close();
        recvSocket->deleteLater();
        m_recvFileNameSize = 0;

        qDebug() << "Receive file finished !";
        qDebug() << SEPARATION;
        emit fileRecvDone();
    }
    else
        return;
    m_bytesReceived = 0;
    m_totalBytesRecv = 0;
    m_recvFileNameSize = 0;
}


// Read the content of a binary file, just for the TEST
void CTcpNetwork::readFile()
{
    QFile readFile(m_recvFileName);
    if (!readFile.open(QIODevice::ReadOnly))
        qDebug() << "Open file failed !";

    QDataStream baIn(&readFile);
    QString readStr;
    baIn >> readStr;
    qDebug() << readStr;
    readFile.close();
    m_recvFileName.clear();
}


// Check the status of connection by sending and checking specific information
void CTcpNetwork::checkSend()
{
    if (m_checkSocket->state() == m_checkSocket->ConnectingState)
    {
        emit socketBlockError();
        qDebug() << "Socket block !";
    }

    else
    {
        connectServer(2);

        QByteArray baOut;
        QDataStream out(&baOut, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_6);

        m_receipt = genReceipt();

        out << qint64(2)
            << m_receipt;

        m_checkSocket->write(baOut);
        qDebug() << "Send check signal finished...";
    }
}


// Check the send-back data to confirm the validity of connection
void CTcpNetwork::checkRecp()
{
    // test the signal of readyRead
    //qDebug() << "readyRead...";

    QDataStream in(m_checkSocket);
    in.setVersion(QDataStream::Qt_4_6);

    QString receipt;
    //  test
    //qDebug() << "bytesAvail:" << m_checkSocket->bytesAvailable();
    in >> receipt;
    m_checkSocket->close();

    //  test
    //  qDebug() << "receipt:" << receipt;
    //  Check the consistency of the send-back data
    //  not thread-safe
    if(m_receipt == receipt)
    {
        qDebug() << "Receipt checked";
        // qCDebug(SERVER()) << SERVER().categoryName() << ":" << "SUCESSFULLY SEND TREATMENT PLAN.";
        m_receipt.clear();
        qDebug() << SEPARATION;
    }
    else
    {
        // EMIT ERROR SIGNAL;
    }
}


void CTcpNetwork::checkBack(QTcpSocket * recvSocket)
{
    qDebug() << "Received check signal, sending back...";

    QDataStream in(recvSocket);
    in.setVersion(QDataStream::Qt_4_6);

    QString checkStr;
    in >> checkStr;

    qDebug() << checkStr;

    QByteArray baCheck;
    QDataStream out(&baCheck, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);

    out << checkStr;
    qint64 bytesSent;

    qDebug() << "@DEBUG 1";

    bytesSent = recvSocket->write(baCheck);

    qDebug() << "@DEBUG 2";

    baCheck.clear();
    recvSocket->close();
    recvSocket->deleteLater();

    qDebug() << "@DEBUG 3";

    qDebug() << "Write send-back finished:" << bytesSent;
    qDebug() << SEPARATION;
}


// Generate the log information of treatment plan sending
QString CTcpNetwork::genReceipt()
{
    QLocale curLocale(QLocale("C"));
    QLocale::setDefault(curLocale);
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeString = QLocale().toString(dateTime, "ddd,d MMM yyyy, hh:mm:ss");

    QString server = "ServerName";
    QString client = "ClientName";
    QString receipt;

    receipt = "From: " + server + ", " + "To: " + client + ", " + "At: " + dateTimeString;

    return receipt;
}


// Display build-in error information
void CTcpNetwork::displayError(QAbstractSocket::SocketError)
{
    //qCWarning(SERVER()) << SERVER().categoryName() << m_sendSocket->errorString();
    qDebug() << m_sendSocket->errorString();
    m_sendSocket->close();
    //qCDebug(SERVER()) << SERVER().categoryName() << "Send socket was closed.";
    qDebug() << "Send socket was closed";
}


void CTcpNetwork::sendFileS(QString fileName)
{
//    m_sendTempFile = new QFile(fileName);
//    if(!m_sendTempFile->open(QFile::ReadOnly))
//    {
//        qDebug() << "Open file failed !";
//        return;
//    }
//    m_totalBytesSend = m_sendTempFile->size();

//    QDataStream outStream(&m_baOut, QIODevice::WriteOnly);
//    outStream.setVersion(QDataStream::Qt_4_6);
//    outStream << qint64(1) << qint64(0) << qint64(0) << fileName;    // Write total bytes space, filename space, filename in order
//    qDebug() << "File name:" << fileName;

//    m_totalBytesSend += m_baOut.size();    // m_totalBytesSend is the bytes information of filename space and the actual bytes of the file

//    outStream.device()->seek(8);
//    outStream << m_totalBytesSend << qint64((m_baOut.size() - sizeof(qint64)*3));   // Return the start of m_outBlock,
//    // replace two qint64 spaces by actual length information

//    qDebug() << "Total bytes to send:" << m_totalBytesSend;
//    m_bytesToWrite = m_totalBytesSend - m_sendSocket->write(m_baOut);        // The rest data length after the head information

//    qDebug() << "Write file head finished...";
//    qDebug() << "BytesToWrite:" << m_bytesToWrite;

//    m_baOut.clear();
    fileName = "";
}


void CTcpNetwork::receiveFileS(QTcpSocket * recvSocket)
{
    qDebug() << "Receiving plan...";

    QDataStream inStream(recvSocket);
    inStream.setVersion(QDataStream::Qt_4_6);

    qDebug() << "bytesAvailable:" << recvSocket->bytesAvailable();
    inStream >> m_totalBytesRecv >> m_recvFileNameSize;
    qDebug() << "m_totalBytesRecv:" << m_totalBytesRecv << endl << "m_fileNameSize:" << m_recvFileNameSize;

    // Receive the filename, and build the file
    inStream >> m_recvFileName;
    m_receiveFile = new QFile(m_recvFileName);
    qDebug() << "m_fileName:" << m_recvFileName;
    if(!m_receiveFile->open(QFile::WriteOnly))
    {
        qDebug() << "Open file error !";
        return;
    }

    m_baIn = recvSocket->readAll();
    m_receiveFile->write(m_baIn);
    m_baIn.resize(0);

    m_receiveFile->close();
    recvSocket->close();
    recvSocket->deleteLater();
    m_recvFileNameSize = 0;
    m_totalBytesRecv = 0;

    qDebug() << "Receive file finished !";
    qDebug() << SEPARATION;
    emit fileRecvDone();
}
