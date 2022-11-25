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
    return 0;
}
