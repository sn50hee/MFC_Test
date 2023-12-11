#include "pch.h"
#include "WavFileHandler.h"

#pragma comment(lib, "winmm.lib")

void PlayMusic(const char* data, int dataSize, const WavHeader& wavHeader, WAVEFORMATEX wfx) {
    // WAV 출력을 위한 설정
    HWAVEOUT hWaveOut;
    WAVEHDR wavehdr;

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

BufferedMusicPlayer::BufferedMusicPlayer(const WavHeader& wavHeader) : wavHeader_(wavHeader), isPlaying_(false) {}

BufferedMusicPlayer::~BufferedMusicPlayer() {
    stop();
}

void BufferedMusicPlayer::play(const char* data, int dataSize) {
    std::lock_guard<std::mutex> lock(mutex_);
    buffer_.insert(buffer_.end(), data, data + dataSize);
    conditionVariable_.notify_one();
}

void BufferedMusicPlayer::start() {
    if (!isPlaying_) {
        isPlaying_ = true;
        playThread_ = std::thread(&BufferedMusicPlayer::playThreadFunc, this);
    }
}

void BufferedMusicPlayer::stop() {
    if (isPlaying_) {
        isPlaying_ = false;
        if (playThread_.joinable()) {
            playThread_.join();
        }
    }
}

void BufferedMusicPlayer::playThreadFunc() {
    WAVEFORMATEX wfx;
    wfx.nSamplesPerSec = wavHeader_.sampleRate;
    wfx.wBitsPerSample = wavHeader_.bitsPerSample;
    wfx.nChannels = wavHeader_.numChannels;
    wfx.cbSize = 0;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nBlockAlign = (wfx.wBitsPerSample * wfx.nChannels) >> 3;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

    while (isPlaying_) {
        std::unique_lock<std::mutex> lock(mutex_);

        if (!buffer_.empty()) {
            const int bufferSize = buffer_.size();
            PlayMusic(buffer_.data(), bufferSize, wavHeader_, wfx);
            buffer_.clear();
        }
        else {
            conditionVariable_.wait(lock);
        }
    }
}

void ReadDataAndBufferMusic(const CString& filePath, BufferedMusicPlayer& player) {
    WavHeader wavHeader;
    std::ifstream file(filePath, std::ios::binary | std::ios::in);

    if (file.is_open()) {
        file.read((char*)&wavHeader, sizeof(WavHeader));

        while (strncmp(wavHeader.data, "data", 4) != 0 && !file.eof()) {
            char* infoChunk = new char[wavHeader.dataSize];
            file.read(infoChunk, wavHeader.dataSize);
            delete[] infoChunk;

            file.read((char*)&wavHeader.data, sizeof(wavHeader.data));
            file.read((char*)&wavHeader.dataSize, sizeof(wavHeader.dataSize));
        }

        if (strncmp(wavHeader.data, "data", 4) != 0) {
            std::cerr << "Error: Invalid WAV file format. Missing 'data' chunk." << std::endl;
            file.close();
            return;
        }

        const int bufferSize = wavHeader.dataSize;
        std::vector<char> buffer(bufferSize);
        const int bytesPerRead = bufferSize / 1024;

        int bytesRead = 0;
        while (bytesRead < bufferSize) {
            int remainingBytes = bufferSize - bytesRead;
            int bytesToRead = min(remainingBytes, bytesPerRead);

            file.seekg(bytesRead);
            file.read(buffer.data() + bytesRead, bytesToRead);
            bytesRead += bytesToRead;

            player.play(buffer.data() + bytesRead - bytesToRead, bytesToRead);
        }

        file.close();
    }
    else {
        std::cerr << "Error: Could not open the WAV file." << std::endl;
    }
}

void WavFileHandler::ReadWavFile(const CString& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::in);

    if (file.is_open()) {
        file.read((char*)&wavHeader, sizeof(WavHeader));
        BufferedMusicPlayer player(wavHeader);
        player.start();
        ReadDataAndBufferMusic(filePath, player);
        player.stop();
        file.close();
    }
    else {
        std::cerr << "Error: Could not open the WAV file." << std::endl;
    }
}
