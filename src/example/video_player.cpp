#include <chrono>
#include <thread>
#include <iostream>
#include <memory>
#include <string>
#include <math.h>
#include <stdio.h>

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


void MyLabel::onImageAvailable(const QImage &newValue){
    this->setPixmap(QPixmap::fromImage(newValue));
};
MyLabel* label = nullptr;
fep3::base::DataSample data_sample;


class MySink : public QVideoSink {
public:
    MySink(MyLabel* label) {
    }

    void process() { // fep3::base::DataSample data_sample
        auto new_ba = QByteArray::fromRawData((char*)data_sample.cdata(), data_sample.getSize());
        if (new_ba.isEmpty()){
            return;
        }
        auto new_image = QImage::fromData(new_ba, "JPEG");
        if (new_image.isNull()){
            return;
        }
        emit  label->imageAvailable(new_image);
    }
    
};
MySink* sink = nullptr;

class EasyReceiverJob : public IJob
{
public:
    using ExecuteCall = std::function<fep3::Result(fep3::Timestamp)>;

    explicit EasyReceiverJob(
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

class EasyCoreReceiverElement : public core::ElementConfigurable
{
public:
    EasyCoreReceiverElement()
        : core::ElementConfigurable("Demo Element Base Receiver Type",
            FEP3_PARTICIPANT_LIBRARY_VERSION_STR)
    {
        //this Job will connect the process methods to the scheduler
        //you may also use another option, consider cpp::DataJob i.e.
        _my_job = std::make_shared<EasyReceiverJob>(
            [this](fep3::Timestamp sim_time)-> fep3::Result { return processDataIn(sim_time); },
            [this](fep3::Timestamp sim_time)-> fep3::Result { return process(sim_time); },
            [this](fep3::Timestamp sim_time)-> fep3::Result { return {}; });
    }
    
    fep3::Result load() override
    {
        //register the job
        return core::addToComponents("receiver_job", _my_job, { 20ms }, *getComponents());

    }

    fep3::Result initialize() override
    {
        auto video_type = base::StreamType(fep3::base::arya::meta_type_video);
        video_type.setProperty("height", "3840", "uint32_t");
        video_type.setProperty("width", "2160", "uint32_t");
        video_type.setProperty("pixelformat", "R(8)G(9)B(8)", "string");
        auto raw_type = base::StreamType(fep3::base::arya::meta_type_raw);

        _data_reader_video = std::make_shared<core::DataReader>("video_stream", raw_type);

        //register the data
        auto data_adding_res = core::addToComponents(*_data_reader_video, *getComponents());
        if (isFailed(data_adding_res)) return data_adding_res;
        return {};
    }

    void deinitialize() override
    {
        //very important in the core API ... you need to synchronously register and unregister your data
        core::removeFromComponents(*_data_reader_video, *getComponents());
    }
    
    void unload() override
    {
        //very important in the core API ... you need to synchronously register and unregister your jobs
        core::removeFromComponents("receiver_job", *getComponents());
    }

    fep3::Result processDataIn(fep3::Timestamp sim_time_of_execution)
    {
        //this is to receive samples and store them temporarily in the corresponding data readers
        _data_reader_video->receiveNow(sim_time_of_execution);
        return {};
    }

    fep3::Result process(fep3::Timestamp sim_time_of_execution)
    {
        auto read_sample = _data_reader_video->readSampleLatest();
        if (read_sample == nullptr){
            return {};
        }

        std::lock_guard<std::mutex> g(mymutex);
        read_sample->read(data_sample);
        if (!(data_sample.getSize() == 0)){
            sink->process();
        }
        // if (received_string_value.has_value())
        // {
        //     FEP3_LOG_INFO("received string value:" + received_string_value.value());
        // }
        auto log_time = high_resolution_clock::now();
        int duration = (log_time - _last_log_time).count() / 1e6;
        printf(a_util::strings::format("delta time: %d ms\n", duration).c_str());
        _last_log_time = log_time;
        return {};
    }


    //in core API you need to deal with everything by yourself
    //have a look at the fep3::cpp::DataJob in cpp API
    std::shared_ptr<EasyReceiverJob> _my_job;
    std::shared_ptr<core::DataReader> _data_reader_video;
    std::mutex mymutex;
    steady_clock::time_point _last_log_time{};
};


int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    int argc2 = 0;
    char **argv2; 
    QApplication  app(argc2, argv2);
    label = new MyLabel();
    label->setFixedSize(1066, 600);
    label->setScaledContents(true);
    label->show();
    QObject::connect(label, &MyLabel::imageAvailable, label, &MyLabel::onImageAvailable);

    std::thread t([argc, argv, &app](){
        try
        {
            auto part = core::createParticipant<core::ElementFactory<EasyCoreReceiverElement>>(
                argc, argv,
                "My Demo Participant Version 1.0",
                { "video_player", "video_system", "" });
            part.exec();
            app.quit();
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
