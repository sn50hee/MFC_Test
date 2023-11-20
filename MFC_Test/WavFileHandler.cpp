#include "pch.h"
#include "WavFileHandler.h"

#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <fstream>

#pragma comment(lib, "winmm.lib")

void WavFileHandler::ReadWavFile(const CString& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::in);

    if (file.is_open())
    {
        file.read((char*)&wavHeader, sizeof(WavHeader));

        char* buffer = new char[wavHeader.dataSize];
        file.read(buffer, wavHeader.dataSize);

        //while (strncmp(wavHeader.data, "LIST", 4) == 0) {
        //    char listChunkID[4];
        //    int listChunkSize;

        //    file.read(listChunkID, 4);
        //    file.read(reinterpret_cast<char*>(&listChunkSize), sizeof(int));

        //    // 건너뛰고자 하는 LIST 청크를 건너뜁니다.
        //    file.seekg(listChunkSize, std::ios::cur);

        //    // 다음 헤더 읽기
        //    file.read(reinterpret_cast<char*>(&wavHeader), sizeof(WavHeader));
        //}

        //// 'data' 청크 확인
        //if (strncmp(wavHeader.data, "data", 4) != 0) {
        //    std::cerr << "Error: 'data' chunk not found." << std::endl;
        //}

        HWAVEOUT hWaveOut;
        WAVEFORMATEX wfx;
        WAVEHDR wavehdr;

        wfx.nSamplesPerSec = wavHeader.sampleRate;
        wfx.wBitsPerSample = wavHeader.bitsPerSample;
        wfx.nChannels = wavHeader.numChannels;
        wfx.cbSize = 0;
        wfx.wFormatTag = WAVE_FORMAT_PCM;
        wfx.nBlockAlign = (wfx.wBitsPerSample * wfx.nChannels) >> 3;
        wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

        if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR)
        {
            wavehdr.lpData = buffer;
            wavehdr.dwBufferLength = wavHeader.dataSize;
            wavehdr.dwFlags = 0;
            waveOutPrepareHeader(hWaveOut, &wavehdr, sizeof(WAVEHDR));
            waveOutWrite(hWaveOut, &wavehdr, sizeof(WAVEHDR));
            Sleep(wavHeader.dataSize / wfx.nAvgBytesPerSec * 1000);
            waveOutUnprepareHeader(hWaveOut, &wavehdr, sizeof(WAVEHDR));
            waveOutClose(hWaveOut);
        }

        delete[] buffer;
    }

    file.close();
}

const std::vector<short>& WavFileHandler::GetAudioData() const {
    return audioData;
}