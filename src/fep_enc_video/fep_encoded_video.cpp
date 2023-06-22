#include "fep_encoded_video_component.h"

class EXPORTED EncodedVideoComponent : public fep3::base::Component<fep3::arya::IEncodedVideo> {
public:
    EncodedVideoComponent();
    ~EncodedVideoComponent();
    EncodedVideoComponent(const EncodedVideoComponent&) = delete;
    EncodedVideoComponent(EncodedVideoComponent&&) = delete;
    EncodedVideoComponent& operator=(const EncodedVideoComponent&) = delete;
    EncodedVideoComponent& operator=(EncodedVideoComponent&&) = delete;