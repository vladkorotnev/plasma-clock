#pragma once
#include <views/framework.h>

class FreeTextView: public Screen {
public:
    FreeTextView();
    void prepare() override;
    void cleanup() override;
    int desired_display_time() override;

private:
    StringScroll * view;
    char * next_str;
    int last_str_idx;
    void enqueue_next_string();
};