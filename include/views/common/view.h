#pragma once
#include <graphics/fanta_manipulator.h>
#include <vector>

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

class Composable: public Renderable {
public:
    int x_offset = 0;
    int width = -1;
    bool hidden = false;
};

class ClipView: public Composable {
public:
    ClipView(Renderable* content):
        content(content)
    {}

    ~ClipView() {
        delete content;
    }

    void prepare() { content->prepare(); }
    void render(FantaManipulator*fb) { content->render(fb); }
    void step() { content->step(); }
    void cleanup() { content->cleanup(); }

protected:
    Renderable * content;
};

class Composite: public Composable {
public:
    void add_composable(Composable * r) { composables.push_back(r); }
    void add_subrenderable(Renderable * r) { add_composable(new ClipView(r)); }
    void prepare() {
        for(Composable *r: composables) r->prepare();
    }
    void render(FantaManipulator*fb) {
        for(Composable *r: composables) {
            if(r->hidden) continue;
            if(r->x_offset <= 0 && r->width < 0) {
                r->render(fb);
            } else if(r->width > 0) {
                FantaManipulator * temp = fb->slice(r->x_offset, r->width);
                r->render(temp);
                delete temp;
            } else if(r->width == -1) {
                FantaManipulator * temp = fb->slice(r->x_offset, fb->get_width() - r->x_offset);
                r->render(temp);
                delete temp;
            }
        }
    }
    void step() {
        for(Composable *r: composables) r->step();
    }
    void cleanup() {
        for(Renderable *r: composables) r->cleanup();
    }
protected:
    std::vector<Composable*> composables = {};
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

class Screen: public DisplayTimeable, public Composite {};