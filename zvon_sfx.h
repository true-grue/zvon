/* Author: Peter Sovietov */

#ifndef ZVON_SFX_H
#define ZVON_SFX_H

#include "zvon.h"

enum {
    ZV_NOTE_ON,
    ZV_NOTE_OFF,
    ZV_VOLUME,
    ZV_TIME,
    ZV_FEEDBACK,
    ZV_GAIN,
    ZV_WAVE_TYPE,
    ZV_END
};

extern struct sfx_proto sfx_synth;
extern struct sfx_proto sfx_delay;
extern struct sfx_proto sfx_dist;

#endif
