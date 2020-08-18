#include "fs_dev.hpp"
#include <fstream>
#include <iostream>
#include <string>

bool openDevice_write_and_read()
{
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
        std::cout <<fs.gcount() << " bytes could be read";

    // ...buffer contains the entire file...

        delete[] buffer;
        // fs >> rd;

        // std::cout << "Read string " << rd << " with size (w. nullterm) " << rd.size()+1 << std::endl;

    }

    return ret;
}
