/* Author: Peter Sovietov */

#include <stdio.h>
#include <stdlib.h>
#include "zvon.h"
#include "zvon_boxes.h"

void pcm_test(struct box_proto *test_box_proto, double freq, int num_samples) {
    struct chan_state channels[1];
    mix_init(channels, 1);
    chan_set(&channels[0], 1, 1, 0);
    chan_push(&channels[0], test_box_proto);
    struct box_state *box;
    box = &channels[0].stack[0];
    box->proto->change(box->state, ZV_NOTE_ON, freq, 1);
    float *samples = calloc(num_samples * 2, sizeof(float));
    mix_process(channels, 1, 1, samples, num_samples);
    FILE *fp = fopen("test.pcm", "wb");
    fwrite(samples, num_samples * 2, sizeof(float), fp);
    fclose(fp);
    chan_free(&channels[0]);
    free(samples);
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    pcm_test(&test_square_proto, 440, SEC(1));
    return 0;
}
