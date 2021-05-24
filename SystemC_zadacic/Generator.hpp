#ifndef GENERATOR_HPP_INCLUDED
#define GENERATOR_HPP_INCLUDED

#include "common.hpp"
// #include "Cache.hpp"
#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include "DRAM_data.hpp"
#include "PB.hpp"
#include "Cache.hpp"

class Generator : public sc_core::sc_module
{
    public:

        SC_HAS_PROCESS(Generator);
        Generator(sc_core::sc_module_name);

        // sc_core::sc_port<DRAM_cache_if> DRAM_port;
        tlm_utils::simple_initiator_socket<cache> gen_soc;

    protected:

        sc_core::sc_signal<bool> signal_channel;
        DRAM_data* dram;
        PB* pb;
        cache* c;
        // void Test();
        typedef tlm::tlm_base_protocol_types::tlm_payload_type pl_t;
};

#endif // GENERATOR_HPP_INCLUDED
