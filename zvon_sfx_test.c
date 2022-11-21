/* Author: Peter Sovietov */

#include <stdio.h>
#include <stdlib.h>
#include "zvon_sfx.h"

void gen_pcm(struct sfx_proto *test_box_proto, double freq, int num_samples) {
    struct chan_state channels[1];
    mix_init(channels, 1);
    chan_set(&channels[0], 1, 1, 0);
    chan_push(&channels[0], test_box_proto);
    struct sfx_box *box;
    box = &channels[0].stack[0];
    box->proto->change(box->state, ZV_NOTE_ON, freq, 1);
    float *samples = calloc(num_samples * 2, sizeof(float));
    mix_process(channels, 1, 1, samples, num_samples);
    FILE *fp = fopen("sfx_test.pcm", "wb");
    fwrite(samples, num_samples * 2, sizeof(float), fp);
    fclose(fp);
    chan_free(&channels[0]);
    free(samples);
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    gen_pcm(&test_square_proto, 440, sec(1));
    return 0;
}
