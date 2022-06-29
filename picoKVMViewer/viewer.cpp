#include "viewer.h"
#include "ui_viewer.h"


Viewer::Viewer() : ui(new Ui::Viewer)
{
    ui->setupUi(this);

    m_player = new QMediaPlayer;
    m_player->setVideoOutput(ui->videoview);
    m_player->setMedia(QUrl("gst-pipeline: v4l2src ! jpegdec ! xvimagesink name=\"qtvideosink\""));
    m_player->play();

}
