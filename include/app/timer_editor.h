#pragma once
#include <views/framework.h>
#include <views/common/dropping_digits.h>
#include <views/menu/melody_selection_item.h>
#include <app/proto/navigation_stack.h>
#include <sound/sequencer.h>

class AppShimTimerEditor: public ProtoShimNavigationStack {
public:
    AppShimTimerEditor(Beeper *);
    ~AppShimTimerEditor();
private:
    class TimerEditorMainScreen;
    TimerEditorMainScreen * appRoot;
};