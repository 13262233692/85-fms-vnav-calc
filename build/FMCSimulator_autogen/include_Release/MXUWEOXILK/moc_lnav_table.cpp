/****************************************************************************
** Meta object code from reading C++ file 'lnav_table.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/gui/lnav_table.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'lnav_table.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN3fmc3gui9LNAVTableE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN3fmc3gui9LNAVTableE = QtMocHelpers::stringData(
    "fmc::gui::LNAVTable",
    "refreshDisplay",
    "",
    "updateActiveLeg",
    "legIndex",
    "formatLatitude",
    "lat",
    "formatLongitude",
    "lon",
    "formatDistance",
    "distanceNm",
    "formatBearing",
    "bearing",
    "formatAltitude",
    "altitudeFt",
    "formatETA",
    "minutes",
    "formatTAS",
    "tasKT",
    "formatGS",
    "gsKT",
    "formatMach",
    "mach",
    "formatVS",
    "vsFpm",
    "formatEFOB",
    "fuelKg",
    "formatFuelFlow",
    "ffKgMin"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN3fmc3gui9LNAVTableE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   98,    2, 0x0a,    1 /* Public */,
       3,    1,   99,    2, 0x0a,    2 /* Public */,
       5,    1,  102,    2, 0x10a,    4 /* Public | MethodIsConst  */,
       7,    1,  105,    2, 0x10a,    6 /* Public | MethodIsConst  */,
       9,    1,  108,    2, 0x10a,    8 /* Public | MethodIsConst  */,
      11,    1,  111,    2, 0x10a,   10 /* Public | MethodIsConst  */,
      13,    1,  114,    2, 0x10a,   12 /* Public | MethodIsConst  */,
      15,    1,  117,    2, 0x10a,   14 /* Public | MethodIsConst  */,
      17,    1,  120,    2, 0x10a,   16 /* Public | MethodIsConst  */,
      19,    1,  123,    2, 0x10a,   18 /* Public | MethodIsConst  */,
      21,    1,  126,    2, 0x10a,   20 /* Public | MethodIsConst  */,
      23,    1,  129,    2, 0x10a,   22 /* Public | MethodIsConst  */,
      25,    1,  132,    2, 0x10a,   24 /* Public | MethodIsConst  */,
      27,    1,  135,    2, 0x10a,   26 /* Public | MethodIsConst  */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::QString, QMetaType::Double,    6,
    QMetaType::QString, QMetaType::Double,    8,
    QMetaType::QString, QMetaType::Double,   10,
    QMetaType::QString, QMetaType::Double,   12,
    QMetaType::QString, QMetaType::Double,   14,
    QMetaType::QString, QMetaType::Double,   16,
    QMetaType::QString, QMetaType::Double,   18,
    QMetaType::QString, QMetaType::Double,   20,
    QMetaType::QString, QMetaType::Double,   22,
    QMetaType::QString, QMetaType::Double,   24,
    QMetaType::QString, QMetaType::Double,   26,
    QMetaType::QString, QMetaType::Double,   28,

       0        // eod
};

Q_CONSTINIT const QMetaObject fmc::gui::LNAVTable::staticMetaObject = { {
    QMetaObject::SuperData::link<QTableWidget::staticMetaObject>(),
    qt_meta_stringdata_ZN3fmc3gui9LNAVTableE.offsetsAndSizes,
    qt_meta_data_ZN3fmc3gui9LNAVTableE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN3fmc3gui9LNAVTableE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LNAVTable, std::true_type>,
        // method 'refreshDisplay'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateActiveLeg'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'formatLatitude'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'formatLongitude'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'formatDistance'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'formatBearing'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'formatAltitude'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'formatETA'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'formatTAS'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'formatGS'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'formatMach'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'formatVS'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'formatEFOB'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'formatFuelFlow'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>
    >,
    nullptr
} };

void fmc::gui::LNAVTable::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<LNAVTable *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->refreshDisplay(); break;
        case 1: _t->updateActiveLeg((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: { QString _r = _t->formatLatitude((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 3: { QString _r = _t->formatLongitude((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 4: { QString _r = _t->formatDistance((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 5: { QString _r = _t->formatBearing((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 6: { QString _r = _t->formatAltitude((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 7: { QString _r = _t->formatETA((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 8: { QString _r = _t->formatTAS((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 9: { QString _r = _t->formatGS((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 10: { QString _r = _t->formatMach((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 11: { QString _r = _t->formatVS((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 12: { QString _r = _t->formatEFOB((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 13: { QString _r = _t->formatFuelFlow((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
}

const QMetaObject *fmc::gui::LNAVTable::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *fmc::gui::LNAVTable::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN3fmc3gui9LNAVTableE.stringdata0))
        return static_cast<void*>(this);
    return QTableWidget::qt_metacast(_clname);
}

int fmc::gui::LNAVTable::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTableWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 14;
    }
    return _id;
}
QT_WARNING_POP
