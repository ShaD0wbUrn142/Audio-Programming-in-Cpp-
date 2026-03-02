// https://www.youtube.com/watch?v=8nOi-0kBv2Y
// https://www.youtube.com/watch?v=JqJPBu7GXvw
// https://www.youtube.com/watch?v=rHqkeLxAsTc&t=99s
#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cmath>
#include <string>
#include <filesystem>
#include <array>

namespace fs = std::filesystem;

struct WavHeader
{
    // RIFF chunk
    std::array<char, 4> file_type_bloc_id{'R', 'I', 'F', 'F'};
    uint32_t file_size = 0;
    std::array<char, 4> file_format_id{'W', 'A', 'V', 'E'};

    // fmt sub-chunk
    std::array<char, 4> format_bloc_id{'f', 'm', 't', ' '};
    uint32_t format_data_length = 16; // PCM
    uint16_t format_type = 1;         // PCM
    uint16_t number_of_channels = 1;  // Mono
    uint32_t sample_rate = 44100;
    uint32_t byte_rate = sample_rate * number_of_channels * 16 / 8;
    uint16_t block_align = number_of_channels * 16 / 8;
    uint16_t bits_per_sample = 16;

    // data sub-chunk
    std::array<char, 4> data_bloc_id{'d', 'a', 't', 'a'};
    uint32_t data_size = 0;
};

// Get our different waves we could use
double sine_wave(double phase)
{
    return sin(2.0 * M_PI * phase);
}

double square_wave(double phase)
{
    return (phase < 0.5) ? 1.0 : -1.0;
}

double sawtooth_wave(double phase)
{
    return 2.0 * phase - 1.0;
}

double triangle_wave(double phase)
{
    return 2.0 * fabs(2.0 * phase - 1.0) - 1.0;
}

// forces little endian
void write_16(std::ofstream &file, int16_t n)
{
    char bytes[2];
    bytes[0] = n & 0xFF;        // least significant byte
    bytes[1] = (n >> 8) & 0xFF; // most significant byte
    file.write(bytes, 2);
}

void write_32(std::ofstream &file, int32_t n)
{
    char bytes[4];
    bytes[0] = n & 0xFF; // & 0xFF performs a bitwise AND operation with the value 0xFF. Limit its value to the lowest 8 bits
    bytes[1] = (n >> 8) & 0xFF;
    bytes[2] = (n >> 16) & 0xFF;
    bytes[3] = (n >> 24) & 0xFF;
    file.write(bytes, 4);
}

int main()
{
    // Get the file name and where it is going
    std::cout << "Please enter the name of the file you want to create (e.g., test.wav): ";
    std::string file_name;
    std::cin >> file_name;

    // Ask user for folder path
    std::cout << "Please enter the path of the folder where you want to put the file: ";
    std::string folder_path_input;
    std::cin >> folder_path_input;

    fs::path folder_path(folder_path_input);

    if (!fs::exists(folder_path))
    {
        std::cout << "Error: There is no folder called " << folder_path << "\n";
        return 1;
    }
    fs::path full_path = folder_path / file_name;

    struct WavHeader wavh;

    std::cout << "Setting duration, frequency and amplitude..." << "\n";
    // How long it should last
    const int duration_seconds = 40;
    const double chord_duration = 1; // 1 seconds per chord
    int samples_per_chord = wavh.sample_rate * chord_duration;

    // notes needed for chords
    const double freq_A2 = 110.00;
    const double freq_C3 = 130.81;
    const double freq_E3 = 164.81;
    const double freq_F3 = 174.61;
    const double freq_G3 = 196.00;
    const double freq_A3 = 220.00;
    const double freq_B3 = 246.94;
    const double freq_C4 = 261.63;

    // Am – Em – F – C progression
    // Am (A C E)
    double chord1[] = {freq_A2, freq_C3, freq_E3};

    // Em (E G B)
    double chord2[] = {freq_E3, freq_G3, freq_B3};

    // F (F A C)
    double chord3[] = {freq_F3, freq_A3, freq_C4};

    // C (C E G)
    double chord4[] = {freq_C3, freq_E3, freq_G3};

    // all our chords
    double *chords[] = {chord1, chord2, chord3, chord4};
    int chord_sizes[] = {3, 3, 3, 3}; // triads
    int total_chords = 4;             // number of chords

    int notes_per_chord = 3; // triads
    int samples_per_note = samples_per_chord / notes_per_chord;

    const int amplitude = 9000;

    // buffering
    std::cout << "Setting buffer size..." << "\n";
    const int buffer_size = wavh.sample_rate * duration_seconds;
    wavh.data_size = buffer_size * wavh.block_align;
    wavh.file_size = 36 + wavh.data_size;

    short int *buffer = new short int[buffer_size];

    // Phase
    double phase[5] = {0.0};          // Initialize phase for up to 5 notes
    double phaseIncrement[5] = {0.0}; // Phase increment per note (determines frequency step per sample)

    for (int i = 0; i < buffer_size; i++)
    {
        int chord_index = (i / samples_per_chord) % total_chords;
        int chord_sample_index = i % samples_per_chord;

        // Determine which note of the chord is active
        int note_index = (chord_sample_index / samples_per_note) % notes_per_chord;

        // Position within the current note
        int note_sample_index = chord_sample_index % samples_per_note;

        double note_progress = note_sample_index / (double)samples_per_note;

        // Per-note envelope (smooth pluck style)
        double envelope = sin(note_progress * M_PI);

        double freq = chords[chord_index][note_index];

        // Use only one phase accumulator for arpeggio
        phaseIncrement[0] = freq / wavh.sample_rate;

        double sample = triangle_wave(phase[0]);

        phase[0] += phaseIncrement[0];
        if (phase[0] >= 1.0)
            phase[0] -= 1.0;

        sample *= envelope;
        sample *= amplitude;

        buffer[i] = static_cast<short>(sample);
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
        sample *= amplitude; // Scale the final sample by overall amplitude

        buffer[i] = static_cast<short>(sample); // Store sample in buffer (casting to 16-bit PCM)
        */
    }

    std::cout << "Writing header and audio data to file..." << "\n";
    std::ofstream wav(full_path, std::ios::binary);
    if ((wav.is_open()))
    {
        // RIFF
        wav.write(wavh.file_type_bloc_id.data(), 4);
        write_32(wav, wavh.file_size);
        wav.write(wavh.file_format_id.data(), 4);

        // fmt chunk
        wav.write(wavh.format_bloc_id.data(), 4);
        write_32(wav, wavh.format_data_length);
        write_16(wav, wavh.format_type);
        write_16(wav, wavh.number_of_channels);
        write_32(wav, wavh.sample_rate);
        write_32(wav, wavh.byte_rate);
        write_16(wav, wavh.block_align);
        write_16(wav, wavh.bits_per_sample);

        // data chunk
        wav.write(wavh.data_bloc_id.data(), 4);
        write_32(wav, wavh.data_size);

        // write samples
        wav.write((char *)buffer, wavh.data_size);

        wav.close();
    }
    std::cout << "Saved to file: " << full_path << "\n";
    std::cout << "Deleting unused memory..." << "\n";
    delete[] buffer; // don't want to keep storing unused memory

    std::cout << "Program complete..." << std::endl;
    return 0;
}