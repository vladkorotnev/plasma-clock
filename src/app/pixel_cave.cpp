#include <app/pixel_cave.h>
#include <service/prefs.h>
#include <service/disk.h>
#include <state.h>

#if HAS(PIXEL_CAVE)

const char LOG_TAG[] = "CAVE";
const char MELODY_FNAME[] = (FS_MOUNTPOINT "/music/043_pixel_cave.pomf");
const prefs_key_t HISCORE_PREFS_KEY = "PCAVE_hiscore";

AppPixelCave::AppPixelCave(NewSequencer *s) {
    sequencer = s;
    state = PCGAME_TITLE;
    cur_score = 0;
    melody = nullptr;
    playfield_buf = nullptr;
    playfield = nullptr;
    unsettled = false;
    player_yVel = 0;
    score_font = find_font(FONT_STYLE_HUD_DIGITS);
    gui_font = find_font(FONT_STYLE_UI_TEXT);
}

void AppPixelCave::prepare() {
    Renderable::prepare();
    hi_score = prefs_get_int(HISCORE_PREFS_KEY);
    state = PCGAME_TITLE;

    if(melody == nullptr) {
        melody = new PomfMelodySequence(MELODY_FNAME);
    }

    if(melody != nullptr) {
        sequencer->play_sequence(melody,SEQUENCER_REPEAT_INDEFINITELY);
        // Make it go all the way till the hook and loop the hook until the game is started
        sequencer->flags = (sequence_playback_flags_t) ((int)sequencer->flags | (int)SEQUENCER_PLAY_HOOK_ONLY);
    }
}

void AppPixelCave::render(FantaManipulator* fb) {
    Renderable::render(fb);

    if(playfield_buf == nullptr) {
        size_t pf_size = fb->get_width() * fb->get_height() / 8;
        playfield_buf = (fanta_buffer_t) malloc(pf_size);
        playfield = new FantaManipulator(playfield_buf, pf_size, fb->get_width(), fb->get_height(), NULL, &pf_dirty);
        ESP_LOGI(LOG_TAG, "Playfield W=%i H=%i", playfield->get_width(), playfield->get_height());
        playfield->clear();
    }

    framecount = (framecount + 1) % 101;

    switch(state) {
        case PCGAME_TITLE:
            {
                static const char ttl_txt[] = "Pixel Cave";
                static int ttl_width = measure_string_width(gui_font, ttl_txt, TEXT_OUTLINED | TEXT_NO_BACKGROUND);
                int ofs = ((int)(framecount/2)) - 50;
                fb->clear();
                fb->put_string(gui_font, ttl_txt, fb->get_width()/2 - ttl_width/2 + ofs, 0, TEXT_OUTLINED | TEXT_NO_BACKGROUND);

                // TODO: show hi score
            }
            break;

        case PCGAME_OVER:
            {
                fb->clear();
                // TODO GUI
                if(crash_flash < 2) {
                    render_game(fb, false);
                    if(crash_flash == 0)
                        fb->invert();
                    crash_flash ++;
                }
                
                static const char over_txt[] = "GAME OVER";
                static int over_width = measure_string_width(gui_font, over_txt, TEXT_OUTLINED | TEXT_NO_BACKGROUND);
                fb->put_string(gui_font, over_txt, fb->get_width()/2 - over_width/2, 0, TEXT_OUTLINED | TEXT_NO_BACKGROUND);

                // TODO: show score and hi score
            }
            break;

        case PCGAME_GAME:
            render_game(fb, true);
            break;
    }
}

void AppPixelCave::render_game(FantaManipulator* fb, bool show_score) {
    fb->clear();
    fb->put_fanta(playfield->get_fanta(), 0, 0, playfield->get_width(), playfield->get_height());

    fb->plot_pixel(playerPos.x, playerPos.y, true);

    uint8_t divisor = 2;
    for(int i = 0; i < PLAYER_TRAIL_LEN; i++) {
        if(framecount % divisor == 0) {
            fb->plot_pixel(playerTrail[i].x, playerTrail[i].y, true);
        }
        divisor++;
    }

    static char score_buf[8] = { 0 };
    if(show_score) {
        snprintf(score_buf, 8, "%04d", cur_score);
        int score_width = measure_string_width(score_font, score_buf, TEXT_OUTLINED | TEXT_NO_BACKGROUND);
        fb->put_string(score_font, score_buf, fb->get_width() - score_width, fb->get_height() - score_font->height - 1, TEXT_OUTLINED | TEXT_NO_BACKGROUND);
    }
}

void AppPixelCave::step() {
    Renderable::step();

    switch(state) {
        case PCGAME_TITLE:
        case PCGAME_OVER:
            if (hid_test_key_state(KEY_LEFT) == KEYSTATE_HIT) {
                leave();
                return;
            }
            else if(hid_test_key_any() == KEYSTATE_HIT) {
                new_game();
                return;
            }
            break;

        case PCGAME_GAME:
            step_game();
            break;
    }
}

void AppPixelCave::step_game() {
    for(int i = PLAYER_TRAIL_LEN - 1; i > 0; i--) {
        playerTrail[i] = playerTrail[i - 1];
        playerTrail[i].x--;
    }
    playerTrail[0].x = playerPos.x - 1;
    playerTrail[0].y = playerPos.y;

    if(playerPos.x < PLAYER_MAX_X) {
        playerPos.x++;
    }

    if(unsettled) {
        if(framecount % 5 == 0)
            playerPos.y += player_yVel;
        if(playerPos.y >= playfield->get_height() || playerPos.y <= 0) {
            player_crashed();
        }
    }

    // TODO: variable speed
    if(framecount % 5 == 0) {
        playfield->scroll(1, 0);
        playfield->line(0, 0, 0, 3);
        playfield->line(0, playfield->get_height(), 0, playfield->get_height() - 3);
        // TODO: render walls
        if(unsettled) {
            cur_score ++;
        }
    }

    // TODO: collision check with walls

    if(hid_test_key_any()) {
        if(framecount % 8 == 0) {
            unsettled = true;
            player_yVel = std::max(player_yVel - 1, -4);
        }
    } else if(unsettled) {
        if(framecount % 8 == 0) {
            player_yVel = std::min(player_yVel + 1, 4);
        }
    }
}

void AppPixelCave::player_crashed() {
    state = PCGAME_OVER;
    if(cur_score > hi_score) {
        hi_score = cur_score;
        prefs_set_int(HISCORE_PREFS_KEY, hi_score);
    }
}

void AppPixelCave::leave() {
    pop_state(STATE_PIXEL_CAVE);
}

void AppPixelCave::new_game() {
    sequencer->flags = (sequence_playback_flags_t) ((int)sequencer->flags & (int)~SEQUENCER_PLAY_HOOK_ONLY);
    state = PCGAME_GAME;
    playerPos = { 0, 8 };
    for(int i = 0; i < PLAYER_TRAIL_LEN; i++) playerTrail[i] = { 0, 8 };
    playfield->clear();
    framecount = 0;
    unsettled = false;
    player_yVel = 0;
    cur_score = 0;
    crash_flash = 0;
}

void AppPixelCave::cleanup() {
    Renderable::cleanup();
    sequencer->stop_sequence();
}

#endif