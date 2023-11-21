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
        // 헤더 정보 읽기
        file.read((char*)&wavHeader, sizeof(WavHeader));

        // "data" 청크 찾기
        while (strncmp(wavHeader.data, "data", 4) != 0 && !file.eof()) {
            char fint_d;
            file.read(&fint_d, sizeof(fint_d));
            if (fint_d == 'd') {
                char fint_a;
                file.read(&fint_a, sizeof(fint_a));
                if (fint_a == 'a') {
                    char fint_t;
                    file.read(&fint_t, sizeof(fint_t));
                    if (fint_t == 't') {
                        char fint_a2;
                        file.read(&fint_a2, sizeof(fint_a2));
                        if (fint_a2 == 'a') {
                            wavHeader.data[0] = fint_d;
                            wavHeader.data[1] = fint_a;
                            wavHeader.data[2] = fint_t;
                            wavHeader.data[3] = fint_a2;
                            file.read((char*)&wavHeader.dataSize, sizeof(wavHeader.dataSize));
                        }
                    }
                }
            }
        }

        // "data" 청크를 찾지 못한 경우 에러 출력
        if (strncmp(wavHeader.data, "data", 4) != 0) {
            std::cerr << "Error: Invalid WAV file format. Missing 'data' chunk." << std::endl;
            file.close();
            return;
        }

        // 데이터 크기만큼 버퍼 할당
        char* buffer = new char[wavHeader.dataSize];

        // 데이터를 버퍼에 읽기
        file.read(buffer, wavHeader.dataSize);

        // WAV 파일 닫기
        file.close();

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

        // WAV 출력 열기
        if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR) {
            // 출력할 데이터 설정
            wavehdr.lpData = buffer;
            wavehdr.dwBufferLength = wavHeader.dataSize;
            wavehdr.dwFlags = 0;

            // 출력할 데이터 준비
            waveOutPrepareHeader(hWaveOut, &wavehdr, sizeof(WAVEHDR));

            // 출력 시작
            waveOutWrite(hWaveOut, &wavehdr, sizeof(WAVEHDR));

            // 데이터 재생이 끝날 때까지 대기
            Sleep(wavHeader.dataSize / wfx.nAvgBytesPerSec * 1000);

            // 준비된 데이터 정리
            waveOutUnprepareHeader(hWaveOut, &wavehdr, sizeof(WAVEHDR));
            // WAV 출력 닫기
            waveOutClose(hWaveOut);
        }

        // 할당된 버퍼 메모리 해제
        delete[] buffer;
    }
    else {
        std::cerr << "Error: Could not open the WAV file." << std::endl;
    }

}