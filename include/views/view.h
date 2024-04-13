#pragma once
#include <plasma/fanta_manipulator.h>

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