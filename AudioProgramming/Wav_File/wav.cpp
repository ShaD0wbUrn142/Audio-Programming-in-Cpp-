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
        exit;
    }
    fs::path full_path = folder_path / file_name;

    struct WavHeader wavh;

    std::cout << "Setting duration, frequency and amplitude..." << "\n";
    // How long it should last
    const int duration_seconds = 10;

    // The different notes as frequencies
    const double frequency_d2 = 73.42;
    const double frequency_a2 = 110.00;
    const double frequency_f3 = 185.00; // F sharp
    const double frequency_a3 = 220.00;
    const double frequency_c4 = 261.63;
    const double frequency_e4 = 311.13; // E flat
    // not a proper dm9 chord, I changed it to D7♭9

    const int amplitude = 10000; // 16-bit amplitude

    // buffering
    std::cout << "Setting buffer size..." << "\n";
    const int buffer_size = wavh.sample_rate * duration_seconds;
    wavh.data_size = buffer_size * wavh.block_align;
    wavh.file_size = 36 + wavh.data_size;

    short int *buffer = new short int[buffer_size];

    // creating audio data
    std::cout << "Creating audio data..." << "\n";
    double chord_frequencies[6] = {frequency_d2, frequency_a2, frequency_f3, frequency_a3, frequency_c4, frequency_e4};
    int note_length = wavh.sample_rate / 8; // one eighth-second per note

    for (int i = 0; i < buffer_size; i++)
    {
        int note_index = (i / note_length) % 6; // 6 notes in the Dm9 chord, // current note in the arpeggio
        double note_progress = (i % note_length) / static_cast<double>(note_length);
        double envelope = (note_progress < 0.5) ? (note_progress * 2) : ((1.0 - note_progress) * 2);
        envelope *= amplitude; // apply volume

        double sample_value = sin(2.0 * M_PI * chord_frequencies[note_index] * i / wavh.sample_rate);

        buffer[i] = static_cast<short int>(sample_value * envelope);
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