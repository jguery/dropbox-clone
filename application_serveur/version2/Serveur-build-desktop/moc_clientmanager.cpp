/****************************************************************************
** Meta object code from reading C++ file 'clientmanager.h'
**
** Created: Thu May 5 04:45:58 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../Serveur/clientmanager.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'clientmanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ClientManager[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      29,   15,   14,   14, 0x05,
      72,   64,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
     116,  108,   14,   14, 0x08,
     152,  150,   14,   14, 0x08,
     178,   14,   14,   14, 0x08,
     199,   14,   14,   14, 0x08,
     228,   14,   14,   14, 0x08,
     249,   64,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ClientManager[] = {
    "ClientManager\0\0clientManager\0"
    "disconnectedClient(ClientManager*)\0"
    "request\0sendDetectionRequested(QByteArray*)\0"
    "message\0receiveMessageAction(QByteArray*)\0"
    "r\0receivedRequest(Request*)\0"
    "clientDisconnected()\0erreursSsl(QList<QSslError>)\0"
    "connexionEncrypted()\0sendDetection(QByteArray*)\0"
};

const QMetaObject ClientManager::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_ClientManager,
      qt_meta_data_ClientManager, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ClientManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ClientManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ClientManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ClientManager))
        return static_cast<void*>(const_cast< ClientManager*>(this));
    return QThread::qt_metacast(_clname);
}

int ClientManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: disconnectedClient((*reinterpret_cast< ClientManager*(*)>(_a[1]))); break;
        case 1: sendDetectionRequested((*reinterpret_cast< QByteArray*(*)>(_a[1]))); break;
        case 2: receiveMessageAction((*reinterpret_cast< QByteArray*(*)>(_a[1]))); break;
        case 3: receivedRequest((*reinterpret_cast< Request*(*)>(_a[1]))); break;
        case 4: clientDisconnected(); break;
        case 5: erreursSsl((*reinterpret_cast< const QList<QSslError>(*)>(_a[1]))); break;
        case 6: connexionEncrypted(); break;
        case 7: sendDetection((*reinterpret_cast< QByteArray*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void ClientManager::disconnectedClient(ClientManager * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ClientManager::sendDetectionRequested(QByteArray * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
