/* Author: Peter Sovietov */

#include <math.h>
#include <stdlib.h>
#include "zvon.h"

int sec(double t) {
    return t * SR;
}

double midi_freq(int m) {
    return 440 * pow(2, (m - 69) * (1 / 12.));
}

double limit(double x, double low, double high) {
    return MIN(MAX(x, low), high);
}

double lerp(double a, double b, double x) {
    return a * (1 - x) + b * x;
}

double hertz(double t, double freq) {
    return (2 * PI / SR) * freq * t;
}

double dsf(double phase, double mod, double width) {
    double mphase = mod * phase;
    double n = sin(phase) - width * sin(phase - mphase);
    return n / (1 + width * (width - 2 * cos(mphase)));
}

double dsf2(double phase, double mod, double width) {
    double mphase = mod * phase;
    double n = sin(phase) * (1 - width * width);
    return n / (1 + width * (width - 2 * cos(mphase)));
}

double saw(double phase, double width) {
    return dsf(phase, 1, width);
}

double square(double phase, double width) {
    return dsf(phase, 2, width);
}

double pwm(double phase, double offset, double width) {
    return saw(phase, width) - saw(phase + offset, width);
}

unsigned int lfsr(unsigned int state, int bits, int *taps, int taps_size) {
    unsigned int x = 0;
    for (int i = 0; i < taps_size; i++) {
        x ^= state >> taps[i];
    }
    return (state >> 1) | ((~x & 1) << (bits - 1));
}

double softclip(double x, double drive) {
    return tanh(x * drive);
}

void phasor_init(struct phasor_state *s) {
    s->phase = 0;
}

void phasor_reset(struct phasor_state *s) {
    s->phase = 0;
}

double phasor_next(struct phasor_state *s, double freq) {
    double p = s->phase;
    s->phase = fmod(s->phase + (2 * PI / SR) * freq, SR * PI);
    return p;
}

void env_init(struct env_state *s, int env_size, int *deltas, double *levels, double level_0) {
    s->deltas = deltas;
    s->levels = levels;
    s->env_size = env_size;
    s->is_loop = 0;
    s->is_full_reset = 0;
    s->sustain_pos = -1;
    s->level_0 = level_0;
    s->level = level_0;
    env_reset(s);
}

void env_set(struct env_state *s, int pos, int delta, double level) {
    s->deltas[pos] = delta;
    s->levels[pos] = level;
}

void env_free(struct env_state *s) {
    free(s->deltas);
    free(s->levels);
}

void env_reset(struct env_state *s) {
    s->is_end = 0;
    s->t = 0;
    s->pos = 0;
    s->t_at_pos = 0;
    if (s->is_full_reset) {
        s->level = s->level_0;
    }
    s->level_at_pos = s->level;
}

static double env_next_tail(struct env_state *s, env_func func) {
    double dt = s->deltas[s->pos];
    if (s->t < s->t_at_pos + dt) {
        s->level = func(s->level_at_pos, s->levels[s->pos], (s->t - s->t_at_pos) / dt);
        s->t++;
        return s->level;
    }
    if (s->sustain_pos == s->pos) {
        return s->level;
    }
    s->t_at_pos += dt;
    s->level_at_pos = s->levels[s->pos];
    s->pos++;
    return s->level;
}

double env_next_head(struct env_state *s, env_func func) {
    if (s->pos >= s->env_size) {
        if (!s->is_loop) {
            s->is_end = 1;
            return s->level;
        }
        env_reset(s);
    }
    return env_next_tail(s, func);
}

static double step(double a, double b, double x) {
    (void) a;
    (void) x;
    return b;
}

double env_next(struct env_state *s) {
    return env_next_head(s, lerp);
}

double seq_next(struct env_state *s) {
    return env_next_head(s, step);
}

void adsr_init(struct adsr_state *s, int is_sustain_on) {
    env_init(&s->env, 3, s->deltas, s->levels, 0);
    env_set(&s->env, 0, sec(0.01), 1);
    env_set(&s->env, 1, sec(0.1), 0.5);
    env_set(&s->env, 2, sec(0.3), 0);
    s->sustain_mode = is_sustain_on ? 1 : -1;
}

void adsr_set_attack(struct adsr_state *s, double t) {
    s->env.deltas[0] = sec(t);
}

void adsr_set_decay(struct adsr_state *s, double t) {
    s->env.deltas[1] = sec(t);
}

void adsr_set_sustain(struct adsr_state *s, double level) {
    s->env.levels[1] = level;
}

void adsr_set_release(struct adsr_state *s, double t) {
    s->env.deltas[2] = sec(t);
}

void adsr_note_on(struct adsr_state *s) {
    env_reset(&s->env);
    s->env.sustain_pos = s->sustain_mode;
}

void adsr_note_off(struct adsr_state *s) {
    s->env.sustain_pos = -1;
}

double adsr_next(struct adsr_state *s) {
    return env_next(&s->env);
}

void delay_init(struct delay_state *s, double *buf, size_t buf_size, double level, double fb) {
    s->buf = buf;
    s->buf_size = buf_size;
    s->level = level;
    s->fb = fb;
    s->pos = 0;
}

double delay_next(struct delay_state *s, double x) {
    double y = x + s->buf[s->pos] * s->level;
    s->buf[s->pos] = s->buf[s->pos] * s->fb + x;
    s->pos = (s->pos + 1) % s->buf_size;
    return y;
}

void filter_init(struct filter_state *s) {
    s->y = 0;
}

double filter_lp_next(struct filter_state *s, double x, double width) {
    s->y += width * (x - s->y);
    return s->y;
}

double filter_hp_next(struct filter_state *s, double x, double width) {
    return x - filter_lp_next(s, x, width);
}

void glide_init(struct glide_state *s, double source, double rate) {
    s->source = source;
    s->rate = rate * (1. / SR);
}

double glide_next(struct glide_state *s, double target) {
    double step = s->rate * fabs(s->source - target) + 1e-6;
    if (s->source < target) {
        s->source = MIN(s->source + step, target);
    } else {
        s->source = MAX(s->source - step, target);
    }
    return s->source;
}

void noise_init(struct noise_state *s, int bits, int *taps, int taps_size) {
    s->bits = bits;
    s->taps_size = limit(taps_size, 1, MAX_TAPS);
    for (int i = 0; i < s->taps_size; i++) {
        s->taps[i] = taps[i];
    }
    s->state = 1;
    s->phase = 0;
}

double noise_next(struct noise_state *s, double freq) {
    s->phase += freq * (1. / SR);
    if (s->phase >= 1) {
        s->phase -= 1;
        s->state = lfsr(s->state, s->bits, s->taps, s->taps_size);
    }
    return 2 * (s->state & 1) - 1;
}

void mix_init(struct chan_state *channels, int num_channels) {
    for (int i = 0; i < num_channels; i++) {
        struct chan_state *c = &channels[i];
        chan_set(c, 0, 0, 0);
        c->stack_size = 0;
    }
}

void chan_set(struct chan_state *c, int is_on, double vol, double pan) {
    c->is_on = is_on;
    c->vol = vol;
    c->pan = pan;
}

void chan_drop(struct chan_state *c) {
    for (int i = 0; i < c->stack_size; i++) {
        if (c->stack[i].proto->free) {
            c->stack[i].proto->free(c->stack[i].state);
        }
        free(c->stack[i].state);
    }
    c->stack_size = 0;
}

struct sfx_box *chan_push(struct chan_state *c, struct sfx_proto *proto) {
    if (c->stack_size < MAX_SFX_BOXES) {
        struct sfx_box *box = &c->stack[c->stack_size];
        box->proto = proto;
        box->state = calloc(1, proto->state_size);
        if (box->state || !box->proto->state_size) {
            proto->init(box->state);
            c->stack_size++;
            return box;
        }
    }
    return NULL;
}

static void chan_process(struct sfx_box *stack, int stack_size, double *l, double *r) {
    for (int i = 0; i < stack_size; i++) {
        if (stack[i].proto->stereo) {
            stack[i].proto->stereo(stack[i].state, l, r);
        } else {
            *l = stack[i].proto->mono(stack[i].state, *l);
            *r = *l;
        }
    }
}

void mix_process(struct chan_state *channels, int num_channels, double vol, float *samples, int num_samples) {
    for (; num_samples; num_samples--, samples += 2) {
        double left = 0, right = 0;
        for (int i = 0; i < num_channels; i++) {
            struct chan_state *c = &channels[i];
            double l = 0, r = 0;
            if (c->is_on) {
                chan_process(c->stack, c->stack_size, &l, &r);
                double pan = (c->pan + 1) * 0.5;
                left += c->vol * l * (1 - pan);
                right += c->vol * r * pan;
            }
        }
        samples[0] = vol * left;
        samples[1] = vol * right;
    }
}
