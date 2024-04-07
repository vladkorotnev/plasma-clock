#pragma once

class Renderable {
public:
    virtual void prepare() { }
    virtual void render() { }
    virtual void cleanup() { }
};