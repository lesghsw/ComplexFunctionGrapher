#include <math.h>
#include <complex>
#include <thread>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>

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
    return { trslPX >= WID - 1 || trslPX < 1 || trslPY >= HGT - 1 || trslPY < 1 ? -1 : trslPX, trslPY }; // returns negative value as x coordinate when mouse is outside window.
}

float funcHtR(LES_hsl color, float n) {
    float k = fmod(n + color.h / 30.0f, 12);
    float a = color.s * fmin(color.l, 1.0f - color.l);
    return color.l - a * fmax(-1.0f, fmin(k - 3.0, fmin(9.0 - k, 1.0f)));
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
bool checkbrdEnabled = 0;

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

std::complex<float> complexFunc(std::complex<float> z, float t) {
    return ((z*z-1.0f)*(z-2.0f-1if)) / (z*z+t+2if);
    //return sqrt(1.0f-z*z);
    //return gamma(z);
    //return sin(z);
    //return pow(gamma(z), gamma(z));
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

void fCalcRN(SDL_Surface* outsf, int bWID, int eWID, float t, unsigned short rnType) {
    const float threshold = 0.01f;
    for (int y = 0; y < HGT; y++) {
        for (int x = bWID; x < eWID; x++) {
            iv2d Screen = { x, y };
            fv2d World;
            StW(Screen, World);

            std::complex<float> temp(World.x, -World.y);
            temp = complexFunc(temp, t);

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
            
            if (checkbrdEnabled) {
                if (std::fabs(temp.real() - std::round(temp.real())) < threshold || std::fabs(temp.imag() -    std::round(temp.imag())) < threshold) {
                    rgbColor = {255, 255, 255};
                }
            }
            
            //OPTIMIZE THIS!
            unsigned int* row = (unsigned int*)((char*)outsf->pixels + outsf->pitch * y);
            row[x] = SDL_MapRGBA(outsf->format, rgbColor.r, rgbColor.g, rgbColor.b, 255);
        }
    }
}

void drawAxes(SDL_Surface* outsf) {
    SDL_Color axisColor = {255, 255, 255, 255};
    int centerX = (int)((-Off.x) * Scal.x);
    int centerY = (int)((-Off.y) * Scal.y);

    SDL_Rect xaxis = {0, centerY, WID, 1};
    SDL_FillRect(outsf, &xaxis, SDL_MapRGB(outsf->format, axisColor.r, axisColor.g, axisColor.b));

    SDL_Rect yaxis = {centerX, 0, 1, HGT};
    SDL_FillRect(outsf, &yaxis, SDL_MapRGB(outsf->format, axisColor.r, axisColor.g, axisColor.b));
    
    int tickSize = 7;

    for (float x = floor(Off.x); x <= ceil(Off.x + WID / Scal.x); x += 1.0f) {
       int xPos = (int)((x - Off.x) * Scal.x);
       SDL_Rect xTick = {xPos, centerY - tickSize / 2, 1, tickSize};
       SDL_FillRect(outsf, &xTick, SDL_MapRGB(outsf->format, axisColor.r, axisColor.g, axisColor.b));
    }

    for (float y = floor(Off.y); y <= ceil(Off.y + HGT / Scal.y); y += 1.0f) {
       int yPos = (int)((y - Off.y) * Scal.y);
       SDL_Rect yTick = {centerX - tickSize / 2, yPos, tickSize, 1};
       SDL_FillRect(outsf, &yTick, SDL_MapRGB(outsf->format, axisColor.r, axisColor.g, axisColor.b));
    }

}

void safelyQuit (SDL_Window *wn, SDL_Surface *outsf, TTF_Font *font) {
    if (wn) {
        SDL_DestroyWindowSurface(wn);
        SDL_DestroyWindow(wn);
    }
    if (outsf) {
        SDL_FreeSurface(outsf);
    }
    if (font) {
        TTF_CloseFont(font);
    }
    if (TTF_WasInit() == 1)
        TTF_Quit();
    if (SDL_WasInit(0) == 0)
        SDL_Quit();
}

int main(int argc, char **argv) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        exit(1); // Error couldn't init sdl2
    
    SDL_Window *wn = SDL_CreateWindow("Complex graph", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WID, HGT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!wn) {
        safelyQuit(wn, 0x0, 0x0);
        exit(2); // Error 2; couldn't create window
    }
    SDL_Surface *wnsf = SDL_GetWindowSurface(wn);
    SDL_Surface *outsf = SDL_CreateRGBSurfaceWithFormat(0x0, WID, HGT, 32, SDL_PIXELFORMAT_RGBX8888);
    if (!outsf) {
        safelyQuit(wn, outsf, 0x0);
        exit(3); // Error 3: couldn't create surface
    }
    SDL_Event event;
    
    if (TTF_Init() != 0) {
        safelyQuit(wn, outsf, 0x0);
        exit(4); // Error code 4: couldn't initialize TTF.
    }

    TTF_Font *font = TTF_OpenFont("Your Font Here.ttf", 18); // Adjust the path and font size
    if (!font) {
        safelyQuit(wn, outsf, font);
        exit(5); // Error code 5: couldn't load font.
    }
    
    SDL_Surface* mPosTextSurface;
    SDL_Surface* mPosFTextSurface;
    
    std::thread t[MAXT];
    unsigned short rnType = 0;
    bool tHeld = 0, aHeld = 0, pHeld = 0, cHeld = 0, axisEnabled = 0, textEnabled = 0, run = 1;
    
    std::stringstream mPosText;
    mPosText.precision(2);
    std::stringstream mPosFText;
    mPosFText.precision(2);
    mPosText << "z:";
    mPosFText << "f(z):";
    
    while (run) {
        SDL_LockSurface(outsf);
        
        while (SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) run = 0;
            else if (event.type == SDL_MOUSEWHEEL) {
                if (event.wheel.y > 0) wZU(wn, 0x1);
                else wZU(wn, 0x0);
            }
            if (event.type == SDL_KEYDOWN) { // Detect key preses and disable them until the key is released.
                if (event.key.keysym.sym == SDLK_q) run = 0;
                else if (event.key.keysym.sym == SDLK_t) {
                    if (!tHeld) rnType = (rnType + 1) % 3;
                    tHeld = 1;
                }
                else if (event.key.keysym.sym == SDLK_a) {
                    if (!aHeld) axisEnabled = !axisEnabled;
                    aHeld = 1;
                }
                else if (event.key.keysym.sym == SDLK_p) {
                    if (!pHeld) textEnabled = !textEnabled;
                    pHeld = 1;
                }
                else if (event.key.keysym.sym == SDLK_c) {
                    if (!cHeld) checkbrdEnabled = !checkbrdEnabled;
                    cHeld = 1;
                }
            }
            else if (event.type == SDL_KEYUP) { // Enable key presses again
                if (event.key.keysym.sym == SDLK_t) tHeld = 0;
                if (event.key.keysym.sym == SDLK_a) aHeld = 0;
                if (event.key.keysym.sym == SDLK_p) pHeld = 0;
                if (event.key.keysym.sym == SDLK_c) cHeld = 0;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                mouseClicked(getMousePos(wn));
                mHeld = 1;
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
                mHeld = 0;
            if (event.type == SDL_MOUSEMOTION) {
                iv2d mPos = getMousePos(wn);
                
                if (mPos.x > 0) {
                    if (textEnabled) {
                        fv2d mWPos;
                        StW(mPos, mWPos); // Get mouse world pos.
                        
                        mPosText.str(std::string());
                        mPosText.clear();
                        mPosText << "z: (" << mWPos.x << ", " << mWPos.y << "i)"; // Prepare mouse pos string
                        
                        std::complex<float> mFPos = complexFunc({mWPos.x, -mWPos.y}, 0);
                        
                        mPosFText.str(std::string());
                        mPosFText.clear();
                        mPosFText << "f(z): (" << mFPos.real() << ", " << mFPos.imag() << "i)"; // Prepare string to show f(mPos)
                    }
                    if (mHeld) { mouseDragged(mPos); } // Check if mouse is inside the window to drag (Check getMousePos for explanation)
                }
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
            t[i] = std::thread(fCalcRN, outsf, start, start + partWid, 0, rnType);
            start += partWid;
        }
        t[MAXT-1] = std::thread(fCalcRN, outsf, start, WID, 0, rnType);
        for (int i = 0; i < MAXT; i++)
            t[i].join();
        
        if (axisEnabled)
            drawAxes(outsf);
        
        SDL_UnlockSurface(outsf);
        SDL_BlitSurface(outsf, 0x0, wnsf, 0x0);
        
        if (textEnabled) {
            // Create text (z and f(z) respectively)
            mPosTextSurface = TTF_RenderText_Solid(font, mPosText.str().c_str(), {255, 255, 255});
            if (!mPosTextSurface) {
                safelyQuit(wn, outsf, font);
                exit(6); // error 6: Couldn't create text surface
            }
            mPosFTextSurface = TTF_RenderText_Solid(font, mPosFText.str().c_str(), {255, 255, 255});
            if (!mPosTextSurface) {
                safelyQuit(wn, outsf, font);
                exit(6); // error 6: Couldn't create text surface
            }
            
            // Blit and free text z
            SDL_Rect textLocation = {10, 10, mPosTextSurface->w, mPosTextSurface->h};
            SDL_BlitSurface(mPosTextSurface, NULL, wnsf, &textLocation);
            SDL_FreeSurface(mPosTextSurface);
            // Blit and free text f(z)
            SDL_Rect textFLocation = {10, 30, mPosFTextSurface->w, mPosFTextSurface->h};
            SDL_BlitSurface(mPosFTextSurface, NULL, wnsf, &textFLocation);
            SDL_FreeSurface(mPosFTextSurface);
        }
        
        SDL_UpdateWindowSurface(wn);
        SDL_Delay(1000.0f / 60.0f);
    }
    
    safelyQuit(wn, outsf, font);
    return 0;
}
