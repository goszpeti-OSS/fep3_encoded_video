#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <string.h>

#include <fep3/fep3_participant_types.h>
#include <fep3/fep3_errors.h>
#include <fep3/fep3_timestamp.h>
#include <fep3/fep3_optional.h>
#include <fep3/components/base/component_iid.h>
#include <fep3/base/stream_type/stream_type.h>
#include <fep3/base/sample/data_sample_intf.h>


// for later usage - this needs to be supported by the simbus -> not exchangeable then
// 

//const StreamMetaType meta_type_video{"video", std::list<std::string>{ "height", "width", "pixelformat", meta_type_prop_name_max_byte_size }};
// Maybe implement pixelformat as a key=value stringdict and stream every other attribute in it -> ugly but compatbile

//const StreamMetaType meta_type_encoded_video{"encoded_video", 
//std::list<std::string>{ 
//    "height", "width", 
//    // codec
//    "codec_id", "pixelformat",
//    "bits_per_coded_sample", "bit_rate",
//    // color
//    "color_range", "color_primaries", "color_trc", "color_space", "chroma_location",
//    // Possibly neede extra data
//    "profile", "level",  "sample_aspect_ratio", "field_order",
//     meta_type_prop_name_max_byte_size 
//     }};


// Api


//StreamMetaType getHEVCStreamMetaType(){
//    auto meta_type_video{};
//    pixelformat_hevc = a_util::string::format(pixelformat, );
//    meta_type_video.setProperty("pixelformat", pixelformat)
//    return meta_type_video;


namespace fep3
{
namespace arya
{

/**
 * @brief Interface for the simulation bus
 */
class IEncodedVideo
{
public:
    // Definition of the component interface identifier for the simulation bus
    FEP_COMPONENT_IID("encoded_video.arya.fep3.iid")

protected:
    /**
     * @brief DTOR
     * @note This DTOR is explicitly protected to prevent destruction via this interface.
     */
    ~IEncodedVideo() = default;

public:
    std::string pixelformat = "codec_id=;pixelformat=;bits_per_coded_sample=;bit_rate=;color_range=;color_primaries=;color_trc=;color_space=;chroma_location=;";

    class VideoPlayer {
        //create_display(fps_overlay, size)
        //initialize() // only once preallocate everything
        //display_video(AVFrame frame)
        //deinitalize() // cleanup
    };

    //AddNewVideoSignal(signal_name, codec_parameters)

    //// InputSignal for built-in support
    //selectInput(file)

    //selectHardwareInput(device)

    //activateHWAcceleration()

    //selectCodec()

    //openStream() // in initialize

    //frame readFrame()

    //transcodeToSelectedCodec(packet) -> framePacket

    //// TODO how?
    //setRawFrame(sample, frame)

    //// Receiver

    //AVFrame readSampleToFrame(sample)


    //virtual void startBlockingReception(const std::function<void()>& reception_preparation_done_callback) = 0;

    //virtual void stopBlockingReception() = 0;
};

}
using arya::IEncodedVideo;
}