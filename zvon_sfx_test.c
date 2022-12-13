/* Author: Peter Sovietov */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "zvon_sfx.h"

#define CHANNELS 4

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

void play(int num_samples, double vol) {
    float *samples = calloc(num_samples * 2, sizeof(float));
    mix_process(channels, CHANNELS, vol, samples, num_samples);
    double s_min = samples[0];
    double s_max = samples[0];
    for (int i = 0; i < num_samples * 2; i++) {
        s_min = MIN(s_min, samples[i]);
        s_max = MAX(s_max, samples[i]);
    }
    printf("%f %f\n", s_min, s_max);
    fwrite(samples, num_samples * 2, sizeof(float), fp);
    free(samples);
}

double song[][CHANNELS] = {
    {78, 0, 87 * 1, 87 * 2},
    {78, 0, 87 * 2, 0},
    {0, 78, 87 * 1, 0},
    {0, 0, 87 * 2, 0},
    {78, 0, 87 * 1, 0},
    {78, 0, 87 * 2, 0},
    {0, 78, 87 * 1, 0},
    {0, 0, 87 * 2, 0},
    {78, 0, 87 * 1, 87 * 3},
    {78, 0, 87 * 2, 0},
    {0, 78, 87 * 1, 0},
    {0, 0, 87 * 2, 0},
    {78, 0, 87 * 1, 0},
    {78, 0, 87 * 2, 0},
    {0, 78, 87 * 1, 0},
    {0, 0, 87 * 2, 0},
    {78, 0, 87 * 2, 233},
    {78, 0, 87 * 3, 0},
    {0, 78, 87 * 2, 0},
    {0, 0, 87 * 3, 0},
    {78, 0, 87 * 2, 0},
    {78, 0, 87 * 3, 0},
    {0, 78, 87 * 2, 0},
    {0, 0, 87 * 3, 0},
    {78, 0, 87 * 2, 233 * 2},
    {78, 0, 87 * 3, 0},
    {0, 78, 87 * 2, 0},
    {0, 0, 87 * 3, 0},
    {78, 0, 87 * 2, 0},
    {78, 0, 87 * 3, 0},
    {0, 78, 87 * 2, 0},
    {0, 0, 87 * 3, 0}
};

void play_song(double song[][CHANNELS], int song_size, int num_chans, double vol) {
    for (int i = 0; i < song_size; i++) {
        for (int j = 0; j < num_chans; j++) {
            if (song[i][j]) {
                zv_note_on(j, song[i][j]);
            }
        }
        play(sec(0.2), vol);
    }
}

void bass_drum_preset(struct sfx_box *box) {
    sfx_box_change(box, ZV_MODE, 0, OSC_SAW);
    sfx_box_change(box, ZV_WIDTH, 0, 0);
    sfx_box_change(box, ZV_DECAY, 0, 0.3);
    sfx_box_change(box, ZV_SUSTAIN, 0, 0);
    sfx_box_change(box, ZV_RELEASE, 0, 0);
    sfx_box_change(box, ZV_LFO_ASSIGN, 0, LFO_TARGET_FREQ);
    sfx_box_change(box, ZV_LFO_FUNC, 0, LFO_SAW);
    sfx_box_change(box, ZV_LFO_FREQ, 0, 25);
    sfx_box_change(box, ZV_LFO_LOW, 0, 150);
    sfx_box_change(box, ZV_LFO_HIGH, 0, -50);
    sfx_box_change(box, ZV_LFO_SET_LOOP, 0, 0);
    sfx_box_change(box, ZV_LFO_ASSIGN, 1, LFO_TARGET_WIDTH);
    sfx_box_change(box, ZV_LFO_FUNC, 1, LFO_SAW);
    sfx_box_change(box, ZV_LFO_FREQ, 1, 20);
    sfx_box_change(box, ZV_LFO_LOW, 1, 0.65);
    sfx_box_change(box, ZV_LFO_HIGH, 1, 0.1);
    sfx_box_change(box, ZV_LFO_SET_LOOP, 1, 0);
}

void snare_drum_preset(struct sfx_box *box) {
    sfx_box_change(box, ZV_MODE, 0, OSC_SIN_RESO_NOISE);
    sfx_box_change(box, ZV_AMP, 0, 10000);
    sfx_box_change(box, ZV_WIDTH, 0, 10000);
    sfx_box_change(box, ZV_OFFSET, 0, 0);
    sfx_box_change(box, ZV_DECAY, 0, 0.15);
    sfx_box_change(box, ZV_SUSTAIN, 0, 0);
    sfx_box_change(box, ZV_RELEASE, 0, 0);
    sfx_box_change(box, ZV_LFO_ASSIGN, 0, LFO_TARGET_FREQ);
    sfx_box_change(box, ZV_LFO_FUNC, 0, LFO_SAW);
    sfx_box_change(box, ZV_LFO_FREQ, 0, 10);
    sfx_box_change(box, ZV_LFO_LOW, 0, 150);
    sfx_box_change(box, ZV_LFO_HIGH, 0, 0);
    sfx_box_change(box, ZV_LFO_SET_LOOP, 0, 0);
    sfx_box_change(box, ZV_LFO_ASSIGN, 1, LFO_TARGET_OFFSET);
    sfx_box_change(box, ZV_LFO_FUNC, 1, LFO_SAW);
    sfx_box_change(box, ZV_LFO_FREQ, 1, 5);
    sfx_box_change(box, ZV_LFO_LOW, 1, 10000);
    sfx_box_change(box, ZV_LFO_HIGH, 1, 5000);
    sfx_box_change(box, ZV_LFO_SET_LOOP, 1, 0);
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
    sfx_box_change(box, ZV_LFO_SET_LOOP, 0, 1);
}

double semi(double x) {
    return pow(2, x / 12.) - 1;
}

void lead_synth_preset(struct sfx_box *box) {
    sfx_box_change(box, ZV_MODE, 0, OSC_PWM);
    sfx_box_change(box, ZV_WIDTH, 0, 0);
    sfx_box_change(box, ZV_OFFSET, 0, 1.7);
    sfx_box_change(box, ZV_DECAY, 0, 0.5);
    sfx_box_change(box, ZV_SUSTAIN, 0, 1);
    sfx_box_change(box, ZV_RELEASE, 0, 2.5);
    sfx_box_change(box, ZV_FREQ_MUL, 0, 2);
    sfx_box_change(box, ZV_LFO_ASSIGN, 0, LFO_TARGET_FREQ_MUL);
    sfx_box_change(box, ZV_LFO_FUNC, 0, LFO_SEQ);
    sfx_box_change(box, ZV_LFO_FREQ, 0, 10);
    sfx_box_change(box, ZV_LFO_LOW, 0, 0);
    sfx_box_change(box, ZV_LFO_HIGH, 0, 1);
    sfx_box_change(box, ZV_LFO_SET_LOOP, 0, 1);
    sfx_box_change(box, ZV_LFO_SEQ_POS, 0, 0);
    sfx_box_change(box, ZV_LFO_SEQ_VAL, 0, semi(0));
    sfx_box_change(box, ZV_LFO_SEQ_POS, 0, 1);
    sfx_box_change(box, ZV_LFO_SEQ_VAL, 0, semi(7));
    sfx_box_change(box, ZV_LFO_SEQ_POS, 0, 2);
    sfx_box_change(box, ZV_LFO_SEQ_VAL, 0, semi(12));
    sfx_box_change(box, ZV_LFO_SEQ_SIZE, 0, 3);
    sfx_box_change(box, ZV_LFO_SET_LIN_SEQ, 0, 0);
    sfx_box_change(box, ZV_LFO_ASSIGN, 1, LFO_TARGET_WIDTH);
    sfx_box_change(box, ZV_LFO_FUNC, 1, LFO_SAW);
    sfx_box_change(box, ZV_LFO_FREQ, 1, 10);
    sfx_box_change(box, ZV_LFO_LOW, 1, 0.75);
    sfx_box_change(box, ZV_LFO_HIGH, 1, 0);
    sfx_box_change(box, ZV_LFO_SET_LOOP, 1, 1);
    sfx_box_change(box, ZV_LFO_SET_LIN_SEQ, 1, 0);
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    fp = fopen("sfx_test.raw", "wb");
    mix_init(channels, CHANNELS);
    for (int i = 0; i < CHANNELS; i++) {
        chan_set_vol(&channels[i], 1);
    }
    chan_set_on(&channels[0], 1);
    chan_set_on(&channels[1], 1);
    chan_set_on(&channels[2], 1);
    chan_set_on(&channels[3], 1);
    chan_set_pan(&channels[0], -0.5);
    chan_set_pan(&channels[1], 0.5);
    chan_set_pan(&channels[2], 0);
    chan_set_pan(&channels[3], 0);
    struct sfx_box *box1 = chan_push(&channels[0], &sfx_synth);
    bass_drum_preset(box1);
    sfx_box_change(box1, ZV_VOLUME, 0, 0.25);
    struct sfx_box *box2 = chan_push(&channels[1], &sfx_synth);
    snare_drum_preset(box2);
    sfx_box_change(box2, ZV_VOLUME, 0, 0.25);
    struct sfx_box *box3 = chan_push(&channels[2], &sfx_synth);
    bass_synth_preset(box3);
    sfx_box_change(box3, ZV_VOLUME, 0, 0.2);
    struct sfx_box *box4 = chan_push(&channels[3], &sfx_synth);
    lead_synth_preset(box4);
    struct sfx_box *box5 = chan_push(&channels[3], &sfx_delay);
    sfx_box_change(box5, ZV_TIME, 0, 0.2);
    sfx_box_change(box4, ZV_VOLUME, 0, 0.1);
    play_song(song, (sizeof(song) / sizeof(song[0])) / 2, 2, 0.8);
    play_song(song, (sizeof(song) / sizeof(song[0])) / 2, 3, 0.8);
    play_song(song, (sizeof(song) / sizeof(song[0])) / 2, 3, 0.8);
    play_song(song, (sizeof(song) / sizeof(song[0])), 4, 0.8);
    play_song(song, (sizeof(song) / sizeof(song[0])), 4, 0.8);
    for (int i = 0; i < CHANNELS; i++) {
        chan_drop(&channels[i]);
    }
    fclose(fp);
    return 0;
}
