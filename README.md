# Chorus Echo
by Daniel Schwartz

## Description

A chorus echo guitar pedal for the Daisy Seed embedded platform.

## Knobs

1. Wet/Dry Mix
    - Controls the mix between the dry and processed signal.  This ranges from vibrato at 100% wet (fully clockwise) to chorus at 50%.
2. Tone
    - A subtle tone control for the processed signal.  This rolls off high-end from the processed signal as it is turned counter-clockwise.  The dry signal will remain unaffected.
3. Delay Time
    - Sets the delay time ranging from a short chorus at full counter-clockwise to 1 second at full clockwise.
4. Feedback
    - Sets the feedback amount for the delay line.  This will begin to self-oscillate at full clockwise.
5. Rate
    - Sets the rate of the modulation.  Ranges from .1Hz to 10Hz.
5. Depth
    - Sets the depth of the modulation.  This can range from a suble chorus effect at low levels to over the top.

## Foot switches

1. Bypass 
2. Tap tempo

Holding tap and pressing bypss will reset the delay time to a short chorus delay

## Toggle Switches

1. Sine/Triangle waveform select
2. Highpass filter (wet only)

## TODO

- [ ] Finish building prototype
- [ ] Change led and switch mappings for terrarium

