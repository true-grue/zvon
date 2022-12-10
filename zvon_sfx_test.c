/* Author: Peter Sovietov */

#include <stdio.h>
#include <stdlib.h>
#include "zvon_sfx.h"

#define CHANNELS 3

FILE *fp;
struct chan_state channels[CHANNELS];

void zv_note_on(int chan, double freq) {
    struct sfx_box *box = &channels[chan].stack[0];
    sfx_box_change(box, ZV_NOTE_ON, 0, freq);
}

void zv_note_off(int chan) {
    struct sfx_box *box = &channels[chan].stack[0];
    sfx_box_change(box, ZV_NOTE_OFF, 0, 0);
}

void play(int num_samples) {
    float *samples = calloc(num_samples * 2, sizeof(float));
    mix_process(channels, CHANNELS, 1, samples, num_samples);
    fwrite(samples, num_samples * 2, sizeof(float), fp);
    free(samples);
}

void song(void) {
    double notes[][CHANNELS] = {
        {78, 0, 87 * 1},
        {78, 0, 87 * 2},
        {0, 78, 87 * 1},
        {0, 0, 87 * 2},
        {78, 0, 87 * 1},
        {78, 0, 87 * 2},
        {0, 78, 87 * 1},
        {0, 0, 87 * 2},
        {78, 0, 87 * 1},
        {78, 0, 87 * 2},
        {0, 78, 87 * 1},
        {0, 0, 87 * 2},
        {78, 0, 87 * 1},
        {78, 0, 87 * 2},
        {0, 78, 87 * 1},
        {0, 0, 87 * 2}

    };
    for (int i = 0; i < (int) (sizeof(notes) / sizeof(notes[0])); i++) {
        for (int j = 0; j < CHANNELS; j++) {
            if (notes[i][j]) {
                zv_note_on(j, notes[i][j]);
            }
        }
        play(sec(0.2));
    }
}

void bass_drum_preset(struct sfx_box *box) {
    sfx_box_change(box, ZV_MODE, 0, OSC_SQUARE);
    sfx_box_change(box, ZV_WIDTH, 0, 0);
    sfx_box_change(box, ZV_DECAY, 0, 0.2);
    sfx_box_change(box, ZV_SUSTAIN, 0, 0);
    sfx_box_change(box, ZV_RELEASE, 0, 0);
    sfx_box_change(box, ZV_LFO_ASSIGN, 0, LFO_TARGET_FREQ);
    sfx_box_change(box, ZV_LFO_FUNC, 0, LFO_SAW);
    sfx_box_change(box, ZV_LFO_FREQ, 0, 25);
    sfx_box_change(box, ZV_LFO_LOW, 0, 100);
    sfx_box_change(box, ZV_LFO_HIGH, 0, -50);
    sfx_box_change(box, ZV_LFO_LOOP, 0, 0);
    sfx_box_change(box, ZV_LFO_ASSIGN, 1, LFO_TARGET_WIDTH);
    sfx_box_change(box, ZV_LFO_FUNC, 1, LFO_SAW);
    sfx_box_change(box, ZV_LFO_FREQ, 1, 28);
    sfx_box_change(box, ZV_LFO_LOW, 1, 0.8);
    sfx_box_change(box, ZV_LFO_HIGH, 1, 0);
    sfx_box_change(box, ZV_LFO_LOOP, 1, 0);
}

void snare_drum_preset(struct sfx_box *box) {
    sfx_box_change(box, ZV_MODE, 0, OSC_SNARE);
    sfx_box_change(box, ZV_AMP, 0, 10000);
    sfx_box_change(box, ZV_WIDTH, 0, 10000);
    sfx_box_change(box, ZV_OFFSET, 0, 0);
    sfx_box_change(box, ZV_DECAY, 0, 0.13);
    sfx_box_change(box, ZV_SUSTAIN, 0, 0);
    sfx_box_change(box, ZV_RELEASE, 0, 0);
    sfx_box_change(box, ZV_LFO_ASSIGN, 0, LFO_TARGET_FREQ);
    sfx_box_change(box, ZV_LFO_FUNC, 0, LFO_SAW);
    sfx_box_change(box, ZV_LFO_FREQ, 0, 5);
    sfx_box_change(box, ZV_LFO_LOW, 0, 120);
    sfx_box_change(box, ZV_LFO_HIGH, 0, 0);
    sfx_box_change(box, ZV_LFO_LOOP, 0, 0);
    sfx_box_change(box, ZV_LFO_ASSIGN, 1, LFO_TARGET_OFFSET);
    sfx_box_change(box, ZV_LFO_FUNC, 1, LFO_SAW);
    sfx_box_change(box, ZV_LFO_FREQ, 1, 5);
    sfx_box_change(box, ZV_LFO_LOW, 1, 10000);
    sfx_box_change(box, ZV_LFO_HIGH, 1, 5000);
    sfx_box_change(box, ZV_LFO_LOOP, 1, 0);
}

void bass_synth_preset(struct sfx_box *box) {
    sfx_box_change(box, ZV_MODE, 0, OSC_SAW);
    sfx_box_change(box, ZV_WIDTH, 0, 0);
    sfx_box_change(box, ZV_DECAY, 0, 0.05);
    sfx_box_change(box, ZV_SUSTAIN, 0, 0.3);
    sfx_box_change(box, ZV_LFO_ASSIGN, 0, LFO_TARGET_WIDTH);
    sfx_box_change(box, ZV_LFO_FUNC, 0, LFO_SAW);
    sfx_box_change(box, ZV_LFO_FREQ, 0, 5);
    sfx_box_change(box, ZV_LFO_LOW, 0, 0.8);
    sfx_box_change(box, ZV_LFO_HIGH, 0, 0.3);
    sfx_box_change(box, ZV_LFO_LOOP, 0, 0);
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    fp = fopen("sfx_test.raw", "wb");
    mix_init(channels, CHANNELS);
    for (int i = 0; i < CHANNELS; i++) {
        chan_set_on(&channels[i], 1);
        chan_set_vol(&channels[i], 1);
    }
    chan_set_on(&channels[0], 1);
    chan_set_pan(&channels[0], -0.5);
    chan_set_on(&channels[1], 1);
    chan_set_pan(&channels[1], 0.5);
    chan_set_on(&channels[2], 1);
    chan_set_pan(&channels[2], 0);
    struct sfx_box *box1 = chan_push(&channels[0], &sfx_synth);
    bass_drum_preset(box1);
    sfx_box_change(box1, ZV_VOLUME, 0, 0.4);
    struct sfx_box *box2 = chan_push(&channels[1], &sfx_synth);
    snare_drum_preset(box2);
    sfx_box_change(box2, ZV_VOLUME, 0, 0.4);
    struct sfx_box *box3 = chan_push(&channels[2], &sfx_synth);
    bass_synth_preset(box3);
    sfx_box_change(box3, ZV_VOLUME, 0, 0.15);
    song();
    for (int i = 0; i < CHANNELS; i++) {
        chan_drop(&channels[i]);
    }
    fclose(fp);
    return 0;
}
