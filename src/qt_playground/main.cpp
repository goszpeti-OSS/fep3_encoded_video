#include <QtMultimedia/QVideoFrame.h>
#include <QtMultimedia/QMediaPlayer.h>
#include <QMediaDevices>
#include <QtMultimediaWidgets>
#include <QAudioDevice>
#include <QAudioOutput>
#include <QApplication>
#include <QWidget>
#include <a_util/logging.h>

#include <stdio.h>

bool checkCameraAvailability()
{
    if (QMediaDevices::videoInputs().count() > 0)
        return true;
    else
        return false;
}

class MySink : public QVideoSink {
public slots:
    void onVideoFrameChanged(const QVideoFrame& frame) {
        frame;
        LOG_INFO("Hallo");
    }
};

void videoChanged(const QVideoFrame& frame) {
   
}

int main(int argc, char *argv[])
{


    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication  app(argc, argv);

    auto default_device = QMediaDevices::defaultVideoInput();
    bool camera_available = checkCameraAvailability();
    QCamera* camera = new QCamera(default_device);
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    for (const QCameraDevice& cameraDevice : cameras) {
        LOG_INFO(cameraDevice.description().toStdString().c_str());
        if (cameraDevice.description() == "mycamera")
            camera = new QCamera(cameraDevice);
    }
    /*auto player = new QMediaPlayer;
    player->setSource(QUrl("C:/Repos/ffmpeg-video-player/Iron_Man-Trailer_HD.mp4"));*/

    auto videoWidget = new QVideoWidget;
    //player->setVideoOutput(videoWidget);
    //const auto audioTracks = player->audioTracks();
    //auto audioOutput = new QAudioOutput(videoWidget);
    //player->setAudioOutput(audioOutput);
    QMediaCaptureSession captureSession;
    captureSession.setCamera(camera);
    //captureSession.setVideoOutput(videoWidget);
    MySink sink;
    //captureSession.setVideoSink(&sink);
    QMediaRecorder rec;
    captureSession.setRecorder(&rec);
    videoWidget->show();
    camera->start();
    //QMediaFormat
    //player->play();
    auto format = QMediaFormat();
    format.setFileFormat(QMediaFormat::MPEG4);
    format.setVideoCodec(QMediaFormat::VideoCodec::H264);

    rec.setMediaFormat(format);
    rec.setOutputLocation(QUrl::fromLocalFile("D:/Repos/test"));
    rec.setVideoResolution(800, 600);
    //QObject::connect(&sink, &QVideoSink::videoFrameChanged, &sink, &MySink::onVideoFrameChanged);
    QTimer* timer = new QTimer();
    timer->setInterval(2000);
    rec.record();
    QObject::connect(timer, &QTimer::timeout, &rec, &QMediaRecorder::stop);

    qDebug() << rec.errorString();
    qDebug() << rec.error();

    app.exec();
    return 0;
}
