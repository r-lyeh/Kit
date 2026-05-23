#include "kit.h"

void main(event ev) {
    unsigned sample = audio.open(os.arg("--audio=demos/art/tada.wav"));
    if(!sample) os.die("cannot load audio file");

    float at[3] = {0,0,0};
    listener.position(at);

    float pos[3] = {10,0,10};
    unsigned source = speaker.open(pos);
    speaker.play(source, sample);

    double t = -elapsed.ss();

    while(speaker.playing(source)) {
        pos[0] = sin(elapsed.ss()) * 10;
        pos[2] = cos(elapsed.ss()) * 10;
        speaker.position(source, pos);
        sleep.ms(16);
    }

    t += elapsed.ss();
    printf("%5.2fs\n", t);

    speaker.close(&source);
    audio.close(&sample);

    app.quit(0);
}

const char *hints;
