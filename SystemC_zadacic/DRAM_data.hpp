#ifndef DRAM_DATA_HPP_INCLUDED
#define DRAM_DATA_HPP_INCLUDED

#include <systemc>
#include "interfaces.hpp"
#include "common.hpp"

class DRAM_data :
    public sc_core::sc_channel,
    public DRAM_cache_if
{
    public:

        DRAM_data(sc_core::sc_module_name);

    protected:

        type dram_data[DATA_DEPTH * DATA_WIDTH * DATA_HEIGHT]; // Sadrzi podatke o ulazu
        unsigned int dram_table[DATA_HEIGHT]; // Sadrzi podatke o pocetnim adresama

        void read_DRAM_cache(type** stick_data, const unsigned int &address);
        void read_DRAM_cache(unsigned int** address_data, const unsigned int &address);
};


#endif // DRAM_DATA_HPP_INCLUDED
