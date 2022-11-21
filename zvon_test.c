/* Author: Peter Sovietov */

#include <math.h>
#include <stdio.h>
#include <assert.h>
#include "zvon.h"

int feq(double x, double y) {
    return fabs(x - y) < 1e-10;
}

void test_basic_funcs(void) {
    assert(feq(sec(0.5), 22050));
    assert(feq(midi_freq(69), 440));
    assert(feq(limit(0.5, 0, 1), 0.5));
    assert(feq(limit(10, 0, 1), 1));
    assert(feq(limit(-10, 0, 1), 0));
    assert(feq(lerp(1, 10, 0), 1));
    assert(feq(lerp(1, 10, 1), 10));
    assert(feq(lerp(1, 10, 0.5), 5.5));
    assert(feq(hertz(sec(1), 100), 628.3185307179587));
    assert(feq(dsf(hertz(sec(0.001), 200), 2, 0.5), 0.6933356074226505));
    assert(feq(dsf2(hertz(sec(0.001), 200), 2, 0.5), 0.3466678037113253));
    assert(feq(pwm(100, 0.5, 0.8), -1.1859547865063869));
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
    int deltas[] = {3, 5, 10};
    double levels[] = {1, 0.5, 0};
    env_init(&e, 3, deltas, levels, 0);
    for (int i = 0; i < 16; i++) {
        assert(feq(env_next(&e), correct[i]));
    }
}

void test_delay(void) {
    double correct[] = {
        42.0, 42.0, 63.0, 63.0, 73.5,
        73.5, 78.75, 78.75, 81.375, 81.375
    };
    struct delay_state d;
    double delay_buf[2] = {0};
    delay_init(&d, delay_buf, 2, 0.5, 0.5);
    for (int i = 0; i < 10; i++) {
        assert(feq(delay_next(&d, 42), correct[i]));
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
        assert(feq(filter_hp_next(&f, 42, 0.5), correct[i]));
    }
}

void test_glide(void) {
    double correct[] = {
        245.3514749229025, 280.4191686841491, 307.53500444964817, 328.50212462206355, 344.71479577579066,
        357.2511242642735, 366.9447478800845, 374.4402709027411, 380.23612884089505, 384.7177332783338
    };
    struct glide_state g;
    glide_init(&g, 200, 10000);
    for (int i = 0; i < 10; i++) {
        assert(feq(glide_next(&g, 400), correct[i]));
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

void test_synth_change(struct test_synth_state *s, int param, double val1, double val2) {
    (void) val2;
    if (param == 0) {
        s->freq = val1;
    }
}

double test_synth_mono(struct test_synth_state *s, double l) {
    (void) l;
    return square(phasor_next(&s->p, s->freq), 0.5);
}

struct sfx_proto test_synth_proto = {
    .name = "test_synth",
    .init = (sfx_init_func) test_synth_init,
    .change = (sfx_change_func) test_synth_change,
    .mono = (sfx_mono_func) test_synth_mono,
    .state_size = sizeof(struct test_synth_state)
};

void test_mix(void) {
    float correct[] = {
        0, 0, 36445, 52626, 66688, 87663, 87663, 103106,
        99749, 105989, 105081, 102887, 105989, 97593, 104300, 91904,
        101247, 86582, 97593, 81899, 93784, 77921, 90070, 74629,
        86582, 71974, 83382, 69902, 80495, 68369, 77921, 67337
    };
    struct chan_state channels[2];
    mix_init(channels, 2);
    chan_set(&channels[0], 1, 1, -1);
    chan_set(&channels[1], 1, 1, 1);
    chan_push(&channels[0], &test_synth_proto);
    chan_push(&channels[1], &test_synth_proto);
    struct sfx_box *box;
    box = &channels[0].stack[0];
    box->proto->change(box->state, 0, 440, 1);
    box = &channels[1].stack[0];
    box->proto->change(box->state, 0, 440 * 1.5, 1);
    float samples[16 * 2] = {0};
    mix_process(channels, 2, 1, samples, 16);
    chan_free(&channels[0]);
    chan_free(&channels[1]);
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
