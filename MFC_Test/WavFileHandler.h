#pragma once
#include <vector>


struct WavHeader
{
    char riff[4];
    int chunkSize;
    char wave[4];
    char fmt[4];
    int subchunk1Size;
    short audioFormat;
    short numChannels;
    int sampleRate;
    int byteRate;
    short blockAlign;
    short bitsPerSample;
    char data[4];
    int dataSize;
};

class WavFileHandler
{
public:
	void ReadWavFile(const CString& filePath);
    void PlayMusic(const char* data, int dataSize, const WavHeader& wavHeader);

private:
	WavHeader wavHeader;
	std::vector<short> audioData;
};

