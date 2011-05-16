/****************************************************************************
** Meta object code from reading C++ file 'configurationdata.h'
**
** Created: Mon May 16 21:09:03 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../Client/configurationdata.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'configurationdata.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ConfigurationFile[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      19,   18,   18,   18, 0x05,
      39,   33,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
      73,   67,   18,   18, 0x08,
      99,   18,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ConfigurationFile[] = {
    "ConfigurationFile\0\0saveRequest()\0depot\0"
    "depotHasBeenRemoved(Depot*)\0media\0"
    "putMediaDetection(Media*)\0"
    "depotHasProbablyBeenRemoved()\0"
};

const QMetaObject ConfigurationFile::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ConfigurationFile,
      qt_meta_data_ConfigurationFile, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ConfigurationFile::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ConfigurationFile::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ConfigurationFile::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ConfigurationFile))
        return static_cast<void*>(const_cast< ConfigurationFile*>(this));
    return QObject::qt_metacast(_clname);
}

int ConfigurationFile::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: saveRequest(); break;
        case 1: depotHasBeenRemoved((*reinterpret_cast< Depot*(*)>(_a[1]))); break;
        case 2: putMediaDetection((*reinterpret_cast< Media*(*)>(_a[1]))); break;
        case 3: depotHasProbablyBeenRemoved(); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void ConfigurationFile::saveRequest()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void ConfigurationFile::depotHasBeenRemoved(Depot * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_ConfigurationData[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      33,   24,   19,   18, 0x0a,
      47,   18,   19,   18, 0x2a,

       0        // eod
};

static const char qt_meta_stringdata_ConfigurationData[] = {
    "ConfigurationData\0\0bool\0savePath\0"
    "save(QString)\0save()\0"
};

const QMetaObject ConfigurationData::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ConfigurationData,
      qt_meta_data_ConfigurationData, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ConfigurationData::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ConfigurationData::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ConfigurationData::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ConfigurationData))
        return static_cast<void*>(const_cast< ConfigurationData*>(this));
    return QObject::qt_metacast(_clname);
}

int ConfigurationData::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: { bool _r = save((*reinterpret_cast< QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 1: { bool _r = save();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
