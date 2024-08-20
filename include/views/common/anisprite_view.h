#pragma once
#include <graphics/sprite.h>
#include <views/common/view.h>

class SpriteView: public Composable {
public:
    SpriteView(const sprite_t * sprite = nullptr) {
        set_sprite(sprite);
    }

    void set_sprite(const sprite_t * sprite) {
        current = sprite;
    }

    void render(FantaManipulator*fb) override {
        if(current != nullptr) {
            fb->put_sprite(current, 0, 0);
        }
    }

protected:
    const sprite_t * current;
};

class AniSpriteView: public SpriteView {
public:
    AniSpriteView(const ani_sprite * sprite = nullptr) {
        set_sprite(sprite);
    }

    void set_sprite(const ani_sprite * sprite) {
        icon = sprite;
        if(icon != nullptr && icon->data != nullptr) {
            icon_state = ani_sprite_prepare(icon);
            current_icon_frame = ani_sprite_frame(&icon_state);
            current = &current_icon_frame;
        } else {
            icon_state = { 0 };
            current_icon_frame = { 0 };
            current = nullptr;
        }
    }

    void step() override {
        if(icon->data == nullptr || icon_state.ani_sprite == nullptr) return;
        current_icon_frame = ani_sprite_frame(&icon_state);
        current = &current_icon_frame;
    }

private:
    const ani_sprite * icon;
    ani_sprite_state_t icon_state;
    sprite_t current_icon_frame;
};