#include "Weights_mem.hpp"

using namespace std;
using namespace sc_core;


WMEM::WMEM(sc_module_name name) : sc_channel(name)
{
    cout << "WMEM::Konstruisan je WMEM!" << endl;

}

void WMEM::write_WMEM_cache(const unsigned char* compressed_stick_address, const unsigned char &compressed_stick_address_length,
                            const unsigned int &address, const unsigned int &cache_line)
{
    for(int i = 0; i < compressed_stick_address_length; i++)
    {
        *(compressed_index_memory + cache_line * DATA_DEPTH) = compressed_stick_address[i];
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
            cache_line = i;
    }

    compressed_index_length = 0;

    for(int i = 0; i < (int)compressed_index_len[cache_line]; i++)
    {
        weights_stick[i] = W[kn][kh][kw][compressed_index_memory[cache_line * DATA_DEPTH + i]];
        compressed_index_length++;
    }

    *compressed_weights = weights_stick;
}



