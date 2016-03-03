// ************************************* //
// File: ctcpnetwork_global.h
// Author: Bofan ZHOU
// Create date: Feb. 29, 2016
// Last modify date: Feb. 29, 2016
// Description:
// ************************************* //

#ifndef CTCPNETWORK_GLOBAL_H
#define CTCPNETWORK_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CTCPNETWORK_LIBRARY)
#  define CTCPNETWORKSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CTCPNETWORKSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CTCPNETWORK_GLOBAL_H
