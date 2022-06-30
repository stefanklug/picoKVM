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
    void mediaPlayerError(QMediaPlayer::Error error);
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void updateSerialPort(QAction *action);
    void updateVideoDevice(QAction *action);

private:
    void tryNextPipeline();
    void buildPipelines();

    Ui::Viewer *ui;

    QMediaPlayer* m_player;
    int m_nextPipelineIdx;
    QList<QString> m_pipelines;

    QString m_videoDevice;
};

#endif
