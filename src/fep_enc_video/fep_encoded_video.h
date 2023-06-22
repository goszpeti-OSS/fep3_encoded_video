#pragma once

#include "fep_encoded_video_component.h"

#include <fep3/base/properties/propertynode.h>
#include <fep3/components/base/component.h>
#include <fep3/components/logging/logging_service_intf.h>

#if defined _WIN32
#ifdef BUILD_PLUGIN
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __declspec(dllimport)
#endif
#else
#define EXPORTED
#endif

class EXPORTED EncodedVideoComponent : public fep3::base::Component<fep3::arya::IEncodedVideo> {
public:
    EncodedVideoComponent();
    ~EncodedVideoComponent() = default;
    EncodedVideoComponent(const EncodedVideoComponent&) = delete;
    EncodedVideoComponent(EncodedVideoComponent&&) = delete;
    EncodedVideoComponent& operator=(const EncodedVideoComponent&) = delete;
    EncodedVideoComponent& operator=(EncodedVideoComponent&&) = delete;

};