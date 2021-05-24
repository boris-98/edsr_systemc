#include "PB.hpp"
#include <string>

using namespace std;
using namespace sc_core;

PB::PB(sc_module_name name) : sc_channel(name)
{
    SC_THREAD(Test_proc);
    sensitive << done_pb_cache;
    dont_initialize();
    cout << "Konstruisan je PB" << endl;
}

void PB::write_cache_pb(type** stick_data, unsigned char &stick_lenght)
{
    data = *stick_data;
    data_length = stick_lenght;
}

void PB::Test_proc()
{

    sc_dt::uint64 address; // 8 bajtova, 4 je za x, a 4 je za y

    for(int x = 0; x < DATA_HEIGHT - 2; x++)
    {
        for(int y = 0; y < DATA_WIDTH - 2; y++)
        {
            for(int kw = 0; kw < 3; kw++)
            {
                for(int kh = 0; kh < 3; kh++)
                {
                    // cout << endl;
                    cout << "-----------------------------" << endl;

                    cout << "PB::Adresa stapica je: (" << x + kh << ", " << y + kw << ")" << endl;
                    address = 0;
                    address |= (sc_dt::uint64)(x + kh) << 32;
                    address |= (sc_dt::uint64)(y + kw);

                    // cout << "PB::Adresa koja se salje je: " << (address >> 32) << " " << (address & 0x00000000ffffffff) << endl;

                    // wait(done_pb_cache.default_event()); // Desava se na opadajucu ivicu done signala
                    pb_cache_port->write_pb_cache(address);
                    cout << "PB::PB ceka na dogadjaj!" << endl;

                    wait(done_pb_cache.default_event()); // Desava se na rastucu ivicu done signala

                    // cout << "PB::Primljen je podatak iz kesa" << endl;
                    cout << endl;
                    // cout << "-----------------------------" << endl;
                    cout << "PB::Primljeni podaci su" << endl;

                    for(unsigned char i = 0; i < data_length; i++)
                        cout << data[i] << endl;

                    cout << endl;
                    cout << "PB::Njihova duzina je: " << to_string(data_length) << endl;
                    // cout << "-----------------------------" << endl;
                    // cout << endl;

                    cout << "-----------------------------" << endl;
                    cout << endl;
                }
            }
        }
    }
}



