
# Generating Audio in a Buffer

## 1. Dm9 Chord Example

This loop generates a D minor 9 chord by summing six sine waves and applying a simple fade-in/fade-out envelope.

```cpp
for (int i = 0; i < buffer_size; i++)
{
    double progress = static_cast<double>(i) / buffer_size;

    // Linear fade-in and fade-out envelope
    double envelope = (progress < 0.5) 
        ? (progress * 2.0) 
        : ((1.0 - progress) * 2.0);

    envelope *= amplitude;

    // D minor 9 chord (Dm9)
    double sample_value =
        (sin(2.0 * M_PI * frequency_d2 * i / wavh.sample_rate) +
         sin(2.0 * M_PI * frequency_a2 * i / wavh.sample_rate) +
         sin(2.0 * M_PI * frequency_f3 * i / wavh.sample_rate) +
         sin(2.0 * M_PI * frequency_a3 * i / wavh.sample_rate) +
         sin(2.0 * M_PI * frequency_c4 * i / wavh.sample_rate) +
         sin(2.0 * M_PI * frequency_e4 * i / wavh.sample_rate))
        / 6.0;

    buffer[i] = static_cast<short>(sample_value * envelope);
}
```

### What This Does

* Each `sin()` generates one note.
* The notes are averaged to prevent clipping.
* A linear envelope fades the sound in and out.
* The result is written into a 16-bit audio buffer.

---

## 2. Basic Single Sine Wave Oscillator

### Setup

```cpp
const int duration_seconds = 10;
const double frequency = 261.63;   // C4
const int amplitude = 10000;       // 16-bit amplitude

double phase = 0.0;
double phaseIncrement = frequency / wavh.sample_rate;
```

### Buffer Allocation

```cpp
const int buffer_size = wavh.sample_rate * duration_seconds;
wavh.data_size = buffer_size * wavh.block_align;
wavh.file_size = 36 + wavh.data_size;

short int* buffer = new short int[buffer_size];
```

### Audio Generation

```cpp
for (int i = 0; i < buffer_size; i++)
{
    double sample = sin(2.0 * M_PI * phase);
    buffer[i] = static_cast<short>(sample * amplitude);

    phase += phaseIncrement;

    if (phase >= 1.0)
        phase -= 1.0;
}
```

### Explanation

* `phase` represents position in the waveform cycle (0 to 1).
* `phaseIncrement` determines how fast the waveform advances.
* `sin(2π * phase)` converts phase into amplitude.
* Wrapping phase at 1.0 keeps it in one repeating cycle.

This technique is called **phase accumulation**, and it is a standard method in digital signal processing (DSP) for generating periodic waveforms.

---

# Waveform Types

All periodic waveforms follow the same structure:

* **Phase** answers: “Where am I in the cycle?”
* **Waveform function** answers: “What amplitude should I output at this phase?”

The only difference between waveforms is the function that maps phase (0 → 1) to amplitude (-1 → 1).

---

## 1. Square Wave

```cpp
double sample = (phase < 0.5) ? 1.0 : -1.0;
```

### Definition

A square wave:

* Outputs +1 for the first half of the cycle
* Outputs -1 for the second half

It switches instantly between high and low values.

### Why It Sounds Bright

The sudden jump creates many high-frequency harmonics.

---

## 2. Sawtooth Wave

```cpp
double sample = 2.0 * phase - 1.0;
```

### Definition

A sawtooth wave:

* Starts at -1
* Ramps linearly to +1
* Instantly drops back to -1
* Repeats

### Mapping Explanation

We map phase from [0, 1] to amplitude [-1, 1]:

* If phase = 0 → output = -1
* If phase = 1 → output = +1

This is a simple linear transformation.

### Why It Sounds Bright

The sharp drop introduces many harmonics.

---

## 3. Triangle Wave

```cpp
double sample = 2.0 * fabs(2.0 * phase - 1.0) - 1.0;
```

### How It Works

Step 1:

```cpp
2.0 * phase - 1.0
```

Creates a sawtooth from -1 to +1.

Step 2:

```cpp
fabs(...)
```

Reflects negative values upward, folding the waveform.

Step 3:
Scale and shift back into the [-1, 1] range.

### Concept

A triangle wave is essentially a folded sawtooth wave.

### Why It Sounds Softer Than a Saw

It has sharp corners, but no vertical jumps.
This results in fewer high harmonics than a sawtooth.

---

# Why Waveforms Sound Different

The difference comes from **smoothness**.

* **Sine wave** → perfectly smooth → one frequency only
* **Triangle wave** → sharp corners → some harmonics
* **Sawtooth wave** → sharp drop → many harmonics
* **Square wave** → instant jump → many harmonics

Sharper edges introduce more high-frequency components.
This behavior is explained by Fourier theory: any periodic waveform can be decomposed into sine waves at different frequencies.

---

# The Core Oscillator Concept

An oscillator has two parts:

1. **Phase accumulation**

   * Keeps track of cycle position.
   * Advances according to frequency.

2. **Waveform function**

   * Converts phase into amplitude.

In abstract form:

``` cpp
amplitude = waveform(phase)
```

That is the entire structure behind digital oscillators.

Everything else — chords, envelopes, modulation — builds on this foundation.

============================================================================

/*
        int chord_index = (i / samples_per_chord) % total_chords; // Determine which chord is currently playing
int chord_sample_index = i % samples_per_chord;           // Position within the current chord's duration

double note_progress = chord_sample_index / (double)samples_per_chord; // Normalized progress (0.0 to 1.0) of the note

double envelope = sin(note_progress * M_PI); // Simple amplitude envelope: rises and falls smoothly like a sine

double sample = 0.0;                   // Initialize output sample for this frame
int voices = chord_sizes[chord_index]; // Number of notes in the current chord

for (int n = 0; n < voices; n++) // Loop through each note in the chord
{
    double freq = chords[chord_index][n]; // Get the frequency of this note

    phaseIncrement[n] = freq / wavh.sample_rate; // Calculate how much to advance the phase for this note

    sample += sawtooth_wave(phase[n]); // Generate sine wave for this note and add to the sample

    phase[n] += phaseIncrement[n]; // Increment phase for next sample
    if (phase[n] >= 1.0)           // Wrap phase around to stay in [0,1)
        phase[n] -= 1.0;
}

sample /= voices; // Average all notes to prevent clipping

sample *= envelope;  // Apply the envelope to shape the amplitude
sample*= amplitude; // Scale the final sample by overall amplitude

buffer[i] = static_cast<short>(sample); // Store sample in buffer (casting to 16-bit PCM)
*/
