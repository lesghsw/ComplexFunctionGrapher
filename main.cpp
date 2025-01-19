#include <math.h>
#include <complex>
#include <thread>
#include <SDL2/SDL.h>

using namespace std::complex_literals;

int WID = 1000;
int HGT = 800;
int MAXT = 12;

struct LES_hsl {
    float h = 0.0f, s = 0.0f, l = 0.0f;
    LES_hsl() {}
    LES_hsl(float th, float ts, float tl) {
        h = th;
        s = ts;
        l = tl;
    }
};

struct fv2d {
    float x = 0.0f, y = 0.0f;
    fv2d() {}
    fv2d(float tx, float ty) {
        x = tx;
        y = ty;
    }
};

struct iv2d {
    int x = 0, y = 0;
    iv2d() {}
    iv2d(int tx, int ty) {
        x = tx;
        y = ty;
    }
};

iv2d getMousePos(SDL_Window* wn) {
    int xMouse, yMouse, xWin, yWin;
    SDL_GetGlobalMouseState(&xMouse, &yMouse);
    SDL_GetWindowPosition(wn, &xWin, &yWin);
    int trslPX = (xMouse - xWin), trslPY = (yMouse - yWin);
    return { trslPX >= WID - 1 || trslPX < 1 || trslPY >= HGT - 1 || trslPY < 1 ? -1 : trslPX, trslPY };
}

float funcHtR(LES_hsl color, float n) {
    float k = fmod((double)n + (double)color.h / 30.0f, 12);
    float a = color.s * fmin(color.l, 1.0 - (double)color.l);
    return color.l - a * fmax(-1.0f, fmin((double)k - 3.0, fmin(9.0 - (double)k, 1.0f)));
}

void HSLtRGB(LES_hsl color, SDL_Color &out) { //(360, 1, 1) -> (255, 255, 255)
    out.r = (Uint8)(funcHtR(color, 0.0f) * 255.0f);
    out.g = (Uint8)(funcHtR(color, 8.0f) * 255.0f);
    out.b = (Uint8)(funcHtR(color, 4.0f) * 255.0f);
}

fv2d Scal = { 60.0f, 60.0f };
fv2d Off = { -WID / 2.0f / Scal.x, -HGT / 2.0f / Scal.y };
fv2d SPan = { 0.0f, 0.0f };

float hueOff = 0.5f;

bool mHeld = 0;

void mouseClicked(iv2d mPos) {
    SPan.x = mPos.x;
    SPan.y = mPos.y;
}

void mouseDragged(iv2d mPos) {
    Off.x -= (mPos.x - SPan.x) / Scal.x;
    Off.y -= (mPos.y - SPan.y) / Scal.y;

    SPan.x = mPos.x;
    SPan.y = mPos.y;
}

void WtS(fv2d W, iv2d &S) {
    S.x = (int)((W.x - Off.x) * Scal.x);
    S.y = (int)((W.y - Off.y) * Scal.y);
}
void StW(iv2d S, fv2d &W) {
    W.x = (float)S.x / Scal.x + Off.x;
    W.y = (float)S.y / Scal.y + Off.y;
}

/// TODO: Rewrite everything to use double precision

std::complex<float> lanczos_g(7);
std::complex<float> lanczos_p[] = {
    {676.5203681,0},
    {-1259.1392167,0},
    {771.3234287,0},
    {-176.6150291,0},
    {12.5073432,0},
    {-0.1385710,0},
    {9.9843695780195716e-6,0},
    {1.5056327351493116e-7,0}
};

std::complex<float> gamma(std::complex<float> z) {
    std::complex<float> t;
    std::complex<float> pi(M_PI, 0);
    std::complex<float> one(1, 0);
    std::complex<float> half(0.5, 0);
    std::complex<float> ci(1, 0);
    std::complex<float> x(1,0);
    if (z.real() < 0.5)
        return pi / (sin(pi * z) * gamma(one - z));
    else {
        z -= one;
        for (int i = 0; i < 8; i+=1) {
            x += lanczos_p[i] / (z + ci);
            ci+=one;
        }
        t = z + lanczos_g + half;
        return sqrt((one+one) * pi) * pow(t, (z + half)) * exp(-t) * x;
    }
}

std::complex<float> complexFunc(std::complex<float> z) {
    //return ((z*z-1.0f)*(z-2.0f-1if)) / (z*z+2.0f+2if);
    //return sqrt(1.0f-z*z);
    return gamma(z);
}

void wZU(SDL_Window* wn, short ZU) {
    iv2d mPos = getMousePos(wn);

    fv2d MWbZ;
    StW({ mPos.x, mPos.y }, MWbZ);

    if (ZU) Scal = { 1.04f * Scal.x, 1.04f * Scal.y };
    else  Scal = { 0.96f * Scal.x, 0.96f * Scal.y };

    mPos = getMousePos(wn);

    fv2d MWaZ;
    StW({ mPos.x, mPos.y }, MWaZ);

    Off.x += MWbZ.x - MWaZ.x;
    Off.y += MWbZ.y - MWaZ.y;
}

void fCalcRN(SDL_Surface* outsf, int bWID, int eWID, unsigned short rnType) {
    for (int y = 0; y < HGT; y++) {
        for (int x = bWID; x < eWID; x++) {
            iv2d Screen = { x, y };
            fv2d World;
            StW(Screen, World);

            std::complex<float> temp(World.x, -World.y);
            temp = complexFunc(temp);

            float narg = 1.0f - (std::arg(temp) + M_PI) / (M_PI * 2) + hueOff;
            if (narg > 1.0f) narg -= 1.0f;

            float abs = std::abs(temp);

            LES_hsl hslColor;
            if (rnType == 0) hslColor = { narg * 360.0f, 1.0f, std::tanhf(SDL_logf(0.1f * abs + 1.3f) - 0.1f) };
            else if (rnType == 1) {
                if (abs > 0.2183f) hslColor = { narg * 360.0f, 1.0f, std::tanhf(0.2f * std::abs(1.0f / SDL_sinf(M_PI * abs))) - 0.1f };
                else hslColor = { narg * 360.0f, 1.0f, abs*abs*abs * 42.12f + -abs*abs * 25.3f + abs * 4.0f + 0.1f };
            }
            else {
                if (abs > 0.3314f) hslColor = { narg * 360.0f, 1.0f, std::tanhf(0.4f * SDL_abs(1.0f / SDL_sinf(M_PI * abs))) - 0.15f };
                else hslColor = { narg * 360.0f, 1.0f, abs * abs * abs * 42.0f -abs * abs * 32.915f + abs * 7.3f };
            }
            SDL_Color rgbColor;
            HSLtRGB(hslColor, rgbColor);
            
            //OPTIMIZE THIS!
            unsigned int* row = (unsigned int*)((char*)outsf->pixels + outsf->pitch * y);
            row[x] = SDL_MapRGBA(outsf->format, rgbColor.r, rgbColor.g, rgbColor.b, 255);
        }
    }
}

int main(int argc, char **argv) {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *wn = SDL_CreateWindow("Complex graph", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WID, HGT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_Surface *wnsf = SDL_GetWindowSurface(wn);
    SDL_Surface *outsf = SDL_CreateRGBSurfaceWithFormat(0x0, WID, HGT, 32, SDL_PIXELFORMAT_RGBX8888);
    SDL_Event event;
    std::thread t[MAXT];
    unsigned short rnType = 0;
    bool tHeld = 0, run = 1;
    while (run) {
        SDL_LockSurface(outsf);
        
        while (SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) run = 0;
            else if (event.type == SDL_MOUSEWHEEL) {
                if (event.wheel.y > 0) wZU(wn, 0x1);
                else wZU(wn, 0x0);
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_q) run = 0;
                else if (event.key.keysym.sym == SDLK_t) {
                    if (!tHeld) rnType = (rnType + 1) % 3;
                    tHeld = 1;
                }
            }
            else if (event.type == SDL_KEYUP)
                if (event.key.keysym.sym == SDLK_t) tHeld = 0;
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                mouseClicked(getMousePos(wn));
                mHeld = 1;
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
                mHeld = 0;
            if (event.type == SDL_MOUSEMOTION && mHeld) {
                iv2d mPos = getMousePos(wn);
                if (mPos.x > 0) mouseDragged(mPos);
            }
            
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                WID = event.window.data1;
                HGT = event.window.data2;
                free(outsf);
                outsf = SDL_CreateRGBSurfaceWithFormat(0x0, WID, HGT, 32, SDL_PIXELFORMAT_RGBX8888);
                SDL_DestroyWindowSurface(wn);
                wnsf = SDL_GetWindowSurface(wn);
            }
        }
        int partWid = WID / MAXT;
        int start = 0;
        for (int i = 0; i < MAXT-1; i++) {
            t[i] = std::thread(fCalcRN, outsf, start, start + partWid, rnType);
            start += partWid;
        }
        t[MAXT-1] = std::thread(fCalcRN, outsf, start, WID, rnType);
        for (int i = 0; i < MAXT; i++)
            t[i].join();
        
        SDL_UnlockSurface(outsf);
        SDL_BlitSurface(outsf, 0x0, wnsf, 0x0);
        SDL_UpdateWindowSurface(wn);
        SDL_Delay(1000.0f / 60.0f);
    }
    
    SDL_DestroyWindowSurface(wn);
    SDL_DestroyWindow(wn);
    free(outsf);
    SDL_Quit();
    return 0;
}
