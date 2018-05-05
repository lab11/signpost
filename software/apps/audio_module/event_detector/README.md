# Universal acoustic event detection app
The first step into any audio processing chain.

Detect a large class of "acoustic events" (bird calls, gunshots, broken glasses, etc.) from noisy background.
Further classifications are reserved for downstream processing.

## Quick start
1. Plug the Audio Module into the `Module 1` slot

2.  **Important** Make sure the programming knob is turned to `MOD1`.

3. Get all the prerequisites (See below)

4. Flash the app

    ```bash
        cd signpost/software/apps/audio_module/event_detector
        make flash
    ```

## Prerequisites
* Fixed-point FFT: git clone https://github.com/longle2718/kiss_fft
* Fixed-point log: git clone https://github.com/dmoulding/log2fix
* Ridge tracker: git clone https://bitbucket.org/longle1/gcwa
  * Fixed-point implementation in C is under gcwa/c/
