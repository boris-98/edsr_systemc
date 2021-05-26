#include "Generator.hpp"

using namespace std;
using namespace tlm;
using namespace sc_core;
using namespace sc_dt;

Generator::Generator(sc_module_name name) : sc_module(name)//, cache_mem("Cache")
{
    // SC_METHOD(Test);
    dram = new DRAM_data("DRAM");   // Pravim novi DRAM_data
    pb = new PB("PB");              // Pravim novi PB
    c = new cache("Cache");         // Pravim novi Cache
    memory = new WMEM("Memory");    // Pravim novi WMEM
    c->DRAM_cache_port.bind(*dram);
    pb->pb_cache_port.bind(*c);
    c->cache_pb_port.bind(*pb);
    c->WMEM_cache_port.bind(*memory);
    pb->pb_WMEM_port.bind(*memory);
    pb->done_pb_cache.bind(signal_channel);
    c->done_pb_cache.bind(signal_channel);
    gen_soc.bind(c->PROCESS_soc);
    signal_channel.write(true); // Inicijalizacija za done
    cout << "Kreiran je Generator!" << endl;
}

/*
void Generator::Test()
{

    pl_t pl;
    uint64 address;
    sc_time offset;
    tlm_command cmd;
    // unsigned char* data;
    unsigned int length;

    // Upis start_address_address

    cout << "Zapocinjem upis u start_address_address" << endl;

    address = START_ADDRESS_ADDRESS;
    unsigned int data_saa = 15390;
    unsigned int* saa = &data_saa;
    length = 20;
    cmd = TLM_WRITE_COMMAND;

    pl.set_command(cmd);
    pl.set_address(address);
    pl.set_data_ptr(reinterpret_cast<unsigned char*> (saa));
    pl.set_data_length(length);
    gen_soc->b_transport(pl, offset);
}
*/
