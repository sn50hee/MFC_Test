#include "pch.h"
#include "WavFileHandler.h"

#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>

#pragma comment(lib, "winmm.lib")

void PlayMusic(const char* data, int dataSize, const WavHeader& wavHeader) {
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
        return;
    }

    // ����� ������ ����
    wavehdr.lpData = const_cast<LPSTR>(data);
    wavehdr.dwBufferLength = dataSize;
    wavehdr.dwFlags = 0;

    // ����� ������ �غ�
    waveOutPrepareHeader(hWaveOut, &wavehdr, sizeof(WAVEHDR));

    // ������ ���
    waveOutWrite(hWaveOut, &wavehdr, sizeof(WAVEHDR));

    // ������ ����� ���� ������ ���
    while (!(wavehdr.dwFlags & WHDR_DONE)) {
        Sleep(10);
    }

    // �غ�� ������ ����
    waveOutUnprepareHeader(hWaveOut, &wavehdr, sizeof(WAVEHDR));
    // WAV ��� �ݱ�
    waveOutClose(hWaveOut);
}

void BufferAndPlayMusic(const CString& filePath) {
    WavHeader wavHeader;
    std::vector<char> buffer;

    std::ifstream file(filePath, std::ios::binary | std::ios::in);

    if (file.is_open()) {
        // ��� ���� �б�
        file.read((char*)&wavHeader, sizeof(WavHeader));
        int wavHeaderSize = sizeof(WavHeader);

        // "data" ûũ�� ã�� ������ �ݺ�
        while (strncmp(wavHeader.data, "data", 4) != 0 && !file.eof()) {
            char* infoChunk = new char[wavHeader.dataSize];
            file.read(infoChunk, wavHeader.dataSize);
            wavHeaderSize += wavHeader.dataSize;
            file.read((char*)&wavHeader.data, sizeof(wavHeader.data));
            wavHeaderSize += wavHeader.dataSize + sizeof(wavHeader.data);
            file.read((char*)&wavHeader.dataSize, sizeof(wavHeader.dataSize));

            // �������� �Ҵ��� �޸� ����
            delete[] infoChunk;
        }

        // "data" ûũ�� ã�� ���� ��� ���� ���
        if (strncmp(wavHeader.data, "data", 4) != 0) {
            std::cerr << "Error: Invalid WAV file format. Missing 'data' chunk." << std::endl;
            file.close();
            return;
        }

        // ������ ũ�⸸ŭ ���� �Ҵ�
        buffer.resize(wavHeader.dataSize);

        // ù ��° �����͸� ���ۿ� �о�α�
        // file.read(buffer.data(), wavHeader.dataSize);

        // �����͸� 1024���� ���÷� ������ �б�
        const int bytesPerRead = wavHeader.dataSize/1024;

        int bytesRead = wavHeaderSize;
        while (bytesRead < wavHeader.dataSize) {
            // ���ο� �����͸� ���ۿ� �߰�
            int bytesToRead = min(bytesPerRead, wavHeader.dataSize - bytesRead);
            file.seekg(bytesRead);
            file.read(buffer.data() + bytesRead, bytesToRead);
            bytesRead += bytesToRead;

            // �����͸� ���� ������ ���
            PlayMusic(buffer.data(), bytesRead, wavHeader);
        }

        // WAV ���� �ݱ�
        file.close();
    }
    else {
        std::cerr << "Error: Could not open the WAV file." << std::endl;
    }
}

void WavFileHandler::ReadWavFile(const CString& filePath) {
    std::thread readThread(BufferAndPlayMusic, filePath);
    readThread.join();
}