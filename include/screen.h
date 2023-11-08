#ifndef SCREEN_H
#define SCREEN_H

#include <vector>
#include <string>
#include <cstdint>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "texture.h"

class Screen {
 public:
  Screen();
  ~Screen();

  void Init();
  void LoadMedia();
  void Render();

 private:
  const int kScreenWidth_;
  const int kScreenHeight_;
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  TTF_Font* font_;
  std::vector<Texture> texturevec_;

  void CheckError(int where, bool check, std::string msg);
};

#endif // SCREEN_H