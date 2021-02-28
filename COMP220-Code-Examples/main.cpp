#include "main.h"

/// <summary>
/// Toggles a window between fullscreen and windowed modes
/// </summary>
/// <param name="window"> The window to toggle </param>
/// <param name="currentState"> If the window is currently fullscreened or not </param>
/// <returns></returns>
bool ToggleWindowFullscreen(SDL_Window* window, bool fullscreened)
{
	SDL_SetWindowFullscreen(window, !fullscreened);

	// Turns cursor off while in fullscreen
	// SDL_ShowCursor(currentState);

	return !fullscreened;
}

bool LoadModel(const char* filepath, std::vector<Vertex>& vertices, std::vector<unsigned>& indeces, std::string& texturePath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs |
														aiProcess_GenSmoothNormals | aiProcess_GenUVCoords);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode || !scene->HasMeshes())
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Loading model failed", importer.GetErrorString(), NULL);
		return false;
	}

	// Cheat texture

	bool hasTexture = false;
	aiString texPath("");
	if (scene->HasMaterials())
	{
		aiMaterial* material = scene->mMaterials[0];
		hasTexture = material && material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) >= 0;
	}

	texturePath = texPath.C_Str();

	// Cheat mesh

	aiMesh* mesh = scene->mMeshes[0];
	if (!mesh)
	{
		return false;
	}

	vertices.clear();
	indeces.clear();
	aiVector3D* texCoords = hasTexture ? mesh->mTextureCoords[0] : nullptr;
	vertices.resize(mesh->mNumVertices);

	for (unsigned i = 0; i < mesh->mNumVertices; i++)
	{
		vertices[i].x = mesh->mVertices[i].x;
		vertices[i].y = mesh->mVertices[i].y;
		vertices[i].z = mesh->mVertices[i].z;

		if (texCoords)
		{
			vertices[i].u = texCoords[i].x;
			vertices[i].v = texCoords[i].y;
		}
	}

	for (unsigned i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace& face = mesh->mFaces[i];
		for (unsigned j = 0; j < face.mNumIndices; j++)
		{
			indeces.push_back(face.mIndices[j]);
		}
	}

	return !(vertices.empty() || indeces.empty());
}

int main(int argc, char** argsv)
{
	bool isFullScreen = false;

	//Initialises the SDL Library, passing in SDL_INIT_VIDEO to only initialise the video subsystems
	//https://wiki.libsdl.org/SDL_Init
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		//Display an error message box
		//https://wiki.libsdl.org/SDL_ShowSimpleMessageBox
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL_Init failed", SDL_GetError(), NULL);
		return 1;
	}

	//Create a window, note we have to free the pointer returned using the DestroyWindow Function
	//https://wiki.libsdl.org/SDL_CreateWindow
	SDL_Window* window = SDL_CreateWindow("Configurable", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 960, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

	//Checks to see if the window has been created, the pointer will have a value of some kind
	if (window == nullptr)
	{
		//Show error
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL_CreateWindow failed", SDL_GetError(), NULL);
		//Close the SDL Library
		//https://wiki.libsdl.org/SDL_Quit
		SDL_Quit();
		return 1;
	}

	std::vector<Vertex> vertices;
	std::vector<unsigned> indices;
	std::string texturePath;

	if (!LoadModel("cube.nff", vertices, indices, texturePath))
	{
		//Show error
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Load model failed", " ", NULL);

		return 1;
	}
	

	SDL_Surface* image = IMG_Load("Crate.jpg");
	if (!image)
	{
		//Show error
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "IMG_Load failed", IMG_GetError(), NULL);

		//https://wiki.libsdl.org/SDL_DestroyWindow
		SDL_DestroyWindow(window);
		//https://wiki.libsdl.org/SDL_Quit
		SDL_Quit();

		return 1;
	}

	SDL_SetRelativeMouseMode(SDL_TRUE);

	SDL_GLContext glContext = SDL_GL_CreateContext(window);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	//Initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unable to initialise GLEW", (char*)glewGetErrorString(glewError), NULL);
	}

	//Create a Vertex Array object to deal with vertex formats
	GLuint vertexArray;
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);


	//// An array of 3 vectors which represents 3 vertices
	//static const GLfloat vertices[] = {
	//	// Position					// Colour				// Texture
	//	-0.75f, -0.75f, 0.0f,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
	//	0.75f, -0.75f, 0.0f,		0.0f, 1.0f, 0.0f,		1.0f, 0.0f,
	//	0.75f,  0.75f, 0.0f,		0.0f, 0.0f, 1.0f,		1.0f, 1.0f,
	//	-0.75f, 0.75f, 0.0f,		1.0f, 1.0f, 0.0f,		0.0f, 1.0f,
	//};

	//// Tri constructions
	//static const GLuint g_vertex_indeces[] = {
	//	0, 1, 2,
	//	2, 3, 0,
	//};

	// This will identify our vertex buffer
	GLuint vertexBuffer;
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexBuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(
		0,										// attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,										// size
		GL_FLOAT,								// type
		GL_FALSE,								// normalized?
		sizeof(Vertex),							// stride
		(void*)0								// array buffer offset
	);

	//glEnableVertexAttribArray(1);
	//glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	//glVertexAttribPointer(
	//	1,										// attribute 0. No particular reason for 0, but must match the layout in the shader.
	//	3,										// size
	//	GL_FLOAT,								// type
	//	GL_FALSE,								// normalized?
	//	8 * sizeof(GL_FLOAT),					// stride
	//	(void*)(3 * sizeof(GL_FLOAT))           // array buffer offset
	//);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(
		2,										// attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,										// size
		GL_FLOAT,								// type
		GL_FALSE,								// normalized?
		sizeof(Vertex),							// stride
		(void*)(3 * sizeof(GL_FLOAT))           // array buffer offset
	);

	GLuint elementBuffer;
	glGenBuffers(1, &elementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * indices.size(), &indices[0], GL_STATIC_DRAW);

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	int Mode = GL_RGB;

	if (image->format->BytesPerPixel == 4) {
		Mode = GL_RGBA;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, Mode, image->w, image->h, 0, Mode, GL_UNSIGNED_BYTE, image->pixels);

	// Nice trilinear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("BasicVert.glsl", "BasicFrag.glsl");

	// Transforms
	glm::mat4 model = glm::mat4(1.0f);
	// model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));

	glm::mat4 mvp, view, projection;
	glm::vec3 position(0.0f, 0.0f, 5.0f), forward(0.0f, 0.0f, -1.0f), rotation(0.0f);

	const glm::vec4 cameraFace(0.0f, 0.0f, 1.0f, 0.0f);
	const float walkSpeed = 0.5f, rotationSpeed = 0.1f;

	unsigned int transformLoc = glGetUniformLocation(programID, "transform");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);

	//Event loop, we will loop until running is set to false, usually if escape has been pressed or window is closed
	bool running = true;
	//SDL Event structure, this will be checked in the while loop
	SDL_Event ev;
	while (running)
	{
		//Poll for the events which have happened in this frame
		//https://wiki.libsdl.org/SDL_PollEvent
		while (SDL_PollEvent(&ev))
		{
			//Switch case for every message we are intereted in
			switch (ev.type)
			{
				//QUIT Message, usually called when the window has been closed
			case SDL_QUIT:
				running = false;
				break;

				// MOUSE has moved
			case SDL_MOUSEMOTION:
				rotation.y -= ev.motion.xrel * rotationSpeed;
				rotation.x -= ev.motion.yrel * rotationSpeed;

				glm::mat4 viewRotate(1.0f);
				viewRotate = glm::rotate(viewRotate, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
				viewRotate = glm::rotate(viewRotate, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));

				forward = glm::normalize(glm::vec3(viewRotate * cameraFace));

				break;

				//KEYDOWN Message, called when a key has been pressed down
			case SDL_KEYDOWN:
				//Check the actual key code of the key that has been pressed
				switch (ev.key.keysym.sym)
				{
					//Escape key
				case SDLK_ESCAPE:
					running = false;
					break;

					//F key
				case SDLK_f:
					//Toggles between fullscreen and windowed modes
					isFullScreen = ToggleWindowFullscreen(window, isFullScreen);
					break;

				case SDLK_w:
					position += walkSpeed * forward;
					break;

				case SDLK_s:
					position -= walkSpeed * forward;
					break;
				}
			}
		}

		glClearColor(0.0f, 0.0f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(programID);

		view = glm::lookAt(
			position,
			position + forward,
			glm::vec3(0, 1, 0)
		);

		projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

		mvp = projection * view * model;

		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(mvp));

		// Draw the frame
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);

		SDL_GL_SwapWindow(window);
	}

	glDisableVertexAttribArray(0);

	glDeleteProgram(programID);
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteVertexArrays(1, &vertexArray);

	SDL_FreeSurface(image);
	SDL_GL_DeleteContext(glContext);

	//Destroy the window and quit SDL2, NB we should do this after all cleanup in this order!!!
	//https://wiki.libsdl.org/SDL_DestroyWindow
	SDL_DestroyWindow(window);
	//https://wiki.libsdl.org/SDL_Quit
	SDL_Quit();

	return 0;
}