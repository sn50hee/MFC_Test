#pragma once
#include <vector>
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>

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

class BufferedMusicPlayer {
public:
    BufferedMusicPlayer(const WavHeader& wavHeader);
    ~BufferedMusicPlayer();

    void play(const char* data, int dataSize);
    void start();
    void stop();

private:
    void playThreadFunc();

private:
    WavHeader wavHeader_;
    std::vector<char> buffer_;
    bool isPlaying_;
    std::thread playThread_;
    std::mutex mutex_;
    std::condition_variable conditionVariable_;
};

class WavFileHandler {
public:
    void ReadWavFile(const CString& filePath);
    void PlayMusic(const char* data, int dataSize, const WavHeader& wavHeader);

private:
    WavHeader wavHeader;
    std::vector<short> audioData;
};
