#include "viewer.h"
#include "ui_viewer.h"

#include <QMessageBox>
#include <QSerialPort>
#include <QSerialPortInfo>

#include <QtWidgets>
#include <QSettings>

#ifdef Q_OS_WINDOWS
#include <QCameraInfo>
Q_DECLARE_METATYPE(QCameraInfo) // needed for ->setData(QVariant::fromValue(cameraInfo))
#endif

const char* kSettingsSerialPortKey = "serialPort";
const char* kSettingsVideoDeviceKey = "videoDevice";

Viewer::Viewer() : ui(new Ui::Viewer)
{
    QSettings settings;

    ui->setupUi(this);

    ui->toolBar->hide();
    ui->statusbar->hide();

    QActionGroup *serialPortsGroup = new QActionGroup(this);
    serialPortsGroup->setExclusive(true);

    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QAction *serialPortAction = new QAction(info.portName(), serialPortsGroup);
        serialPortAction->setCheckable(true);
        ui->menuSettings->addAction(serialPortAction);
        if(serialPortAction->text() == settings.value(kSettingsSerialPortKey)) {
            updateSerialPort(serialPortAction);
        }

        qDebug() << "Serial Port " << info.portName();
    }

    connect(serialPortsGroup, &QActionGroup::triggered, this, &Viewer::updateSerialPort);

    ui->menuSettings->addSeparator();

    QActionGroup *videoDevicesGroup = new QActionGroup(this);
    serialPortsGroup->setExclusive(true);

#ifdef Q_OS_LINUX
    m_videoDevice = settings.value(kSettingsVideoDeviceKey, "/dev/video0" ).toString();
    QDir dir("/dev", "video*");
    dir.setFilter(QDir::System);
    QFileInfoList files = dir.entryInfoList();
    for (QFileInfo info : files) {
        QAction *videoDeviceAction = new QAction(info.absoluteFilePath(), videoDevicesGroup);
        videoDeviceAction->setCheckable(true);
        if(videoDeviceAction->text() == m_videoDevice) {
            videoDeviceAction->setChecked(true);
        }
        ui->menuSettings->addAction(videoDeviceAction);

        qDebug() << "Video Device: " << info.absoluteFilePath();
    }
#endif

#ifdef Q_OS_WINDOWS
    m_videoDevice = settings.value(kSettingsVideoDeviceKey, "USB3. 0 capture").toString();
    const QList<QCameraInfo> availableCameras = QCameraInfo::availableCameras();
    for (const QCameraInfo &cameraInfo : availableCameras) {
        qDebug() << "Video Device:" << cameraInfo.description() << cameraInfo.deviceName();
        QAction *videoDeviceAction = new QAction(cameraInfo.description(), videoDevicesGroup);
        videoDeviceAction->setCheckable(true);
        videoDeviceAction->setData(QVariant::fromValue(cameraInfo));
        ui->menuSettings->addAction(videoDeviceAction);
        if(videoDeviceAction->text() == m_videoDevice) {
            videoDeviceAction->setChecked(true);
            updateVideoDevice(videoDeviceAction);
        }
    }
#endif

    connect(videoDevicesGroup, &QActionGroup::triggered, this, &Viewer::updateVideoDevice);

#ifdef Q_OS_LINUX
    buildPipelines();

    m_player.reset(new QMediaPlayer);
    m_player->setVideoOutput(ui->videoview);
    connect(m_player.get(), QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, &Viewer::mediaPlayerError);
    connect(m_player.get(), &QMediaPlayer::mediaStatusChanged, this, &Viewer::mediaStatusChanged);
    tryNextPipeline();
#endif
}

void Viewer::buildPipelines()
{
    m_pipelines.clear();
    if (m_videoDevice.isEmpty()) {
        return;
    }

    QString sink="qtvideosink name=\"qtvideosink\"";
    QString format="width=1920,height=1080";
    m_pipelines.append(QString("v4l2src device=%1 ! video/x-raw,%2 ! %3").arg(m_videoDevice, format, sink));
    m_pipelines.append(QString("v4l2src device=%1 ! video/x-raw,%2 ! videoconvert ! %3").arg(m_videoDevice, format, sink));
    m_pipelines.append(QString("v4l2src device=%1 ! image/jpeg,%2 ! jpegdec ! videoconvert ! %3").arg(m_videoDevice, format, sink));

    m_nextPipelineIdx = 0;
}

void Viewer::tryNextPipeline()
{
    if (m_nextPipelineIdx < 0) {
        return;
    }

    if (m_nextPipelineIdx >= m_pipelines.count()) {
        m_nextPipelineIdx = -1;
        QMessageBox::warning(this, tr("Image Capture Error"), "Failed to find a suitable video device");
        return;
    }
    QString pipeline = m_pipelines.at(m_nextPipelineIdx++);
    qDebug() << "Trying pipeline: " << pipeline;
    m_player->setMedia(QUrl("gst-pipeline: "+pipeline));
    m_player->play();
}

void Viewer::mediaPlayerError(QMediaPlayer::Error error) {
    qDebug() << "Error: " << error;
    if(error == QMediaPlayer::ResourceError) {
        tryNextPipeline();
    }
}

void Viewer::mediaStatusChanged(QMediaPlayer::MediaStatus status) {
    qDebug() << "Status: " << status;
    if(status == QMediaPlayer::InvalidMedia) {
        tryNextPipeline();
    }
}

void Viewer::updateSerialPort(QAction *action) {
    QSettings settings;
    ui->videoview->setSerialPort(action->text());
    action->setChecked(true);
    settings.setValue(kSettingsSerialPortKey, action->text());
}

void Viewer::updateVideoDevice(QAction *action)
{
    QSettings settings;
    m_videoDevice = action->text();
    settings.setValue(kSettingsVideoDeviceKey, action->text());

#ifdef Q_OS_LINUX
    buildPipelines();
    tryNextPipeline();
#endif

#ifdef Q_OS_WINDOWS
    QCameraInfo cameraInfo = qvariant_cast<QCameraInfo>(action->data());
    m_camera.reset(new QCamera(cameraInfo));
    m_camera->setViewfinder(ui->videoview);
    m_camera->setCaptureMode(QCamera::CaptureViewfinder);
    m_camera->start();
#endif
}


