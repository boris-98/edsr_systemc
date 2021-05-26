#include "PB.hpp"
#include <string>

using namespace std;
using namespace sc_core;

PB::PB(sc_module_name name) : sc_channel(name)
{
    SC_THREAD(conv2D);
    sensitive << done_pb_cache;
    dont_initialize();
    relu = true;

    for(int i = 0; i < W_kn; i++)     // inicijalizacija biasa za potrebe modelovanja
        bias[i] = 1;

    cout << "Konstruisan je PB" << endl;
}

void PB::write_cache_pb(type** stick_data, unsigned char &stick_lenght)
{
    data = *stick_data;
    data_length = stick_lenght;
}

void PB::conv2D()
{

    sc_dt::uint64 address; // 8 bajtova, 4 je za x, a 4 je za y
    type sum;   // promenljiva za akumulaciju rezultata jednog izlaznog piksela (64)
    type* weights;
    unsigned char w_length;

    for(int x = 0; x < DATA_HEIGHT; x++)
    {
        int temp_x_i = x - 1;

        for(int y = 0; y < DATA_WIDTH; y++)
        {
            int temp_y_i = y - 1;

            for(int kn = 0; kn < W_kn; kn++)
            {
                sum = 0;

                for(int kw = 0; kw < W_kw; kw++)
                {
                    int y_i = temp_y_i + kw;

                    for(int kh = 0; kh < W_kh; kh++)
                    {
                        int x_i = temp_x_i + kh;


                        cout << "PB::Apsolutne adrese: (" << x_i << ", " << y_i << ")" << endl;

                        if(!((x_i == -1) || (y_i == -1) || (x_i == DATA_HEIGHT) || (y_i == DATA_WIDTH)))
                        {
                            cout << "-----------------------------" << endl;

          //                  cout << "PB::Adresa stapica je: (" << x_i << ", " << y_i << ")" << endl;
                            address = 0;
                            address |= (sc_dt::uint64)(x_i) << 32;
                            address |= (sc_dt::uint64)(y_i);

                            pb_WMEM_port->read_pb_WMEM(&weights, w_length, x_i, y_i, kn, kh, kw);   // trazimo odgovarajuce tezine

                            cout << "PB::procitane tezine su: " << endl;
                            for(int i = 0; i < w_length; i++)
                                cout << weights[i] << endl;

                            pb_cache_port->write_pb_cache(address);     // zahtevamo podatke o ulazu iz kesa
                            cout << "PB::PB ceka na dogadjaj!" << endl;

                            wait(done_pb_cache.default_event());        // desava se na svaku ivicu signala done

                            cout << endl;
                            cout << "PB::Primljeni su podaci iz kesa!" << endl;



                            //   cout << "PB::Duzina tezina je: " << w_length << endl;

                            for(int kd = 0; kd < w_length; kd++)    // sam proracun
                                sum += data[kd] * weights[kd];

                            cout << "-----------------------------" << endl;

                        }
                    }
                }


                sum += bias[kn];

                if(relu)
                {
                    if(sum < 0)
                        OFM[x][y][kn] = 0;
                    else
                        OFM[x][y][kn] = sum;

                }
                else
                {
                    OFM[x][y][kn] = sum;

                }
            }
        }
    }

    cout << endl << endl << endl << endl;
    cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl << endl;

    for(int c = 0; c < W_kn; c++)
    {
        for(int x = 0; x < DATA_HEIGHT; x++)
        {
            for(int y = 0; y < DATA_WIDTH; y++)
            {
                cout << OFM[x][y][c] << " ";
            }
            cout << endl;
        }
        cout << endl;
    }


    cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
    cout << endl << endl << endl << endl;
}



