#include "pch.h"
#include "WavFileHandler.h"

#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <fstream>

#pragma comment(lib, "winmm.lib")

void WavFileHandler::ReadWavFile(const CString& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::in);

    if (file.is_open()) {
        // ��� ���� �б�
        file.read((char*)&wavHeader, sizeof(WavHeader));

        while (strncmp(wavHeader.data, "data", 4) != 0 && !file.eof()) {
            char* infoChunk = new char[wavHeader.dataSize];
            file.read(infoChunk, wavHeader.dataSize);
            file.read((char*)&wavHeader.data, sizeof(wavHeader.data));
            file.read((char*)&wavHeader.dataSize, sizeof(wavHeader.dataSize));
        }

        // "data" ûũ�� ã�� ���� ��� ���� ���
        if (strncmp(wavHeader.data, "data", 4) != 0) {
            std::cerr << "Error: Invalid WAV file format. Missing 'data' chunk." << std::endl;
            file.close();
            return;
        }

        // ������ ũ�⸸ŭ ���� �Ҵ�
        char* buffer = new char[wavHeader.dataSize];

        // �����͸� ���ۿ� �б�
        file.read(buffer, wavHeader.dataSize);

        // WAV ���� �ݱ�
        file.close();

        // WAV ����� ���� ����
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

        if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
            std::cerr << "����: ����� ����� �� �� �����ϴ�." << std::endl;
            file.close();
            return;
        }

        //int chunkSize = wavHeader.dataSize/1024;
        //char* buffer = new char[chunkSize];

        // WAV ��� ����
        if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR) {
            // ����� ������ ����
            wavehdr.lpData = buffer;
            wavehdr.dwBufferLength = wavHeader.dataSize;
            wavehdr.dwFlags = 0;

            // ����� ������ �غ�
            waveOutPrepareHeader(hWaveOut, &wavehdr, sizeof(WAVEHDR));

            // ��� ����
            waveOutWrite(hWaveOut, &wavehdr, sizeof(WAVEHDR));

            // ������ ����� ���� ������ ���
            Sleep(wavHeader.dataSize / wfx.nAvgBytesPerSec * 1000);

            // �غ�� ������ ����
            waveOutUnprepareHeader(hWaveOut, &wavehdr, sizeof(WAVEHDR));
            // WAV ��� �ݱ�
            waveOutClose(hWaveOut);
        }

        // �Ҵ�� ���� �޸� ����
        delete[] buffer;
    }
    else {
        std::cerr << "Error: Could not open the WAV file." << std::endl;
    }

}