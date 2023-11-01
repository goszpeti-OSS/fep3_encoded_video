#include <QtMultimedia/QVideoFrame.h>
#include <QtMultimedia/QMediaPlayer.h>
#include <QMediaDevices>
#include <QtMultimediaWidgets>
#include <QAudioDevice>
#include <QAudioOutput>
#include <QApplication>
#include <QWidget>

class MyLabel: public QLabel{
Q_OBJECT
public:
    MyLabel(){

    }
signals:
    void imageAvailable(const QImage &newValue);
public slots:
    void onImageAvailable(const QImage &newValue);
};