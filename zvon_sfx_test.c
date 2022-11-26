/* Author: Peter Sovietov */

#include <stdio.h>
#include <stdlib.h>
#include "zvon_sfx.h"

FILE *fp;
struct chan_state channels[1];

void note_on(double freq) {
    struct sfx_box *box = &channels[0].stack[0];
    box->proto->change(box->state, ZV_NOTE_ON, freq);
}

void play(int num_samples) {
    float *samples = calloc(num_samples * 2, sizeof(float));
    mix_process(channels, 1, 1, samples, num_samples);
    fwrite(samples, num_samples * 2, sizeof(float), fp);
    free(samples);
}

void song(void) {
    double notes[] = {
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
        play(sec(0.25));
    }
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    fp = fopen("sfx_test.pcm", "wb");
    mix_init(channels, 1);
    chan_set(&channels[0], 1, 1, 0);
    struct sfx_box *b1 = chan_push(&channels[0], &sfx_synth);
    struct sfx_box *b2 = chan_push(&channels[0], &sfx_filter);
    struct sfx_box *b3 = chan_push(&channels[0], &sfx_filter);

    b1->proto->change(b1->state, ZV_WAVE_TYPE, ZV_NOISE);
    b1->proto->change(b1->state, ZV_VOLUME, 8);
    b1->proto->change(b1->state, ZV_FREQ_SCALER, 32);

    b2->proto->change(b2->state, ZV_FILTER_TYPE, ZV_FILTER_HP);
    b2->proto->change(b2->state, ZV_FILTER_WIDTH, 0.2);

    b3->proto->change(b3->state, ZV_FILTER_TYPE, ZV_FILTER_LP);
    b3->proto->change(b3->state, ZV_FILTER_WIDTH, 0.2);

    song();
    chan_drop(&channels[0]);
    fclose(fp);
    return 0;
}
