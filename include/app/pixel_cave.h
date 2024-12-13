#include <views/framework.h>
#include <sound/sequencer.h>

#if HAS(PIXEL_CAVE)
class AppPixelCave: public Renderable {
public:
    AppPixelCave(NewSequencer *);
    void prepare() override;
    void render(FantaManipulator*) override;
    void step() override;
    void cleanup() override;

protected:
    enum PixelCaveState {
        PCGAME_TITLE,
        PCGAME_GAME,
        PCGAME_OVER
    };

    struct PxcCoord {
        int16_t x;
        uint16_t y;
    };

    NewSequencer * sequencer;
    MelodySequence * melody;
    PixelCaveState state;

    const font_definition_t * score_font;
    const font_definition_t * gui_font;
    uint32_t cur_score;
    uint32_t hi_score;
    uint8_t framecount;

    static const size_t PLAYER_TRAIL_LEN = 4;
    static const int16_t PLAYER_MAX_X = 16;
    PxcCoord playerPos;
    int16_t player_yVel;
    bool unsettled;
    int crash_flash;
    PxcCoord playerTrail[PLAYER_TRAIL_LEN];

    fanta_buffer_t playfield_buf;
    FantaManipulator * playfield;
    bool pf_dirty;

    void leave();
    void new_game();
    void render_game(FantaManipulator*, bool);
    void step_game();
    void player_crashed();
};
#endif