/* Author: Peter Sovietov */

#include <stdio.h>
#include <assert.h>
#include "zvon.h"

void test_basic_funcs(void) {
    assert(midi_freq(69) == 440);
    assert(sec(0.5) == 22050);
    assert(limit(0.5, 0, 1) == 0.5);
    assert(limit(10, 0, 1) == 1);
    assert(limit(-10, 0, 1) == 0);
    assert(lerp(1, 10, 0) == 1);
    assert(lerp(1, 10, 1) == 10);
    assert(lerp(1, 10, 0.5) == 5.5);
    assert(hz(sec(1), 100) == 628.3185307179587);
    assert(dsf(hz(sec(0.001), 200), 2, 0.5) == 0.6933356074226505);
    assert(dsf2(hz(sec(0.001), 200), 2, 0.5) == 0.3466678037113253);
    assert(pwm(100, 0.5, 0.8) == -1.1859547865063869);
    unsigned int state = 1;
    for (int i = 0; i < 16; i++) {
      state = lfsr(state, 7, (int[]) {0, 3, 4}, 3);
    }
    assert(state == 90);
}

void test_phasor(void) {
    struct phasor_state p;
    phasor_init(&p);
    for (int i = 0; i < 16; i++) {    
        phasor_next(&p, 440);
    }
    assert(phasor_next(&p, 440) == 1.0030300354318433);
}

void test_env(void) {
    double correct[] = {
        0.0, 0.3333333333333333, 0.6666666666666666, 0.6666666666666666,
        1.0, 0.9, 0.8, 0.7,
        0.6, 0.6, 0.5, 0.45,
        0.4, 0.35, 0.3, 0.25
    };
    struct env_state e;
    double deltas[] = {3, 5, 10};
    double levels[] = {1, 0.5, 0};
    env_init(&e, deltas, 0, levels, 3);
    for (int i = 0; i < 16; i++) {    
        assert(env_next(&e) == correct[i]);
    }
}

void test_delay(void) {
    double correct[] = {
        42.0, 42.0, 63.0, 63.0, 73.5,
        73.5, 78.75, 78.75, 81.375, 81.375
    };
    struct delay_state d;
    delay_init(&d, 2, 0.5, 0.5);
    for (int i = 0; i < 10; i++) {
        assert(delay_next(&d, 42) == correct[i]);
    }
}

void test_filter(void) {
    double correct[] = {
        21.0, 10.5, 5.25, 2.625, 1.3125,
        0.65625, 0.328125, 0.1640625, 0.08203125, 0.041015625
    };
    struct filter_state f;
    filter_init(&f);
    for (int i = 0; i < 10; i++) {
        assert(filter_hp_next(&f, 42, 0.5) == correct[i]);
    }
}

void test_glide(void) {
    double correct[] = {
        660.000001, 770.0000015, 825.00000175, 852.5000018750001, 866.2500019375001,
        873.1250019687501, 876.5625019843751, 878.2812519921875, 879.1406269960938, 879.570314498047
    };
    struct glide_state g;
    glide_init(&g, 440, 0.5);
    for (int i = 0; i < 10; i++) {
        assert(glide_next(&g, 880) == correct[i]);
    }
}

void test_noise(void) {
    unsigned int correct[] = {
        1, 1, -1, -1, -1,
        -1, -1, -1, 1, 1
    };
    struct noise_state n;
    noise_init(&n, 3, (int[]) {0, 1, 2}, 3);
    for (int i = 0; i < 10; i++) {
        assert(noise_next(&n, 20000) == correct[i]);
    }
}

struct test_synth_state {
    struct phasor_state p;
    double freq;
};

void test_synth_init(struct test_synth_state *s) {
    phasor_init(&s->p);
    s->freq = 0;
}

void test_synth_change(struct test_synth_state *s, int param, int elem, double val) {
    (void) param;
    (void) elem;
    s->freq = val;
}

double test_synth_next(struct test_synth_state *s, double x) {
    (void) x;
    return square(phasor_next(&s->p, s->freq), 0.5);
}

struct box_def test_box_def = {
    .change = (box_change_func) test_synth_change,
    .next = (box_next_func) test_synth_next,
    .state_size = sizeof(struct test_synth_state),
    .init = (box_init_func) test_synth_init
};

void test_mix(void) {
    double correct[] = {
        0, 0, 36445, 52626, 66688, 87663, 87663, 103106,
        99749, 105989, 105081, 102887, 105989, 97593, 104300, 91904,
        101247, 86582, 97593, 81899, 93784, 77921, 90070, 74629,
        86582, 71974, 83382, 69902, 80495, 68369, 77921, 67337
    };
    struct chan_state channels[2];
    for (int i = 0; i < 2; i++) {
        chan_init(&channels[i]);
    }
    chan_set(&channels[0], 1, 1, -1);
    chan_set(&channels[1], 1, 1, 1);
    chan_push(&channels[0], &test_box_def);
    chan_push(&channels[1], &test_box_def);
    struct box_state *box;
    box = &channels[0].stack[0];
    box->change(box->state, 0, 0, 440);
    box = &channels[1].stack[0];
    box->change(box->state, 0, 0, 440 * 1.5);
    double samples[16 * 2] = {0};
    chan_mix(channels, 2, 1, samples, 16);
    for (int i = 0; i < 16; i++) {
        assert(round(100000 * samples[i]) == correct[i]);
    }
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    test_basic_funcs();
    test_phasor();
    test_env();
    test_delay();
    test_filter();
    test_glide();
    test_noise();
    test_mix();
    return 0;
}
