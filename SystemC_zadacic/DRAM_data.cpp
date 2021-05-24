#include "DRAM_data.hpp"

using namespace std;
using namespace sc_core;

DRAM_data::DRAM_data(sc_module_name name) : sc_channel(name)
{
    cout << "DRAM::Konstruisan je DRAM_data!" << endl;
    // Upisati te podatke stvarno u memorije

    for(int i = 0; i < DATA_DEPTH * DATA_HEIGHT * DATA_WIDTH; i++)
    {
        dram_data[i] = i;
    }

    cout << "DRAM::Upisani podaci u DRAM_data" << endl;

    for(int i = 0; i < DATA_HEIGHT; i++)
    {
        dram_table[i] = i * DATA_WIDTH * DATA_DEPTH; // x = 0, x = 1, x = 2,
    }

    cout << "DRAM::Upisani podaci u DRAM_table" << endl;
}

void DRAM_data::read_DRAM_cache(type** stick_data, const unsigned int &address)
{
    cout << "DRAM::Citanje za stapice na adresi: " << address << endl;
    *stick_data = &dram_data[address];
}

void DRAM_data::read_DRAM_cache(unsigned int** address_data, const unsigned int &address)
{
    cout << "DRAM::Citanje za table" << endl;
    *address_data = &dram_table[0];
    // cout << address_data << endl;
}
