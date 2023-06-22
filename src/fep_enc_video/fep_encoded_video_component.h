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

const StreamMetaType meta_type_encoded_video{"encoded_video", 
std::list<std::string>{ 
    "height", "width", 
    // codec
    "codec_id", "pixelformat",
    "bits_per_coded_sample", "bit_rate",
    // color
    "color_range", "color_primaries", "color_trc", "color_space", "chroma_location",
    // Possibly neede extra data
    "profile", "level",  "sample_aspect_ratio", "field_order",
     meta_type_prop_name_max_byte_size 
     }};

std::string pixelformat = "codec_id=;pixelformat=;bits_per_coded_sample=;bit_rate=;color_range=;color_primaries=;color_trc=;color_space=;chroma_location=;";

// Api


StreamMetaType getHEVCStreamMetaType(){
    auto meta_type_video{};
    pixelformat_hevc = a_util::string::format(pixelformat, );
    meta_type_video.setProperty("pixelformat", pixelformat)
    return meta_type_video;


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
    ~ISimulationBus() = default;

public:
    class VideoPlayer{
        create_display(fps_overlay, size)
        initialize() // only once preallocate everything
        display_video(AVFrame frame)
        deinitalize() // cleanup
    }
    AddNewVideoSignal(signal_name, codec_parameters)

// InputSignal for built-in support
selectInput(file)

selectHardwareInput(device)

activateHWAcceleration()

selectCodec()

openStream() // in initialize

frame readFrame()

transcodeToSelectedCodec(packet) -> framePacket

// TODO how?
setRawFrame(sample, frame)

// Receiver

AVFrame readSampleToFrame(sample)


      /**
     * @brief This is an overloaded member function that gets a writer for data on an output
     * signal of dynamic stream type with the given signal \p name whose queue capacity is \p
     * queue_capacity. The queue behaves like a FIFO: If the queue is full the oldest sample
     * (according to the samples' timestamp) will be discarded upon arrival of a new sample.
     *
     * @param[in] name The name of the output signal (must be unique)
     * @param[in] queue_capacity The capacity of the queue.
     * @return Data writer if succeeded, nullptr otherwise (e. g. if this method has already been
     *         called for the signal with the given signal \p name)
     */
    virtual std::unique_ptr<arya::ISimulationBus::IDataWriter> getWriter(const std::string& name, size_t queue_capacity) = 0;

    /**
     * @brief Starts passing all incoming items to the receivers of the data readers.
     * @see IDataReader::reset
     * This method registers for reception of incoming data at the transmission layer,
     * prepares for a call to @ref stopBlockingReception and then calls @p reception_preparation_done_callback.
     * If at least one existing reader has a receiver set via @ref IDataReader::reset, this method
     * blocks until @ref stopBlockingReception is called. Otherwise this method does not block.
     * Use this method to implement "data triggered" behavior.
     * This method is thread-safe against calls to @ref stopBlockingReception.
     * @note The receivers of the data readers will be called from within the thread context this method is called from.
     * @note To ensure the Simulation Bus is fully prepared for data reception as well as for a call to @ref stopBlockingReception,
     * wait until @p reception_preparation_done_callback has been called before calling any other method on the Simulation Bus.
     *
     * @param reception_preparation_done_callback Callback to be called once the reception is prepared
     */
    virtual void startBlockingReception(const std::function<void()>& reception_preparation_done_callback) = 0;
    /**
     * @brief Stops all receptions running due to calls of method @ref startBlockingReception. This causes all blocking calls of
     * method @ref startBlockingReception to return. If there are currently no blocking calls to @ref startBlockingReception,
     * this method does nothing.
     * This method is thread-safe against calls to @ref startBlockingReception.
     */
    virtual void stopBlockingReception() = 0;
};

}
using arya::ISimulationBus;
}
