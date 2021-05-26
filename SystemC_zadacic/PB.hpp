#ifndef PB_HPP_INCLUDED
#define PB_HPP_INCLUDED

#include <systemc>
#include "common.hpp"
#include "interfaces.hpp"

class PB :
    public sc_core::sc_channel,
    public cache_pb_if
{
    public:

        SC_HAS_PROCESS(PB);
        PB(sc_core::sc_module_name);

        sc_core::sc_port<pb_cache_if> pb_cache_port; // Port za komunikaciju sa Cache
        sc_core::sc_port<pb_WMEM_if> pb_WMEM_port;   // Port za komunikaciju sa WMEM
        sc_core::sc_in<bool> done_pb_cache;

    protected:

        type* data;
        unsigned char data_length;
        bool relu;
        type OFM[DATA_HEIGHT][DATA_WIDTH][W_kn];    // ovo nije deo hardvera (sluzi samo za modelovanje)
        type bias[W_kn];                            // u hardveru ce biti biasi za sve konvolucije i neophodno je da procesor posalje info o kojoj konvoluciji se radi
        void conv2D();                              // implementira proracun konvolucije
        void write_cache_pb(type** stick_data, unsigned char &stick_lenght);
};

#endif // PB_HPP_INCLUDED
