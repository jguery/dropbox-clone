/****************************************************************************
** Meta object code from reading C++ file 'networkinterface.h'
**
** Created: Tue May 24 21:15:36 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../Client/networkinterface.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'networkinterface.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_NetworkInterface[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x05,
      30,   17,   17,   17, 0x05,
      43,   17,   17,   17, 0x05,
      58,   17,   17,   17, 0x05,
      87,   17,   17,   17, 0x05,
     114,   17,   17,   17, 0x05,

 // slots: signature, parameters, type, tag, flags
     146,   17,   17,   17, 0x08,
     195,   17,   17,   17, 0x08,
     215,   17,   17,   17, 0x08,
     240,   17,   17,   17, 0x08,
     261,   17,   17,   17, 0x08,
     298,  290,   17,   17, 0x08,
     332,   17,   17,   17, 0x08,
     350,   17,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_NetworkInterface[] = {
    "NetworkInterface\0\0connected()\0"
    "identified()\0disconnected()\0"
    "receiveErrorMessage(QString)\0"
    "connectToServerRequested()\0"
    "disconnectFromServerRequested()\0"
    "stateChangedAction(QAbstractSocket::SocketState)\0"
    "connectedToServer()\0disconnectedFromServer()\0"
    "connexionEncrypted()\0erreursSsl(QList<QSslError>)\0"
    "message\0receiveMessageAction(QByteArray*)\0"
    "connectToServer()\0disconnectFromServer()\0"
};

const QMetaObject NetworkInterface::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_NetworkInterface,
      qt_meta_data_NetworkInterface, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &NetworkInterface::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *NetworkInterface::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *NetworkInterface::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_NetworkInterface))
        return static_cast<void*>(const_cast< NetworkInterface*>(this));
    return QThread::qt_metacast(_clname);
}

int NetworkInterface::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: connected(); break;
        case 1: identified(); break;
        case 2: disconnected(); break;
        case 3: receiveErrorMessage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: connectToServerRequested(); break;
        case 5: disconnectFromServerRequested(); break;
        case 6: stateChangedAction((*reinterpret_cast< QAbstractSocket::SocketState(*)>(_a[1]))); break;
        case 7: connectedToServer(); break;
        case 8: disconnectedFromServer(); break;
        case 9: connexionEncrypted(); break;
        case 10: erreursSsl((*reinterpret_cast< const QList<QSslError>(*)>(_a[1]))); break;
        case 11: receiveMessageAction((*reinterpret_cast< QByteArray*(*)>(_a[1]))); break;
        case 12: connectToServer(); break;
        case 13: disconnectFromServer(); break;
        default: ;
        }
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void NetworkInterface::connected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void NetworkInterface::identified()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void NetworkInterface::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void NetworkInterface::receiveErrorMessage(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void NetworkInterface::connectToServerRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}

// SIGNAL 5
void NetworkInterface::disconnectFromServerRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 5, 0);
}
QT_END_MOC_NAMESPACE
