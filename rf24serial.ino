#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// NB: This sketch assumes 32 bit addresses

static long int address = 0x66996699L;  // So that's 0x0066996699
const int payload = 16;  // 32 is supported but not very reliable

RF24 rf(/*ce*/ 8, /*cs*/ 10);

static union {
    unsigned char buf[36];
    struct {
        uint32_t address;
        unsigned char message[payload];
    } packet;
} in;

unsigned char hexdigit(byte c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
}

void setup() {
    Serial.begin(115200);
    rf.begin();
    rf.setRetries(15, 15);
    rf.enableDynamicPayloads();
    rf.openReadingPipe(1, address);
    rf.startListening();
}



void loop() {
    static int inpos = 0;  // nibble position
    static bool uriencoded = false;
    bool ok;
    unsigned char buf[payload];
    unsigned char c, h;

    while (Serial.available()) {
        c = Serial.read();
        if (c == '\r' || c == '\n') {
            if (inpos > sizeof(in.packet.address)) {
                in.packet.message[0] = inpos / 2 - 4 - 1;
                rf.openWritingPipe(in.packet.address);
                rf.stopListening();
                rf.write(in.packet.message, inpos / 2 - 4);
                rf.startListening();
            }
            inpos = 0;
            memset(in.buf, 0, sizeof(in.buf));
            uriencoded = false;
        }
        else if (inpos > sizeof(in)) {
            // silently ignore :-)
        }
        else if (inpos < 2 * sizeof(in.packet.address)) {
            h = hexdigit(c);
            if ((inpos % 2) == 0) h <<= 4;
            in.buf[inpos / 2] |= h;
            inpos++;
        }
        else if (inpos >= 2 * sizeof(in.packet.address)) {
            h = hexdigit(c);
            if ((inpos % 2) == 0) h <<= 4;
            in.buf[inpos / 2] |= h;
            inpos++;
        }
    }

    while (rf.available()) {
        if (rf.read(&buf, sizeof(buf))) {
            hexdump(&buf[1], buf[0]);
        }
    }
}

void hexdump(unsigned char* string, int size) {
  for (int i = 0; i < size; i++) {
    int n = string[i] >> 4;
    if (n < 10)  Serial.write('0' + n);
    if (n >= 10) Serial.write('A' + (n - 10));
    n = string[i] & 0xF;
    if (n < 10)  Serial.write('0' + n);
    if (n >= 10) Serial.write('A' + (n - 10));
  }
  Serial.print("\r\n");
}





// vim: ft=cpp
