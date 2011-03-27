/****************************************************************************
** Meta object code from reading C++ file 'clientmanager.h'
**
** Created: Fri Mar 25 18:22:05 2011
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "clientmanager.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'clientmanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ClientManager[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      23,   15,   14,   14, 0x0a,
      57,   14,   14,   14, 0x08,
      82,   74,   14,   14, 0x08,
     122,  117,   14,   14, 0x08,
     166,  160,   14,   14, 0x08,
     205,  198,   14,   14, 0x08,
     238,   14,   14,   14, 0x08,
     255,   14,   14,   14, 0x08,
     275,   14,   14,   14, 0x08,
     316,   14,   14,   14, 0x08,
     335,  333,   14,   14, 0x08,
     399,   14,   14,   14, 0x08,
     447,   14,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ClientManager[] = {
    "ClientManager\0\0message\0"
    "receiveMessageAction(QByteArray*)\0"
    "slot_encrypted()\0written\0"
    "slot_encryptedBytesWritten(qint64)\0"
    "mode\0slot_modeChanged(QSslSocket::SslMode)\0"
    "error\0slot_peerVerifyError(QSslError)\0"
    "errors\0slot_sslErrors(QList<QSslError>)\0"
    "slot_connected()\0slot_disconnected()\0"
    "slot_error(QAbstractSocket::SocketError)\0"
    "slot_hostFound()\0,\0"
    "slot_proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)\0"
    "slot_stateChanged(QAbstractSocket::SocketState)\0"
    "slot_readyRead()\0"
};

const QMetaObject ClientManager::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ClientManager,
      qt_meta_data_ClientManager, 0 }
};

const QMetaObject *ClientManager::metaObject() const
{
    return &staticMetaObject;
}

void *ClientManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ClientManager))
        return static_cast<void*>(const_cast< ClientManager*>(this));
    return QObject::qt_metacast(_clname);
}

int ClientManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: receiveMessageAction((*reinterpret_cast< QByteArray*(*)>(_a[1]))); break;
        case 1: slot_encrypted(); break;
        case 2: slot_encryptedBytesWritten((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 3: slot_modeChanged((*reinterpret_cast< QSslSocket::SslMode(*)>(_a[1]))); break;
        case 4: slot_peerVerifyError((*reinterpret_cast< const QSslError(*)>(_a[1]))); break;
        case 5: slot_sslErrors((*reinterpret_cast< const QList<QSslError>(*)>(_a[1]))); break;
        case 6: slot_connected(); break;
        case 7: slot_disconnected(); break;
        case 8: slot_error((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        case 9: slot_hostFound(); break;
        case 10: slot_proxyAuthenticationRequired((*reinterpret_cast< const QNetworkProxy(*)>(_a[1])),(*reinterpret_cast< QAuthenticator*(*)>(_a[2]))); break;
        case 11: slot_stateChanged((*reinterpret_cast< QAbstractSocket::SocketState(*)>(_a[1]))); break;
        case 12: slot_readyRead(); break;
        default: ;
        }
        _id -= 13;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
