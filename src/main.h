#include <libevdev/libevdev.h>
#include <linux/uinput.h>
#include <vector>
#pragma once
bool isKeyboard(std::vector<int>* p_device, std::vector<libevdev*> *p_device_pointer, libevdev* dev, int p_device_num);
void releaseAll(int fd);    

class EMULATOR {
    public:
        EMULATOR();
        ~EMULATOR();

        struct COOL {
            uinput_setup conf{};
            int fd;
        } CON0, CON1, CON2;

        bool failed_init = false;

        void initKeyboards(int count);
        void emulateManager(int p_device, int key, int action);
    private:
        std::vector<COOL*> keyboards;
};

