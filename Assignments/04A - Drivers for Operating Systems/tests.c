#include <graph.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "drivers/mouse.h"
#include "drivers/vga.h"

#define COLUMN 25
#define SCREEN_WIDTH 320

void update_data();
void calculate_values();
void set_pixels(uint16_t x, uint16_t y, uint8_t color, uint8_t runlength);

struct mouse_state bios, calc;
int16_t mickeys_x, mickeys_y, press_l, press_r, press_m, release_l, release_r, release_m;
int16_t sensitivity_x, sensitivity_y;
int16_t rate_x, rate_y;
uint8_t *VIDEO;

int main()
{
    bool active = true;
    bool exit_clicked = false;
    bool left_clicked = false;
    uint8_t index = 0, row = 0;

    // Set initial variable values.
    calc.x = 50;
    calc.y = 25;
    calc.left = calc.right = calc.middle = false;
    VIDEO = vga_get_address();
    rate_x = 2;
    rate_y = 4;

    // Initialize the mouse and set the video mode. Exit on error. Draw part of palette on-screen.
    if (!mouse_reset())
    {
        printf("Error: mouse not available. exiting.\n");
        return 1;
    }
    vga_set_mode(0x13);

    // Set the palette (standard 3:3:2)
    for (index = 0; index < 255; index++)
        vga_set_color(index, (index >> 5) * 9, ((index >> 2) & 0x07) * 9, (index & 0x03) * 21);
//        vga_set_color(index, (index & 0x07) * 9, ((index >> 3) & 0x07) * 9, (index >> 6) * 21);

    // Draw colors 0:79 along the top
    for (index = 0; index < 80; index++)
        for (row = 0; row < 4; row++)
            set_pixels(index * 4, row, index, 4);

    // Draw colors 80:127 along left edge
    for (index = 0; index < 48; index++)
        for (row = 0; row < 4; row++)
            set_pixels(0, 4 + index * 4 + row, index + 80, 4);

    // Draw colors 128:175 along right edge
    for (index = 0; index < 48; index++)
        for (row = 0; row < 4; row++)
            set_pixels(316, 4 + index * 4 + row, index + 128, 4);

    // Draw colors 176:255 along the top
    for (index = 0; index < 80; index++)
        for (row = 0; row < 4; row++)
            set_pixels(index * 4, 196 + row, index + 176, 4);

    // Set position, limits and movement rate, then show the cursor.
    mouse_set_position(calc.x, calc.y);
    mouse_set_limits_x(8, 383);
    mouse_set_limits_y(4, 195);
    mouse_set_movement_rate(rate_x, rate_y);
    mouse_set_sensitivity(100, 100);
    mouse_get_sensitivity(&sensitivity_x, &sensitivity_y); // Fetch this; sanity check.
    mickeys_x = mickeys_y = press_l = press_r = press_m = release_l = release_r = release_m = 0;
    mouse_show_cursor();

    while (active)
    {
        mouse_get_state(&bios);
        press_l += mouse_get_button_presses(0);
        press_r += mouse_get_button_presses(1);
        press_m += mouse_get_button_presses(2);
        release_l += mouse_get_button_releases(0);
        release_r += mouse_get_button_releases(1);
        release_m += mouse_get_button_releases(2);
        mouse_get_mickeys(&mickeys_x, &mickeys_y);

        // If both buttons are down, get ready to exit on release.
        if (bios.left && bios.right)
        {
            left_clicked = false;
            exit_clicked = true;
        }
        // If both buttons are released, and previously both were down, we're done.
        else if (!bios.left && !bios.right && exit_clicked)
            active = false;
        // Otherwise, if the left button is down, note it; when it is released, we'll process the data.
        else if (bios.left)
            left_clicked = true;
        // If the left button is up, but it was pressed before, process data and reset.
        else if (left_clicked)
        {
            left_clicked = false;
            calculate_values();
        }

        update_data();
    }

    mouse_hide_cursor();
    vga_set_mode(0x03);

    return 0;
}

void set_pixels(uint16_t x, uint16_t y, uint8_t color, uint8_t runlength)
{
    uint16_t offset;
    for (offset = x + SCREEN_WIDTH * y; runlength > 0; offset++)
    {
        VIDEO[offset] = color;
        runlength--;
    }
}

void update_data()
{
    _settextposition(2, COLUMN);
    printf("BIOS State");
    fflush(stdout);
    _settextposition(3, COLUMN);
    printf("----------");
    fflush(stdout);
    _settextposition(4, COLUMN);
    printf("BIOS-XY %3d,%3d", bios.x, bios.y);
    fflush(stdout);
    _settextposition(5, COLUMN);
    printf("BIOS-LBtn %s", bios.left == 0 ? "  Up" : "Down");
    fflush(stdout);
    _settextposition(6, COLUMN);
    printf("BIOS-RBtn %s", bios.right == 0 ? "  Up" : "Down");
    fflush(stdout);
    _settextposition(7, COLUMN);
    printf("BIOS-MBtn %s", bios.middle == 0 ? "  Up" : "Down");
    fflush(stdout);

    _settextposition(9, COLUMN);
    printf("Calc. State");
    fflush(stdout);
    _settextposition(10, COLUMN);
    printf("-----------");
    fflush(stdout);
    _settextposition(11, COLUMN);
    printf("Calc-XY %3d,%3d", calc.x, calc.y);
    fflush(stdout);
    _settextposition(12, COLUMN);
    printf("Calc-LBtn %s", calc.left == 0 ? "  Up" : "Down");
    fflush(stdout);
    _settextposition(13, COLUMN);
    printf("Calc-RBtn %s", calc.right == 0 ? "  Up" : "Down");
    fflush(stdout);
    _settextposition(14, COLUMN);
    printf("Calc-MBtn %s", calc.middle == 0 ? "  Up" : "Down");
    fflush(stdout);

    _settextposition(16, COLUMN);
    printf("Raw,Unprocessed");
    fflush(stdout);
    _settextposition(17, COLUMN);
    printf("---------------");
    fflush(stdout);
    _settextposition(18, COLUMN);
    printf("MickeysX %6d", mickeys_x);
    fflush(stdout);
    _settextposition(19, COLUMN);
    printf("MickeysY %6d", mickeys_y);
    fflush(stdout);
    _settextposition(20, COLUMN);
    printf("Pr/Rl-L %3d,%3d", press_l, release_l);
    fflush(stdout);
    _settextposition(21, COLUMN);
    printf("Pr/Rl-R %3d,%3d", press_r, release_r);
    fflush(stdout);
    _settextposition(22, COLUMN);
    printf("Pr/Rl-M %3d,%3d", press_m, release_m);
    fflush(stdout);

    _settextposition(24, COLUMN);
    printf("Sens-XY %3d,%3d", sensitivity_x, sensitivity_y);
    fflush(stdout);
}

void calculate_values()
{
    press_l -= release_l;
    press_r -= release_r;
    press_m -= release_m;
    release_l = release_r = release_m = 0;
    calc.left = (press_l >0);
    calc.right = (press_r > 0);
    calc.middle = (press_m > 0);

    mouse_calculate_pos(&calc.x, &mickeys_x, rate_x);
    mouse_calculate_pos(&calc.y, &mickeys_y, rate_y);
}
