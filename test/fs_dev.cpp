#include "fs_dev.hpp"
#include <fstream>
#include <iostream>
#include <string>

#include <chrono>
#include <thread>
#include <fcntl.h>
#include <unistd.h>

bool openDevice_write_and_wait_5s()
{
    std::cout << __func__ << std::endl;
    bool ret = false;
    int f = open("/dev/pcm3060-0",O_RDWR); //fstream has internal buffers-> direct linux access


    if (f < 0)
    {
        std::cout << "Could not open device!" << std::endl;
    }
    else
    {
        using namespace std::this_thread;     // sleep_for, sleep_until
        using namespace std::chrono_literals; // ns, us, ms, s, h, etc.

        char s[] = "testor";


        std::cout << "Writting " <<  write(f, s, 6) << std::endl;
        std::cout << "Write finished" << std::endl;
        sleep_for(5s);

        close(f);
        // fs >> rd;

        // std::cout << "Read string " << rd << " with size (w. nullterm) " << rd.size()+1 << std::endl;
        ret = true;
    }
    return ret;
}
bool openDevice_write_and_read()
{
    std::cout << __func__ << std::endl;
    bool ret = false;
    std::fstream fs ("/dev/pcm3060-0");


    if (!fs)
    {
        std::cout << "Could not open device!" << std::endl;
    }
    else
    {
        std::string s = "testor";
        char * buffer = new char [s.size()+1];


        std::cout << "Writting string of size (w. nullterm) " << s.size()+1 << std::endl;
        fs << s;
        std::cout << "Reading" << std::endl;

        fs.read (buffer,s.size()+1);
        size_t nBytesRead = fs.gcount();
        std::cout << nBytesRead << " bytes could be read" << std::endl;
        std::string out(buffer,nBytesRead);
        std::cout << "Read" << out << std::endl;

        delete[] buffer;
        // fs >> rd;

        // std::cout << "Read string " << rd << " with size (w. nullterm) " << rd.size()+1 << std::endl;

    }

    return ret;
}
