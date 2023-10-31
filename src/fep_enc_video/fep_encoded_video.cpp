#include "fep_encoded_video.h"

EncodedVideoComponent::EncodedVideoComponent() {
	printf("HELLO FEP VIDEO!");

}

fep3::Result EncodedVideoComponent::create() {
	printf("HELLO FEP VIDEO2!");
	return {};
}
fep3::Result EncodedVideoComponent::destroy() {
	return {};

}
fep3::Result EncodedVideoComponent::initialize() {
	return {};

}
fep3::Result EncodedVideoComponent::deinitialize() {
	return {};
}