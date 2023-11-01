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

void preciseSleep(double seconds) {
    using namespace std;
    using namespace std::chrono;

    static double estimate = 5e-3;
    static double mean = 5e-3;
    static double m2 = 0;
    static int64_t count = 1;

    while (seconds > estimate) {
        auto start = high_resolution_clock::now();
        this_thread::sleep_for(milliseconds(1));
        auto end = high_resolution_clock::now();

        double observed = (end - start).count() / 1e9;
        seconds -= observed;

        ++count;
        double delta = observed - mean;
        mean += delta / count;
        m2   += delta * (observed - mean);
        double stddev = sqrt(m2 / (count - 1));
        estimate = mean + stddev;
    }

    // spin lock
    auto start = high_resolution_clock::now();
    while ((high_resolution_clock::now() - start).count() / 1e9 < seconds);
}

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
        auto log_time = a_util::system::getCurrentMicroseconds();
        if (!frame.isMapped()) {
            LOG_INFO(a_util::strings::format("frame delta time: %d us", frame.endTime() - frame.startTime()).c_str());
            LOG_INFO(a_util::strings::format("real time: %d us", log_time - last_log_time).c_str());
        }
    }
    timestamp_t last_log_time = 0;

};



class MySink : public QVideoSink {
public:
    MySink(QLabel* label) : label(label) {
        timer.start(10, this);
    }
public slots:

    void process() {

        auto log_time = high_resolution_clock::now();
        int duration = (log_time - last_log_time).count() / 1e6;

        //LOG_INFO(a_util::strings::format("frame delta time: %d us", frame.endTime() - frame.startTime()).c_str());
        printf(a_util::strings::format("delta time: %d ms\n", duration).c_str());
        last_log_time = log_time;
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
       //auto i1 = frame.toImage();
       //label->setPixmap(QPixmap::fromImage(i1));
       auto buffer = std::malloc(size);
       std::memcpy(buffer, begin, size);
       frame.unmap();
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
    //QObject::connect(&qic, &QImageCapture::imageAvailable, &qic, &MyCapture::onImageAvailable);
    //rec.record();

    //auto timer = new QTimer();
    //timer->setTimerType(Qt::TimerType::PreciseTimer);
    //timer->setInterval(10);
    //QObject::connect(timer, &QTimer::timeout, &sink, &MySink::process);
    //timer->start();
    qDebug() << qic.errorString();
    qDebug() << qic.error();
    using namespace std::chrono;

    app.exec();
    /*while (true) {
        preciseSleep(0.01);
        sink.process();
    }*/
    return 0;
}
