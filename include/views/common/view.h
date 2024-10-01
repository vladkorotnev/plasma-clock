#pragma once
#include <graphics/fanta_manipulator.h>
#include <device_config.h>
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

    // EXPERIMENTAL API FOLLOWS
    virtual void render_plane(FantaManipulator* fb, RenderPlane plane) {
        if(plane == RPLANE_NEUTRAL) this->render(fb);
    }
};

class Composable: public Renderable {
public:
    int x_offset = 0;
    int width = -1;
    bool hidden = false;
    bool gray = false;
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
    void render_plane(FantaManipulator*fb, RenderPlane p) override { content->render_plane(fb, p); }
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
    void render_plane(FantaManipulator*fb, RenderPlane p) override {
        if(wants_clear_surface) {
            fb->clear();
        }
        for(Composable *r: composables) {
            if(r->hidden) continue;
#ifndef COMPOSABLE_NO_EVENODD
            if(r->gray && !even_odd) continue;
#endif
            if(r->x_offset <= 0 && r->width < 0) {
                r->render_plane(fb, p);
            } else if(r->width > 0) {
                FantaManipulator * temp = fb->slice(r->x_offset, r->width);
                r->render_plane(temp, p);
                delete temp;
            } else if(r->width == -1) {
                FantaManipulator * temp = fb->slice(r->x_offset, fb->get_width() - r->x_offset);
                r->render_plane(temp, p);
                delete temp;
            }
        }
        even_odd ^= 1;
    }
    void step() {
        for(Composable *r: composables) r->step();
    }
    void cleanup() {
        for(Renderable *r: composables) r->cleanup();
    }
protected:
    std::vector<Composable*> composables = {};
    bool wants_clear_surface = false;
private:
    bool even_odd = false;
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