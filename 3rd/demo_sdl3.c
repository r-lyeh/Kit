#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

int main(int argc, char **argv) {
    int ret = SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("HelloWorld SDL3", 640, 480, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    SDL_RenderPresent(renderer);
    SDL_Delay(2000);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return ret;
}
