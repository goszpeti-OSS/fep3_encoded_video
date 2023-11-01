#include <QtMultimedia/QVideoFrame.h>
#include <QtMultimedia/QMediaPlayer.h>
#include <QtMultimediaWidgets>
#include <QAudioDevice>
#include <QAudioOutput>
#include <QApplication>
#include <QWidget>

#include <stdio.h>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication app(argc, argv);
    auto player = new QMediaPlayer;
    player->setSource(QUrl("C:/Repos/ffmpeg-video-player/Iron_Man-Trailer_HD.mp4"));

    auto videoWidget = new QVideoWidget;
    player->setVideoOutput(videoWidget);
    const auto audioTracks = player->audioTracks();
    auto audioOutput = new QAudioOutput(videoWidget);

    player->setAudioOutput(audioOutput);
    videoWidget->show();
    player->play();
    return app.exec();
}
