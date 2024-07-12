#pragma once
#include <graphics/fanta_manipulator.h>

/// @brief A renderable view
class Renderable {
public:
    /// @brief Prepare the view to be rendered. Called once per lifecycle.
    virtual void prepare() { }
    /// @brief Render the view in the provided manipulator. Called each frame while the framebuffer is locked. Keep concise to avoid framerate drops and hiccups.
    virtual void render(FantaManipulator*) { }
    /// @brief Prepare for the next frame to be rendered. Called each frame after the framebuffer lock was released. Do the heavy lifting here instead.
    virtual void step() { }
    /// @brief Called when the view is about to go off-screen. Release any big unused resources here.
    virtual void cleanup() { }
};

/// @brief Do not override the user-specified display time
const int DISP_TIME_NO_OVERRIDE = -1;
/// @brief Skip this screen if possible.
const int DISP_TIME_DONT_SHOW = 0;

class DisplayTimeable {
public:
    /// @brief Return the desired display time to stay on-screen for longer than the user-specified setting.
    /// Return DISP_TIME_NO_OVERRIDE to not override anything. Return DISP_TIME_DONT_SHOW to avoid display if possible (but still be prepared to render!).
    virtual int desired_display_time() { return DISP_TIME_NO_OVERRIDE; }
};

class Screen: public DisplayTimeable, public Renderable {};