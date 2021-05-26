#ifndef CACHE_HPP_INCLUDED
#define CACHE_HPP_INCLUDED

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/simple_initiator_socket.h>
#include "common.hpp"
#include "interfaces.hpp"

#define START_ADDRESS_ADDRESS 0x1
#define START 0x2
#define ACK 0x3
#define MAX_X 0x4
#define MAX_Y 0x5
#define CLK_PERIOD 10
#define INVALID_ADDRESS -1

// Dodati jos jedan header sa zajednickim stvarima

class cache :
    public sc_core::sc_channel,
    public pb_cache_if
{
    public:

        SC_HAS_PROCESS(cache);
        cache(sc_core::sc_module_name);

        /* DRAM <-> Cache interface */
        sc_core::sc_port<DRAM_cache_if> DRAM_cache_port;

        /* WMEM <-> Cache interface */
        sc_core::sc_port<WMEM_cache_if> WMEM_cache_port;

        /* PB <-> Cache interface */
        sc_dt::uint64 stick_address_cache; // Sadrzi konkatenirane x i y indekse za trazeni stapic podataka
        sc_core::sc_out<bool> done_pb_cache;
        sc_core::sc_port<cache_pb_if> cache_pb_port;

        /* Processor <-> Cache interface */
        tlm_utils::simple_target_socket<cache> PROCESS_soc;

    protected:

        void compress_data_stick(type* data_stick, type* cache_mem_pos, unsigned char* compressed_stick_index, unsigned char &compressed_stick_lenght); // kompresuje podatke
        void copy_cache_line(type* cache_mem_read_line, unsigned char &stick_lenght, type* cache_mem_start_address);

        // Deklaracija write i read funkcija za hijararhijski kanal
        void write_pb_cache(const sc_dt::uint64 &stick_address);

        // Procesi
        void read();
        void write();

        // Dogadjaji potrebni za komunikaciju izmedju procesa
        sc_core::sc_event read_enable;
        sc_core::sc_event write_enable;
        sc_core::sc_event start_event;
        sc_core::sc_event start_read;

        /* Interconnedct <-> Cache interface */
        unsigned int start_address_address; // Pocetna adresa za tabelu pocetnih adresa
        unsigned int start_address_length;
        bool ack; // ACK bit za potvrdu ispravnosti primljenog podatka
        unsigned int max_x; // Sadrzi vrednost broja redoav u slici
        unsigned int max_y; // 124 ili 496

        // Unutrasnji resursi
        type cache_mem[CACHE_SIZE * DATA_DEPTH];
        unsigned int address_hash[CACHE_SIZE]; // x, y -> x * max(x) + y = x * 3 + y = 2 * x + x + y = (x << 1) + x + y
        unsigned char amount_hash[CACHE_SIZE];
        unsigned char cache_line_length[CACHE_SIZE];
        unsigned int* start_address; // Cuva pocetne adrese blokova podataka
        bool write_en[CACHE_SIZE]; // Govori kada i koju liniju kesa moze da upisuje write
        unsigned int write_cache_line;
        unsigned int x, y; // one ce da sacuvaju x i y dobijene preko b_transport_pb
        type cache_mem_read_line[DATA_DEPTH];

        // TLM
        typedef tlm::tlm_base_protocol_types::tlm_payload_type pl_t;
        void b_transport_pb(pl_t&, sc_core::sc_time&);
        void b_transport_proc(pl_t&, sc_core::sc_time&);
};


#endif // CACHE_HPP_INCLUDED
