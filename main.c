
#include <stdio.h>
#include <stdint.h>
#include <math.h>

// header fields
struct WavHeader{
    uint32_t numsamples; // srate*(unsigned long)(1000/lengthms);
     
    uint32_t ChunkID; // "RIFF"
    uint32_t ChunkSize; // 36+a.Subchunk2Size
    uint32_t Format; // "WAVE" 
    uint32_t Subchunk1ID; //"fmt"
    uint32_t Subchunk1Size; // usually 16
    uint16_t AudioFormat; // 1 = pcm
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate; // samplerate*numchannels*bitdepth)/8
    uint16_t BlockAlign; // (numchannels*bitdepth)/8    
    uint16_t BitsPerSample;
    uint32_t Subchunk2ID; // "data"
    uint32_t Subchunk2Size; // (numsamples*numchannels*bitdepth)/8
    
    uint32_t header[11];       
};

// assuming machine is little-endian, byteswap text id fields back to big-endian 
uint32_t swap4(uint32_t num){
    return ((num>>24)&0xff) | ((num<<8)&0xff0000) | ((num>>8)&0xff00) | ((num<<24)&0xff000000);
}
//
uint16_t swap2(uint16_t num){
    return (num>>8) | (num<<8);
}
// concatenate two words and swap to little-endian 
uint32_t concat_flip(uint16_t a, uint16_t b){
    return b<<16 | a;
}

// set header 
struct WavHeader setParams(int numchannels, int bitdepth, int srate, unsigned int lengthms){
    struct WavHeader a;
    
    a.numsamples = srate*(unsigned long)(1000/lengthms);   
    // set fields
    a.Subchunk2Size = (a.numsamples*numchannels*bitdepth)/8; ; //adjust if dynamic length
    a.Subchunk2ID = swap4(0x64617461); //"data"
    a.BitsPerSample = bitdepth;
    a.BlockAlign = (numchannels*bitdepth)/8;
    a.ByteRate = (srate*numchannels*bitdepth)/8;
    a.SampleRate = srate;
    a.NumChannels = numchannels;
    a.AudioFormat = 1;
    a.Subchunk1Size = 16;
    a.Subchunk1ID = swap4(0x666d7420); //"fmt"
    a.Format = swap4(0x57415645); //"WAVE"
    a.ChunkSize = 36 + a.Subchunk2Size; //adjust if dynamic length
    a.ChunkID = swap4(0x52494646); //"RIFF"
    // create header     
    a.header[0] = a.ChunkID;
    a.header[1] = a.ChunkSize;
    a.header[2] = a.Format; 
    a.header[3] = a.Subchunk1ID;
    a.header[4] = a.Subchunk1Size;
    a.header[5] = concat_flip(a.AudioFormat, a.NumChannels);
    a.header[6] = a.SampleRate;
    a.header[7] = a.ByteRate;
    a.header[8] = concat_flip(a.BlockAlign, a.BitsPerSample);  
    a.header[9] = a.Subchunk2ID;
    a.header[10] = a.Subchunk2Size;
    
    return a;   
}

// write callback prototype
typedef void(*writecb)(uint8_t* data, unsigned long num_bytes, unsigned int frame_size);

// write wav file
void writeWav(const char* fstring, writecb cb, int numchannels, int bitdepth, int srate, unsigned int lengthms){
   
   struct WavHeader a = setParams(numchannels, bitdepth, srate, lengthms);  
   uint8_t data[a.Subchunk2Size]; 
    
   FILE *f;
   f = fopen(fstring, "wb" );
   // write header
   fwrite(a.header, 4 , 22 , f );
   // fill data
   cb(data, a.Subchunk2Size, a.BitsPerSample/8);
   // write data
   fwrite(data, 1, a.Subchunk2Size, f);
   fclose(f);
       
}

double phase = 0; 
double step = 3.141592653589*440/44100.0;

void writeCallBack(uint8_t* data, unsigned long num_bytes, unsigned int frame_size){

    for(unsigned int i = 0; i < num_bytes; i+=frame_size){
         uint16_t sig = (uint16_t)(sin(phase)*5000);
         phase += step;
         data[i] = sig & 0xff;
         data[i+1] = sig >> 8; 
    }   
    
}

int main(int argc, char** argv) {     
    writeWav("thewav.wav", writeCallBack, 1, 16, 44100, 1000);
    return 0;
}

