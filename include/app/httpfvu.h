#pragma once
#include <sound/melodies.h>
#include <views/framework.h>
#include <views/common/swoopie.h>

class HttpFvuApp: public Composite {
public:
    HttpFvuApp(NewSequencer *s);
    void prepare() override;
    void cleanup() override;
    void step() override;
    void work_task();
private:
    enum State {
        FVUAPP_INIT,
        FVUAPP_CHECKING,
        FVUAPP_INSTALLING_FS,
        FVUAPP_INSTALLING_APP,
        FVUAPP_NO_UPDATES,
        FVUAPP_FAILED,
        FVUAPP_SUCCESS
    };
    State appState;
    State _oldState;
    uint16_t delayCounter;
    Swoopie * progBar;
    StringScroll * label;
    NewSequencer * sequencer;
};