/* Author: Peter Sovietov */

#include <math.h>
#include "zvon.h"
#include "zvon_boxes.h"

struct test_square_state {
    double freq;
    double vol;
    struct phasor_state ph1;
    struct phasor_state ph2;
    struct env_state env;
};

void test_square_init(struct test_square_state *s) {
    s->freq = 0;
    s->vol = 0;
    phasor_init(&s->ph1);
    phasor_init(&s->ph2);
    static int deltas[] = {SEC(0.01), SEC(0.5)};
    static double levels[] = {1, 0};
    env_init(&s->env, deltas, 0, levels, 2);
}

void test_square_change(struct test_square_state *s, int param, double val1, double val2) {
    if (param == ZV_NOTE_ON) {
        s->freq = val1;
        s->vol = val2;
        env_reset(&s->env);
    }
}

double test_square_next(struct test_square_state *s, double l) {
    (void) l;
    double a = square(phasor_next(&s->ph1, s->freq) + 2 * sin(phasor_next(&s->ph2, 4)), 0.5);
    return s->vol * a * env_next(&s->env);
}

struct box_proto test_square_proto = {
    .name = "test_square",
    .state_size = sizeof(struct test_square_state),
    .init = (box_init_func) test_square_init,
    .change = (box_change_func) test_square_change,
    .next = (box_next_func) test_square_next
};
