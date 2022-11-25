/* Author: Peter Sovietov */

#include <stdio.h>
#include <stdlib.h>
#include "zvon_sfx.h"

FILE *fp;
struct chan_state channels[1];

void note_on(double note) {
    struct sfx_box *box = &channels[0].stack[0];
    box->proto->change(box->state, ZV_NOTE_ON, midi_freq(note), 0);
}

void play(int num_samples) {
    float *samples = calloc(num_samples * 2, sizeof(float));
    mix_process(channels, 1, 1, samples, num_samples);
    fwrite(samples, num_samples * 2, sizeof(float), fp);
    free(samples);
}

void song(void) {
    note_on(60);
    play(sec(0.5));
    note_on(63);
    play(sec(0.5));
    note_on(65);
    play(sec(0.5));
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    fp = fopen("sfx_test.pcm", "wb");
    mix_init(channels, 1);
    chan_set(&channels[0], 1, 1, 0);
    struct sfx_box *box = chan_push(&channels[0], &sfx_synth);
    box->proto->change(box->state, ZV_WAVE_TYPE, 2, 0);
    song();
    chan_drop(&channels[0]);
    fclose(fp);
    return 0;
}
