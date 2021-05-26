#include "Cache.hpp"
#include <tlm>
#include <string>

using namespace std;
using namespace sc_core;
using namespace tlm;
using namespace sc_dt;

cache::cache(sc_module_name name) :
    sc_channel(name),
    //DRAM_soc("DRAM_soc"),
    PROCESS_soc("PROCESS_soc")
{
    //PB_soc.register_b_transport(this, &cache::b_transport_pb);
    PROCESS_soc.register_b_transport(this, &cache::b_transport_proc);

    SC_THREAD(write);
    sensitive << start_event;
    // dont_initialize();
    SC_THREAD(read);
    sensitive << start_read;
    // dont_initialize();

    /* ----------------------------------- */
    max_x = DATA_HEIGHT;
    max_y = DATA_WIDTH;

    /* ----------------------------------- */

    for(int i = 0; i < 9; i++)
        write_en[i] = false;

    // done_pb_cache.write(true);
    cout << "Cache::Napravljen je Cache modul!" << endl;
}

// Implementacija funkcije za kompresiju stapica podataka
void cache::compress_data_stick(type* data_stick, type* cache_mem_pos, unsigned char* compressed_stick_index, unsigned char &compressed_stick_lenght)
{
    unsigned char length = 0;
    for(int i = 0; i < DATA_DEPTH; i++)
    {
        if(data_stick[i] != 0)
        {
            // [0, 1, 1, 0, 0, 0, -1] -> 1, 1, -1 | 1, 2, 6
            *(cache_mem_pos + length) = data_stick[i];
            compressed_stick_index[length] = i;
            length++;
        }
    }

    compressed_stick_lenght = length;
}

void cache::copy_cache_line(type* cache_mem_read_line, unsigned char &stick_lenght, type* cache_mem_start_address)
{
    for(int i = 0; i < stick_lenght; i++)
    {
        cache_mem_read_line[i] = *(cache_mem_start_address + i);
    }
}

// proces
void cache::write()
{

    // Inicijalno

    DRAM_cache_port->read_DRAM_cache(&start_address, start_address_address); // Citam podatke iz DRAM-a
    unsigned char compressed_stick_index[DATA_DEPTH]; // Niz u kom se nalaze indeksi nenultih elemenata unutar jednog stika
    unsigned char compressed_stick_length; // Ovde ce se upisati duzina kompresovanog niza podataka
    type* stick_data; // Pokazivac na stapic podataka

    // Prvo treba napuniti linije kesa
    // cout << "Cache::Upisujem prvobitne podatke!" << endl;
    // x : 0 -> 6 | y : 0 -> 5 | dubina je 8 || (0, 0) -> 0, (0, 1) -> 8, (0, 2) -> 16
    // cout << "Cache::max_x je: " << max_x << endl;
    int cnt = 0;
    for(int i = 0; i < CACHE_SIZE; i++)
    {
        DRAM_cache_port->read_DRAM_cache(&stick_data, start_address[i / (CACHE_SIZE / 2)] + cnt * DATA_DEPTH); // 0 - 63; (0, 0) | 64 - 127; (0, 1) | 128 - 255; (0, 2)...
        compress_data_stick(stick_data, cache_mem + i * DATA_DEPTH, compressed_stick_index, compressed_stick_length);
        cache_line_length[i] = compressed_stick_length; // Upisi i duzinu linije kesa u neku memoriju
        address_hash[i] = (i / (CACHE_SIZE / 2)) * max_y + cnt; // 0, 1, 2, 3, 4
        if(cnt == 0)
        {
            amount_hash[i] = 2 * W_kn;  // U hardveru petlja 'kn' je u potpunosti ramotana pa ne treba *kn, ovo je samo zbog softverskog modela
        }
        else
        {
            if(cnt == DATA_WIDTH - 1)
            {
                amount_hash[i] = 2 * W_kn;
            }
            else
            {
                amount_hash[i] = 3 * W_kn;
            }
        }

        cnt = (cnt + 1) % (CACHE_SIZE / 2); // 0, 1, 2, 3, 4, 0, 1, 2, ...
        write_en[i] = false;

        WMEM_cache_port->write_cache_WMEM(compressed_stick_index, compressed_stick_length,
                                          address_hash[i], i);   // upisuje potrebne informacije u WMEM
    }

    // Sad za ostatak dvoreda


    for(unsigned int y_i = CACHE_SIZE / 2; y_i < max_y; y_i++)
    {
        for(unsigned int d = 0; d < 2; d++)
        {
            unsigned char full = 0;

            // Proveravamo da li ima slobodnih mesta za upis u kes
            for(int i = 0; i < CACHE_SIZE; i++)
            {
                full += (unsigned char)write_en[i];
            }

            if(!full)
            {
                cout << "Cache::Write ceka na slobodno mesto!" << endl;
                wait(write_enable); // Ovo treba da se desava samo ako je cache pun
            }


            // Trazi koja je prva slobodna linija
            unsigned char free_cache_line;
            for(int i = 0; i < CACHE_SIZE; i++)
            {
                if(write_en[i])
                {
                    free_cache_line = i;
                    break;
                }
            }

            cout << "Cache::Write cita podatak: " << "(" << d << ", " << y_i << ")" << endl;

            DRAM_cache_port->read_DRAM_cache(&stick_data, start_address[d] + y_i * DATA_DEPTH);
            compress_data_stick(stick_data, cache_mem + free_cache_line * DATA_DEPTH, compressed_stick_index, compressed_stick_length);
            *(cache_line_length + free_cache_line) = compressed_stick_length; // cache_line_length[free_cache_line] = ...
            address_hash[free_cache_line] = d * max_y + y_i;
            write_en[free_cache_line] = false;

            switch(y_i)
            {
                case 0:
                case Y_LIMIT1:
                case Y_LIMIT2:
                    {
                        amount_hash[free_cache_line] = 2 * W_kn;
                    }
                    break;

                default:
                    {
                        amount_hash[free_cache_line] = 3 * W_kn;
                    }
                    break;

            }

            // Iscitaj koji stick zeli read da procita
            unsigned int temp_x = stick_address_cache >> 32;
            unsigned int temp_y = stick_address_cache & 0x00000000ffffffff;

            // Ako je upisa stick koji read zeli, onda odblokiraj read
            if(temp_x * max_y + temp_y == d * max_y + y_i)
                read_enable.notify();

            cout << "CACHE::adresa koja se salje WMEM-u je: " << to_string(address_hash[free_cache_line]) << endl;
            WMEM_cache_port->write_cache_WMEM(compressed_stick_index, compressed_stick_length,
                                              address_hash[free_cache_line], free_cache_line);   // upisuje potrebne informacije u WMEM
        }
    }


    // Sada normalno

    for(unsigned int x_i = 0; x_i < max_x - 1; x_i++)
    {
        for(unsigned int y_i = 0; y_i < max_y; y_i++)
        {
            for(unsigned int d = 0; d < 3; d++)
            {

                if(!((x_i == max_x - 2) && (d == 2)))
                {
                    unsigned char full = 0;

                    // Proveravamo da li ima slobodnih mesta za upis u kes
                    for(int i = 0; i < CACHE_SIZE; i++)
                    {
                        full += (unsigned char)write_en[i];
                    }

                    if(!full)
                    {
                        cout << "Cache::Write ceka na slobodno mesto!" << endl;
                        wait(write_enable); // Ovo treba da se desava samo ako je cache pun
                    }


                    // Trazi koja je prva slobodna linija
                    unsigned char free_cache_line;
                    for(int i = 0; i < CACHE_SIZE; i++)
                    {
                        if(write_en[i])
                        {
                            free_cache_line = i;
                            break;
                        }
                    }

                    cout << "Cache::Write cita podatak: " << "(" << x_i + d << ", " << y_i << ")" << endl;

                    DRAM_cache_port->read_DRAM_cache(&stick_data, start_address[x_i + d] + y_i * DATA_DEPTH);
                    compress_data_stick(stick_data, cache_mem + free_cache_line * DATA_DEPTH, compressed_stick_index, compressed_stick_length);
                    *(cache_line_length + free_cache_line) = compressed_stick_length; // cache_line_length[free_cache_line] = ...
                    address_hash[free_cache_line] = (x_i + d) * max_y + y_i;
                    write_en[free_cache_line] = false;

                    switch(y_i)
                    {
                        case 0:
                        case Y_LIMIT1:
                        case Y_LIMIT2:
                            {
                                amount_hash[free_cache_line] = 2 * W_kn;
                            }
                            break;

                        default:
                            {
                                amount_hash[free_cache_line] = 3 * W_kn;
                            }
                            break;

                    }

                    // Iscitaj koji stick zeli read da procita
                    unsigned int temp_x = stick_address_cache >> 32;
                    unsigned int temp_y = stick_address_cache & 0x00000000ffffffff;

                    // Ako je upisa stick koji read zeli, onda odblokiraj read
                    if(temp_x * max_y + temp_y == (x_i + d) * max_y + y_i)
                        read_enable.notify();

                    cout << "CACHE::adresa koja se salje WMEM-u je: " << to_string(address_hash[free_cache_line]) << endl;
                    WMEM_cache_port->write_cache_WMEM(compressed_stick_index, compressed_stick_length,
                                                      address_hash[free_cache_line], free_cache_line);   // upisuje potrebne informacije u WMEM
                }
            }
        }
    }
}

// proces
void cache::read()
{
    bool flag = true;

    while(true)
    {
        if(flag)
        {
            done_pb_cache.write(false);
            flag = false;
        }

        // cout << "Cekam koji podatak PB zeli!" << endl;

        cout << "Cache::Read se zaustavio!" << endl;
        wait(); // ceka dok ne dobije neki podatak
        cout << "Cache::Read je nastavio sa radom!" << endl;

        // cout << "PB je rekao svoje!" << endl;

        // Izdvoji x i y adrese
        unsigned int x = stick_address_cache >> 32;
        unsigned int y = stick_address_cache & 0x00000000ffffffff;

        cout << "Cache::Od read se traze podaci: (" << x << ", " << y << ")" << endl;

        // Trazi na kojoj liniji kesa se nalazi podatak adresiran sa x i y
        /* --------------------------------------------- */

        unsigned int cache_line = INVALID_ADDRESS;
        for(int i = 0; i < CACHE_SIZE; i++)
        {
            if(address_hash[i] == x * max_y + y)
                cache_line = i;
        }


        // Proveri da li si nasao ipak taj podatak
        if(cache_line == INVALID_ADDRESS)
        {
            cout << "Cache::Invalidna adresa!" << endl;
            wait(read_enable); // write proces mora da digne event ako je upisao podataka sa adresom x * x_max + y, u stick_address_cache se nalaze koje x i y trazim
            // Write mora da javi read gde je upisao taj podatak
            cache_line = write_cache_line; // ovu promenljivu odredjuje write
        }

        cout << "Cache::Linija kesa na kojoj se nalazi trazeni podatak: " << cache_line << endl;

        /* --------------------------------------------- */

        // Umanji za jedan iskoristivost podatka
        amount_hash[cache_line]--;
        cout << "Cache::Nakon ovog read, amount_hash je: " << to_string(amount_hash[cache_line]) << endl;

        /* --------------------------------------------- */

        // Proveri da li moze write da upisuje novu liniju
        if(amount_hash[cache_line] == 0)
        {
            write_en[cache_line] = true;
            write_enable.notify();
        }

        // Upisi lokaciju prvog elementa niza
        type* stick_data_cache = cache_mem + cache_line * DATA_DEPTH;
        unsigned char stick_length = cache_line_length[cache_line];
        copy_cache_line(cache_mem_read_line, stick_length, stick_data_cache);
        stick_data_cache = cache_mem_read_line;
        cache_pb_port->write_cache_pb(&stick_data_cache, stick_length); // Ovako saljemo podatke ka PB

        bool done = done_pb_cache.read();
        done_pb_cache.write(!done); // Ne koristimo bukvalno vrednost done signala, vec nam sluzi za generisanje dogadjaja

    }
}

/* ----------------------------------------------------------------------------------- */
/* Implementacija PB <-> Cache interfejsa */

void cache::write_pb_cache(const uint64 &stick_address)
{
    stick_address_cache = stick_address; // Upisi x i y koje PB trazi
    start_read.notify();
}

/* ----------------------------------------------------------------------------------- */

void cache::b_transport_proc(pl_t& pl, sc_time& offset)
{

    uint64 address = pl.get_address();
    tlm_command cmd = pl.get_command();

    switch(cmd)
    {
        case TLM_READ_COMMAND:
        {
            switch(address)
            {
                case START_ADDRESS_ADDRESS: // Ova situacija je samo za debug
                {
                    unsigned int* data = reinterpret_cast<unsigned int*>(pl.get_data_ptr());
                    *data = start_address_address;

                    pl.set_response_status(TLM_OK_RESPONSE);

                    // offset += sc_time(CLK_PERIOD, SC_NS);

                    break;
                }

                case ACK:
                {
                    unsigned char* data = pl.get_data_ptr();
                    *data = (unsigned char)ack;
                    pl.set_response_status(TLM_OK_RESPONSE);

                    offset += sc_time(50 * CLK_PERIOD, SC_NS);

                    break;
                }
                default:

                    pl.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
                    cout << "Cache::Error while trying to read data" << endl;

                    break;
            }

            break;
        }

        case TLM_WRITE_COMMAND:
        {
            switch(address)
            {
                case START_ADDRESS_ADDRESS:
                {
                    start_address_length = pl.get_data_length();
                    start_address_address = *(reinterpret_cast<unsigned int*>(pl.get_data_ptr()));
                    pl.set_response_status(TLM_OK_RESPONSE);

                    cout << "Cache::" << start_address_address << endl;
                    cout << start_address_length << endl;

                    ack = true;

                    offset += sc_time(50 * CLK_PERIOD, SC_NS); // Samo adresa se salje, a ne cela tabela

                    break;
                }
                case START:
                {
                    start_event.notify();
                    pl.set_response_status(TLM_OK_RESPONSE);

                    offset += sc_time(50 * CLK_PERIOD, SC_NS);

                    break;
                }
                case MAX_X:
                {
                    max_x = *(reinterpret_cast<unsigned int*>(pl.get_data_ptr()));
                    pl.set_response_status(TLM_OK_RESPONSE);

                    offset += sc_time(50* CLK_PERIOD, SC_NS);

                    break;
                }
                case MAX_Y:
                {
                    max_y = *(reinterpret_cast<unsigned int*>(pl.get_data_ptr()));
                    pl.set_response_status(TLM_OK_RESPONSE);

                    offset += sc_time(50 * CLK_PERIOD, SC_NS);

                    break;
                }
                default:

                    pl.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
                    cout << "Cache::Error while trying to write data" << endl;

                    break;
            }

            break;

        }
        default:

            pl.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
            cout << "Cache::Wrong command!" << endl;

            break;


    }
}
