import math

PI = math.pi
SR = 44100

def midi_freq(m):
    return 440 * 2 ** ((m - 69) * (1 / 12))

def sec(x):
    return int(x * SR)

def limit(x, lo, hi):
    return min(max(x, lo), hi)

def lerp(x, y, a):
     return x * (1 - a) + y * a

def hertz(t, freq):
    return (2 * PI / SR) * freq * t
    
def dsf(phase, mod, width):
    mphase = mod * phase
    num = math.sin(phase) - width * math.sin(phase - mphase)
    return num / (1 + width * (width - 2 * math.cos(mphase)))

def dsf2(phase, mod, width):
    mphase = mod * phase
    num = math.sin(phase) * (1 - width * width)
    return num / (1 + width * (width - 2 * math.cos(mphase)))

def saw(phase, width):
    return dsf(phase, 1, width)

def square(phase, width):
    return dsf(phase, 2, width)

def pwm(phase, offset, width):
    return saw(phase, width) - saw(phase + offset, width)

def lfsr(state, size, taps):
    x = 0
    for t in taps:
        x ^= state >> t 
    return (state >> 1) | ((~x & 1) << (size - 1))

class Phasor:
    __slots__ = ('phase',)

def phasor_init(s):
    s.phase = 0

def phasor_reset(s):
    s.phase = 0

def phasor_next(s, freq):
    p = s.phase
    s.phase = (s.phase + (2 * PI / SR) * freq) % (SR * PI)
    return p

class Env:
    __slots__ = ('deltas', 'levels', 'is_ended', 'is_loop', 'is_full_reset',
    'sustain_pos', 'level_0', 'level', 't', 'level_pos', 't_pos', 'pos')

def env_init(s, deltas, level_0, levels):
    s.deltas = deltas
    s.levels = levels
    s.is_loop = False
    s.is_full_reset = False
    s.sustain_pos = -1
    s.level_0 = level_0
    s.level = level_0
    env_reset(s)

def env_reset(s):
    s.is_ended = False
    s.t = 0
    s.pos = 0
    s.t_pos = 0
    if s.is_full_reset:
        s.level = s.level_0
    s.level_pos = s.level

def env_next_head(s, func):
    if s.pos >= len(s.deltas):
        if not s.is_loop:
            s.is_ended = True
            return s.level
        env_reset(s)
    return env_next_tail(s, func)

def env_next_tail(s, func):
    dt = s.deltas[s.pos]
    if s.t < s.t_pos + dt:
        s.level = func(s.level_pos, s.levels[s.pos], (s.t - s.t_pos) / dt)
        s.t += 1
        return s.level
    if s.sustain_pos == s.pos:
        return s.level
    s.t_pos, s.level_pos = s.t_pos + dt, s.levels[s.pos]
    s.pos += 1
    return s.level

def step(a, b, x):
    return b

def env_next(s):
    return env_next_head(s, lerp)

def seq_next(s):
    return env_next_head(s, step)

class Delay:
    __slots__ = ('buf', 'size', 'level', 'fb', 'pos')

def delay_init(s, size, level, fb):
    s.buf = [0] * size
    s.level = level
    s.fb = fb
    s.pos = 0

def delay(buf, pos, x, level, fb=0.5):
    old = buf[pos]
    y = x + old * level
    buf[pos] = old * fb + x
    pos = (pos + 1) % len(buf)
    return y, pos

def delay_next(s, x):
    y, s.pos = delay(s.buf, s.pos, x, s.level, s.fb)
    return y

class Filter:
    __slots__ = ('y',)

def filter_init(s):
    s.y = 0

def lp_next(s, x, width):
    s.y += width * (x - s.y)
    return s.y

def hp_next(s, x, width):
    return x - lp_next(s, x, width)

class Glide:
    __slots__ = ('source', 'rate')

def glide_init(s, source, rate):
    s.source = source
    s.rate = rate * (1 / SR)

def glide_next(s, target):
    step = s.rate * abs(s.source - target) + 1e-6
    if s.source < target:
        s.source = min(s.source + step, target)
    else:
        s.source = max(s.source - step, target)
    return s.source

class Noise:
    __slots__ = ('size', 'taps', 'state', 'phase')

def noise_init(s, size, taps):
    s.size = size
    s.taps = taps
    s.state = 1
    s.phase = 0
    
def noise_next(s, freq):
    s.phase += freq * (1 / SR)
    if s.phase >= 1:
        s.phase -= 1
        s.state = lfsr(s.state, s.size, s.taps)
    return 2 * (s.state & 1) - 1

class Chan:
    __slots__ = ('is_on', 'pan', 'vol', 'stack', 'stack_size')

MAX_BOXES = 8

def mix_init(channels):
    for c in channels: 
        chan_set(c, False, 0, 0)        
        c.stack = [Box() for _ in range(MAX_BOXES)]
        c.stack_size = 0

def chan_set(c, is_on, vol, pan):
    c.is_on = is_on
    c.vol = vol
    c.pan = pan

class Box:
    __slots__ = ('change', 'next', 'state', 'is_stereo')

def chan_push(c, box_init):
    box_init(c.stack[c.stack_size])
    c.stack_size += 1

def chan_process(stack, stack_size):
    l, r = 0, 0
    for i in range(stack_size):
        box = stack[i]
        if box.is_stereo:
            l, r = box.next(box.state, l, r)
        else:
            l = box.next(box.state, l)
            r = l
    return l, r

def mix_process(channels, num_channels, tick, vol):
    samples = []
    for _ in range(tick):
        left, right = 0, 0
        for i in range(num_channels):
            chan = channels[i]
            if chan.is_on:
                l, r = chan_process(chan.stack, chan.stack_size)
                pan = (chan.pan + 1) * 0.5
                left += chan.vol * l * (1 - pan)
                right += chan.vol * r * pan
        samples.append((vol * left, vol * right))
    return samples

MAX_CHANNELS = 32

Channels = [Chan() for _ in range(MAX_CHANNELS)]