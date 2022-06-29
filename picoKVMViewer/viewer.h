#ifndef VIEWER_H
#define VIEWER_H

#include <QMediaPlayer>
#include <QScopedPointer>

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class Viewer; }
QT_END_NAMESPACE

class Viewer : public QMainWindow
{
    Q_OBJECT

public:
    Viewer();

private slots:

private:
    Ui::Viewer *ui;

    QMediaPlayer* m_player;
};

#endif
