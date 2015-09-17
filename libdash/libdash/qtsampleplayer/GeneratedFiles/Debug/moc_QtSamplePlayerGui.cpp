/****************************************************************************
** Meta object code from reading C++ file 'QtSamplePlayerGui.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../UI/QtSamplePlayerGui.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QtSamplePlayerGui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_sampleplayer__QtSamplePlayerGui_t {
    QByteArrayData data[18];
    char stringdata0[482];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_sampleplayer__QtSamplePlayerGui_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_sampleplayer__QtSamplePlayerGui_t qt_meta_stringdata_sampleplayer__QtSamplePlayerGui = {
    {
QT_MOC_LITERAL(0, 0, 31), // "sampleplayer::QtSamplePlayerGui"
QT_MOC_LITERAL(1, 32, 28), // "on_cb_mpd_currentTextChanged"
QT_MOC_LITERAL(2, 61, 0), // ""
QT_MOC_LITERAL(3, 62, 4), // "arg1"
QT_MOC_LITERAL(4, 67, 32), // "on_cb_period_currentIndexChanged"
QT_MOC_LITERAL(5, 100, 5), // "index"
QT_MOC_LITERAL(6, 106, 45), // "on_cb_video_adaptationset_cur..."
QT_MOC_LITERAL(7, 152, 46), // "on_cb_video_representation_cu..."
QT_MOC_LITERAL(8, 199, 45), // "on_cb_audio_adaptationset_cur..."
QT_MOC_LITERAL(9, 245, 46), // "on_cb_audio_representation_cu..."
QT_MOC_LITERAL(10, 292, 21), // "on_button_mpd_clicked"
QT_MOC_LITERAL(11, 314, 23), // "on_button_start_clicked"
QT_MOC_LITERAL(12, 338, 22), // "on_button_stop_clicked"
QT_MOC_LITERAL(13, 361, 30), // "SetVideoSegmentBufferFillState"
QT_MOC_LITERAL(14, 392, 10), // "percentage"
QT_MOC_LITERAL(15, 403, 23), // "SetVideoBufferFillState"
QT_MOC_LITERAL(16, 427, 30), // "SetAudioSegmentBufferFillState"
QT_MOC_LITERAL(17, 458, 23) // "SetAudioBufferFillState"

    },
    "sampleplayer::QtSamplePlayerGui\0"
    "on_cb_mpd_currentTextChanged\0\0arg1\0"
    "on_cb_period_currentIndexChanged\0index\0"
    "on_cb_video_adaptationset_currentIndexChanged\0"
    "on_cb_video_representation_currentIndexChanged\0"
    "on_cb_audio_adaptationset_currentIndexChanged\0"
    "on_cb_audio_representation_currentIndexChanged\0"
    "on_button_mpd_clicked\0on_button_start_clicked\0"
    "on_button_stop_clicked\0"
    "SetVideoSegmentBufferFillState\0"
    "percentage\0SetVideoBufferFillState\0"
    "SetAudioSegmentBufferFillState\0"
    "SetAudioBufferFillState"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_sampleplayer__QtSamplePlayerGui[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   79,    2, 0x08 /* Private */,
       4,    1,   82,    2, 0x08 /* Private */,
       6,    1,   85,    2, 0x08 /* Private */,
       7,    1,   88,    2, 0x08 /* Private */,
       8,    1,   91,    2, 0x08 /* Private */,
       9,    1,   94,    2, 0x08 /* Private */,
      10,    0,   97,    2, 0x08 /* Private */,
      11,    0,   98,    2, 0x08 /* Private */,
      12,    0,   99,    2, 0x08 /* Private */,
      13,    1,  100,    2, 0x0a /* Public */,
      15,    1,  103,    2, 0x0a /* Public */,
      16,    1,  106,    2, 0x0a /* Public */,
      17,    1,  109,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void, QMetaType::Int,   14,

       0        // eod
};

void sampleplayer::QtSamplePlayerGui::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtSamplePlayerGui *_t = static_cast<QtSamplePlayerGui *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_cb_mpd_currentTextChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->on_cb_period_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->on_cb_video_adaptationset_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->on_cb_video_representation_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->on_cb_audio_adaptationset_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->on_cb_audio_representation_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->on_button_mpd_clicked(); break;
        case 7: _t->on_button_start_clicked(); break;
        case 8: _t->on_button_stop_clicked(); break;
        case 9: _t->SetVideoSegmentBufferFillState((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->SetVideoBufferFillState((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->SetAudioSegmentBufferFillState((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->SetAudioBufferFillState((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject sampleplayer::QtSamplePlayerGui::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_sampleplayer__QtSamplePlayerGui.data,
      qt_meta_data_sampleplayer__QtSamplePlayerGui,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *sampleplayer::QtSamplePlayerGui::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *sampleplayer::QtSamplePlayerGui::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_sampleplayer__QtSamplePlayerGui.stringdata0))
        return static_cast<void*>(const_cast< QtSamplePlayerGui*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int sampleplayer::QtSamplePlayerGui::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
