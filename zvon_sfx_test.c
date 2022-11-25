/* Author: Peter Sovietov */

#include <stdio.h>
#include <stdlib.h>
#include "zvon_sfx.h"

void gen_pcm(struct sfx_proto *test_box, double freq, int num_samples) {
    struct chan_state channels[1];
    mix_init(channels, 1);
    chan_set(&channels[0], 1, 1, 0);
    chan_push(&channels[0], test_box);
    struct sfx_box *box;
    box = &channels[0].stack[0];
    box->proto->change(box->state, ZV_NOTE_ON, freq, 0);
    box->proto->change(box->state, ZV_VOLUME, 1, 0);
    box->proto->change(box->state, ZV_WAVE_TYPE, 2, 0);
    float *samples = calloc(num_samples * 2, sizeof(float));
    mix_process(channels, 1, 1, samples, num_samples);
    FILE *fp = fopen("sfx_test.pcm", "wb");
    fwrite(samples, num_samples * 2, sizeof(float), fp);
    fclose(fp);
    chan_drop(&channels[0]);
    free(samples);
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    gen_pcm(&sfx_synth, 440, sec(5));
    return 0;
}
