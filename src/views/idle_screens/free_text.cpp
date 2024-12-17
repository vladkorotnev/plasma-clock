#include <views/idle_screens/free_text.h>
#include <service/prefs.h>

static char LOG_TAG[] = "FRTXT";

FreeTextView::FreeTextView() {
    next_str = nullptr;
    view = new StringScroll(find_font(FONT_STYLE_TALL_TEXT));

    view->scroll_only_if_not_fit = false;

    add_composable(view);
    last_str_idx = -1;
    enqueue_next_string();
}

int FreeTextView::desired_display_time() {
    if(next_str) {
        return view->estimated_frame_count() * 1000 / 62;
    }

    return DISP_TIME_DONT_SHOW;
}

void FreeTextView::prepare() {
    if(next_str != nullptr) {
        view->set_string(next_str);
    }
    
    Screen::prepare();
}

void FreeTextView::cleanup() {
    Screen::cleanup();
    enqueue_next_string();
}

void FreeTextView::enqueue_next_string() {
    if(next_str != nullptr) {
        free(next_str);
        next_str = nullptr;
    }

    String prefs = prefs_get_string(PREFS_KEY_FREE_TEXT_STRING);
    prefs.trim();
    const char * str_lst = prefs.c_str(); // Arduino string is SHIT

    if(prefs.length() > 0) {
        ESP_LOGV(LOG_TAG, "String data: %s", prefs.c_str());
        int count_entries = 1;
        const char * tmp = str_lst;
        while(char16_t uchar = iterate_utf8(&tmp)) {
            if(uchar == '\n') count_entries++;
        }
        ESP_LOGV(LOG_TAG, "Count of entries: %i", count_entries);

        int str_index = last_str_idx;
        if(prefs_get_int(PREFS_KEY_FREE_TEXT_RANDOM)) {
            while(str_index == last_str_idx)
                str_index = esp_random() % count_entries;
        } else {
            str_index = (last_str_idx + 1) % count_entries;
        }

        last_str_idx = str_index;
        ESP_LOGV(LOG_TAG, "New index: %i", str_index);

        tmp = str_lst;
        ESP_LOGV(LOG_TAG, "Start ptr 0x%x", tmp);
        const char * start_ptr = tmp;
        const char * end_ptr = tmp;
        while(char16_t uchar = iterate_utf8(&tmp)) {
            if(uchar == '\n') {
                str_index--;
                if(str_index < 0) {
                    end_ptr = tmp - 1;
                    break;
                } else {
                    start_ptr = tmp;
                }
            }
        }
        
        // When last index, there will be no \n terminator
        if(end_ptr < start_ptr) end_ptr = tmp;

        size_t byte_count = end_ptr - start_ptr;
        ESP_LOGI(LOG_TAG, "From 0x%x to 0x%x, %i bytes", start_ptr, end_ptr, byte_count);

        next_str = (char*) malloc(byte_count + 1);
        strncpy(next_str, start_ptr, byte_count);
        next_str[byte_count] = 0;

        ESP_LOGV(LOG_TAG, "Next message: %s", next_str);
    }
}