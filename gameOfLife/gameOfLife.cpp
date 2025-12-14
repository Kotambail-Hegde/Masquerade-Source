#pragma endregion GOL_SPECIFIC_INCLUDES
#include "gameOfLife.h"
#pragma endregion GOL_SPECIFIC_INCLUDES

#pragma region GOL_SPECIFIC_MACROS
#define ALIVE_CELL 1
#define DEAD_CELL 0
#define _MY_CURRENT_CELL 0;
#pragma endregion GOL_SPECIFIC_MACROS

#pragma region GOL_SPECIFIC_DECLARATIONS
static uint32_t gol_texture;
static uint32_t matrix_texture;
static uint32_t matrix[16] = { 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF };
#pragma endregion GOL_SPECIFIC_DECLARATIONS

gameOfLife_t::gameOfLife_t(boost::property_tree::ptree& config)
{
	isBiosEnabled = NO;

	setEmulationID(EMULATION_ID::GAME_OF_LIFE_ID);

	this->pt = config;

#ifndef __EMSCRIPTEN__
	_SAVE_LOCATION = pt.get<std::string>("gameoflife._save_location");
#else
	_SAVE_LOCATION = "assets/saves";
#endif

	if (to_bool(pt.get<std::string>("gameoflife._enable_custom_dimensions")) == YES)
	{
		 this->delta_screen_width = pt.get<std::int32_t>("gameoflife._screen_width") - (int32_t)this->screen_width;
		 this->delta_screen_height = pt.get<std::int32_t>("gameoflife._screen_height") - (int32_t)this->screen_height;
		 this->delta_pixel_width = pt.get<std::int32_t>("gameoflife._pixel_width") - (int32_t)this->pixel_width;
		 this->delta_pixel_height = pt.get<std::int32_t>("gameoflife._pixel_height") - (int32_t)this->pixel_width;
	}
	else
	{
		this->delta_screen_width = ZERO;
		this->delta_screen_height = ZERO;
		this->delta_pixel_width = ZERO;
		this->delta_pixel_height = ZERO;
	}

	// check if directory mentioned by "_SAVE_LOCATION" exists, if not we need to explicitly create it
	ifNoDirectoryThenCreate(_SAVE_LOCATION);
}

void gameOfLife_t::setupTheCoreOfEmulation(void* masqueradeInstance, void* audio, void* network)
{
	if (masqueradeInstance != nullptr)
	{
		gameoflifeGameEngine = masqueradeInstance;
	}

	if (!initializeEmulator())
	{
		LOG("memory allocation failure");
		throw std::runtime_error("memory allocation failure");
	}

	wasGamePlayActive = false;
}

uint32_t gameOfLife_t::getScreenWidth()
{
	RETURN this->screen_width + this->delta_screen_width;
}

uint32_t gameOfLife_t::getScreenHeight()
{
	RETURN this->screen_height + this->delta_screen_height;
}

uint32_t gameOfLife_t::getPixelWidth()
{
	RETURN this->pixel_width + this->delta_pixel_width;
}

uint32_t gameOfLife_t::getPixelHeight()
{
	RETURN this->pixel_height + this->delta_pixel_height;
}

uint32_t gameOfLife_t::getTotalScreenWidth()
{
	RETURN this->screen_width;
}

uint32_t gameOfLife_t::getTotalScreenHeight()
{
	RETURN this->screen_height;
}

uint32_t gameOfLife_t::getTotalPixelWidth()
{
	RETURN this->pixel_width;
}

uint32_t gameOfLife_t::getTotalPixelHeight()
{
	RETURN this->pixel_height;
}

const char* gameOfLife_t::getEmulatorName()
{
	RETURN this->NAME;
}

float gameOfLife_t::getEmulationFPS()
{
	RETURN this->myFPS;
}

void gameOfLife_t::setEmulationID(EMULATION_ID ID)
{
	myID = ID;
}

EMULATION_ID gameOfLife_t::getEmulationID()
{
	RETURN myID;
}

bool gameOfLife_t::initializeEmulator()
{
	// create an instance

	pAbsolute_gol_instance = std::make_shared<absolute_gol_instance_t>();

	// for readability

	pGol_instance = &(pAbsolute_gol_instance->absolute_gol_state.gol_instance);

	// quirks 

	pGol_instance->gol_state.others._isTorroidal = to_bool(pt.get<std::string>("gameoflife._is_torroidal"));

	// initialize the graphics

	uint32_t scanner = 0;
	for (uint32_t y = 0; y < getScreenHeight(); y++)
	{
		for (uint32_t x = 0; x < getScreenWidth(); x++)
		{
			pGol_instance->gol_state.display.imGuiBuffer.imGuiBuffer1D[scanner++] = BLACK;
		}
	}

	pGol_instance->gol_state.others.anyActivity = true;
	pGol_instance->gol_state.others.hostCPUCycle = ZERO;
	pGol_instance->gol_state.emulatedCPUCycle = ZERO;

	pGol_instance->gol_state.display.transformation.fStartPanX = ZERO;
	pGol_instance->gol_state.display.transformation.fStartPanY = ZERO;

	pGol_instance->gol_state.display.transformation.fScaleX = ONE;
	pGol_instance->gol_state.display.transformation.fScaleY = ONE;

#if 1

	auto x0 = getScreenWidth() / 2;
	auto y0 = getScreenHeight() / 2;

	// Nothing
	//birth1DLife(x0, y0 + 0, L".....................................");
	//birth1DLife(x0, y0 + 1, L".....................................");
	//birth1DLife(x0, y0 + 2, L".....................................");
	//birth1DLife(x0, y0 + 3, L".....................................");
	//birth1DLife(x0, y0 + 4, L".....................................");
	//birth1DLife(x0, y0 + 5, L".....................................");
	//birth1DLife(x0, y0 + 6, L".....................................");
	//birth1DLife(x0, y0 + 7, L".....................................");
	//birth1DLife(x0, y0 + 8, L".....................................");

	// Gosper Glider Gun
	birth1DLife(x0, y0 + 0, L"........................#............");
	birth1DLife(x0, y0 + 1, L"......................#.#............");
	birth1DLife(x0, y0 + 2, L"............##......##............##.");
	birth1DLife(x0, y0 + 3, L"...........#...#....##............##.");
	birth1DLife(x0, y0 + 4, L"##........#.....#...##...............");
	birth1DLife(x0, y0 + 5, L"##........#...#.##....#.#............");
	birth1DLife(x0, y0 + 6, L"..........#.....#.......#............");
	birth1DLife(x0, y0 + 7, L"...........#...#.....................");
	birth1DLife(x0, y0 + 8, L"............##.......................");

	// R Pentamino
	//birth1DLife(x0, y0 + 0, L".....................................");
	//birth1DLife(x0, y0 + 1, L".....................................");
	//birth1DLife(x0, y0 + 2, L".....................................");
	//birth1DLife(x0, y0 + 3, L".....................................");
	//birth1DLife(x0, y0 + 4, L"..................#..................");
	//birth1DLife(x0, y0 + 5, L".................###.................");
	//birth1DLife(x0, y0 + 6, L"...................#.................");
	//birth1DLife(x0, y0 + 7, L".....................................");
	//birth1DLife(x0, y0 + 8, L".....................................");

	// All
	//birth1DLife(x0, y0 + 0, L"#####################################");
	//birth1DLife(x0, y0 + 1, L"#####################################");
	//birth1DLife(x0, y0 + 2, L"#####################################");
	//birth1DLife(x0, y0 + 3, L"#####################################");
	//birth1DLife(x0, y0 + 4, L"#####################################");
	//birth1DLife(x0, y0 + 5, L"#####################################");
	//birth1DLife(x0, y0 + 6, L"#####################################");
	//birth1DLife(x0, y0 + 7, L"#####################################");
	//birth1DLife(x0, y0 + 8, L"#####################################");
#endif

	// initialization specific to OpenGL
#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
	glEnable(GL_TEXTURE_2D);
	glGenFramebuffers(1, &frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

	glGenTextures(1, &masquerade_texture);
	glBindTexture(GL_TEXTURE_2D, masquerade_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth() * FRAME_BUFFER_SCALE, getScreenHeight() * FRAME_BUFFER_SCALE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, masquerade_texture, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenTextures(1, &gol_texture);
	glBindTexture(GL_TEXTURE_2D, gol_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth(), getScreenHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pGol_instance->gol_state.display.imGuiBuffer.imGuiBuffer1D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// for "Dot Matrix"
	glGenTextures(1, &matrix_texture);

	glBindTexture(GL_TEXTURE_2D, matrix_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, (GLvoid*)matrix);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#else
	// 1. Setup framebuffer
	GL_CALL(glGenFramebuffers(1, &frame_buffer));
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));

	// 2. Create texture to attach to framebuffer (masquerade_texture)
	GL_CALL(glGenTextures(1, &masquerade_texture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, masquerade_texture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth() * FRAME_BUFFER_SCALE, getScreenHeight() * FRAME_BUFFER_SCALE, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, masquerade_texture, 0));

	// Optional: Check framebuffer status
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG("Error: Framebuffer is not complete!");
	}
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0)); // Unbind

	// 3. Game Of Life texture (used to upload emulated framebuffer)
	GL_CALL(glGenTextures(1, &gol_texture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, gol_texture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth(), getScreenHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pGol_instance->gol_state.display.imGuiBuffer.imGuiBuffer1D));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	// 4. Dot Matrix overlay texture
	GL_CALL(glGenTextures(1, &matrix_texture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, matrix_texture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, (GLvoid*)matrix));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	// 5. Fullscreen Quad VAO/VBO (for textured quad rendering)
	float fullscreenVertices[] = {
		//  X     Y      U     V
		-1.0f,  1.0f,  0.0f, 1.0f,  // Top-left
		-1.0f, -1.0f,  0.0f, 0.0f,  // Bottom-left
		 1.0f, -1.0f,  1.0f, 0.0f,  // Bottom-right

		-1.0f,  1.0f,  0.0f, 1.0f,  // Top-left
		 1.0f, -1.0f,  1.0f, 0.0f,  // Bottom-right
		 1.0f,  1.0f,  1.0f, 1.0f   // Top-right
	};

	GL_CALL(glGenVertexArrays(1, &fullscreenVAO));
	GL_CALL(glBindVertexArray(fullscreenVAO));

	GL_CALL(glGenBuffers(1, &fullscreenVBO));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, fullscreenVBO));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreenVertices), fullscreenVertices, GL_STATIC_DRAW));

	// Attribute 0: position (vec2)
	GL_CALL(glEnableVertexAttribArray(0));
	GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));

	// Attribute 1: UV (vec2)
	GL_CALL(glEnableVertexAttribArray(1));
	GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))));

	GL_CALL(glBindVertexArray(0));

	std::string shaderPath;
#ifndef __EMSCRIPTEN__
	shaderPath = pt.get<std::string>("internal._working_directory");
#else
	shaderPath = "assets/internal";
#endif

	// 6. Compile passthrough shader
	shaderProgramSource_t passthroughShader = parseShader(shaderPath + "/shaders/passthrough.shaders");
	shaderProgramBasic = createShader(passthroughShader.vertexSource, passthroughShader.fragmentSource);
	// 7. Compile blend shader (for LCD effect)
	shaderProgramSource_t blendShader = parseShader(shaderPath + "/shaders/blend.shaders");
	shaderProgramBlend = createShader(blendShader.vertexSource, blendShader.fragmentSource);

	DEBUG("PASSTHROUGH VERTEX");
	DEBUG("%s", passthroughShader.vertexSource.c_str());
	DEBUG("PASSTHROUGH FRAGMENT");
	DEBUG("%s", passthroughShader.fragmentSource.c_str());
	DEBUG("BLEND VERTEX");
	DEBUG("%s", blendShader.vertexSource.c_str());
	DEBUG("BLEND FRAGMENT");
	DEBUG("%s", blendShader.fragmentSource.c_str());
#endif

	RETURN SUCCESS;
}

void gameOfLife_t::destroyEmulator()
{
	pAbsolute_gol_instance.reset();

#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
	glDeleteTextures(1, &gol_texture);
	glDeleteTextures(1, &matrix_texture);
#else
	glDeleteTextures(1, &gol_texture);
	glDeleteTextures(1, &matrix_texture);
#endif
}

uint32_t gameOfLife_t::getHostCPUCycle()
{
	RETURN pAbsolute_gol_instance->absolute_gol_state.gol_instance.gol_state.others.hostCPUCycle;
}

int64_t gameOfLife_t::getEmulatedCPUCycle()
{
	RETURN pAbsolute_gol_instance->absolute_gol_state.gol_instance.gol_state.emulatedCPUCycle;
}

void gameOfLife_t::setEmulatedCPUCycle(int64_t cycles)
{
	pAbsolute_gol_instance->absolute_gol_state.gol_instance.gol_state.emulatedCPUCycle = cycles;
}

void gameOfLife_t::birth1DLife(int x, int y, std::wstring s)
{
	int p = 0;
	for (auto c : s)
	{
		if (c == L'#')
		{
			const uint32_t index = y * getScreenWidth() + x + p;
			activeNext.insert(index);
			insertNeighbors(index);
			pGol_instance->gol_state.display.imGuiBuffer.imGuiBuffer1D[index].n = WHITE.n;
		}
		p++;
	}
}

void gameOfLife_t::captureIO()
{
	performUserActivity();
}

void gameOfLife_t::displayCompleteScreen()
{
#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

	glDisable(GL_BLEND);

	// Handle for gameboy system's texture

	glBindTexture(GL_TEXTURE_2D, gol_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RGBA, GL_UNSIGNED_BYTE, 
		(GLvoid*)pGol_instance->gol_state.display.imGuiBuffer.imGuiBuffer1D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (currEnVFilter == VIDEO_FILTERS::BILINEAR_FILTER)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, getScreenWidth() * FRAME_BUFFER_SCALE, 0, getScreenHeight() * FRAME_BUFFER_SCALE, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, getScreenWidth() * FRAME_BUFFER_SCALE, getScreenHeight() * FRAME_BUFFER_SCALE);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(0.0, 0.0);
	glTexCoord2f(1.0, 0.0);
	glVertex2f(getScreenWidth() * FRAME_BUFFER_SCALE, 0.0);
	glTexCoord2f(1.0, 1.0);
	glVertex2f(getScreenWidth() * FRAME_BUFFER_SCALE, getScreenHeight() * FRAME_BUFFER_SCALE);
	glTexCoord2f(0.0, 1.0);
	glVertex2f(0.0, getScreenHeight() * FRAME_BUFFER_SCALE);
	glEnd();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Handle for dot matrix texture

	if (currEnVFilter == VIDEO_FILTERS::LCD_FILTER)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
		glEnable(GL_BLEND);

		glColor4f(1.0f, 1.0f, 1.0f, 0.3f / 4.0f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindTexture(GL_TEXTURE_2D, matrix_texture);

		int viewportWidth = getScreenWidth() * FRAME_BUFFER_SCALE;
		int viewportHeight = getScreenHeight() * FRAME_BUFFER_SCALE;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glOrtho(0, viewportWidth, 0, viewportHeight, -1, 1);

		glMatrixMode(GL_MODELVIEW);
		glViewport(0, 0, viewportWidth, viewportHeight);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);
		glVertex2f(0.0, 0.0);
		glTexCoord2f(getScreenWidth(), 0.0);
		glVertex2f(viewportWidth, 0.0);
		glTexCoord2f(getScreenWidth(), getScreenHeight());
		glVertex2f(viewportWidth, viewportHeight);
		glTexCoord2f(0.0, getScreenHeight());
		glVertex2f(0.0, viewportHeight);
		glEnd();

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		glDisable(GL_BLEND);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// Handle for renderer's texture

	glBindTexture(GL_TEXTURE_2D, masquerade_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (currEnVFilter == VIDEO_FILTERS::LCD_FILTER)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
#else
	// 1. Upload emulator framebuffer to gol_texture
	GL_CALL(glBindTexture(GL_TEXTURE_2D, gol_texture));
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
		(GLvoid*)pGol_instance->gol_state.display.imGuiBuffer.imGuiBuffer1D));

	// Choose filtering mode (NEAREST or LINEAR)
	GLint filter = (currEnVFilter == VIDEO_FILTERS::BILINEAR_FILTER) ? GL_LINEAR : GL_NEAREST;
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));

	// 2. Render gameboy_texture into framebuffer (masquerade_texture target)
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));
	GL_CALL(glViewport(0, 0, getScreenWidth() * FRAME_BUFFER_SCALE, getScreenHeight() * FRAME_BUFFER_SCALE));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

	// Pass 1: Render base texture (Game Boy framebuffer)
	GL_CALL(glUseProgram(shaderProgramBasic));
	GL_CALL(glActiveTexture(GL_TEXTURE0));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, gol_texture));
	GL_CALL(glUniform1i(glGetUniformLocation(shaderProgramBasic, "u_Texture"), 0));

	GL_CALL(glBindVertexArray(fullscreenVAO));
	GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));
	GL_CALL(glBindVertexArray(0));
	GL_CALL(glUseProgram(0));

	// 3. Optional: LCD matrix overlay (dot matrix)
	if (currEnVFilter == VIDEO_FILTERS::LCD_FILTER)
	{
		GL_CALL(glEnable(GL_BLEND));
		GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		GL_CALL(glUseProgram(shaderProgramBlend));

		// Set alpha (0.3 / 4.0)
		GL_CALL(glUniform1f(glGetUniformLocation(shaderProgramBlend, "u_Alpha"), 0.075f));

		// Set texture
		GL_CALL(glActiveTexture(GL_TEXTURE0));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, matrix_texture));
		GL_CALL(glUniform1i(glGetUniformLocation(shaderProgramBlend, "u_Texture"), 0));

		// Set texel size (1 / 4) to repeat the matrix texture per pixel
		float texelSize[2] = { 1.0f / 4.0f, 1.0f / 4.0f };
		GL_CALL(glUniform2fv(glGetUniformLocation(shaderProgramBlend, "u_TexelSize"), 1, texelSize));

		GL_CALL(glBindVertexArray(fullscreenVAO));
		GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));
		GL_CALL(glBindVertexArray(0));

		GL_CALL(glUseProgram(0));
		GL_CALL(glDisable(GL_BLEND));
	}

	// 4. Done rendering to framebuffer (masquerade_texture)
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	// 5. Setup filtering for final display (e.g., ImGui::Image or screen blit)
	GL_CALL(glBindTexture(GL_TEXTURE_2D, masquerade_texture));

	filter = (currEnVFilter == VIDEO_FILTERS::LCD_FILTER) ? GL_LINEAR : GL_NEAREST;
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));
#endif
}

bool gameOfLife_t::runEmulationLoopAtHostRate(uint32_t currentFrame)
{
	RETURN true;
}

bool gameOfLife_t::runEmulationAtHostRate(uint32_t currentFrame)
{
	RETURN true;
}

bool gameOfLife_t::runEmulationAtFixedRate(uint32_t currentFrame)
{
	bool status = true;

	captureIO();

	if (pGol_instance->gol_state.display.freeRunning == YES || ImGui::IsKeyDown(ImGuiKey_Space))
	{
		activeNow = activeNext;
		activeNext.clear();
		activeNext.reserve(activeNow.size());

		potentialNow = potentialNext;

		potentialNext = activeNow;

		for (const auto& pixel : potentialNow)
		{
			performBackgroundActivity(pixel);
		}
	}

	displayCompleteScreen();

	RETURN status;
}

bool gameOfLife_t::runEmulationLoopAtFixedRate(uint32_t currentFrame)
{
	RETURN true;
}

void gameOfLife_t::insertNeighbors(uint32_t pixel, bool erase) 
{
	uint32_t px = pixel & (getScreenWidth() - 1);							// faster than % when power of two
	uint32_t py = pixel >> std::countr_zero(getScreenHeight());                 // using shift

	for (int dy = -1; dy <= 1; dy++)
	{
		int ny = py + dy;

		if (pGol_instance->gol_state.others._isTorroidal)
		{
			ny = (ny + getScreenHeight()) & (getScreenHeight() - 1);
		}
		else
		{
			if (ny < 0 || ny >= (int)getScreenHeight()) continue;
		}

		uint32_t row_start = ny << std::countr_zero(getScreenHeight());			// multiply by 1024 via shift

		for (int dx = -1; dx <= 1; dx++)
		{ 
			int nx = px + dx;

			if (pGol_instance->gol_state.others._isTorroidal)
			{
				nx = (nx + getScreenWidth()) & (getScreenWidth() - 1);
			}
			else
			{
				if (nx < 0 || nx >= (int)getScreenWidth()) continue;
			}

			if (erase)
			{
				potentialNext.erase(row_start + nx);
			}
			else
			{
				potentialNext.insert(row_start + nx);
			}
		}
	}
}

// NOTE: Always update the lifeDatabase from GFX buffer in the below function
void gameOfLife_t::performUserActivity()
{
	float xMouse = 0.0f, yMouse = 0.0f;

	// Handle free-running GOL
	if (ImGui::IsKeyPressed(ImGuiKey_R))
	{
		pGol_instance->gol_state.display.freeRunning = (pGol_instance->gol_state.display.freeRunning == YES ? NO : YES);
	}

	// Handle other keyboard inputs
	if (
		ImGui::IsMouseDown(ImGuiMouseButton_Left)
		&&
		xMouse >= ZERO
		&&
		yMouse >= ZERO
		)
	{
		getMouseRelPosIfDocked(&xMouse, &yMouse, getScreenWidth(), getScreenHeight());

		//wasGamePlayActive = true;
		uint32_t x = (uint32_t)(xMouse);
		uint32_t y = (uint32_t)(yMouse);
		uint32_t index = ((y * getScreenWidth()) + x);
		pGol_instance->gol_state.display.imGuiBuffer.imGuiBuffer1D[index].n = WHITE.n;

		activeNext.insert(index);
		insertNeighbors(index);
	}
	else if (
		ImGui::IsMouseDown(ImGuiMouseButton_Right)
		&&
		xMouse >= ZERO
		&&
		yMouse >= ZERO
		)
	{
		getMouseRelPosIfDocked(&xMouse, &yMouse, getScreenWidth(), getScreenHeight());

		//wasGamePlayActive = true;
		uint32_t x = (uint32_t)(xMouse);
		uint32_t y = (uint32_t)(yMouse);
		uint32_t index = ((y * getScreenWidth()) + x);
		pGol_instance->gol_state.display.imGuiBuffer.imGuiBuffer1D[index].n = BLACK.n;

		activeNext.erase(index);
		insertNeighbors(index, YES);
	}
}

// Convert coordinates from World Space --> Screen Space
void gameOfLife_t::worldToScreen(float fWorldX, float fWorldY, int& nScreenX, int& nScreenY)
{
	nScreenX = (int)((fWorldX - pGol_instance->gol_state.display.transformation.fOffsetX) * pGol_instance->gol_state.display.transformation.fScaleX);
	nScreenY = (int)((fWorldY - pGol_instance->gol_state.display.transformation.fOffsetY) * pGol_instance->gol_state.display.transformation.fScaleY);
}

// Convert coordinates from Screen Space --> World Space
void gameOfLife_t::screenToWorld(int nScreenX, int nScreenY, float& fWorldX, float& fWorldY)
{
	fWorldX = ((float)nScreenX / pGol_instance->gol_state.display.transformation.fScaleX) + pGol_instance->gol_state.display.transformation.fOffsetX;
	fWorldY = ((float)nScreenY / pGol_instance->gol_state.display.transformation.fScaleY) + pGol_instance->gol_state.display.transformation.fOffsetY;
}

// NOTE: Never update the lifeDatabase in the below function
void gameOfLife_t::performBackgroundActivity(uint32_t pixel)
{
	++pAbsolute_gol_instance->absolute_gol_state.gol_instance.gol_state.others.hostCPUCycle;
	++pAbsolute_gol_instance->absolute_gol_state.gol_instance.gol_state.emulatedCPUCycle;

	wasGamePlayActive = true;

	uint32_t numberOfLivingNeighbours = 0;

	const uint32_t w = getScreenWidth();
	const uint32_t h = getScreenHeight();
	const uint32_t wMask = w - 1;
	const uint32_t hMask = h - 1;
	const uint32_t wShift = ctz32_portable(w);

	uint32_t x = pixel & wMask;
	uint32_t y = pixel >> wShift;

	constexpr int dx[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	constexpr int dy[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };

	auto isCellAlive = [&](uint32_t idx) 
		{
			RETURN activeNow.contains(idx) ? ALIVE_CELL : DEAD_CELL;
		};

	for (int i = 0; i < 8; ++i)
	{
		int nx = x + dx[i];
		int ny = y + dy[i];

		if (pGol_instance->gol_state.others._isTorroidal == TRUE)
		{
			nx &= wMask;
			ny &= hMask;
		}
		else
		{
			if (nx < 0 || nx >= (int)w || ny < 0 || ny >= (int)h)
			{
				CONTINUE;
			}
		}

		numberOfLivingNeighbours += isCellAlive((ny << wShift) + nx);
	}

	const bool alive = (isCellAlive(pixel) == ALIVE_CELL);

	if (alive)
	{
		if ((numberOfLivingNeighbours < 2) || (numberOfLivingNeighbours > 3))
		{
			pGol_instance->gol_state.display.imGuiBuffer.imGuiBuffer1D[pixel].n = BLACK.n;
			insertNeighbors(pixel);
		}
		else
		{
			activeNext.insert(pixel);
		}
	}
	else if (numberOfLivingNeighbours == 3)
	{
		pGol_instance->gol_state.display.imGuiBuffer.imGuiBuffer1D[pixel].n = WHITE.n;
		activeNext.insert(pixel);
		insertNeighbors(pixel);
	}

	RETURN;
}

bool gameOfLife_t::saveState(uint8_t id)
{
	bool status = false;

	std::string saveStateNameForThisROM = "_save_gol_";

	saveStateNameForThisROM = saveStateNameForThisROM + std::to_string(id);

	std::ofstream save;

#if ZERO
	time_t rawtime;
	struct tm timeinfo;
	char buffer[80];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	strftime(buffer, sizeof(buffer), "%d-%m-%Y-%H:%M:%S", &timeinfo);
	LOG("Saved on: %s\n", buffer);
	std::string dt(buffer);
	saveFile.append(dt);
#endif

	LOG("Saved as: %s\n", saveStateNameForThisROM.c_str());

	saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

	save.open(saveStateNameForThisROM.c_str(), std::ios::binary);
	save.write(reinterpret_cast<char*>(&(pGol_instance->gol_memoryState)), sizeof(pGol_instance->gol_memoryState));
	save.close();

	status = true;

	RETURN status;
}

bool gameOfLife_t::loadState(uint8_t id)
{
	bool status = false;

	std::string saveStateNameForThisROM = "_save_gol_";

	saveStateNameForThisROM = saveStateNameForThisROM + std::to_string(id);

	std::ifstream save;

	saveStateNameForThisROM = _SAVE_LOCATION + "\\" + saveStateNameForThisROM;

	save.open(saveStateNameForThisROM, std::ios::binary);
	save.read(reinterpret_cast<char*>(&(pGol_instance->gol_memoryState)), sizeof(pGol_instance->gol_memoryState));
	save.close();

	status = true;

	RETURN status;
}

bool gameOfLife_t::fillGamePlayStack()
{
	// assume minimum frame rate is 60 fps
	// so for 5 seconds worth of rewind, 300 elements is required
	// if fps is 1000, for 5 seconds worth of rewind, 5000 elements is required
	// Hence, we will (for now) set the limit to 5000 elements
	if (wasGamePlayActive)
	{
		if (gamePlay.size() <= _REWIND_BUFFER_SIZE)
		{
			gamePlay.push_front(pGol_instance->gol_state);
			RETURN true;
		}
		else
		{
			gamePlay.pop_back();
			gamePlay.push_front(pGol_instance->gol_state);
			RETURN false;
		}
	}
	else
	{
		RETURN false;
	}
}

bool gameOfLife_t::rewindGamePlay()
{
	if (gamePlay.empty())
	{
		RETURN false;
	}
	else
	{
		memcpy(&pGol_instance->gol_memoryState, &gamePlay.front(), sizeof(pGol_instance->gol_memoryState));
		gamePlay.pop_front();
		RETURN true;
	}
}