/* Author: Peter Sovietov */

#include <stdio.h>
#include <stdlib.h>
#include "zvon_sfx.h"

FILE *fp;
struct chan_state channels[1];

void note_on(double freq) {
    struct sfx_box *box = &channels[0].stack[0];
    box->proto->change(box->state, ZV_NOTE_ON, freq, 0);
}

void play(int num_samples) {
    float *samples = calloc(num_samples * 2, sizeof(float));
    mix_process(channels, 1, 1, samples, num_samples);
    fwrite(samples, num_samples * 2, sizeof(float), fp);
    free(samples);
}

void song(void) {
    double notes[8] = {
        261.6255653005986,
        391.99543598174927,
        329.6275569128699,
        391.99543598174927,
        523.2511306011972,
        391.99543598174927,
        329.6275569128699,
        391.99543598174927
    };
    for (int i = 0; i < 8; i++) {
        note_on(notes[i]);
        play(sec(2./8));
    }
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    fp = fopen("sfx_test.pcm", "wb");
    mix_init(channels, 1);
    chan_set(&channels[0], 1, 1, 0);
    struct sfx_box *box = chan_push(&channels[0], &sfx_synth);    
    box->proto->change(box->state, ZV_WAVE_TYPE, ZV_SAW, 0);
    box->proto->change(box->state, ZV_WAVE_WIDTH, 0.7, 0);
    box->proto->change(box->state, ZV_VOLUME, 0.5, 0);
    box->proto->change(box->state, ZV_FREQ_LFO_WAVE_TYPE, ZV_SQUARE, 0);
    box->proto->change(box->state, ZV_FREQ_LFO_WAVE_SIGN, 1, 0);
    box->proto->change(box->state, ZV_FREQ_LFO_FREQ, 10, 0);
    box->proto->change(box->state, ZV_FREQ_LFO_LEVEL, 100, 0);
    box->proto->change(box->state, ZV_FREQ_LFO_IS_ONESHOT, 0, 0);
    song();
    chan_drop(&channels[0]);
    fclose(fp);
    return 0;
}
