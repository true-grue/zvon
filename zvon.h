/* Author: Peter Sovietov */

#ifndef ZVON_H
#define ZVON_H

double midi_freq(int m);
double sec(double t);
double limit(double x, double low, double high);
double lerp(double a, double b, double x);
double herz(double t, double freq);
double dsf(double phase, double mod, double width);
double dsf2(double phase, double mod, double width);
double saw(double phase, double width);
double square(double phase, double width);
double pwm(double phase, double offset, double width);
unsigned int lfsr(unsigned int state, int bits, int *taps, int taps_size);

struct phasor_state {
    double phase;
};

void phasor_init(struct phasor_state *s);
void phasor_reset(struct phasor_state *s);
double phasor_next(struct phasor_state *s, double freq);

struct env_state {
    double *deltas;
    double *levels;
    int size;
    int is_end;
    int is_loop;
    int is_full_reset;
    int sustain_pos;
    double level_0;
    double level;
    int t;
    double level_at_pos;
    int t_at_pos;
    int pos;
};

typedef double (*env_func)(double a, double b, double x);

void env_init(struct env_state *s, double *deltas, double level_0, double *levels, int size);
void env_reset(struct env_state *s);
double env_next_head(struct env_state *s, env_func func);
double env_next(struct env_state *s);
double seq_next(struct env_state *s);

#define MAX_DELAY_SIZE 65536

struct delay_state {
    double buf[MAX_DELAY_SIZE];
    int size;
    double level;
    double fb;
    int pos;
};

void delay_init(struct delay_state *s, int size, double level, double fb);
double delay_next(struct delay_state *s, double x);

struct filter_state {
    double y;
};

void filter_init(struct filter_state *s);
double filter_lp_next(struct filter_state *s, double x, double width);
double filter_hp_next(struct filter_state *s, double x, double width);

struct glide_state {
    double source;
    double rate;
};

void glide_init(struct glide_state *s, double source, double rate);
double glide_next(struct glide_state *s, double target);

#define MAX_TAPS 32

struct noise_state {
    int bits;
    int taps[MAX_TAPS];
    int taps_size;
    unsigned int state;
    double phase;    
};

void noise_init(struct noise_state *s, int bits, int *taps, int taps_size);
double noise_next(struct noise_state *s, double freq);

#define MAX_BOXES 8

typedef void (*box_change_func)(void *state, int param, int elem, double val);
typedef double (*box_next_func)(void *state, double x);
typedef void (*box_init_func)(void *state);

struct box_state {
    box_change_func change;
    box_next_func next;
    void *state;
};

struct box_def {
    box_change_func change;
    box_next_func next;
    box_init_func init;
    int state_size;
};

struct chan_state {
    int is_on;
    double vol;
    double pan;
    struct box_state stack[MAX_BOXES];
    int stack_depth;
};

void chan_init(struct chan_state *c);
void chan_set(struct chan_state *c, int is_on, double vol, double pan);
void chan_free(struct chan_state *c);
void chan_push(struct chan_state *c, struct box_def *def);
void chan_mix(struct chan_state *channels, int num_channels, double vol, double *samples, int num_samples);

#endif
