/****************************************************************************
** Meta object code from reading C++ file 'socket.h'
**
** Created: Fri Mar 25 18:22:09 2011
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "socket.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'socket.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Socket[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // signals: signature, parameters, type, tag, flags
      16,    8,    7,    7, 0x05,

 // slots: signature, parameters, type, tag, flags
      44,    7,    7,    7, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Socket[] = {
    "Socket\0\0message\0receiveMessage(QByteArray*)\0"
    "inputStream()\0"
};

const QMetaObject Socket::staticMetaObject = {
    { &QSslSocket::staticMetaObject, qt_meta_stringdata_Socket,
      qt_meta_data_Socket, 0 }
};

const QMetaObject *Socket::metaObject() const
{
    return &staticMetaObject;
}

void *Socket::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Socket))
        return static_cast<void*>(const_cast< Socket*>(this));
    return QSslSocket::qt_metacast(_clname);
}

int Socket::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QSslSocket::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: receiveMessage((*reinterpret_cast< QByteArray*(*)>(_a[1]))); break;
        case 1: inputStream(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void Socket::receiveMessage(QByteArray * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
