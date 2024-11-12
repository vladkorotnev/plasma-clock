#include <views/framework.h>
#include <views/overlays/rain_ovl.h>
#include <views/overlays/fireworks.h>
#include <sound/sequencer.h>
#include <sound/yukkuri.h>

class NewYearAppShim: public Composite {
public:
    NewYearAppShim(Beeper *, NewSequencer *, Yukkuri *);

    void prepare() override;
    void render(FantaManipulator*) override;
    void step() override;
    void cleanup() override;

private:
    enum DemoPhase {
        INTRO,
        INTENSE,
        ANNOUNCE,
        FIREWORKS,
        CHILLOUT,
        FIN
    };
    DemoPhase phase;

    NewSequencer * sequencer;
    Beeper * beeper;
    Yukkuri * balabol;
    FireworksOverlay * fireworks;
    RainOverlay * snow;
    const char * utterance_localized;
    char digits[4] = { '0', '0', '0', '0' };
    TickType_t start_time;
    const font_definition_t * font;
    
    // This app is only executed once a year at most
    // so we should not keep any resources before `prepare` or after `cleanup`
    bool allocated;
    void allocate_all();
};