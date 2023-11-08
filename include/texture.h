#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


class Texture {
 public:
  Texture();
  ~Texture();

  bool LoadFromFile(SDL_Renderer *renderer, std::string fname);
  bool LoadFromRenderedText(SDL_Renderer *renderer, 
                            std::string text, 
                            SDL_Color textcolor, 
                            TTF_Font *font);

  void Free();

  void SetColor(uint8_t red, uint8_t green, uint8_t blue);
  void SetBlendMode(SDL_BlendMode blending);
  void SetAlpha(uint8_t alpha);

  void Render(SDL_Renderer *rend, int x, int y, 
              SDL_Rect *clip = nullptr, double angle = 0.0,
              SDL_Point *center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE);

  int GetWidth();
  int GetHeight();

  void CheckError(int where, bool check, std::string msg);

 private:
  SDL_Texture *texture_;
  int width_;
  int height_;
};

#endif // TEXTURE_H