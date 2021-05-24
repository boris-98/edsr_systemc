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
        sc_core::sc_in<bool> done_pb_cache;

    protected:

        type* data;
        unsigned char data_length;
        void Test_proc(); // Sluzi za testiranje Cache modula
        void write_cache_pb(type** stick_data, unsigned char &stick_lenght);
};

#endif // PB_HPP_INCLUDED
