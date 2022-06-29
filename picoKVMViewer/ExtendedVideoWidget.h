#ifndef EXTENDEDCAMERAVIEWFINDER_H
#define EXTENDEDCAMERAVIEWFINDER_H

#include <QVideoWidget>
#include <QSerialPort>
#include <QPoint>
#include <QTime>

class ExtendedVideoWidget : public QVideoWidget
{
    Q_OBJECT
public:
    explicit ExtendedVideoWidget(QWidget *parent = nullptr);
    ~ExtendedVideoWidget();
    
protected:
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    bool event(QEvent *event);
    void focusInEvent(QFocusEvent *event);

    void sendKeyboardMessage();

    void handleMouseEvent(QMouseEvent* event);
    void timerEvent(QTimerEvent* event);

    void sendMouseMessage();

    bool focusNextPrevChild(bool next);

private:
    Q_DISABLE_COPY(ExtendedVideoWidget)

    QSerialPort mPort;
    bool mWaitUntilMouseRelease;
    QPoint mOldPos;
    QPoint mNewPos;
    int mOldButtons;
    int mNewButtons;
    int mMouseTimerId;

    QVector<uint32_t> mPressedKeys;
    uint8_t mModifiers;
};


#endif // EXTENDEDCAMERAVIEWFINDER_H
