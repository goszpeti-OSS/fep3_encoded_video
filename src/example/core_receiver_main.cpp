/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

@endverbatim
 */

#include <iostream>
#include <memory>
#include <string>

#include <fep3/core.h>
#include <fep3/core/element_configurable.h>
#include <chrono>
#include <thread>
#include <math.h>
#include <stdio.h>
using namespace std::chrono;
#include "video.h"


using namespace fep3;
using namespace std::chrono_literals;


void MyLabel::onImageAvailable(const QImage &newValue){
    this->setPixmap(QPixmap::fromImage(newValue));
};
MyLabel* label = nullptr;
class MySink : public QVideoSink {

public:
    MySink(MyLabel* label) : label(label) {
        //timer.start(10, this);
    }
public:

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
       emit  label->imageAvailable(i1);
       frame.unmap();
        auto log_time = high_resolution_clock::now();
        int duration = (log_time - last_log_time).count() / 1e6;

        //LOG_INFO(a_util::strings::format("frame delta time: %d us", frame.endTime() - frame.startTime()).c_str());
        printf(a_util::strings::format("delta time: %d ms\n", duration).c_str());
        // printf("%d", frame.endTime());
        last_log_time = log_time;

    }
    void timerEvent(QTimerEvent* event)
    {
        if (event->timerId() == timer.timerId()) {
            process();
        }
    }
    MyLabel* label = nullptr;
    steady_clock::time_point last_log_time;
    QBasicTimer timer;

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
    //Implementation of the CTOR!
    //ElementConfigurable has no default CTOR
    // you must define a type of your element -> to identify your implementation in a system
    // you must define a implementation version -> to identify your implementation version in a system
    // KEEP in MIND THIS IS NOT THE ELEMENT INSTANCE NAME!
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
        return core::addToComponents("receiver_job", _my_job, { 40ms }, *getComponents());

    }

    fep3::Result initialize() override
    {
        auto video_type = base::StreamType(fep3::base::arya::meta_type_video);
        // video_type.setProperty("height", "3840", "uint32_t");
        // video_type.setProperty("width", "2160", "uint32_t");
        // video_type.setProperty("pixelformat", "R(8)G(9)B(8)", "string");
        //video_type.setProperty(fep3::base::arya::meta_type_prop_name_max_byte_size, "24883200", "uint32_t");
        _data_reader_video = std::make_shared<core::DataReader>("video_stream", video_type);

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
        sink->process();

        //receive string value
        Optional<std::string> received_string_value;
        *_data_reader_video >> received_string_value;
        if (received_string_value.has_value())
        {
            FEP3_LOG_INFO("received string value:" + received_string_value.value());
        }
        return {};
    }


    //in core API you need to deal with everything by yourself
    //have a look at the fep3::cpp::DataJob in cpp API
    std::shared_ptr<EasyReceiverJob> _my_job;
    std::shared_ptr<core::DataReader> _data_reader_video;
};


int main(int argc, char *argv[])
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
    label = new MyLabel();
    sink = new MySink(label);
    captureSession.setVideoSink(sink);

    camera->start();

    std::thread t([argc, argv](){
        try
        {
            auto part = core::createParticipant<core::ElementFactory<EasyCoreReceiverElement>>(
                argc, argv,
                "My Demo Participant Version 1.0",
                { "video_receiver", "video_system", "" });
            return part.exec();
        }
        catch (const std::exception& ex)
        {
            std::cerr << ex.what();
        }
    });
    label->setFixedSize(1066, 600);
    label->setScaledContents(true);
    label->show();
    QObject::connect(label, &MyLabel::imageAvailable, label, &MyLabel::onImageAvailable);
    app.exec();
    t.detach();
}
