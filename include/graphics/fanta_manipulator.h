#pragma once
#include "sprite.h"
#include "font.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Why Fanta?
// Well if a normally stored (LTR in a byte, top-to-bottom within an array) graphical thingamajiggalo is called a "Sprite",
// then why can't a weirdly oriented one prepared to be drawn on an orange display be called a "Fanta"?

/// @brief Manipulates a graphics buffer laid out by-column, ready to be drawn on the display.
class FantaManipulator {
public:
    FantaManipulator(fanta_buffer_t, size_t, int width, int height, SemaphoreHandle_t, bool* dirtyFlag);
    ~FantaManipulator();

    /// @brief Creates a new manipulator for a part of the screen space
    /// @param x Left offset inside the current manipulator
    /// @param width Width of the new manipulator's area
    /// @return Pointer to a new manipulator, owned by the caller
    FantaManipulator* slice(int x, int width);

    /// @brief Lock the buffer for exclusive editing
    bool lock(TickType_t maxDelay = portMAX_DELAY);
    /// @brief Unlock the buffer
    void unlock();

    /// @brief Clear the buffered part of the screen
    void clear();
    /// @brief Set the state of a pixel directly.
    /// @param x Horizontal coordinate of the pixel
    /// @param y Vertical coordinate of the pixel
    /// @param state True if the pixel is lit, False if the pixel is off
    void plot_pixel(int x, int y, bool state);
    /// @brief Place a piece of graphics contained in another Fanta buffer into this buffer
    /// @param x X position of the graphic to be placed
    /// @param y Y position of the graphic to be placed
    /// @param w Width of the graphic to be placed
    /// @param h Height of the graphic to be placed
    /// @param mask Mask for the graphic (can be null)
    void put_fanta(const fanta_buffer_t, int x, int y, int w, int h, const fanta_buffer_t mask = nullptr, bool invert = false);
    /// @brief Place a sprite at a specified position of the Fanta buffer
    void put_sprite(const sprite_t*, int x, int y, bool invert = false);
    /// @brief Place a glyph from a font at a specified position of the Fanta buffer
    /// @param font Font to look up the glyph in
    /// @param glyph Glyph code to place
    /// @param x X position for the glyph
    /// @param y Y position for the glyph
    void put_glyph(const font_definition_t * font, const unsigned char glyph, int x, int y, bool invert = false);
    /// @brief Draw a string with the specified font at the specified position in the Fanta buffer. Does not do any line wrapping.
    void put_string(const font_definition_t *, const char *, int x, int y, bool invert = false);
    /// @brief Offset the contents of the buffer
    /// @param dx Horizontal offset. Negative is to the left.
    /// @param dy Vertical offset. Negative is to the top.
    void scroll(int dx, int dy);
    /// @brief Invert all pixels of the buffer
    void invert();
    /// @brief Draws a line using Bresenham's algorithm
    void line(int x1, int y1, int x2, int y2);
    /// @brief Draws a rectangle
    void rect(int x1, int y1, int x2, int y2, bool fill);

    /// @brief Get the width of the buffer in pixels
    int get_width();
    /// @brief Get the height of the buffer in pixels
    int get_height();
    /// @brief Get the raw pointer to the backing data buffer. Only use this if you know what you're doing.
    const fanta_buffer_t get_fanta();
private:
    fanta_buffer_t buffer;
    size_t buffer_size;
    SemaphoreHandle_t buffer_semaphore;
    bool * dirty;
    int width;
    int height;
    void put_horizontal_data(const uint8_t* data, int x, int y, int width, int height, const uint8_t* mask, bool invert);
};