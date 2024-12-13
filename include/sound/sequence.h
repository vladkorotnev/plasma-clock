#pragma once
#include <sound/types.h>
#include <vector>

class MelodySequence {
public:
    virtual bool load() { return false; } 
    virtual void unload() {}
    virtual const char * get_title() { return ""; }
    virtual const char * get_long_title() { return ""; }
    virtual const melody_item_t * get_array() { return nullptr; }
    virtual int get_num_rows() { return 0; }
    MelodySequence() {}
};

class StaticMelodySequence: public MelodySequence {
public:
    StaticMelodySequence(const melody_item_t * array, int num_rows, const char * title, const char * long_title = nullptr):
        array_ { array },
        num_rows_ { num_rows },
        title_ { title },
        long_title_ { long_title },
        MelodySequence() {}
    
    bool load() override { return true; }
    const char * get_title() override { return title_; }
    const melody_item_t * get_array() override { return array_; }
    int get_num_rows() override { return num_rows_; }

    const char * get_long_title() override {
        if(long_title_ != nullptr) return long_title_;
        return title_;
    }

protected:
    const char * title_;
    const char * long_title_;
    const melody_item_t * array_;
    int num_rows_;
};


class PomfMelodySequence: public MelodySequence {
public:
    PomfMelodySequence(const char * sourcePath);
    ~PomfMelodySequence();

    bool valid() { return exists; }
    bool load() override;
    void unload() override;

    const char * get_title() override { return title_; }
    const char * get_long_title() override { return long_title_; }
    const melody_item_t * get_array() override { return array_; }
    int get_num_rows() override { return num_rows_; }

protected:
    bool exists = false;
    bool loaded = false;
    char path[64] = { 0 };
    char * title_ = nullptr;
    char * long_title_ = nullptr;
    melody_item_t * array_ = nullptr;
    std::vector<rle_sample_t *> samples = {};
    int num_rows_ = 0;

    void unload_samples();
    void try_preload_metadata();
};
