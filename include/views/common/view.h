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
};

class Composable: public Renderable {
public:
    int x_offset = 0;
    int width = -1;
    bool hidden = false;
    // NB: Experimental. Current implementation is less flickery than just showing only every other frame, but doesn't play nice with alpha channel items.
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
        if(wants_clear_surface) {
            fb->clear();
        }
        for(Composable *r: composables) {
            if(r->hidden) continue;
#ifndef COMPOSABLE_NO_EVENODD
            if(r->gray) { 
                int w = (r->width >= 0 ? r->width : fb->get_width());
                size_t bufsz = w * sizeof(uint16_t);
                fanta_buffer_t tmpbuf = (fanta_buffer_t) gralloc(bufsz);
                if(tmpbuf) {
                    fanta_buffer_t mask = (fanta_buffer_t) gralloc(bufsz);
                    if(mask) {
                        bool dirty = false;
                        FantaManipulator *tmp = new FantaManipulator(tmpbuf, bufsz, w, 16, NULL, &dirty);
                        tmp->clear();
                        dirty = false;
                        r->render(tmp);
                        if(dirty) {
                            memcpy(mask, tmpbuf, bufsz);
                            for(int i = 0; i < bufsz; i++) { mask[i] &= (even_odd ? 0b01010101 : 0b10101010); }
                            fb->put_fanta(tmp->get_fanta(), r->x_offset, 0, tmp->get_width(), tmp->get_height(), mask);
                        }
                        delete tmp;
                        free(mask);
                    } else ESP_LOGE("COMP", "Failed to alloc MASK of %i bytes", bufsz);
                    free(tmpbuf);
                } else ESP_LOGE("COMP", "Failed to alloc TMP of %i bytes", bufsz);
                continue;
            }
#endif
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
#ifndef COMPOSABLE_NO_EVENODD
        even_odd ^= 1;
#endif
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
    bool even_odd = false;
};

/// @brief Do not override the user-specified display time
const int DISP_TIME_NO_OVERRIDE = -1;
/// @brief Skip this screen if possible.
const int DISP_TIME_DONT_SHOW = 0;
/// @brief Switch to this screen immediately and stay here if possible.
const int DISP_TIME_ATTENTION = INT32_MAX;

class DisplayTimeable {
public:
    /// @brief Return the desired display time to stay on-screen for longer than the user-specified setting.
    /// Return `DISP_TIME_NO_OVERRIDE` to not override anything.
    /// Return `DISP_TIME_DONT_SHOW` to avoid display if possible (but still be prepared to render for the transition or something!)
    /// Return `DISP_TIME_ATTENTION` to demand switching to the screen instantly. The request may or may not be fulfilled.
    virtual int desired_display_time() { return DISP_TIME_NO_OVERRIDE; }
};

class Screen: public DisplayTimeable, public Composite {};