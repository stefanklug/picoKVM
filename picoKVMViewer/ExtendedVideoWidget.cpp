#include "ExtendedVideoWidget.h"
#include<QDebug>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QSerialPortInfo>
#include <QCursor>

#include "usb_hid_keys.h"

#define MAX_KEYS 6

std::map<uint32_t, int> nativeScanToHID = {
    {0x09, KEY_ESC},

    {0x0a, KEY_1},
    {0x0b, KEY_2},
    {0x0c, KEY_3},
    {0x0d, KEY_4},
    {0x0e, KEY_5},
    {0x0f, KEY_6},
    {0x10, KEY_7},
    {0x11, KEY_8},
    {0x12, KEY_9},
    {0x13, KEY_0},
    {0x14, KEY_MINUS},
    {0x15, KEY_EQUAL},


    {0x16, KEY_BACKSPACE},

    {0x17, KEY_TAB},
    {0x18, KEY_Q},
    {0x19, KEY_W},
    {0x1a, KEY_E},
    {0x1b, KEY_R},
    {0x1c, KEY_T},
    {0x1d, KEY_Y},
    {0x1e, KEY_U},
    {0x1f, KEY_I},
    {0x20, KEY_O},
    {0x21, KEY_P},
    {0x22, KEY_LEFTBRACE},
    {0x23, KEY_RIGHTBRACE},
    
    {0x24, KEY_ENTER},

    {0x26, KEY_A},
    {0x27, KEY_S},
    {0x28, KEY_D},
    {0x29, KEY_F},
    {0x2a, KEY_G},
    {0x2b, KEY_H},
    {0x2c, KEY_J},
    {0x2d, KEY_K},
    {0x2e, KEY_L},
    {0x2f, KEY_SEMICOLON},
    {0x30, KEY_APOSTROPHE},
    {0x31, KEY_GRAVE},

    {0x33, KEY_BACKSLASH},

    {0x34, KEY_Z},
    {0x35, KEY_X},
    {0x36, KEY_C},
    {0x37, KEY_V},
    {0x38, KEY_B},
    {0x39, KEY_N},
    {0x3a, KEY_M},
    {0x3b, KEY_COMMA},
    {0x3c, KEY_DOT},
    {0x3d, KEY_SLASH},

    {0x3f, KEY_KPASTERISK},

    {0x42, KEY_CAPSLOCK},
    {0x43, KEY_F1},
    {0x44, KEY_F2},
    {0x45, KEY_F3},
    {0x46, KEY_F4},
    {0x47, KEY_F5},
    {0x48, KEY_F6},
    {0x49, KEY_F7},
    {0x4a, KEY_F8},
    {0x4b, KEY_F9},
    {0x4c, KEY_F10},

    {0x4d, KEY_NUMLOCK},
    {0x4e, KEY_SCROLLLOCK},
    {0x4f, KEY_KP7},

    {0x50, KEY_KP8},
    {0x51, KEY_KP9},
    {0x52, KEY_KPMINUS},
    {0x53, KEY_KP4},
    {0x54, KEY_KP5},
    {0x55, KEY_KP6},
    {0x56, KEY_KPPLUS},
    {0x57, KEY_KP1},
    {0x58, KEY_KP2},
    {0x59, KEY_KP3},
    {0x5a, KEY_KP0},
    {0x5b, KEY_KPDOT},

    {0x5f, KEY_F11},
    {0x60, KEY_F12},

    {0x68, KEY_KPENTER},

    {0x6a, KEY_KPSLASH},
    {0x6b, KEY_SYSRQ},

    {0x6e, KEY_HOME},
    {0x6f, KEY_UP},
    {0x70, KEY_PAGEUP},
    {0x71, KEY_LEFT},
    {0x72, KEY_RIGHT},
    {0x73, KEY_END},
    {0x74, KEY_DOWN},
    {0x75, KEY_PAGEDOWN},
    {0x76, KEY_INSERT},
    {0x77, KEY_DELETE},
    {0x7f, KEY_PAUSE},

    {0x41, KEY_SPACE}
};

#define PICOKVM_RELEASE_KEY 0x69    // right ctrl
ExtendedVideoWidget::ExtendedVideoWidget(QWidget *parent):QVideoWidget(parent)
{
    setFocusPolicy(Qt::ClickFocus);

    mPort.setBaudRate(57600);

    mOldButtons = 0;
    mNewButtons = 0;
    mModifiers = 0;
    mOldPos = QCursor::pos();
    mNewPos = mOldPos;
    mWaitUntilMouseRelease = false;

    //throttle the mouse messages (gaming mice are really chatty :-)
    mMouseTimerId = startTimer(20);
}

ExtendedVideoWidget::~ExtendedVideoWidget()
{

}

void ExtendedVideoWidget::setSerialPort(const QString &name)
{
    mPort.close();
    mPort.setPortName(name);
    mPort.open(QIODevice::ReadWrite);
}

uint8_t nativeScanCodeToHidModifier(uint32_t scanCode) {
    switch(scanCode) {
        case 0x25: return KEY_MOD_LCTRL;
        case 0x32: return KEY_MOD_LSHIFT;
        case 0x40: return KEY_MOD_LALT;
        case 0x85: return KEY_MOD_LMETA;
        case 0x69: return KEY_MOD_RCTRL;
        case 0x3e: return KEY_MOD_RSHIFT;
        case 0x6c: return KEY_MOD_RALT;
        case 0x87: return KEY_MOD_RMETA;
        default: return 0;
    }
}


void ExtendedVideoWidget::keyPressEvent(QKeyEvent *event)
{
    //Autorepeat is done by the target
    if(event->isAutoRepeat()) return;

    qDebug() << "KeyPress " << hex << event->key() << " vk: " << event->nativeVirtualKey() << " ScanCode: " << event->nativeScanCode() << " " << event;

    //check for release key
    if(event->nativeScanCode() == PICOKVM_RELEASE_KEY) {
        releaseMouse();
        releaseKeyboard();
        clearFocus();
        return;
    }

    bool somethingChanged = false;
    uint32_t scanCode = event->nativeScanCode();

    uint8_t modifier = mModifiers | nativeScanCodeToHidModifier(scanCode);
    if(modifier != mModifiers) {
        mModifiers = modifier;
        somethingChanged = true;
    }

    if( nativeScanToHID.find(scanCode) != nativeScanToHID.end()) {
        if(mPressedKeys.length() >= MAX_KEYS) {
            mPressedKeys.removeFirst();
        }
        mPressedKeys.append(scanCode);
        somethingChanged = true;
    }

    sendKeyboardMessage();
}

void ExtendedVideoWidget::keyReleaseEvent(QKeyEvent *event)
{
    //Autorepeat is done by the target
    if(event->isAutoRepeat()) return;

    //qDebug() << "KeyRelease " << event;
    bool somethingChanged = false;
    uint32_t scanCode = event->nativeScanCode();

    uint8_t modifier = mModifiers & ~nativeScanCodeToHidModifier(scanCode);
    if(modifier != mModifiers) {
        mModifiers = modifier;
        somethingChanged = true;
    }

    if( nativeScanToHID.find(scanCode) != nativeScanToHID.end()) {
        mPressedKeys.removeAll(scanCode);
    }

    sendKeyboardMessage();
}

void ExtendedVideoWidget::sendKeyboardMessage()
{
    //qDebug() << "Keyboard Message " << hex << mModifiers << mPressedKeys;

    QByteArray data("D ");
    data.append(QByteArray::number(mModifiers));

    for (int i = 0; i < mPressedKeys.size(); ++i) {
        data.append(" ");
        data.append(QByteArray::number( nativeScanToHID[mPressedKeys.at(i)] ));
    }
    data.append("\r\n");
    mPort.write(data);

    //qDebug() << data;
}

bool ExtendedVideoWidget::event(QEvent* event)
{
    QEvent::Type t = event->type();
    if(t == QEvent::MouseMove || t == QEvent::MouseButtonPress || t == QEvent::MouseButtonRelease || t == QEvent::MouseButtonDblClick) {
        handleMouseEvent(dynamic_cast<QMouseEvent*>(event));
    }

    return QVideoWidget::event(event);
}

void ExtendedVideoWidget::handleMouseEvent(QMouseEvent *event)
{
    if(mWaitUntilMouseRelease) {
        if(event->button() == Qt::LeftButton && event->type() == QEvent::MouseButtonRelease) {
            mWaitUntilMouseRelease = false;
        }
        return;
    }

    mNewPos = event->pos();
    mNewButtons = event->buttons();

    //changes in buttons must be reflected immediately, to get every change
    if(mNewButtons != mOldButtons) {
        sendMouseMessage();
    }
}

void ExtendedVideoWidget::timerEvent(QTimerEvent *event) {
    if(event->timerId() == mMouseTimerId) {
        sendMouseMessage();
    }

    char buf[1024];
    qint64 lineLength = mPort.readLine(buf, sizeof(buf));
    if(lineLength > 0) {
        qDebug() << "Arduino: " << buf;
    }

}

void ExtendedVideoWidget::sendMouseMessage() {
    bool sendMessage = false;

    if(mNewButtons != mOldButtons) {
        sendMessage = true;
        mOldButtons = mNewButtons;
    }

    if(mNewPos != mOldPos) {
        sendMessage = true;
        mOldPos = mNewPos;
    }

    //norm on 32767
    int x = std::min(std::max(mNewPos.x(), 0), this->width());
    int y = std::min(std::max(mNewPos.y(), 0), this->height());
    x = (int)(x / (float)this->width() * 32767.0);
    y = (int)(y / (float)this->height() * 32767.0);

    if(sendMessage) {
        //qDebug() << "Mouse message";
        QByteArray data("M ");
        data.append(QByteArray::number(mNewButtons & 0x7));
        data.append(" ");
        data.append(QByteArray::number(x));
        data.append(" ");
        data.append(QByteArray::number(y));
        data.append(" 0"); // wheel
        data.append("\r\n");
        mPort.write(data);

        //qDebug() << data;

        //mPort.flush();

    }
}

void ExtendedVideoWidget::focusInEvent(QFocusEvent *event)
{
    QCursor c(Qt::BlankCursor);
    grabKeyboard();
    grabMouse(c);
    mWaitUntilMouseRelease = true;
    setMouseTracking(true);
}

bool ExtendedVideoWidget::focusNextPrevChild(bool next) {
    //We return false, to get the tab key events
    return false;
}
