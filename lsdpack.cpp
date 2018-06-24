#include <cstdio>
#include <cstdlib>

#include "gambatte.h"

#include "input.h"
#include "writer.h"

int written_songs;

gambatte::GB gameboy;
Input input;

void run_one_frame() {
    size_t samples = 35112;
    long unsigned int audioBuffer[35112 + 2064];
    gameboy.runFor(0, 0, &audioBuffer[0], samples);
}

void wait(int seconds) {
    for (int i = 0; i < 60 * seconds; ++i) {
        run_one_frame();
    }
}

void press(unsigned key, int seconds = 1) {
    input.press(key);
    wait(seconds);
}

void load_song(int position) {
    press(SELECT);
    press(SELECT | UP);
    press(0);
    press(DOWN, 3);
    press(0);
    press(A);
    press(0);
    press(A);
    press(0);
    press(UP, 5); // scroll to top
    press(0);
    for (int i = 0; i < position; ++i) {
        input.press(DOWN);
        run_one_frame();
        run_one_frame();
        press(0);
    }
    press(A, 10); // wait for song load
    press(0);
    if (gameboy.isSongEmpty()) {
        puts("ok");
        exit(0);
    }
    printf("Recording song %i...\n", ++written_songs);
}

bool sound_enabled;

void play_song() {
    sound_enabled = false;
    input.press(START);
    record_song_start();
    do {
        wait(1);
    } while(sound_enabled);
}

void on_ff_write(char p, char data) {
    if (p < 0x10 || p >= 0x40) {
        return; // not sound
    }
    switch (p) {
        case 0x26:
            if (sound_enabled && !data) {
                record_song_stop();
                sound_enabled = false;
                return;
            }
            sound_enabled = data;
            break;
    }
    if (sound_enabled) {
        record_write(p, data);
    }
}

void on_lcd_interrupt() {
    if (sound_enabled) {
        record_lcd();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: lsdpack <lsdj.gb>");
        return 1;
    }
    gameboy.setInputGetter(&input);
    gameboy.setWriteHandler(on_ff_write);
    gameboy.setLcdHandler(on_lcd_interrupt);
    gameboy.load(argv[1]);

    press(0, 3);

    int i = 0;
    while (true) {
        load_song(i++);
        play_song();
    }
}
