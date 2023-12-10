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
    // WAV 출력을 위한 설정
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
        std::cerr << "오류: 오디오 출력을 열 수 없습니다." << std::endl;
        return;
    }

    // 출력할 데이터 설정
    wavehdr.lpData = const_cast<LPSTR>(data);
    wavehdr.dwBufferLength = dataSize;
    wavehdr.dwFlags = 0;

    // 출력할 데이터 준비
    waveOutPrepareHeader(hWaveOut, &wavehdr, sizeof(WAVEHDR));

    // 데이터 재생
    waveOutWrite(hWaveOut, &wavehdr, sizeof(WAVEHDR));

    // 데이터 재생이 끝날 때까지 대기
    while (!(wavehdr.dwFlags & WHDR_DONE)) {
        Sleep(10);
    }

    // 준비된 데이터 정리
    waveOutUnprepareHeader(hWaveOut, &wavehdr, sizeof(WAVEHDR));
    // WAV 출력 닫기
    waveOutClose(hWaveOut);
}

void BufferAndPlayMusic(const CString& filePath) {
    WavHeader wavHeader;
    std::vector<char> buffer;

    std::ifstream file(filePath, std::ios::binary | std::ios::in);

    if (file.is_open()) {
        // 헤더 정보 읽기
        file.read((char*)&wavHeader, sizeof(WavHeader));
        int wavHeaderSize = sizeof(WavHeader);

        // "data" 청크를 찾을 때까지 반복
        while (strncmp(wavHeader.data, "data", 4) != 0 && !file.eof()) {
            char* infoChunk = new char[wavHeader.dataSize];
            file.read(infoChunk, wavHeader.dataSize);
            wavHeaderSize += wavHeader.dataSize;
            file.read((char*)&wavHeader.data, sizeof(wavHeader.data));
            wavHeaderSize += wavHeader.dataSize + sizeof(wavHeader.data);
            file.read((char*)&wavHeader.dataSize, sizeof(wavHeader.dataSize));

            // 동적으로 할당한 메모리 해제
            delete[] infoChunk;
        }

        // "data" 청크를 찾지 못한 경우 에러 출력
        if (strncmp(wavHeader.data, "data", 4) != 0) {
            std::cerr << "Error: Invalid WAV file format. Missing 'data' chunk." << std::endl;
            file.close();
            return;
        }

        // 데이터 크기만큼 버퍼 할당
        buffer.resize(wavHeader.dataSize);

        // 첫 번째 데이터를 버퍼에 읽어두기
        // file.read(buffer.data(), wavHeader.dataSize);

        // 데이터를 1024개의 샘플로 나누어 읽기
        const int bytesPerRead = wavHeader.dataSize/1024;

        int bytesRead = wavHeaderSize;
        while (bytesRead < wavHeader.dataSize) {
            // 새로운 데이터를 버퍼에 추가
            int bytesToRead = min(bytesPerRead, wavHeader.dataSize - bytesRead);
            file.seekg(bytesRead);
            file.read(buffer.data() + bytesRead, bytesToRead);
            bytesRead += bytesToRead;

            // 데이터를 읽을 때마다 재생
            PlayMusic(buffer.data(), bytesRead, wavHeader);
        }

        // WAV 파일 닫기
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