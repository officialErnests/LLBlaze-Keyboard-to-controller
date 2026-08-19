#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <linux/uinput.h>
#include <libevdev/libevdev.h>
#include <ostream>
#include <unistd.h>
#include <fcntl.h>
#include "main.h"
#include <vector>
#include <string.h>

using namespace std;

void relAll(EMULATOR p_emul);

int main() {
    struct libevdev *dev = NULL;
    
    vector<int> devices;
    vector<libevdev*> devices_pointers;
    bool found_device = true;
    int t_iterator = 0;
    while (found_device) {
        found_device = isKeyboard(&devices, &devices_pointers, dev, t_iterator);
        t_iterator ++;
    }

    cout << to_string(devices.size()) << endl;
    if (devices.size() == 0) {
        cout << "devices not found";
        return 0;
    }

    EMULATOR emul = EMULATOR();
    emul.initKeyboards(devices_pointers.size());

    int rc;
    bool end = false;
    while (!end) {
        for (size_t i = 0; i < devices_pointers.size(); i++) {
            libevdev *dev = devices_pointers[i];
            struct input_event ev;
            rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
            if (rc == 0 && ev.type == EV_KEY)  {
                cout << "Device: " << i << " | Event: " << libevdev_event_code_get_name(ev.type, ev.code) << " | state: "<< ev.value << endl;
                if (ev.code == KEY_Q) {
                    cout << "Bye bye ;PP";
                    end = true;
                    break;
                }
                if (ev.code == KEY_U) {
                    libevdev_grab(dev, LIBEVDEV_GRAB);
                } else if (ev.code == KEY_I) {
                    libevdev_grab(dev, LIBEVDEV_UNGRAB);
                }
                if (ev.code == KEY_P) {
                    relAll(emul);
                }
                if (ev.value == 2) {continue;}
                emul.emulateManager(i, ev.code, ev.value);
            }
        }
    }
    return 0;
}

bool isKeyboard(std::vector<int>* p_devices, std::vector<libevdev*> *p_device_pointer, libevdev* dev, int p_device_num) {
    int fd;
    int rc = 1;
    cout << "NEW SCAN ON:" << p_device_num << endl;
    fd = open(("/dev/input/event" + to_string(p_device_num)).c_str(), O_RDONLY|O_NONBLOCK);
    cout << fd << endl;
    rc = libevdev_new_from_fd(fd, &dev);
    cout << rc << endl;
    if (rc < 0) {
        cout << "OOPS YOU FUCKED UP XDD" << endl;
        return false;
    }
    printf("input device name: \"%s\"\n", libevdev_get_name(dev));
    printf("input device id: bus %#x vendor %#x product %#x\n",
           libevdev_get_id_bustype(dev),
           libevdev_get_id_vendor(dev),
           libevdev_get_id_product(dev));
    if (!libevdev_has_event_type(dev, EV_KEY) ||
        !libevdev_has_event_code(dev, EV_KEY, KEY_A)) {
            printf("This device does not look like a keyboard\n");
            return true;
    }
    p_devices->push_back(rc);
    p_device_pointer->push_back(dev);
    printf("FOUNDDDD!!");
    return true;
}

EMULATOR::EMULATOR() {

    // =========================
    // 1. CREATION
    // =========================

    for (auto& iter_contr : {&CON0, &CON1, &CON2}){
        int &fd = iter_contr->fd;

        fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

        if (fd < 0) {
            std::cerr << "Failed to open /dev/uinput\n";
            failed_init = true;
            return;
        }
        // Enable buttons
        ioctl(fd, UI_SET_EVBIT, EV_KEY);
        // Buttons
        ioctl(fd, UI_SET_KEYBIT, BTN_SOUTH); // A
        ioctl(fd, UI_SET_KEYBIT, BTN_EAST);  // B
        ioctl(fd, UI_SET_KEYBIT, BTN_WEST);  // X
        ioctl(fd, UI_SET_KEYBIT, BTN_NORTH); // Y
        
        ioctl(fd, UI_SET_KEYBIT, BTN_START); // start

        // D-pad
        ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_UP);
        ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_DOWN);
        ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_LEFT);
        ioctl(fd, UI_SET_KEYBIT, BTN_DPAD_RIGHT);

        uinput_setup &setup = iter_contr->conf;

        setup.id.bustype = BUS_USB;
        setup.id.vendor  = 0x1234;
        setup.id.product = 0x5678;
        setup.id.version = 1;

        strncpy(setup.name, "AAAAAAAAAAAAAAAAAAAAAAAAA",
                UINPUT_MAX_NAME_SIZE);

        ioctl(fd, UI_DEV_SETUP, &setup);

        // Create the virtual controller
        ioctl(fd, UI_DEV_CREATE);
    }
}

void EMULATOR::initKeyboards(int count) {
    cout << "Initing keybvaords:" << count << endl;
    keyboards.resize(count);
}

void EMULATOR::emulateManager(int p_device, int key, int action){
    if (key != KEY_E && 
            key != KEY_R &&
            key != KEY_T &&
            key != KEY_Y &&
            keyboards[p_device] == nullptr) {return;}
    int fd = 0;
    if (keyboards[p_device] != nullptr) {
        fd = keyboards[p_device]->fd;
        cout << "AA: " << fd << endl;
    }  
    input_event ev{};
    switch(key) {
        case KEY_E:
            keyboards[p_device] = &CON0;
            break;
        case KEY_R:
            keyboards[p_device] = &CON1;
            break;
        case KEY_T:
            keyboards[p_device] = &CON2;
            break;
        case KEY_Y:
            keyboards[p_device] = nullptr;
            break;
        case KEY_V:
            ev.type = EV_KEY;
            ev.code = BTN_START;
            ev.value = action;
            (void)write(fd, &ev, sizeof(ev));

            // Tell Linux the event is complete
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            (void)write(fd, &ev, sizeof(ev));
            break;
        case KEY_Z:
            ev.type = EV_KEY;
            ev.code = BTN_WEST;
            ev.value = action;
            (void)write(fd, &ev, sizeof(ev));

            // Tell Linux the event is complete
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            (void)write(fd, &ev, sizeof(ev));
            break;
        case KEY_X:
            ev.type = EV_KEY;
            ev.code = BTN_EAST;
            ev.value = action;
            (void)write(fd, &ev, sizeof(ev));

            // Tell Linux the event is complete
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            (void)write(fd, &ev, sizeof(ev));
            break;
        case KEY_C:
            ev.type = EV_KEY;
            ev.code = BTN_NORTH;
            ev.value = action;
            (void)write(fd, &ev, sizeof(ev));

            // Tell Linux the event is complete
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            (void)write(fd, &ev, sizeof(ev));
            break;
        case KEY_SPACE:
            ev.type = EV_KEY;
            ev.code = BTN_SOUTH;
            ev.value = action;
            (void)write(fd, &ev, sizeof(ev));

            // Tell Linux the event is complete
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            (void)write(fd, &ev, sizeof(ev));
            break;
        case KEY_LEFT:
            ev.type = EV_KEY;
            ev.code = BTN_DPAD_LEFT;
            ev.value = action;
            (void)write(fd, &ev, sizeof(ev));

            // Tell Linux the event is complete
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            (void)write(fd, &ev, sizeof(ev));
            break;
            break;
        case KEY_DOWN:
            ev.type = EV_KEY;
            ev.code = BTN_DPAD_DOWN;
            ev.value = action;
            (void)write(fd, &ev, sizeof(ev));

            // Tell Linux the event is complete
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            (void)write(fd, &ev, sizeof(ev));
            break;
            break;
        case KEY_UP:
            ev.type = EV_KEY;
            ev.code = BTN_DPAD_UP;
            ev.value = action;
            (void)write(fd, &ev, sizeof(ev));

            // Tell Linux the event is complete
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            (void)write(fd, &ev, sizeof(ev));
            break;
            break;
        case KEY_RIGHT:
            ev.type = EV_KEY;
            ev.code = BTN_DPAD_RIGHT;
            ev.value = action;
            (void)write(fd, &ev, sizeof(ev));

            // Tell Linux the event is complete
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            (void)write(fd, &ev, sizeof(ev));
            break;
            break;
    }
    return; 
}

EMULATOR::~EMULATOR() {
    for (auto& iter_contr : {&CON0, &CON1, &CON2}){
        int &fd = iter_contr->fd;
        ioctl(fd, UI_DEV_DESTROY);

        close(fd);
    }
}

void relAll(EMULATOR p_emul) {
    for (auto& iter_contr : {&p_emul.CON0, &p_emul.CON1, &p_emul.CON2}){
        releaseAll(iter_contr->fd);
    }
    return;
}

void releaseAll(int fd)
{
    int buttons[] = {
        BTN_SOUTH,
        BTN_EAST,
        BTN_WEST,
        BTN_NORTH,
        BTN_START,
        BTN_SELECT,
        BTN_THUMBL,
        BTN_THUMBR,
        BTN_DPAD_UP,
        BTN_DPAD_DOWN,
        BTN_DPAD_LEFT,
        BTN_DPAD_RIGHT
    };

    for (int button : buttons) {
        input_event ev{};

        ev.type = EV_KEY;
        ev.code = button;
        ev.value = 0;

        (void)write(fd, &ev, sizeof(ev));
    }

    input_event syn{};
    syn.type = EV_SYN;
    syn.code = SYN_REPORT;

    (void)write(fd, &syn, sizeof(syn));
}
