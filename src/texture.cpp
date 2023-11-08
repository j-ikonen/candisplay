#include "texture.h"

#include <stdexcept>
#include <SDL2/SDL_image.h>


enum TargetLib {
  kSdl,
  kImg,
  kTTF
};

Texture::Texture() {

}


Texture::~Texture() {
  Free();
}


bool Texture::LoadFromFile(SDL_Renderer *renderer, std::string fname) {
  SDL_Surface *image = IMG_Load(fname.c_str());
  CheckError(kImg, image == nullptr, "Unable to load image");

  SDL_Texture *img_texture = SDL_CreateTextureFromSurface(renderer, image);
  CheckError(kImg, img_texture == nullptr, "Unable to create a texture from the image");
  SDL_FreeSurface(image);
  return texture_ != nullptr;
}


bool Texture::LoadFromRenderedText(SDL_Renderer *renderer, 
                                   std::string text, 
                                   SDL_Color textcolor, 
                                   TTF_Font *font) {
  Free();

  SDL_Surface *textsurface = TTF_RenderText_Solid(font, text.c_str(), textcolor);
  if (textsurface == nullptr) {
    std::string str = "Unable to render text surface\n";
    str += TTF_GetError();
    throw std::runtime_error(str);
  } else {
    texture_ = SDL_CreateTextureFromSurface(renderer, textsurface);
    if (texture_ == nullptr) {
      std::string str = "Unable to create texture from rendered text\n"; 
      str += SDL_GetError();
      throw std::runtime_error(str);
    } else {
      width_ = textsurface->w;
      height_ = textsurface->h;
    }
    SDL_FreeSurface(textsurface);
  }
  return texture_ != nullptr;
}

void Texture::Free() {
  SDL_DestroyTexture(texture_);
  texture_ = nullptr;
}


void Texture::SetColor(uint8_t red, uint8_t green, uint8_t blue) {
  SDL_SetTextureColorMod(texture_, red, green, blue);
}


void Texture::SetBlendMode(SDL_BlendMode blending) {
 SDL_SetTextureBlendMode(texture_, blending);
}


void Texture::SetAlpha(uint8_t alpha) {
 SDL_SetTextureAlphaMod(texture_, alpha);
}


void Texture::Render(SDL_Renderer *rend, int x, int y, SDL_Rect *clip, 
                     double angle, SDL_Point *center, 
                     SDL_RendererFlip flip) {
  SDL_Rect render_quad = {x, y, 800, 480};
  SDL_RenderCopyEx(rend, texture_, clip, &render_quad, angle, center, flip);
}


int Texture::GetWidth() {
  return width_;
}


int Texture::GetHeight() {
  return height_;
}

void Texture::CheckError(int where, bool check, std::string msg) {
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
