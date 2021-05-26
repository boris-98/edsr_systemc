#include "Weights_mem.hpp"

using namespace std;
using namespace sc_core;


WMEM::WMEM(sc_module_name name) : sc_channel(name)
{
    cout << "WMEM::Konstruisan je WMEM!" << endl;


    int cnt = 0;
    for(int kn = 0; kn < W_kn; kn++)
        for(int kh = 0; kh < W_kh; kh++)
            for(int kw = 0; kw < W_kw; kw++)
                for(int kd = 0; kd < W_kd; kd++)
                {
                    W[kn][kh][kw][kd] = cnt;
                    cnt += 2;
                }

}

void WMEM::write_cache_WMEM(const unsigned char* compressed_stick_address, const unsigned char &compressed_stick_address_length,
                            const unsigned int &address, const unsigned int &cache_line)
{
    for(int i = 0; i < compressed_stick_address_length; i++)
    {
        compressed_index_memory[cache_line * DATA_DEPTH + i] = compressed_stick_address[i]; // npr. dubina 3, 0-2: prvi stapic, 3-5: drugi stapic...
    }

    data_stick_address[cache_line] = address;   // address sadrzi x * y_max + y
    compressed_index_len[cache_line] = compressed_stick_address_length;
}

void WMEM::read_pb_WMEM(type** compressed_weights, unsigned char &compressed_index_length, const unsigned int &x, const unsigned int &y,
                        const unsigned int &kn, const unsigned int &kh, const unsigned int &kw)
{
    int cache_line;
    type weights_stick[DATA_DEPTH];

    for(int i = 0; i < CACHE_SIZE; i++)
    {
        if(data_stick_address[i] == x * y_max + y)
            cache_line = i;                         // razmisliti o situaciji kada nema slobodne linije
    }

    compressed_index_length = 0;

    for(int i = 0; i < (int)compressed_index_len[cache_line]; i++)
    {
        weights_stick[i] = W[kn][kh][kw][compressed_index_memory[cache_line * DATA_DEPTH + i]];
        compressed_index_length++;
    }

    *compressed_weights = weights_stick;
}



