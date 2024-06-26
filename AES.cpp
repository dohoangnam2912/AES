#include <iostream>
#include <vector>
#include <chrono>

using namespace std;
using namespace std::chrono;

double MeasureEncryptionRate(vector<unsigned char> &data, vector<vector<unsigned char>> &roundKeys) {
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    int chunkSize = 4096;
    int bytesEncrypted = 0;

    while (bytesEncrypted < data.size()) {
        int remainingBytes = data.size() - bytesEncrypted;
        int chunkBytes = min(chunkSize, remainingBytes);

        vector<unsigned char> chunk(data.begin() + bytesEncrypted, data.begin() + bytesEncrypted + chunkBytes);
        AESEncrypt(chunk, roundKeys);

        bytesEncrypted += chunkBytes;
    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    duration<double, std::micro> time_span = duration_cast<duration<double, std::micro>>(t2 - t1);

    double encryptionSpeed = static_cast<double>(bytesEncrypted) / time_span.count();
    return encryptionSpeed;
}

const unsigned char sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x1, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x4, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x5, 0x9a, 0x7, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x9, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x0, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x2, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0xc, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0xb, 0xdb,
    0xe0, 0x32, 0x3a, 0xa, 0x49, 0x6, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x8,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x3, 0xf6, 0xe, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0xd, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0xf, 0xb0, 0x54, 0xbb, 0x16
};

const unsigned char Rcon[10] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

vector<vector<unsigned char>> KeyExpansion(vector<unsigned char> key) {
    int Nb = 4;
    int Nk = key.size() / 4;
    int Nr = Nk + 6;

    vector<vector<unsigned char>> w(Nb * (Nr + 1), vector<unsigned char>(4));

    for (int i = 0; i < Nk; ++i) {
        for (int j = 0; j < 4; ++j) {
            w[i][j] = key[i * 4 + j];
        }
    }

    for (int i = Nk; i < Nb * (Nr + 1); ++i) {
        vector<unsigned char> temp(4);
        for (int j = 0; j < 4; ++j) {
            temp[j] = w[i - 1][j];
        }

        if (i % Nk == 0) {
            unsigned char temp_byte = temp[0];
            temp[0] = sbox[temp[1]];
            temp[1] = sbox[temp[2]];
            temp[2] = sbox[temp[3]];
            temp[3] = sbox[temp_byte];
            temp[0] ^= Rcon[i / Nk - 1];
        } else if (Nk > 6 && i % Nk == 4) {
            for (int j = 0; j < 4; ++j) {
                temp[j] = sbox[temp[j]];
            }
        }

        for (int j = 0; j < 4; ++j) {
            w[i][j] = w[i - Nk][j] ^ temp[j];
        }
    }

    return w;
}

void SubBytes(vector<unsigned char> &state) {
    for (int i = 0; i < 16; ++i) {
        state[i] = sbox[state[i]];
    }
}

void ShiftRows(vector<unsigned char> &state) {
    unsigned char temp;

    temp = state[1];
    state[1] = state[5];
    state[5] = state[9];
    state[9] = state[13];
    state[13] = temp;

    temp = state[2];
    state[2] = state[10];
    state[10] = temp;
    temp = state[6];
    state[6] = state[14];
    state[14] = temp;

    // Shift row 3
    temp = state[15];
    state[15] = state[11];
    state[11] = state[7];
    state[7] = state[3];
    state[3] = temp;
}

unsigned char xtime(unsigned char x) {
    return (x << 1) ^ ((x & 0x80) ? 0x1b : 0x00);
}

void MixColumns(vector<unsigned char> &state) {
    for (int i = 0; i < 4; ++i) {
        unsigned char s0 = state[i * 4];
        unsigned char s1 = state[i * 4 + 1];
        unsigned char s2 = state[i * 4 + 2];
        unsigned char s3 = state[i * 4 + 3];

        state[i * 4] = xtime(s0) ^ (s1 ^ xtime(s1)) ^ (s2 ^ s3);
        state[i * 4 + 1] = (s0 ^ xtime(s1)) ^ xtime(s2) ^ (s3 ^ xtime(s3));
        state[i * 4 + 2] = (s0 ^ s1) ^ xtime(s2) ^ (xtime(s3) ^ s3);
        state[i * 4 + 3] = (xtime(s0) ^ s0) ^ (s1 ^ s2) ^ xtime(s3);
    }
}

void AddRoundKey(vector<unsigned char> &state, vector<unsigned char> &roundKey) {
    for (int i = 0; i < 16; ++i) {
        state[i] ^= roundKey[i];
    }
}

vector<unsigned char> AESEncrypt(vector<unsigned char> input, vector<vector<unsigned char>> &w) {
    vector<unsigned char> state(input);
    int Nb = 4;
    int Nr = w.size() / 4 - 1;

    AddRoundKey(state, w[0]);

    for (int round = 1; round < Nr; ++round) {
        SubBytes(state);
        ShiftRows(state);
        MixColumns(state);
        AddRoundKey(state, w[round * Nb]);
    }

    SubBytes(state);
    ShiftRows(state);
    AddRoundKey(state, w[Nr * Nb]);

    return state;
}

int main() {
    vector<unsigned char> key = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                  0xab, 0xf7, 0x97, 0x17, 0x4, 0x7e, 0x46, 0x2e};
    vector<unsigned char> input = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
                                    0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x7,  0x34};

    vector<vector<unsigned char>> roundKeys = KeyExpansion(key);
    vector<unsigned char> ciphertext = AESEncrypt(input, roundKeys);

    cout << "Please enter your cipher text:" << endl;
    for (auto &byte : ciphertext) {
        cout << hex << (int)byte << " ";
    }
    cout << endl;

    return 0;
}
