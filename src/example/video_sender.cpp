#include <iostream>
#include <string>
#include <chrono>

#include <fep3/core.h>
#include <fep3/core/element_configurable.h>

#include <QtMultimedia/QVideoFrame.h>
#include <QtMultimedia/QMediaPlayer.h>
#include <QMediaDevices>
#include <QtMultimediaWidgets>
#include <QAudioDevice>
#include <QAudioOutput>
#include <QApplication>

#include "video.h"

using namespace fep3;
using namespace std::chrono_literals;
using namespace std::chrono;


class MySink : public QVideoSink {
public:

    fep3::base::DataSample* grab_frame() {

        auto frame = this->videoFrame();
        // if (!frame.isValid()) {
        //     FEP3_LOG_ERROR("Can't read frame");
        // }
        frame.map(QVideoFrame::ReadOnly);
        auto image = frame.toImage();
        frame.unmap();
        image.mirror(true, false);
        QBuffer buffer{};
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer,"JPEG", 40);
        
        const auto data = buffer.data();
        auto size = buffer.data().size();
        data_sample.set(data.data(), size);
        buffer.close();
        auto log_time = high_resolution_clock::now();
        int duration = (log_time - _last_log_time).count() / 1e6;

        //LOG_INFO(a_util::strings::format("frame delta time: %d us", frame.endTime() - frame.startTime()).c_str());
        printf(a_util::strings::format("delta time: %d ms\n", duration).c_str());
        // printf("%d", frame.endTime());
        _last_log_time = log_time;
        return &data_sample;

    }
    void timerEvent(QTimerEvent* event)
    {
        if (event->timerId() == timer.timerId()) {
            grab_frame();
        }
    }
    fep3::base::DataSample data_sample;
    steady_clock::time_point _last_log_time;
    QBasicTimer timer;

};
MySink* sink = nullptr;

class EasySenderJob : public IJob
{
public:
    using ExecuteCall = std::function<fep3::Result(fep3::Timestamp)>;

    explicit EasySenderJob(
        const ExecuteCall& ex_in,
        const ExecuteCall& ex,
        const ExecuteCall& ex_out) :
        _execute_func_data_in(ex_in),
        _execute_func(ex),
        _execute_func_data_out(ex_out)
    {
    }

    fep3::Result executeDataIn(Timestamp time_of_execution)
    {
        return _execute_func_data_in(time_of_execution);
    }
    fep3::Result execute(Timestamp time_of_execution)
    {
        return _execute_func(time_of_execution);
    }
    fep3::Result executeDataOut(Timestamp time_of_execution)
    {
        return _execute_func_data_out(time_of_execution);
    }
    std::function<fep3::Result(fep3::Timestamp)> _execute_func_data_in;
    std::function<fep3::Result(fep3::Timestamp)> _execute_func;
    std::function<fep3::Result(fep3::Timestamp)> _execute_func_data_out;
};

class EasyCoreSenderElement : public core::ElementConfigurable
{
public:
    EasyCoreSenderElement()
        : core::ElementConfigurable("Demo Element Base Sender Type",
                                    FEP3_PARTICIPANT_LIBRARY_VERSION_STR)
    {_my_job = std::make_shared<EasySenderJob>(
            [this](fep3::Timestamp sim_time)-> fep3::Result { return {}; },
            [this](fep3::Timestamp sim_time)-> fep3::Result { return process(sim_time); },
            [this](fep3::Timestamp sim_time)-> fep3::Result { return processDataOut(sim_time); });
    }

    fep3::Result load() override
    {
        //register the job
        core::addToComponents("sender_job", _my_job, { 20ms }, *getComponents());
        // auto default_device = QMediaDevices::defaultVideoInput();
        // _camera.reset(new QCamera(default_device));
        // _sink.reset(new MySink);
        // _captureSession = new QMediaCaptureSession();
        // _captureSession->setCamera(_camera.get());
        // _captureSession->setVideoSink(_sink.get());
        return {};
    }
    
    fep3::Result initialize() override
    {
        //register the data
        auto video_type = base::StreamType(fep3::base::arya::meta_type_video);
        video_type.setProperty("height", "3840", "uint32_t");
        video_type.setProperty("width", "2160", "uint32_t");
        video_type.setProperty("pixelformat", "R(8)G(9)B(8)", "string");
        auto raw_type = base::StreamType(fep3::base::arya::meta_type_raw);
        _data_writer_video = std::make_shared<core::DataWriter>("video_stream", raw_type);

        auto data_adding_res = core::addToComponents(*_data_writer_video, *getComponents());
        if (isFailed(data_adding_res)) return data_adding_res;
        // _camera->start();
        return {};
    }

    void deinitialize() override
    {
        //very important in the core API ... you need to synchronously register and unregister your data
        core::removeFromComponents(*_data_writer_video, *getComponents());
    }
    
    void unload() override
    {
        //very important in the core API ... you need to synchronously register and unregister your jobs
        core::removeFromComponents("sender_job", *getComponents());
    }

    fep3::Result process(fep3::Timestamp sim_time_of_execution)
    {
        //write the data to the string signal queue
        std::lock_guard<std::mutex> g(mymutex);
        auto data_sample = sink->grab_frame();
        _data_writer_video->write(*data_sample);
        return {};
    }

    fep3::Result processDataOut(fep3::Timestamp sim_time_of_execution)
    {
        //this is to flush and write it to the bus
        _data_writer_video->flushNow(sim_time_of_execution);
        return {};
    }

    //use the PropertyVariable as easy readable configuration element
    //but do NOT forget to register the variables in the CTOR with a name
    core::PropertyVariable<int32_t> _prop_to_send_as_integer{ 1 };
    core::PropertyVariable<std::string> _prop_to_send_as_string{ "Hello FEP3 World!" };
    core::PropertyVariable<double> _prop_to_send_as_double{ 0.1 };
    //it is possible to use vector properties
    core::PropertyVariable<std::vector<std::string>> _prop_as_string_array{ {"value1", "value2", "value3"} };

    //in core API you need to deal with everything by yourself
    //have a look at the fep3::cpp::DataJob in cpp API
    std::shared_ptr<EasySenderJob> _my_job;
    std::shared_ptr<core::DataWriter> _data_writer_video;
    std::unique_ptr<QCamera> _camera;
    std::unique_ptr<MySink> _sink;
    QMediaCaptureSession* _captureSession = nullptr;
    std::mutex mymutex;
};


int main(int argc, const char* argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    int argc2 = 0;
    char **argv2; 
    QApplication  app(argc2, argv2);
    auto default_device = QMediaDevices::defaultVideoInput();
    QCamera* camera = new QCamera(default_device);
    QMediaCaptureSession captureSession;
    captureSession.setCamera(camera);
    //captureSession.setImageCapture(&qic);
    // captureSession.setVideoOutput(videoWidget);
    sink = new MySink();

    captureSession.setVideoSink(sink);
    // for(auto format : camera->cameraDevice().videoFormats()){
    //     auto p = format.pixelFormat();
    //     auto r = format.resolution();
    //     auto f = format.maxFrameRate();
    //     if (p ==  QVideoFrameFormat::PixelFormat::Format_Jpeg){
    //     camera->setCameraFormat(format);
    //     break;
    //     }
    // }
    camera->start();

    std::thread t([argc, argv, &app](){
        try
        {
            auto part = core::createParticipant<core::ElementFactory<EasyCoreSenderElement>>(
                argc, argv,
                "My Demo Participant Version 1.0",
                { "video_sender", "video_system", "" });
            part.exec();
            return app.quit();
        }
        catch (const std::exception& ex)
        {
            std::cerr << ex.what();
            app.quit();
        }
    });
    app.exec();
    t.detach();
}
