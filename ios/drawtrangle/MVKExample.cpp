/*
 *  MVKExample.cpp
 *
 *  Copyright (c) 2016-2017 The Brenwill Workshop Ltd.
 *  This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */


#include "MVKExample.h"

void MVKExample::displayLinkOutputCb() {                        // SRS - expose VulkanExampleBase::displayLinkOutputCb() to DemoViewController
    //_vulkanExample->displayLinkOutputCb();
}

void MVKExample::keyPressed(uint32_t keyChar) {					// SRS - handle keyboard key presses only (e.g. Pause, Space, etc)

}

void MVKExample::keyDown(uint32_t keyChar) {					// SRS - handle physical keyboard key down/up actions and presses

}

void MVKExample::keyUp(uint32_t keyChar) {

}

void MVKExample::mouseDown(double x, double y) {
    //_vulkanExample->mouseState.position = glm::vec2(x, y);
    //_vulkanExample->mouseState.buttons.left = true;
}

void MVKExample::mouseUp() {
    //_vulkanExample->mouseState.buttons.left = false;
}

void MVKExample::rightMouseDown(double x, double y) {
	//_vulkanExample->mouseState.position = glm::vec2(x, y);
    //_vulkanExample->mouseState.buttons.right = true;
}

void MVKExample::rightMouseUp() {
    //_vulkanExample->mouseState.buttons.right = false;
}

void MVKExample::otherMouseDown(double x, double y) {
	//_vulkanExample->mouseState.position = glm::vec2(x, y);
    //_vulkanExample->mouseState.buttons.middle = true;
}

void MVKExample::otherMouseUp() {
    //_vulkanExample->mouseState.buttons.middle = false;
}

void MVKExample::mouseDragged(double x, double y) {
    //_vulkanExample->mouseDragged(x, y);
}

void MVKExample::scrollWheel(short wheelDelta) {
    //_vulkanExample->camera.translate(glm::vec3(0.0f, 0.0f, wheelDelta * 0.05f * //_vulkanExample->camera.movementSpeed));
}

void MVKExample::fullScreen(bool fullscreen) {
	//_vulkanExample->settings.fullscreen = fullscreen;
}

MVKExample::MVKExample(void* view, double scaleUI) {
    //_vulkanExample = new VulkanExample();
    //_vulkanExample->initVulkan();
    //_vulkanExample->setupWindow(view);
	//_vulkanExample->settings.vsync = true;		// SRS - set vsync flag since this iOS/macOS example app uses displayLink vsync rendering
	//_vulkanExample->ui.scale = scaleUI;	        // SRS - set UIOverlay scale to maintain relative proportions/readability on retina displays
    //_vulkanExample->prepare();
	//_vulkanExample->renderLoop();				// SRS - this inits destWidth/destHeight/lastTimestamp/tPrevEnd, then falls through and returns
}

MVKExample::~MVKExample() {

}
