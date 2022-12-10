/* Author: Peter Sovietov */

#include <stdio.h>
#include <stdlib.h>
#include "zvon_sfx.h"

#define CHANNELS 2

FILE *fp;
struct chan_state channels[CHANNELS];

void zv_note_on(int chan, double freq) {
    struct sfx_box *box = &channels[chan].stack[0];
    box->proto->change(box->state, ZV_NOTE_ON, 0, freq);
}

void zv_note_off(int chan) {
    struct sfx_box *box = &channels[chan].stack[0];
    box->proto->change(box->state, ZV_NOTE_OFF, 0, 0);
}

void play(int num_samples) {
    float *samples = calloc(num_samples * 2, sizeof(float));
    mix_process(channels, CHANNELS, 1, samples, num_samples);
    fwrite(samples, num_samples * 2, sizeof(float), fp);
    free(samples);
}

void song(void) {
    double notes[][CHANNELS] = {
        {78, 0},
        {78, 0},
        {0, 78},
        {0, 0},
        {78, 0},
        {78, 0},
        {0, 78},
        {0, 0}
    };
    for (int i = 0; i < (int) (sizeof(notes) / sizeof(notes[0])); i++) {
        if (notes[i][0]) {
            zv_note_on(0, notes[i][0]);
        }
        if (notes[i][1]) {
            zv_note_on(1, notes[i][1]);
        }
        play(sec(0.25));
    }
}

void bass_drum_preset(struct sfx_box *box) {
    box->proto->change(box->state, ZV_MODE, 0, OSC_SQUARE);
    box->proto->change(box->state, ZV_WIDTH, 0, 0);
    box->proto->change(box->state, ZV_DECAY, 0, 0.5);
    box->proto->change(box->state, ZV_SUSTAIN, 0, 0);
    box->proto->change(box->state, ZV_RELEASE, 0, 0);
    box->proto->change(box->state, ZV_LFO_ASSIGN, 0, LFO_TARGET_FREQ);
    box->proto->change(box->state, ZV_LFO_FUNC, 0, LFO_SAW);
    box->proto->change(box->state, ZV_LFO_FREQ, 0, 15);
    box->proto->change(box->state, ZV_LFO_LOW, 0, 120);
    box->proto->change(box->state, ZV_LFO_HIGH, 0, -120);
    box->proto->change(box->state, ZV_LFO_LOOP, 0, 0);
    box->proto->change(box->state, ZV_LFO_ASSIGN, 1, LFO_TARGET_WIDTH);
    box->proto->change(box->state, ZV_LFO_FUNC, 1, LFO_SAW);
    box->proto->change(box->state, ZV_LFO_FREQ, 1, 15);
    box->proto->change(box->state, ZV_LFO_LOW, 1, 0.65);
    box->proto->change(box->state, ZV_LFO_HIGH, 1, 0);
    box->proto->change(box->state, ZV_LFO_LOOP, 1, 0);
}

void snare_drum_preset(struct sfx_box *box) {
    box->proto->change(box->state, ZV_MODE, 0, OSC_NOISE);
    box->proto->change(box->state, ZV_AMP, 0, 10000);
    box->proto->change(box->state, ZV_WIDTH, 0, 10000);
    box->proto->change(box->state, ZV_OFFSET, 0, 0);
    box->proto->change(box->state, ZV_DECAY, 0, 0.13);
    box->proto->change(box->state, ZV_SUSTAIN, 0, 0);
    box->proto->change(box->state, ZV_RELEASE, 0, 0);
    box->proto->change(box->state, ZV_LFO_ASSIGN, 0, LFO_TARGET_FREQ);
    box->proto->change(box->state, ZV_LFO_FUNC, 0, LFO_SAW);
    box->proto->change(box->state, ZV_LFO_FREQ, 0, 5);
    box->proto->change(box->state, ZV_LFO_LOW, 0, 120);
    box->proto->change(box->state, ZV_LFO_HIGH, 0, 0);
    box->proto->change(box->state, ZV_LFO_LOOP, 0, 0);
    box->proto->change(box->state, ZV_LFO_ASSIGN, 1, LFO_TARGET_OFFSET);
    box->proto->change(box->state, ZV_LFO_FUNC, 1, LFO_SAW);
    box->proto->change(box->state, ZV_LFO_FREQ, 1, 8);
    box->proto->change(box->state, ZV_LFO_LOW, 1, 8000);
    box->proto->change(box->state, ZV_LFO_HIGH, 1, 5000);
    box->proto->change(box->state, ZV_LFO_LOOP, 1, 0);
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    fp = fopen("sfx_test.raw", "wb");
    mix_init(channels, CHANNELS);
    chan_set_on(&channels[0], 1);
    chan_set_on(&channels[1], 1);
    chan_set_vol(&channels[0], 1);
    chan_set_vol(&channels[1], 1);
    struct sfx_box *box1 = chan_push(&channels[0], &sfx_synth);
    bass_drum_preset(box1);
    sfx_box_set_vol(box1, 0.8);
    struct sfx_box *box2 = chan_push(&channels[1], &sfx_synth);
    snare_drum_preset(box2);
    struct sfx_box *box3 = chan_push(&channels[1], &sfx_dist);
    box3->proto->change(box3->state, ZV_GAIN, 0, 0.2);
    song();
    chan_drop(&channels[0]);
    chan_drop(&channels[1]);
    fclose(fp);
    return 0;
}
