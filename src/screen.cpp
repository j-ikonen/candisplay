#include "screen.h"

#include <stdexcept>

#include <SDL2/SDL_image.h>

enum TargetLib {
  kSdl,
  kImg,
  kTTF
};

Screen::Screen() : kScreenWidth_(800), kScreenHeight_(480), 
                   window_(nullptr), renderer_(nullptr) {}

Screen::~Screen() {
  for (auto t : texturevec_) {
    t.Free();
  }
  texturevec_.clear();
  TTF_CloseFont(font_);
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
  
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}

void Screen::Init() {
  CheckError(kSdl, SDL_Init(SDL_INIT_VIDEO) != 0, "Unable to initialize SDL");
  
  window_ = SDL_CreateWindow("CAN-Display", 
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             kScreenWidth_, kScreenHeight_,
                             SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
  CheckError(kSdl, window_ == nullptr, "Unable to create window");

  renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | 
                                              SDL_RENDERER_PRESENTVSYNC);
  CheckError(kSdl, renderer_ == nullptr, "Unable to create renderer");

  // Init SDL_image
  int flags = IMG_INIT_JPG | IMG_INIT_PNG;
  int imginit = IMG_Init(flags);
  CheckError(kImg, (flags & imginit) != flags, "Unable to initialize SDL_image");

  // Init SDL_ttf
  CheckError(kTTF, TTF_Init() != 0, "Unable to initialize SDL_ttf");

} // Screen::Init()


void Screen::LoadMedia() {
  font_ = TTF_OpenFont("resources/16_true_type_fonts/lazy.ttf", 28);
  CheckError(kTTF, font_ == nullptr, "Unable to open font");
  SDL_Color color = {0, 0, 0, 0};
  Texture txtr;
  CheckError(kTTF, txtr.LoadFromRenderedText(renderer_, "Test text! - 123.09", 
                                             color, font_),
             "Unable to load texture from rendered text");
  texturevec_.push_back(txtr);
} // Screen::LoadMedia()


void Screen::Render() {
  SDL_SetRenderDrawColor(renderer_, 10, 10, 25, 255);
  SDL_RenderClear(renderer_);

  // GO THROUGH ITEMLIST TO RENDER
  texturevec_[0].Render(renderer_, 10, 10);


  SDL_RenderPresent(renderer_);
  SDL_Delay(5000);
}


void Screen::CheckError(int where, bool check, std::string msg) {
  if (check) {
    std::string str = msg + " ";
    switch (where) {
      case kSdl: str += SDL_GetError(); break;
      case kImg: str += IMG_GetError(); break;
      case kTTF: str += TTF_GetError(); break;
      default: break;
    } // switch
    throw std::runtime_error(str);
  } // if check
} // Screen::CheckError()


