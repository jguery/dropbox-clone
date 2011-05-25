/****************************************************************************
** Meta object code from reading C++ file 'dir.h'
**
** Created: Tue May 24 21:15:40 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../Client/dir.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dir.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Dir[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
       7,    5,    4,    4, 0x05,
      32,    4,    4,    4, 0x05,
      52,   50,    4,    4, 0x05,
      89,   50,    4,    4, 0x05,

 // slots: signature, parameters, type, tag, flags
     125,    4,    4,    4, 0x08,
     169,  150,    4,    4, 0x08,
     197,  150,    4,    4, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Dir[] = {
    "Dir\0\0m\0detectChangement(Media*)\0"
    "listenRequested()\0,\0"
    "addSubFileRequested(QString,QString)\0"
    "addSubDirRequested(QString,QString)\0"
    "directoryChangedAction()\0localPath,realPath\0"
    "addSubFile(QString,QString)\0"
    "addSubDir(QString,QString)\0"
};

const QMetaObject Dir::staticMetaObject = {
    { &Media::staticMetaObject, qt_meta_stringdata_Dir,
      qt_meta_data_Dir, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dir::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dir::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dir::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dir))
        return static_cast<void*>(const_cast< Dir*>(this));
    return Media::qt_metacast(_clname);
}

int Dir::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Media::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: detectChangement((*reinterpret_cast< Media*(*)>(_a[1]))); break;
        case 1: listenRequested(); break;
        case 2: addSubFileRequested((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 3: addSubDirRequested((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 4: directoryChangedAction(); break;
        case 5: addSubFile((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 6: addSubDir((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void Dir::detectChangement(Media * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Dir::listenRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void Dir::addSubFileRequested(QString _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Dir::addSubDirRequested(QString _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
