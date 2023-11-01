#include <QtMultimedia/QVideoFrame.h>
#include <QtMultimedia/QMediaPlayer.h>
#include <QMediaDevices>
#include <QtMultimediaWidgets>
#include <QAudioDevice>
#include <QAudioOutput>
#include <QApplication>
#include <QWidget>
#include <a_util/logging.h>
#include <a_util/strings.h>
#include <a_util/system.h>
#include <windows.h>

#include <chrono>
#include <thread>
#include <math.h>
#include <stdio.h>
using namespace std::chrono;

bool checkCameraAvailability()
{
    if (QMediaDevices::videoInputs().count() > 0)
        return true;
    else
        return false;
}
class MyCapture : public QImageCapture {
public slots:

    void  onImageAvailable(int id, const QVideoFrame& frame) {
        auto log_time = high_resolution_clock::now();
        int duration = (log_time - last_log_time).count() / 1e6;
        printf(a_util::strings::format("delta time: %d ms\n", duration).c_str());
        last_log_time = log_time;
    }
    steady_clock::time_point last_log_time;

};



class MySink : public QVideoSink {
public:
    MySink(QLabel* label) : label(label) {
        timer.start(10, this);
    }
public slots:

    void process() {


        auto frame = this->videoFrame();
        if (!frame.isValid()) {
            return;
        }
        frame.map(QVideoFrame::ReadOnly);
       bool b = frame.isReadable();
       auto mode = frame.mapMode();
       auto begin = frame.bits(0);
       auto size = frame.mappedBytes(0);
       auto b1 = frame.videoBuffer();
       auto s1 = frame.pixelFormat();
       auto i1 = frame.toImage();
       
        i1.save("test.jpeg", "JPEG", 80);

       QByteArray ba;
       QBuffer buffer{ &ba };
       //ba.fromRawData(begin, size);

     //buffer.open(QIODevice::WriteOnly);
     //i1.save(&buffer,"JPEG",40);
    //    QFile a;
    //    a.setFileName("test.jpeg");
    //    a.open(QIODevice::WriteOnly);
       //a.close();
        //printf("%s", buffer.data().toBase64().toStdString().c_str());
    //    auto buffer = std::malloc(size);
    //    std::memcpy(buffer, begin, size);
        //buffer.close();
        //label->setPixmap(QPixmap::fromImage(i1));

        frame.unmap();
        auto log_time = high_resolution_clock::now();
        int duration = (log_time - last_log_time).count() / 1e6;

        //LOG_INFO(a_util::strings::format("frame delta time: %d us", frame.endTime() - frame.startTime()).c_str());
        printf(a_util::strings::format("delta time: %d ms\n", duration).c_str());
        last_log_time = log_time;

    }
    
    void onVideoFrameChanged(const QVideoFrame& frame) {
        process();
        //auto f2 = this->videoFrame();
       
    }
    void timerEvent(QTimerEvent* event)
    {
        if (event->timerId() == timer.timerId()) {
            process();
        }
        /*else {
            QWidget::timerEvent(event);
        }*/
    }
    QLabel* label = nullptr;
    steady_clock::time_point last_log_time;
    QBasicTimer timer;

};

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
    //auto player = new QMediaPlayer;
    //player->setSource(QUrl("C:/Repos/ffmpeg-video-player/Iron_Man-Trailer_HD.mp4"));*/
    
    auto videoWidget = new QVideoWidget;
    //auto window = new QWindow();
    auto label = new QLabel();
    label->setFixedSize(800, 600);
    label->show();
    //player->setVideoOutput(videoWidget);
    //const auto audioTracks = player->audioTracks();
    //auto audioOutput = new QAudioOutput(videoWidget);
    //player->setAudioOutput(audioOutput);
    MyCapture qic;
    QMediaCaptureSession captureSession;
    captureSession.setCamera(camera);
    //captureSession.setImageCapture(&qic);
    //captureSession.setVideoOutput(videoWidget);
    MySink sink(label);
    captureSession.setVideoSink(&sink);
    QMediaRecorder rec;
    //captureSession.setRecorder(&rec);
    videoWidget->show();
    camera->start();

    //qic.capture();
    //QMediaFormat
    //player->play();
    //auto format = QMediaFormat();
    //format.setFileFormat(QMediaFormat::MPEG4);
    //format.setVideoCodec(QMediaFormat::VideoCodec::H264);

    //rec.setMediaFormat(format);
    //rec.setOutputLocation(QUrl::fromLocalFile("D:/Repos/test"));
    //rec.setVideoResolution(800, 600);

    //rec.setVideoFrameRate()
    //QObject::connect(&sink, &QVideoSink::videoFrameChanged, &sink, &MySink::onVideoFrameChanged);
    QObject::connect(&qic, &QImageCapture::imageAvailable, &qic, &MyCapture::onImageAvailable);
    //rec.record();

    //auto timer = new QTimer();
    //timer->setTimerType(Qt::TimerType::PreciseTimer);
    //timer->setInterval(10);
    //QObject::connect(timer, &QTimer::timeout, &sink, &MySink::process);
    //timer->start();
    qDebug() << qic.errorString();
    qDebug() << qic.error();

    app.exec();
    return 0;
}
