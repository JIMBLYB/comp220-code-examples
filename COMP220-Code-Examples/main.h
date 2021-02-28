#pragma once

#include <iostream>
#include <SDL.h>
#include <gl\glew.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Shader.h"

bool ToggleWindowFullscreen(SDL_Window* window, bool fullscreened);

struct Vertex { float x, y, z, u = 0.0f, v = 0.0f; };

bool LoadModel(const char* filepath, std::vector<Vertex>& vertices, std::vector<unsigned>& indeces, std::string& texturePath);

int main(int argc, char** argsv);