#pragma region CHIP8_SPECIFIC_INCLUDES
#include "chip8.h"
#pragma endregion CHIP8_SPECIFIC_INCLUDES

#pragma region CHIP8_SPECIFIC_MACROS
#define CHIP8_FONT_BASE									(0x00)
#define SCHIP_FONT_BASE									(0x50)
#pragma endregion CHIP8_SPECIFIC_MACROS

#pragma region CHIP8_SPECIFIC_DECLARATIONS
static uint32_t chip8_texture;
static uint32_t matrix_texture;
static uint32_t matrix[16] = { 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF };
#pragma endregion CHIP8_SPECIFIC_DECLARATIONS

chip8_t::chip8_t(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom, boost::property_tree::ptree& config)
{
	isBiosEnabled = NO;

	setEmulationID(EMULATION_ID::CHIP8_ID);

	this->pt = config;

#ifndef __EMSCRIPTEN__
	_SAVE_LOCATION = pt.get<std::string>("chip8._save_location");
#else
	_SAVE_LOCATION = "assets/saves";
#endif

	// check if directory mentioned by "_SAVE_LOCATION" exists, if not we need to explicitly create it
	ifNoDirectoryThenCreate(_SAVE_LOCATION);

	this->rom[ZERO] = rom[ZERO];
}

void chip8_t::setupTheCoreOfEmulation(void* masqueradeInstance, void* audio, void* network)
{
	if (!initializeEmulator())
	{
		LOG("memory allocation failure");
		throw std::runtime_error("memory allocation failure");
	}

	if (!this->rom[ZERO].empty())
	{
		loadRom(rom);
	}
}

uint32_t chip8_t::getScreenWidth()
{
	RETURN this->screen_width;
}

uint32_t chip8_t::getScreenHeight()
{
	RETURN this->screen_height;
}

uint32_t chip8_t::getPixelWidth()
{
	RETURN this->pixel_width;
}

uint32_t chip8_t::getPixelHeight()
{
	RETURN this->pixel_height;
}

uint32_t chip8_t::getTotalScreenWidth()
{
	RETURN this->screen_width;
}

uint32_t chip8_t::getTotalScreenHeight()
{
	RETURN this->screen_height;
}

uint32_t chip8_t::getTotalPixelWidth()
{
	RETURN this->pixel_width;
}

uint32_t chip8_t::getTotalPixelHeight()
{
	RETURN this->pixel_height;
}

const char* chip8_t::getEmulatorName()
{
	RETURN this->NAME;
}

float chip8_t::getEmulationFPS()
{
	RETURN this->myFPS;
}

void chip8_t::setEmulationID(EMULATION_ID ID)
{
	myID = ID;
}

EMULATION_ID chip8_t::getEmulationID()
{
	RETURN myID;
}

bool chip8_t::getRomLoadedStatus()
{
	RETURN pAbsolute_chip8_instance->absolute_chip8_state.aboutRom.isRomLoaded;
}

bool chip8_t::loadRom(std::array<std::string, MAX_NUMBER_ROMS_PER_PLATFORM> rom)
{
	// Get filename
	std::string filename = rom[ZERO];

	// Extract extension (after last '.')
	std::string ext;
	size_t pos = filename.find_last_of('.');
	if (pos != std::string::npos)
	{
		ext = filename.substr(pos + 1);
	}
	else
	{
		ext = ""; // no extension
	}

	// Convert to lowercase for case-insensitive comparison
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	// Check if extension is valid
	if (ext != "ch8" && ext != "c8" && ext != "sc8" && ext != "xo8")
	{
		LOG("Invalid ROM extension. Only .ch8, .c8, .sc8 and .xo8 are allowed.");
		pAbsolute_chip8_instance->absolute_chip8_state.aboutRom.isRomLoaded = false;
		RETURN false;
	}

	// open the rom file

	FILE* fp = NULL;
	errno_t err = fopen_portable(&fp, rom[ZERO].c_str(), "rb");

	if (!err && (fp != NULL))
	{
		fseek(fp, 0, SEEK_END);
		pAbsolute_chip8_instance->absolute_chip8_state.aboutRom.romSize = ftell(fp);
		rewind(fp);
		fread(pChip8_memory->memory + 0x0200, pAbsolute_chip8_instance->absolute_chip8_state.aboutRom.romSize, 1, fp);
		fclose(fp);
		pAbsolute_chip8_instance->absolute_chip8_state.aboutRom.isRomLoaded = true;

		// Get sha1
		sha1.update(pChip8_memory->memory + 0x0200, pAbsolute_chip8_instance->absolute_chip8_state.aboutRom.romSize);
		rom_sha1 = SHA1_CUSTOM::toHexString(sha1.digest());

		// If database search is enabled, then try to get info if available in database... 
		if (pChip8_quirks->_enable_c8_db == YES)
		{
			if (getRomInfo())
			{
				// Get title
				std::string title = prg.get<std::string>("title", "Unknown");
				LOG("Title: %s", title.c_str());

				// Authors (optional)
				if (auto authors_opt = prg.get_child_optional("authors"))
				{
					int idx = 0;
					for (auto& a : *authors_opt)
					{
						DEBUG("Author %d: %s", idx++, a.second.get_value<std::string>().c_str());
					}
				}

				// Images (optional)
				if (auto images_opt = prg.get_child_optional("images"))
				{
					int idx = 0;
					for (auto& img : *images_opt)
					{
						DEBUG("Image %d: %s", idx++, img.second.get_value<std::string>().c_str());
					}
				}

				// Get release date
				std::string release = prg.get<std::string>("release", "Unknown");
				DEBUG("Release: %s", release.c_str());

				// Description (optional)
				std::string description = prg.get<std::string>("description", "");
				DEBUG("Description: %s", description.c_str());

				// Iterate ROMs (mandatory)
				if (auto roms_opt = prg.get_child_optional("roms"))
				{
					for (auto& rom_pair : *roms_opt)
					{
						std::string sha1 = rom_pair.first;

						// Only process the matching entry
						if (sha1 != rom_sha1)
						{
							continue;
						}

						auto rom = rom_pair.second;
						std::string file = rom.get<std::string>("file", "unknown.ch8");
						DEBUG("ROM file: %s (SHA1: %s)", file.c_str(), sha1.c_str());

						if (auto platforms_opt = rom.get_child_optional("platforms"))
						{
							for (auto& p : *platforms_opt)
							{
								std::string plat = p.second.get_value<std::string>();
								DEBUG("  Platform: %s", plat.c_str());

								if (plat == "xochip")
								{
									setupVariant(VARIANT::XO_CHIP);
									BREAK;
								}
								else if (plat == "superchip")
								{
									setupVariant(VARIANT::SCHIP_MODERN);
									BREAK;
								}
								else if (plat == "superchip1")
								{
									setupVariant(VARIANT::SCHIP_LEGACY);
									BREAK;
								}
								else if (plat == "originalChip8")
								{
									setupVariant(VARIANT::CHIP8);
									BREAK;
								}
								else if (plat == "modernChip8")
								{
									setupVariant(VARIANT::MODERN_CHIP8);
									BREAK;
								}
								else
								{
									FATAL("Unsupported Chip8 Variant");
								}
							}

							// ---- Tickrate ----
							if (auto tickrate_opt = rom.get_optional<uint32_t>("tickrate"))
							{
								LOG("  Tickrate: %d", tickrate_opt.value());
								unsigned varID = TO_UINT(pChip8_quirks->variant);
								if (varID < TO_UINT(VARIANT::TOTAL))
								{
									INST_PER_FRAME[varID] = tickrate_opt.value();
								}
							}

							// ---- Colors ----
							if (auto colors_opt = rom.get_child_optional("colors"))
							{
								auto& colors = *colors_opt;

								// Helper lambda: convert "#RRGGBB" to Pixel
								auto hexToPixel = [](const std::string& hexStr) -> Pixel
									{
										std::string hex = hexStr;
										if (hex.empty()) RETURN Pixel(0, 0, 0);
										if (hex[0] == '#')
										{
											hex = hex.substr(1);
										}

										unsigned int value = 0;
										std::stringstream ss;
										ss << std::hex << hex;
										ss >> value;

										uint8_t r = (value >> 16) & 0xFF;
										uint8_t g = (value >> 8) & 0xFF;
										uint8_t b = value & 0xFF;
										RETURN Pixel(r, g, b, 0xFF);
									};

								// Pixel colors array
								if (auto pixels_opt = colors.get_child_optional("pixels"))
								{
									int idx = 0;
									for (auto& px : *pixels_opt)
									{
										std::string color_hex = px.second.get_value<std::string>();
										Pixel pix = hexToPixel(color_hex);
										DEBUG("    Pixel[%d]: %s -> (R:%d, G:%d, B:%d)",
											idx, color_hex.c_str(), pix.r, pix.g, pix.b);

										switch (idx)
										{
										case 0: color1 = pix; BREAK;
										case 1: color0 = pix; BREAK;
										case 2: color2 = pix; BREAK;
										case 3: color3 = pix; BREAK;
										default: BREAK;
										}
										++idx;
									}
								}

								// ---- Keys ----
								if (auto keys_opt = rom.get_child_optional("keys"))
								{
									auto& keys = *keys_opt;

									for (auto& k : keys)
									{
										std::string keyName = k.first;                // e.g., "up", "left", "a"
										uint32_t chip8KeyIndex = k.second.get_value<uint32_t>(); // e.g., 5, 7, 6

										// Map the index to CHIP8_KEYS enum
										if (chip8KeyIndex >= 16)
										{
											FATAL("Invalid CHIP8 key index: %u", chip8KeyIndex);
											continue;
										}

										CHIP8_KEYS chipKey = keyMap[chip8KeyIndex].first; // original CHIP8 key
										ImGuiKey newKey = ImGuiKey_None;                  // default

										// Map string to ImGuiKey
										if (keyName == "up") newKey = ImGuiKey_UpArrow;
										else if (keyName == "down") newKey = ImGuiKey_DownArrow;
										else if (keyName == "left") newKey = ImGuiKey_LeftArrow;
										else if (keyName == "right") newKey = ImGuiKey_RightArrow;
										else if (keyName == "a") newKey = ImGuiKey_A;
										else if (keyName == "b") newKey = ImGuiKey_B;
										else if (keyName == "c") newKey = ImGuiKey_C;
										else if (keyName == "d") newKey = ImGuiKey_D;
										else if (keyName == "e") newKey = ImGuiKey_E;
										else if (keyName == "f") newKey = ImGuiKey_F;
										// Map Player 2 keys
										else if (keyName == "player2Up") newKey = ImGuiKey_I;
										else if (keyName == "player2Down") newKey = ImGuiKey_K;
										else if (keyName == "player2Left") newKey = ImGuiKey_J;
										else if (keyName == "player2Right") newKey = ImGuiKey_L;
										else
										{
											FATAL("Unsupported key name: %s", keyName.c_str());
											continue;
										}

										// Call the remap API
										reMapKeys(chipKey, newKey);
										LOG("Remapped CHIP8 key %u -> %s", chip8KeyIndex, keyName.c_str());
									}
								}

								// Buzzer color
								if (auto buzzer_opt = colors.get_optional<std::string>("buzzer"))
								{
									DEBUG("    Buzzer Color: %s", buzzer_opt.value().c_str());
								}

								// Silence color
								if (auto silence_opt = colors.get_optional<std::string>("silence"))
								{
									DEBUG("    Silence Color: %s", silence_opt.value().c_str());
								}
							}
						}

						std::string rom_desc = rom.get<std::string>("description", "");
						if (!rom_desc.empty())
						{
							DEBUG("  Description: %s", rom_desc.c_str());
						}

						std::string embTitle = rom.get<std::string>("embeddedTitle", "");
						if (!embTitle.empty())
						{
							DEBUG("  Embedded Title: %s", embTitle.c_str());
						}
					}
				}

				// Origin (optional)
				if (auto origin_opt = prg.get_child_optional("origin"))
				{
					std::string type = origin_opt->get<std::string>("type", "unknown");
					std::string reference = origin_opt->get<std::string>("reference", "none");
					DEBUG("Origin: type=%s, reference=%s", type.c_str(), reference.c_str());
				}

			}
		}
	}
	else
	{
		LOG("Unable to load the rom file");
		pAbsolute_chip8_instance->absolute_chip8_state.aboutRom.isRomLoaded = false;
		RETURN false;
	}

	RETURN true;
}

void chip8_t::dumpRom()
{
	uint32_t scanner = 0;
	uint32_t addressField = 0x10;

	LOG("ROM DUMP");

	LOG("Address\t\t");
	for (int ii = 0; ii < 0x10; ii++)
	{
		LOG("%02x\t", ii);
	}

	LOG_NEW_LINE;
	LOG("00000000\t");

	for (int ii = 0; ii < (int)pAbsolute_chip8_instance->absolute_chip8_state.aboutRom.romSize; ii++)
	{
		LOG("0x%02x\t", pChip8_memory->memory[0x200 + ii]);

		if (++scanner == 0x10)
		{
			scanner = 0;
			LOG_NEW_LINE;
			LOG("%08x\t", addressField);
			addressField += 0x10;
		}

	}
}

float chip8_t::getEmulationVolume()
{
	pChip8_instance->chip8_state.audio.emulatorVolume = SDL_GetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream));
	RETURN pChip8_instance->chip8_state.audio.emulatorVolume;
}

void chip8_t::setEmulationVolume(float volume)
{
	pChip8_instance->chip8_state.audio.emulatorVolume = volume;
	SDL_SetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream), volume);
	pt.put("chip8._volume", volume);
	boost::property_tree::ini_parser::write_ini(_CONFIG_LOCATION, pt);
}

void chip8_t::setupVariant(VARIANT ovrd)
{
	// Lambda to set the quirks
	auto setQuirks = [&](bool vf_reset, bool memory, bool display_wait, bool clip, bool shift, bool jump)
		{
			auto& q = pChip8_instance->chip8_state.quirks;
			q._QUIRK_VF_RESET = vf_reset;
			q._QUIRK_MEMORY = memory;
			q._QUIRK_DISPLAY_WAIT = display_wait;
			q._QUIRK_CLIP = clip;
			q._QUIRK_SHIFT = shift;
			q._QUIRK_JUMP = jump;
		};

	auto& q = pChip8_instance->chip8_state.quirks;
	q._QUIRK_VF_RESET = to_bool(pt.get<std::string>("chip8._QUIRK_VF_RESET"));
	q._QUIRK_MEMORY = to_bool(pt.get<std::string>("chip8._QUIRK_MEMORY"));
	q._QUIRK_DISPLAY_WAIT = to_bool(pt.get<std::string>("chip8._QUIRK_DISPLAY_WAIT"));
	q._QUIRK_CLIP = to_bool(pt.get<std::string>("chip8._QUIRK_CLIP"));
	q._QUIRK_SHIFT = to_bool(pt.get<std::string>("chip8._QUIRK_SHIFT"));
	q._QUIRK_JUMP = to_bool(pt.get<std::string>("chip8._QUIRK_JUMP"));

	if (ovrd == VARIANT::UNKNOWN)
	{
		// Read chip/platform flags
		pChip8_quirks->_quirk_modern_chip8 = to_bool(pt.get<std::string>("chip8._modern_chip8"));
		pChip8_quirks->_quirk_chip8 = to_bool(pt.get<std::string>("chip8._chip8"));
		pChip8_quirks->_quirk_schip_modern = to_bool(pt.get<std::string>("chip8._schip_modern"));
		pChip8_quirks->_quirk_schip_legacy = to_bool(pt.get<std::string>("chip8._schip_legacy"));
		pChip8_quirks->_quirk_xo_chip = to_bool(pt.get<std::string>("chip8._xo_chip"));

		// Determine variant and set quirks
		if (pChip8_quirks->_quirk_xo_chip)
		{
			pChip8_quirks->_quirk_schip_modern = NO;
			pChip8_quirks->_quirk_schip_legacy = NO;
			pChip8_quirks->_quirk_chip8 = NO;
			pChip8_quirks->_quirk_modern_chip8 = NO;
			LOG("XO-Chip Mode");
			pChip8_quirks->variant = VARIANT::XO_CHIP;
			setQuirks(false, true, false, false, false, false);
		}
		else if (pChip8_quirks->_quirk_schip_modern)
		{
			pChip8_quirks->_quirk_xo_chip = NO;
			pChip8_quirks->_quirk_schip_legacy = NO;
			pChip8_quirks->_quirk_chip8 = NO;
			pChip8_quirks->_quirk_modern_chip8 = NO;
			LOG("SChip (Modern) Mode");
			pChip8_quirks->variant = VARIANT::SCHIP_MODERN;
			setQuirks(false, false, false, true, true, true);
		}
		else if (pChip8_quirks->_quirk_schip_legacy)
		{
			pChip8_quirks->_quirk_xo_chip = NO;
			pChip8_quirks->_quirk_schip_modern = NO;
			pChip8_quirks->_quirk_chip8 = NO;
			pChip8_quirks->_quirk_modern_chip8 = NO;
			LOG("SChip (Legacy) Mode");
			pChip8_quirks->variant = VARIANT::SCHIP_LEGACY;
			pChip8_display->res = C8RES::NON_EXTENDED; // legacy display
			setQuirks(false, false, true, true, true, true);
		}
		else if (pChip8_quirks->_quirk_chip8)
		{
			pChip8_quirks->_quirk_xo_chip = NO;
			pChip8_quirks->_quirk_schip_modern = NO;
			pChip8_quirks->_quirk_schip_legacy = NO;
			pChip8_quirks->_quirk_modern_chip8 = NO;
			LOG("Chip8 Mode");
			pChip8_quirks->variant = VARIANT::CHIP8;
			setQuirks(true, true, true, true, false, false);
		}
		else if (pChip8_quirks->_quirk_modern_chip8)
		{
			pChip8_quirks->_quirk_xo_chip = NO;
			pChip8_quirks->_quirk_schip_modern = NO;
			pChip8_quirks->_quirk_schip_legacy = NO;
			pChip8_quirks->_quirk_chip8 = NO;
			LOG("Modern Chip8 Mode");
			pChip8_quirks->variant = VARIANT::MODERN_CHIP8;
			setQuirks(false, true, false, true, false, false);
		}
		else
		{
			LOG("Custom Mode");
		}
	}
	else
	{
		pChip8_quirks->variant = ovrd;
		switch (pChip8_quirks->variant)
		{
		case VARIANT::XO_CHIP:
		{
			LOG("Detected Variant: XO-Chip");
			pChip8_quirks->_quirk_xo_chip = YES;
			pChip8_quirks->_quirk_schip_modern = NO;
			pChip8_quirks->_quirk_schip_legacy = NO;
			pChip8_quirks->_quirk_chip8 = NO;
			pChip8_quirks->_quirk_modern_chip8 = NO;
			setQuirks(false, true, false, false, false, false);
			BREAK;
		}
		case VARIANT::SCHIP_MODERN:
		{
			LOG("Detected Variant: SChip (Modern)");
			pChip8_quirks->_quirk_xo_chip = NO;
			pChip8_quirks->_quirk_schip_modern = YES;
			pChip8_quirks->_quirk_schip_legacy = NO;
			pChip8_quirks->_quirk_chip8 = NO;
			pChip8_quirks->_quirk_modern_chip8 = NO;
			setQuirks(false, false, false, true, true, true);
			BREAK;
		}
		case VARIANT::SCHIP_LEGACY:
		{
			LOG("Detected Variant: SChip (Legacy)");
			pChip8_quirks->_quirk_xo_chip = NO;
			pChip8_quirks->_quirk_schip_modern = NO;
			pChip8_quirks->_quirk_schip_legacy = YES;
			pChip8_quirks->_quirk_chip8 = NO;
			pChip8_quirks->_quirk_modern_chip8 = NO;
			pChip8_display->res = C8RES::NON_EXTENDED; // legacy display
			setQuirks(false, false, true, true, true, true);
			BREAK;
		}
		case VARIANT::CHIP8:
		{
			LOG("Detected Variant: Chip8 Mode");
			pChip8_quirks->_quirk_xo_chip = NO;
			pChip8_quirks->_quirk_schip_modern = NO;
			pChip8_quirks->_quirk_schip_legacy = NO;
			pChip8_quirks->_quirk_chip8 = YES;
			pChip8_quirks->_quirk_modern_chip8 = NO;
			setQuirks(true, true, true, true, false, false);
			BREAK;
		}
		case VARIANT::MODERN_CHIP8:
		{
			LOG("Detected Variant: Modern Chip8 Mode");
			pChip8_quirks->_quirk_xo_chip = NO;
			pChip8_quirks->_quirk_schip_modern = NO;
			pChip8_quirks->_quirk_schip_legacy = NO;
			pChip8_quirks->_quirk_chip8 = NO;
			pChip8_quirks->_quirk_modern_chip8 = YES;
			setQuirks(false, true, false, true, false, false);
			BREAK;
		}
		default:
		{
			FATAL("Unsupported Chip8 Variant");
			RETURN;
		}
		}
	}
}

bool chip8_t::initializeEmulator()
{
	// create an instance

	pAbsolute_chip8_instance = std::make_shared<absolute_chip8_instance_t>();

	// for readability

	pChip8_instance = &(pAbsolute_chip8_instance->absolute_chip8_state.chip8_instance);
	pChip8_registers = &(pChip8_instance->chip8_state.registers);
	pChip8_memory = &(pChip8_instance->chip8_state.ram);
	pChip8_audio = &(pChip8_instance->chip8_state.audio);
	pChip8_display = &(pChip8_instance->chip8_state.display);
	pChip8_quirks = &(pChip8_instance->chip8_state.quirks);
	pChip8_io = &(pChip8_instance->chip8_state.io);

	// quirks

	setupVariant();

	// setup frame rate

	this->myFPS = FRAME_RATE[TO_UINT(pChip8_quirks->variant)];

	// initialize the audio

	pChip8_audio->pitch = 4000.0f;
	pChip8_audio->phase = 0.0;
	
	// initialize the graphics

	uint32_t scanner = 0;
	for (uint32_t y = 0; y < getScreenHeight(); y++)
	{
		for (uint32_t x = 0; x < getScreenWidth(); x++)
		{
			pChip8_display->imGuiBuffer.imGuiBuffer1D[scanner] = color1;
			pChip8_display->gfx.gfx1D[0][scanner] = CLEAR;
			pChip8_display->gfx.gfx1D[1][scanner] = CLEAR;
			pChip8_display->gfx.gfx1D[2][scanner] = CLEAR;
			pChip8_display->gfx.gfx1D[3][scanner] = CLEAR;
			scanner++;
		}
	}

	pChip8_display->planes = 0x01; // By default, enable only plane 0 in the bitmap

	pChip8_instance->chip8_state.cpuInstance.emulatedCPUCycle = ZERO;

	pChip8_registers->pc = 0x200;
	pChip8_registers->I = 0;

	pChip8_registers->sp = 0;

	pChip8_registers->delay_timer = 0;
	pChip8_instance->chip8_state.audio.sound_timer = 0;

	// initialize the opcode variable with the first instruction
	// next time onwards, processOpcode will take care of the updation

	pChip8_instance->chip8_state.cpuInstance.opcode = pChip8_memory->memory[pChip8_registers->pc] << 8 | pChip8_memory->memory[pChip8_registers->pc + 1];

	// load font set

	for (int ii = 0; ii < sizeof(chip8_fontset); ii++)
	{
		pChip8_memory->memory[CHIP8_FONT_BASE + ii] = chip8_fontset[ii];
	}

	for (int ii = 0; ii < sizeof(schip_fontset); ii++)
	{
		pChip8_memory->memory[SCHIP_FONT_BASE + ii] = schip_fontset[ii];
	}

	// check whether to enable the db or not

	pChip8_quirks->_enable_c8_db = to_bool(pt.get<std::string>("chip8._enable_c8_db"));

	// setup the volume for audio

	SDL_InitSubSystem(SDL_INIT_AUDIO);
	const SDL_AudioSpec AudioSettings{ SDL_AUDIO_F32, ONE, TO_UINT(EMULATED_AUDIO_SAMPLING_RATE_FOR_CHIP8) };
	audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &AudioSettings, NULL, NULL);
	SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(audioStream));

	pChip8_instance->chip8_state.audio.emulatorVolume = pt.get<float>("chip8._volume");
	SDL_SetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream), pChip8_instance->chip8_state.audio.emulatorVolume);

	for (size_t ii = 0; ii < TO_UINT(EMULATED_AUDIO_SAMPLING_RATE_FOR_CHIP8); ii++)
	{
		double dt = 1.0 / EMULATED_AUDIO_SAMPLING_RATE_FOR_CHIP8;
		double t = double(ii) * dt;
		tone[ii] = (CHIP8_AUDIO_SAMPLE_TYPE)(0.5 * sin(2.0 * 1529.0 * 3.14159 * t));
	}

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

	glGenTextures(1, &chip8_texture);
	glBindTexture(GL_TEXTURE_2D, chip8_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth(), getScreenHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pChip8_display->imGuiBuffer.imGuiBuffer1D);
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

	// 3. Chip8 texture (used to upload emulated framebuffer)
	GL_CALL(glGenTextures(1, &chip8_texture));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, chip8_texture));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getScreenWidth(), getScreenHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pChip8_display->imGuiBuffer.imGuiBuffer1D));
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

void chip8_t::destroyEmulator()
{
	pAbsolute_chip8_instance.reset();

#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
	glDeleteTextures(1, &chip8_texture);
	glDeleteTextures(1, &matrix_texture);
#else
	glDeleteTextures(1, &chip8_texture);
	glDeleteTextures(1, &matrix_texture);
#endif

	auto audioDevId = SDL_GetAudioStreamDevice(audioStream);
	SDL_PauseAudioDevice(audioDevId);
	SDL_ClearAudioStream(audioStream);
	SDL_UnbindAudioStream(audioStream);
	SDL_DestroyAudioStream(audioStream);
	SDL_CloseAudioDevice(audioDevId);
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

uint32_t chip8_t::getHostCPUCycle()
{
	RETURN TO_UINT32(NULL);
}

void chip8_t::nextCycle()
{
	;
}

int64_t chip8_t::getEmulatedCPUCycle()
{
	RETURN pChip8_instance->chip8_state.cpuInstance.emulatedCPUCycle;
}

void chip8_t::setEmulatedCPUCycle(int64_t cycles)
{
	pChip8_instance->chip8_state.cpuInstance.emulatedCPUCycle = cycles;
}

void chip8_t::reMapKeys(CHIP8_KEYS chipKey, ImGuiKey newKey)
{
	for (auto& pair : keyMap)
	{
		if (pair.first == chipKey)
		{
			pair.second = newKey;
			RETURN;
		}
	}
}

chip8_t::io_status chip8_t::getKeyStatus(ImGuiKey key)
{
	if (ImGui::IsKeyPressed(key))
	{
		RETURN io_status::PRESSED;
	}
	else if (ImGui::IsKeyDown(key))
	{
		RETURN io_status::HELD;
	}
	else if (ImGui::IsKeyReleased(key))
	{
		RETURN io_status::RELEASED;
	}
	else
	{
		RETURN io_status::FREE;
	}
}

void chip8_t::captureIO()
{
	auto updateKey = [this](CHIP8_KEYS key, ImGuiKey imguiKey) {
		pChip8_io->inputKeys[(uint8_t)key] =
			ImGui::IsKeyReleased(imguiKey) ? io_status::RELEASED :
			ImGui::IsKeyPressed(imguiKey) ? io_status::PRESSED :
			ImGui::IsKeyDown(imguiKey) ? io_status::HELD :
			io_status::FREE;
		};

	for (auto& [chipKey, imguiKey] : keyMap)
	{
		updateKey(chipKey, imguiKey);
	}

	// TODO: Find a different place to call this
	loadQuirks();
}

void chip8_t::processTimers()
{
	// periodically called @60Hz

	if (pChip8_registers->delay_timer > 0)
	{
		--pChip8_registers->delay_timer;
	}

	if (pChip8_instance->chip8_state.audio.sound_timer > 0)
	{
		--pChip8_instance->chip8_state.audio.sound_timer;
		pChip8_instance->chip8_state.audio.needAudio = true;
	}
	else
	{
		pChip8_instance->chip8_state.audio.needAudio = false;
		pChip8_audio->phase = 0.0;
	}
}

// Needs to be called immediately after processTimers
void chip8_t::processAudio()
{
	if (ImGui::IsKeyPressed(ImGuiKey_KeypadAdd) == YES)
	{
		auto gain = getEmulationVolume();

		gain += 0.05f;

		if (gain >= 1.000f)
		{
			gain = 0.9998f;
		}
		else if (gain <= 0.000f)
		{
			gain = 0.0001f;
		}

		setEmulationVolume(gain);
	}
	if (ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract) == YES)
	{
		auto gain = getEmulationVolume();

		gain -= 0.05f;

		if (gain >= 1.000f)
		{
			gain = 0.9998f;
		}
		else if (gain <= 0.000f)
		{
			gain = 0.0001f;
		}

		setEmulationVolume(gain);
	}

	if (pChip8_instance->chip8_state.audio.needAudio)
	{
		SDL_SetAudioDeviceGain(SDL_GetAudioStreamDevice(audioStream), pChip8_instance->chip8_state.audio.emulatorVolume);
		SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(audioStream));

		// --- XO-Chip Audio Mode ---
		if (pChip8_quirks->_quirk_xo_chip == YES)
		{
			// --- Frame & audio constants ---
			constexpr size_t PATTERN_BITS = 16 * 8; // 16 bytes * 8 bits = 128

			// Phase increment per sample
			double phaseInc = pChip8_audio->pitch / (EMULATED_AUDIO_SAMPLING_RATE_FOR_CHIP8 * PATTERN_BITS);

			for (size_t i = 0; i < AUDIO_BUFFER_SIZE_FOR_XO_CHIP; ++i)
			{
				int bitIndex = static_cast<int>(pChip8_audio->phase * PATTERN_BITS) & (PATTERN_BITS - 1);
				int byteIndex = bitIndex / 8;
				int bitInByte = bitIndex & 7;

				// Convert bit to float sample: 1.0 or -1.0
				float sample = (pChip8_audio->audioPattern[byteIndex] & (0x80 >> bitInByte)) ? 1.0f : -1.0f;

				pChip8_audio->audioBuffer[i] = sample;
				pChip8_audio->phase += phaseInc;         // Then increment phase

				if (pChip8_audio->phase >= 1.0)
				{
					pChip8_audio->phase -= 1.0;
				}
			}

			SDL_PutAudioStreamData(audioStream, pChip8_audio->audioBuffer, sizeof(pChip8_audio->audioBuffer));
		}
		else
		{
			SDL_PutAudioStreamData(audioStream, tone, sizeof(tone));
		}
	}
	else
	{
		SDL_PauseAudioDevice(SDL_GetAudioStreamDevice(audioStream));
	}
}

void chip8_t::scrollDisplay(INC32 H, INC32 V)
{
	int rows = getScreenHeight();
	int cols = getScreenWidth();

	MAP8 pmask = pChip8_display->planes;

	for (uint8_t bit = 0; pmask; ++bit)
	{
		if ((pmask & (1 << bit)) == 0)
		{
			continue; // skip unused planes
		}

		auto& buffer = pChip8_display->gfx.gfx2D[bit];

		// --- Horizontal shift ---
		if (H > 0)
		{ // right
			for (int r = 0; r < rows; r++)
			{
				for (int c = cols - 1; c >= 0; c--)
				{
					if (c - H >= 0)
						buffer[r][c] = buffer[r][c - H];
					else
						buffer[r][c] = CLEAR;
				}
			}
		}
		else if (H < 0)
		{ // left
			int n = -H;
			for (int r = 0; r < rows; r++)
			{
				for (int c = 0; c < cols; c++)
				{
					if (c + n < cols)
						buffer[r][c] = buffer[r][c + n];
					else
						buffer[r][c] = CLEAR;
				}
			}
		}

		// --- Vertical shift ---
		if (V > 0)
		{ // up
			for (int c = 0; c < cols; c++)
			{
				for (int r = 0; r < rows; r++)
				{
					if (r + V < rows)
						buffer[r][c] = buffer[r + V][c];
					else
						buffer[r][c] = CLEAR;
				}
			}
		}
		else if (V < 0)
		{ // down
			int n = -V;
			for (int c = 0; c < cols; c++)
			{
				for (int r = rows - 1; r >= 0; r--)
				{
					if (r - n >= 0)
						buffer[r][c] = buffer[r - n][c];
					else
						buffer[r][c] = CLEAR;
				}
			}
		}

		pmask &= ~(1 << bit); // clear processed plane
	}
}

void chip8_t::displayCompleteScreen()
{
	if (pChip8_quirks->_quirk_xo_chip == YES)
	{
		/*
		 * NOTE:
		 * Always render both planes.
		 * `pChip8_display->planes` is used only to decide whether to add pixel data or not.
		 * For rendering, all planes MUST be rendered.
		 */


		// --- Configuration ---
		constexpr uint8_t NUM_PLANES = 2; // Change to 3 or 4 for future XO-Chip expansions
		constexpr uint8_t NUM_COLORS = 1 << NUM_PLANES; // 2^N colors

		// Example LUT: index = plane bits combination
		// For 2 planes: 0b00=background, 0b01=plane0 only, 0b10=plane1 only, 0b11=both
		const uint32_t planeColorLUT[NUM_COLORS] = {
			color1.n, // 0b00
			color0.n, // 0b01
			color2.n, // 0b10
			color3.n, // 0b11
			// Add more if NUM_PLANES > 2
		};

		for (uint32_t y = 0; y < getScreenHeight(); y++)
		{
			for (uint32_t x = 0; x < getScreenWidth(); x++)
			{
				uint8_t idx = 0;
				for (uint8_t plane = 0; plane < NUM_PLANES; plane++)
				{
					if (pChip8_display->gfx.gfx2D[plane][y][x] == SET)
					{
						idx |= (1 << plane);
					}
				}
				pChip8_display->imGuiBuffer.imGuiBuffer2D[y][x].n = planeColorLUT[idx];
			}
		}
	}
	else
	{
		for (uint32_t y = 0; y < getScreenHeight(); y++)
		{
			for (uint32_t x = 0; x < getScreenWidth(); x++)
			{
				pChip8_display->imGuiBuffer.imGuiBuffer2D[y][x].n 
					= ((pChip8_display->gfx.gfx2D[0][y][x] == SET) ? color0.n : color1.n);	
			}
		}
	}

#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

	glDisable(GL_BLEND);

	// Handle for chip8 system's texture

	glBindTexture(GL_TEXTURE_2D, chip8_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pChip8_display->imGuiBuffer.imGuiBuffer1D);

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
	// 1. Upload emulator framebuffer to chip8_texture
	GL_CALL(glBindTexture(GL_TEXTURE_2D, chip8_texture));
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getScreenWidth(), getScreenHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
		(GLvoid*)pChip8_display->imGuiBuffer.imGuiBuffer1D));

	// Choose filtering mode (NEAREST or LINEAR)
	GLint filter = (currEnVFilter == VIDEO_FILTERS::BILINEAR_FILTER) ? GL_LINEAR : GL_NEAREST;
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));

	// 2. Render chip8_texture into framebuffer (masquerade_texture target)
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));
	GL_CALL(glViewport(0, 0, getScreenWidth() * FRAME_BUFFER_SCALE, getScreenHeight() * FRAME_BUFFER_SCALE));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

	// Pass 1: Render base texture (Game Boy framebuffer)
	GL_CALL(glUseProgram(shaderProgramBasic));
	GL_CALL(glActiveTexture(GL_TEXTURE0));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, chip8_texture));
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

void chip8_t::clearCompleteScreen()
{
#if (GL_FIXED_FUNCTION_PIPELINE == YES) && !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
	// Bind the framebuffer used for the emulator display
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

	// Clear color to black and clear the color buffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#else
	// Modern OpenGL: bind framebuffer and clear
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));

	// Set viewport to full screen buffer size
	GL_CALL(glViewport(0, 0, getScreenWidth() * FRAME_BUFFER_SCALE, getScreenHeight() * FRAME_BUFFER_SCALE));

	// Clear color to black
	GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));

	// Clear the color buffer
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

	// Unbind framebuffer
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
#endif
}

bool chip8_t::processInterrupts()
{
	RETURN false;
}

bool chip8_t::runEmulationAtHostRate(uint32_t currentFrame)
{
	RETURN true;
}

bool chip8_t::runEmulationLoopAtHostRate(uint32_t currentFrame)
{
	RETURN true;
}

bool chip8_t::runEmulationAtFixedRate(uint32_t currentFrame)
{
	bool status = true;

	pChip8_display->vblank = CLEAR;

	pChip8_io->keyRelEvtA = pChip8_io->keyRelEvtC;
	pChip8_io->keyRelEvtC = CLEAR;

	captureIO();

	processTimers();	// Has to be called @60Hz for Chip8 Family

	processAudio();

	displayCompleteScreen();

	RETURN status;
}

bool chip8_t::runEmulationLoopAtFixedRate(uint32_t currentFrame)
{
	processCPU();

	RETURN pChip8_display->vblank;
}

bool chip8_t::processCPU()
{
	bool status = true;

	status = processOpcode();

	pChip8_io->keyRelEvtA = CLEAR;

	RETURN status;
}

bool chip8_t::processOpcode()
{
	auto clearDisplayBuffer = [&]()
		{
			uint32_t scanner = 0;
			for (uint32_t y = 0; y < getScreenHeight(); y++)
			{
				for (uint32_t x = 0; x < getScreenWidth(); x++)
				{
					pChip8_display->gfx.gfx1D[0][scanner] = CLEAR;
					pChip8_display->gfx.gfx1D[1][scanner] = CLEAR;
					pChip8_display->gfx.gfx1D[2][scanner] = CLEAR;
					pChip8_display->gfx.gfx1D[3][scanner] = CLEAR;
					scanner++;
				}
			}
		};

	auto handleConditionalJump = [&]() 
		{
			// Refer to https://blog.khutchins.com/posts/chip-8-emulation-xo-chip/
			if (pChip8_quirks->_quirk_xo_chip == YES)
			{
				uint16_t nextOpcode =
					(pChip8_memory->memory[pChip8_registers->pc] << 8
						| pChip8_memory->memory[pChip8_registers->pc + 1]);

				if (nextOpcode == 0xF000)
				{
					pChip8_registers->pc += FOUR;
				}
				else
				{
					pChip8_registers->pc += TWO;
				}
			}
			else
			{
				pChip8_registers->pc += TWO;
			}
		};

	bool status = true;

	if (pChip8_instance->chip8_state.cpuInstance.emulatedCPUCycle >= INST_PER_FRAME[TO_UINT(pChip8_quirks->variant)])
	{
		pChip8_instance->chip8_state.cpuInstance.emulatedCPUCycle -= INST_PER_FRAME[TO_UINT(pChip8_quirks->variant)];
		pChip8_display->vblank = YES;
	}

	pChip8_instance->chip8_state.cpuInstance.opcode =
		(pChip8_memory->memory[pChip8_registers->pc] << 8
			| pChip8_memory->memory[pChip8_registers->pc + 1]);

#if DISABLED
	LOG(
		"[%08x] V0:%02x V1:%02x V2:%02x V3:%02x V4:%02x V5:%02x V6:%02x V7:%02x "
		"V8:%02x V9:%02x VA:%02x VB:%02x VC:%02x VD:%02x VE:%02x VF:%02x "
		"I:%04x SP:%x PC:%04x O:%04x",
		pChip8_instance->chip8_state.cpuInstance.emulatedCPUCycle,
		pChip8_registers->V[0], pChip8_registers->V[1], pChip8_registers->V[2], pChip8_registers->V[3],
		pChip8_registers->V[4], pChip8_registers->V[5], pChip8_registers->V[6], pChip8_registers->V[7],
		pChip8_registers->V[8], pChip8_registers->V[9], pChip8_registers->V[0xA], pChip8_registers->V[0xB],
		pChip8_registers->V[0xC], pChip8_registers->V[0xD], pChip8_registers->V[0xE], pChip8_registers->V[0xF],
		pChip8_registers->I,
		pChip8_registers->sp,
		pChip8_registers->pc,
		pChip8_instance->chip8_state.cpuInstance.opcode
	);
#endif

	pChip8_registers->pc += TWO;

	switch ((pChip8_instance->chip8_state.cpuInstance.opcode & 0xF000))
	{
	case 0x0000:
	{
		int scale = (pChip8_display->res == C8RES::LORES) ? 2 : 1; // LORES=2x2 pixels, HIRES=1x1

		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) == 0xC0)
		{
			pChip8_display->scrollV = -(pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F);

			if (pChip8_quirks->_quirk_schip_legacy == YES && pChip8_display->res == C8RES::NON_EXTENDED)
			{
				// Ideally we would have to divide pChip8_display->scrollV by two here
				// But as per https://github.com/janitor-raus/CubeChip/blob/gameboooooy/guides/Legacy%20(Original)%20SuperCHIP%20Display%20Specification.md#extended-mode-off
				// this was just a side effect of the way doubling worked in DxyN instruction
				// Also, as per the same above link, S-Chip Legacy LORES also runs in 64 x 128 mode via the doubling method mentioned above
				// and hence doesnt need separate scaling for offsets
			}

			scrollDisplay(ZERO * scale, pChip8_display->scrollV * scale);
		}
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) == 0xD0)
		{
			if (pChip8_quirks->_quirk_xo_chip == YES)
			{
				pChip8_display->scrollV = (pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F);
			}
			else
			{
				FATAL("CPU Panic; unknown opcode: %04x", pChip8_instance->chip8_state.cpuInstance.opcode);
				status = false;
				BREAK;
			}

			scrollDisplay(ZERO * scale, pChip8_display->scrollV * scale);
		}
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0xE0)
		{
			MAP8 pmask = pChip8_display->planes;
			for (uint8_t bit = 0; pmask; ++bit)
			{
				if ((pmask & (1 << bit)) == 0)
				{
					continue; // skip unused planes
				}

				uint32_t scanner = 0;
				for (uint32_t y = 0; y < getScreenHeight(); y++)
				{
					for (uint32_t x = 0; x < getScreenWidth(); x++)
					{
						pChip8_display->gfx.gfx1D[bit][scanner++] = CLEAR;
					}
				}

				pmask &= ~(1 << bit); // clear processed plane
			}
		}
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0xEE)
		{
			pChip8_registers->pc = pChip8_registers->stack[--pChip8_registers->sp];
		}
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0xFB)
		{
			pChip8_display->scrollH = FOUR;
			if (pChip8_quirks->_quirk_schip_legacy == YES && pChip8_display->res == C8RES::NON_EXTENDED)
			{
				// Ideally we would have to divide pChip8_display->scrollH by two here
				// But as per https://github.com/janitor-raus/CubeChip/blob/gameboooooy/guides/Legacy%20(Original)%20SuperCHIP%20Display%20Specification.md#extended-mode-off
				// this was just a side effect of the way doubling worked in DxyN instruction
				// Also, as per the same above link, S-Chip Legacy LORES also runs in 64 x 128 mode via the doubling method mentioned above
				// and hence doesnt need separate scaling for offsets
			}
			scrollDisplay(pChip8_display->scrollH * scale, ZERO * scale);
		}
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0xFC)
		{
			pChip8_display->scrollH = -FOUR;
			if (pChip8_quirks->_quirk_schip_legacy == YES && pChip8_display->res == C8RES::NON_EXTENDED)
			{
				// Ideally we would have to divide pChip8_display->scrollH by two here
				// But as per https://github.com/janitor-raus/CubeChip/blob/gameboooooy/guides/Legacy%20(Original)%20SuperCHIP%20Display%20Specification.md#extended-mode-off
				// this was just a side effect of the way doubling worked in DxyN instruction
				// Also, as per the same above link, S-Chip Legacy LORES also runs in 64 x 128 mode via the doubling method mentioned above
				// and hence doesnt need separate scaling for offsets
			}
			scrollDisplay(pChip8_display->scrollH * scale, ZERO * scale);
		}
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0xFD)
		{
			FATAL("STOP");
		}
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0xFE)
		{
			if ((pChip8_quirks->_quirk_schip_modern == YES || pChip8_quirks->_quirk_xo_chip == YES)) 
			{
				clearDisplayBuffer();
				pChip8_display->res = C8RES::LORES;
			}
			else if (pChip8_quirks->_quirk_schip_legacy == YES)
			{
				pChip8_display->res = C8RES::NON_EXTENDED;	// Disable Extended Mode
			}
		}
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0xFF)
		{
			if ((pChip8_quirks->_quirk_schip_modern == YES || pChip8_quirks->_quirk_xo_chip == YES))
			{
				clearDisplayBuffer();
				pChip8_display->res = C8RES::HIRES;
			}
			else if (pChip8_quirks->_quirk_schip_legacy == YES)
			{
				pChip8_display->res = C8RES::EXTENDED;	// Enable Extended Mode
			}
		}
		BREAK;
	}
	case 0x1000:
	{
		pChip8_registers->pc = pChip8_instance->chip8_state.cpuInstance.opcode & 0x0FFF;
		BREAK;
	}
	case 0x2000:
	{
		pChip8_registers->stack[pChip8_registers->sp++] = pChip8_registers->pc;
		pChip8_registers->pc = pChip8_instance->chip8_state.cpuInstance.opcode & 0x0FFF;
		BREAK;
	}
	case 0x3000:
	{
		if (pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8]
			== (pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF))
		{
			handleConditionalJump();
		}
		BREAK;
	}
	case 0x4000:
	{
		if (pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8]
			!= (pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF))
		{
			handleConditionalJump();
		}
		BREAK;
	}
	case 0x5000:
	{
		uint8_t x = ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8);
		uint8_t y = ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4);

		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F) == 0x00)
		{
			if (pChip8_registers->V[x] == pChip8_registers->V[y])
			{
				handleConditionalJump();
			}
		}
		else if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F) == 0x02)
		{
			// SAVE Vx�Vy
			int8_t step = (x <= y) ? 1 : -1;
			uint8_t offset = RESET;

			for (int8_t ii = x; ; ii += step)
			{
				auto index = ((pChip8_registers->I + offset) & 0xFFFF);
				pChip8_memory->memory[index] = pChip8_registers->V[ii];
				++offset;

				if (ii == y)
				{
					BREAK;
				}
			}
		}
		else if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F) == 0x03)
		{
			// LOAD Vx�Vy
			int8_t step = (x <= y) ? 1 : -1;
			uint8_t offset = RESET;

			for (int8_t ii = x; ; ii += step)
			{
				auto index = ((pChip8_registers->I + offset) & 0xFFFF);
				pChip8_registers->V[ii] = pChip8_memory->memory[index];
				++offset;

				if (ii == y)
				{
					BREAK;
				}
			}
		}
		BREAK;
	}
	case 0x6000:
	{
		pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8]
			= pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF;
		BREAK;
	}
	case 0x7000:
	{
		pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8]
			+= pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF;
		BREAK;
	}
	case 0x8000:
	{
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F) == 0x00)
		{
			pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8]
				= pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4];
		}
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F) == 0x01)
		{
			pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8]
				|= pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4];

			if (pChip8_quirks->_QUIRK_VF_RESET)
			{
				pChip8_registers->V[0xF] = 0;
			}
		}
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F) == 0x02)
		{
			pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8]
				&= pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4];

			if (pChip8_quirks->_QUIRK_VF_RESET)
			{
				pChip8_registers->V[0xF] = 0;
			}
		}
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F) == 0x03)
		{
			pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8]
				^= pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4];

			if (pChip8_quirks->_QUIRK_VF_RESET)
			{
				pChip8_registers->V[0xF] = 0;
			}
		}
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F) == 0x04)
		{
			pAbsolute_chip8_instance->absolute_chip8_state.emulationSpecific_TestVariable =
				pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8]
				+ pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4];

			pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8]
				+= pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4];

			if (pAbsolute_chip8_instance->absolute_chip8_state.emulationSpecific_TestVariable > 0xFF)
				pChip8_registers->V[0xF] = 1;
			else
				pChip8_registers->V[0xF] = 0;
			pAbsolute_chip8_instance->absolute_chip8_state.emulationSpecific_TestVariable = 0;
		}
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F) == 0x05)
		{
			bool borrowOccurred = false;

			pAbsolute_chip8_instance->absolute_chip8_state.emulationSpecific_TestVariable = pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8]
				- pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4];

			if (pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4] >
				pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8])
			{
				borrowOccurred = true;
				if (((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8) != 0xF) // handle case when Vx = V[F]
				{
					pChip8_registers->V[0xF] = 0;
				}
			}
			else
			{
				borrowOccurred = false;
				if (((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8) != 0xF) // handle case when Vx = V[F]
				{
					pChip8_registers->V[0xF] = 1;
				}
			}

			if (borrowOccurred)
			{
				pAbsolute_chip8_instance->absolute_chip8_state.emulationSpecific_TestVariable += 256;
			}

			pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8] = pAbsolute_chip8_instance->absolute_chip8_state.emulationSpecific_TestVariable & 0xFF;

			// handle case when Vx = V[F]
			if (((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8) == 0xF)
			{
				if (borrowOccurred)
				{
					pChip8_registers->V[0xF] = 0;
				}
				else
				{
					pChip8_registers->V[0xF] = 1;
				}
			}

			pAbsolute_chip8_instance->absolute_chip8_state.emulationSpecific_TestVariable = 0;
		}
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F) == 0x06)
		{
			if (pChip8_quirks->_QUIRK_SHIFT)
			{
				auto bitShiftedOut = (pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8] & 0x01);					// get LSB
				pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8] >>= 1;
				pChip8_registers->V[0xF] = bitShiftedOut;
			}
			else
			{
				auto bitShiftedOut = (pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4] & 0x01);					// get LSB
				pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8] = (BYTE)((pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4]) >> 1);
				pChip8_registers->V[0xF] = bitShiftedOut;
			}
		}
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F) == 0x07)
		{
			bool borrowOccurred = false;

			pAbsolute_chip8_instance->absolute_chip8_state.emulationSpecific_TestVariable = (pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4])
				- (pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8]);

			if (pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8] >
				pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4])
			{
				borrowOccurred = true;
				if (((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8) != 0xF) // handle case when Vx = V[F]
				{
					pChip8_registers->V[0xF] = 0;
				}
			}
			else
			{
				borrowOccurred = false;
				if (((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8) != 0xF) // handle case when Vx = V[F]
				{
					pChip8_registers->V[0xF] = 1;
				}
			}

			if (borrowOccurred)
			{
				pAbsolute_chip8_instance->absolute_chip8_state.emulationSpecific_TestVariable += 256;
			}

			pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8] = pAbsolute_chip8_instance->absolute_chip8_state.emulationSpecific_TestVariable & 0xFF;

			// handle case when Vx = V[F]
			if (((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8) == 0xF)
			{
				if (borrowOccurred)
				{
					pChip8_registers->V[0xF] = 0;
				}
				else
				{
					pChip8_registers->V[0xF] = 1;
				}
			}

			pAbsolute_chip8_instance->absolute_chip8_state.emulationSpecific_TestVariable = 0;
		}
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F) == 0x0E)
		{
			if (pChip8_quirks->_QUIRK_SHIFT)
			{
				auto bitShiftedOut = ((pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8] & 0x80) >> 7);					// get MSB
				pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8] <<= 1;
				pChip8_registers->V[0xF] = bitShiftedOut;
			}
			else
			{
				auto bitShiftedOut = ((pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4] & 0x80) >> 7);					// get MSB
				pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8] = (BYTE)((pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4]) << 1);
				pChip8_registers->V[0xF] = bitShiftedOut;
			}
		}
		BREAK;
	}
	case 0x9000:
	{
		if (pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8]
			!= pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4])
		{
			handleConditionalJump();
		}
		BREAK;
	}
	case 0xA000:
	{
		pChip8_registers->I = pChip8_instance->chip8_state.cpuInstance.opcode & 0x0FFF;
		BREAK;
	}
	case 0xB000:
	{
		if (pChip8_quirks->_QUIRK_JUMP)
		{
			pChip8_registers->pc = (pChip8_instance->chip8_state.cpuInstance.opcode & 0x0FFF) + pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8];
		}
		else
		{
			pChip8_registers->pc = (pChip8_instance->chip8_state.cpuInstance.opcode & 0x0FFF) + pChip8_registers->V[0];
		}
		BREAK;
	}
	case 0xC000:
	{
		pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8]
			= ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) & ((rand() & 255) + 1));
		BREAK;
	}
	case 0xD000:
	{
		/*
		* THEORY:
		* 
		* if _QUIRK_DISPLAY_WAIT is enabled, at max we should allow only 60 sprites per second
		* since we have a 60 Hz interrupt, we can use this to achieve the above goal
		* 60 spites =>  1 second is same as 1 sprite in 1/60 second which is 1 sprite per 60 Hz interrupt
		* Since the only way to draw the sprite is to use the DXYN instruction
		* we can basically limit the number of DXYN instructions per 60 Hz to 1
		* if we get more than one, we spin, i.e. we don't run the DXYN instruction and also not increment the PC
		* and in the next 60 Hz, the first instruction would be the spinning DXYN, which would run and if we encounter more DXYN in same 60 Hz interrupt
		* we repeat the above mentioned process
		* Drawback being we are wasting cycles per 60 Hz interrupt when we spin
		*
		* IMPLEMENATION:
		* Now, instead of keeping count of number of DXYN per 60 Hz, we can execute the DXYN that is the first instruction per 60 Hz
		* Both result in same behaviour as, limitting DXYN to 1 implies spinning if more DXYN occurs until next 60 Hz,
		* which again implies that DXYN would be the first instruction of the consecutive 60 Hz interrupts
		*
		*/
		/*
		*  Drawing Example:
		*
		*  Draws a sprite at coordinate(VX, VY) that has a width of 8 pixels and a height of N pixels.
		*  Each row of 8 pixels is read as bit - coded starting from pChip8_memory->memory location pChip8_registers->I;
		*  pChip8_registers->I value doesn�t change after the execution of this instruction.As described above,
		*  VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn,
		*  and to 0 if that doesn�t happen.
		*
		*  Lets assume it the pChip8_instance->chip8_state.cpuInstance.opcode was 0xD003.
		*  This means it wants to draw a sprite at location 0, 0 which is 3 rows high.At pChip8_memory->memory location pChip8_registers->I,
		*  the following values were set :
		*
		*  pChip8_memory->memory[pChip8_registers->I] = 0x3C;
		*  pChip8_memory->memory[pChip8_registers->I + 1] = 0xC3;
		*  pChip8_memory->memory[pChip8_registers->I + 2] = 0xFF;
		*
		*  How do these 3 bytes represent a sprite ? Take a look at the binary values of each byte :
		*
		*  HEX    BIN        Sprite
		*  0x3C   00111100    ****
		*  0xC3   11000011  **    **
		*  0xFF   11111111  ********
		*/

		// ------------------------------------------------------------
		// DISPLAY WAIT quirk handling
		// ------------------------------------------------------------
		if (pChip8_quirks->_QUIRK_DISPLAY_WAIT == ENABLED)
		{
			// --- Special case: S-CHIP Legacy only in LORES ---
			// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1165733850182332542	
			bool applyDisplayWait = true; // default: wait applies
			if (pChip8_quirks->_quirk_schip_legacy == YES &&
				pChip8_display->res != C8RES::NON_EXTENDED)
			{
				applyDisplayWait = false; // skip WAIT for NON_EXTENDED
			}

			// --- Apply DISPLAY WAIT if needed ---
			if (applyDisplayWait && pChip8_display->vblank == NO)
			{
				// Stall execution until next VBlank, safely decrement PC
				pChip8_registers->pc = (pChip8_registers->pc >= 2) ? pChip8_registers->pc - 2 : 0;

				BREAK;
			}
		}

		// Note that physical display buffer size is set to 64 x 128

		if (pChip8_quirks->_quirk_chip8 == YES || pChip8_quirks->_quirk_modern_chip8 == YES)
		{
			// Chip8 is designed to work only at 32 x 64
			// All quirks w.r.t Dxyn is supposed to assume the display boundary as 32 x 64
			// Only the final display needs to be scaled and set in the imGuiBuffer1D/imGuiBuffer2D buffer
			// One option would be to fill physical display buffer is to scale a 1x1 pixel to 2x2 towards south-east direction

			uint8_t xi = 0;
			uint8_t yi = 0;
			uint8_t zy = 0;
			uint8_t zx = 0;
			uint8_t sprite_height_i = 0;
			const uint8_t sprite_width = 8;
			uint16_t pixel_i = 0;
			const uint8_t scale = 2;
			const uint8_t actual_width = getScreenWidth() / scale;
			const uint8_t actual_height = getScreenHeight() / scale;
			const uint8_t x = ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8);
			const uint8_t y = ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4);

			xi = pChip8_registers->V[x];
			yi = pChip8_registers->V[y];

			if (pChip8_quirks->_QUIRK_CLIP)
			{
				// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
				xi &= (actual_width - 1);	// This is masking, not mod
				yi &= (actual_height - 1);	// This is masking, not mod
			}

			sprite_height_i = pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F;
			pChip8_registers->V[0xF] = 0;																						// VF is cleared

			for (uint32_t yy = 0; yy < sprite_height_i; yy++)																	// loop over height of sprite
			{
				// Handle vertical cliping/wrapping
				if (pChip8_quirks->_QUIRK_CLIP)
				{
					zy = (yy + yi);
					// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
					if (zy >= actual_height)
					{
						BREAK;																									// clip
					}
				}
				else
				{
					zy = (yy + yi) & (actual_height - 1);																		// wrap around the height
				}

				// Read the 1 byte pixel data 
				auto index = ((pChip8_registers->I + yy) & 0xFFFF);
				pixel_i = pChip8_memory->memory[index];

				for (uint32_t xx = 0; xx < sprite_width; xx++)																	// loop over width of sprite
				{
					// Handle horizontal cliping/wrapping
					if (pChip8_quirks->_QUIRK_CLIP)
					{
						zx = (xx + xi);
						// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
						if (zx >= actual_width)
						{
							BREAK;																								// clip
						}
					}
					else
					{
						zx = (xx + xi) & (actual_width - 1);																	// wrap around the width
					}

					// Note that zx and zy are still in 32 x 64 domain
					// We need to scale it to 2x2 pixel i.e. (zx,zy) -> (sx,sy)
					// For this we need to do 2 things
					// zx and zy from here onwards should always skip odd numbers i.e 0->2->4... AND
					// for odd number, we just have to put the same color as its previous pixel

					auto sx = zx * scale; // multiply by 2
					auto sy = zy * scale; // multiply by 2

					// Check bit from MSB to LSB
					if ((pixel_i & (1 << (sprite_width - 1 - xx))) != 0)														// if bit is set in pixel_i
					{
						// Chip8 uses XOR logic while displaying pixels
						if (pChip8_display->gfx.gfx2D[0][sy][sx] == SET)														// was set
						{
							for (int dy = 0; dy < scale; dy++)
							{
								for (int dx = 0; dx < scale; dx++)
								{
									// dx=0,dy=0 is actual pixel
									// dx=0,dy=1 is scaled pixel
									// dx=1,dy=0 is scaled pixel
									// dx=1,dy=1 is scaled pixel
									pChip8_display->gfx.gfx2D[0][sy + dy][sx + dx] = CLEAR;										// to un-set
								}
							}

							// Chip8 indicates collision per sprite, not per sprite row
							pChip8_registers->V[0xF] = 1;																		// VF set to 1
						}
						else																									// was un-set
						{
							for (int dy = 0; dy < scale; dy++)
							{
								for (int dx = 0; dx < scale; dx++)
								{
									// dx=0,dy=0 is actual pixel
									// dx=0,dy=1 is scaled pixel
									// dx=1,dy=0 is scaled pixel
									// dx=1,dy=1 is scaled pixel
									pChip8_display->gfx.gfx2D[0][sy + dy][sx + dx] = SET;										// to set
								}
							}
						}
					}
				}
			}
		}
		else if (pChip8_quirks->_quirk_schip_legacy == YES)
		{
			// Refer to https://github.com/janitor-raus/CubeChip/blob/gameboooooy/guides/Legacy%20(Original)%20SuperCHIP%20Display%20Specification.md
			if (pChip8_display->res == C8RES::NON_EXTENDED)
			{
				auto bitBloat = [](uint32_t value) -> uint32_t {
					if (value == 0) RETURN 0;

					value = (value << 4 | value) & 0x0F0F;
					value = (value << 2 | value) & 0x3333;
					value = (value << 1 | value) & 0x5555;

					RETURN (value << 1) | value;
					};

				// S-Chip (Legacy) LORES is designed to work at 64 x 128 NOT 32 x 64
				// All quirks w.r.t Dxyn is supposed to assume the display boundary as 64 x 128 NOT 32 x 64

				uint32_t xi = 0;
				uint32_t yi = 0;
				uint32_t zy = 0;
				uint32_t zx = 0;
				uint32_t sprite_height_i = 0;
				// Refer https://github.com/janitor-raus/CubeChip/blob/gameboooooy/guides/Legacy%20(Original)%20SuperCHIP%20Display%20Specification.md#extended-mode-off
				const uint32_t sprite_width = 32;
				uint32_t pixel_i = 0;
				const uint32_t actual_width = getScreenWidth();
				const uint32_t actual_height = getScreenHeight();
				const uint32_t x = ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8);
				const uint32_t y = ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4);

				xi = pChip8_registers->V[x];
				yi = pChip8_registers->V[y];

				// Step 1
				uint8_t xOff = (16 - ((xi & 0x07) << 1)); // optimized version of (16 - ((xi % 8) * 2))

				// Step 2a
				xi <<= 1;	// Xly by 2
				yi <<= 1;   // Xly by 2

				// Step 2b
				if (pChip8_quirks->_QUIRK_CLIP)
				{
					// Refer to https://github.com/janitor-raus/CubeChip/blob/gameboooooy/guides/Legacy%20(Original)%20SuperCHIP%20Display%20Specification.md#extended-mode-off
					xi &= 0x70;	// This is masking, not mod
					yi &= 0x3E;	// This is masking, not mod
				}

				sprite_height_i = pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F;											// extract the sprite height 

				// Special case: S-Chip (Legacy) LORES Dxy0 -> 8x16 sprites 
				// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1165733850182332542 for 8x16
				if (sprite_height_i == 0)
				{
					sprite_height_i = 16;   // 16 rows
				}

				pChip8_registers->V[0xF] = 0;																						// VF is cleared

				// Step 3
				for (uint32_t yy = 0; yy < sprite_height_i; yy++)																	// loop over height of sprite
				{
					// Handle vertical cliping/wrapping
					if (pChip8_quirks->_QUIRK_CLIP)
					{
						zy = ((yy * 2) + yi);
						// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
						if (zy >= actual_height)
						{
							BREAK;																									// clip
						}
					}
					else
					{
						zy = ((yy * 2) + yi) & (actual_height - 1);																	// wrap around the height
					}

					// Step 4
					// Read the 1 byte pixel data 
					auto index = ((pChip8_registers->I + yy) & 0xFFFF);
					pixel_i = (uint32_t)pChip8_memory->memory[index];
					// Bloat the byte
					pixel_i = bitBloat(pixel_i);						
					// Shift the byte
					pixel_i <<= xOff;

					// Step 5
					for (uint32_t xx = 0; xx < sprite_width; xx++)																	// loop over 32 columns
					{
						// Handle horizontal cliping/wrapping
						if (pChip8_quirks->_QUIRK_CLIP)
						{
							zx = (xx + xi);
							// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
							if (zx >= actual_width)
							{
								BREAK;																								// clip
							}
						}
						else
						{
							zx = (xx + xi) & (actual_width - 1);																	// wrap around the width
						}

						// Step 6
						// Check bit from MSB to LSB
						if ((pixel_i & (1 << (sprite_width - 1 - xx))) != 0)														// if bit is set in pixel_i
						{
							// Chip8 uses XOR logic while displaying pixels
							if (pChip8_display->gfx.gfx2D[0][zy][zx] == SET)														// was set
							{
								pChip8_display->gfx.gfx2D[0][zy][zx] = CLEAR;														// to un-set

								// Chip8 indicates collision per sprite, not per sprite row
								pChip8_registers->V[0xF] = 1;																		// VF set to 1
							}
							else																									// was un-set
							{
								pChip8_display->gfx.gfx2D[0][zy][zx] = SET;															// to set
							}
						}

						// Step 7
						// Copy to next row...
						pChip8_display->gfx.gfx2D[0][zy + 1][zx] = pChip8_display->gfx.gfx2D[0][zy][zx];
					}
				}
			}
			else if (pChip8_display->res == C8RES::EXTENDED)
			{
				// S-Chip (Legacy) HIRES is designed to work at 64 x 128
				// All quirks w.r.t Dxyn is supposed to assume the display boundary as 64 x 128

				uint8_t xi = 0;
				uint8_t yi = 0;
				uint8_t zy = 0;
				uint8_t zx = 0;
				uint8_t sprite_height_i = 0;
				uint8_t sprite_width = 8;
				uint16_t pixel_i = 0;
				const uint8_t actual_width = getScreenWidth();
				const uint8_t actual_height = getScreenHeight();
				const uint8_t x = ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8);
				const uint8_t y = ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4);
				bool schip_16x16 = false;

				xi = pChip8_registers->V[x];
				yi = pChip8_registers->V[y];

				if (pChip8_quirks->_QUIRK_CLIP)
				{
					// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
					xi &= (actual_width - 1);	// This is masking, not mod
					yi &= (actual_height - 1);	// This is masking, not mod
				}

				sprite_height_i = pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F;											// extract the sprite height 

				// Special case: S-Chip (Legacy) HIRES Dxy0 -> 16x16 sprites 
				if (sprite_height_i == 0)
				{
					sprite_height_i = 16;   // 16 rows
					sprite_width = 16;		// 16 columns
					schip_16x16 = true;
				}

				pChip8_registers->V[0xF] = 0;																						// VF is cleared

				for (uint32_t yy = 0; yy < sprite_height_i; yy++)																	// loop over height of sprite
				{
					// Handle vertical cliping/wrapping
					if (pChip8_quirks->_QUIRK_CLIP)
					{
						zy = (yy + yi);
						// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
						if (zy >= actual_height)
						{
							// S-Chip (Legacy) HIRES mode sets VF to the number of sprite rows with collisions plus the number of rows clipped at the bottom border
							// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1165733850182332542
							++pChip8_registers->V[0xF];																				// increment VF

							continue;																								// clip
						}
					}
					else
					{
						zy = (yy + yi) & (actual_height - 1);																		// wrap around the height
					}
		
					// Read the 2 byte pixel data for 16x16 sprite 
					if (schip_16x16 == true)
					{
						// First byte always @ even index
						// Second byte always @ odd index
						auto index0 = ((pChip8_registers->I + (yy * 2)) & 0xFFFF);
						auto index1 = ((pChip8_registers->I + ((yy * 2) + 1)) & 0xFFFF);
						pixel_i = (pChip8_memory->memory[index0] << 8) | pChip8_memory->memory[index1];
					}
					// Read the 1 byte pixel data 
					else
					{
						auto index = ((pChip8_registers->I + yy) & 0xFFFF);
						pixel_i = pChip8_memory->memory[index];
					}

					bool row_collision_detected = NO;																				// flag to detect sprite row collision
					for (uint32_t xx = 0; xx < sprite_width; xx++)																	// loop over width of sprite
					{
						// Handle horizontal cliping/wrapping
						if (pChip8_quirks->_QUIRK_CLIP)
						{
							zx = (xx + xi);
							// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
							if (zx >= actual_width)
							{
								BREAK;																								// clip
							}
						}
						else
						{
							zx = (xx + xi) & (actual_width - 1);																	// wrap around the width
						}

						// Check bit from MSB to LSB
						if ((pixel_i & (1 << (sprite_width - 1 - xx))) != 0)														// if bit is set in pixel_i
						{
							// Chip8 uses XOR logic while displaying pixels
							if (pChip8_display->gfx.gfx2D[0][zy][zx] == SET)														// was set
							{
								pChip8_display->gfx.gfx2D[0][zy][zx] = CLEAR;														// to un-set

								if (row_collision_detected == NO)
								{
									// S-Chip (Legacy) HIRES mode sets VF to the number of sprite rows with collisions plus the number of rows clipped at the bottom border
									// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1165733850182332542
									++pChip8_registers->V[0xF];																		// increment VF

									row_collision_detected = YES;																	// Set the row collision detected flag
								}
							}
							else																									// was un-set
							{
								pChip8_display->gfx.gfx2D[0][zy][zx] = SET;															// to set
							}
						}
					}
				}
			}
		}
		else if (pChip8_quirks->_quirk_schip_modern == YES)
		{
			if (pChip8_display->res == C8RES::LORES)
			{
				// S-Chip (Modern) LORES is designed to work at 32 x 64
				// All quirks w.r.t Dxyn is supposed to assume the display boundary as 32 x 64
					
				// Ideally S-Chip (Modern) LORES should not have doubling, 
				// but then this would result in variable SDL window size and variable size imGuiBuffer1D/imGuiBuffer2D buffer
				// So, we still scale final display and set in the imGuiBuffer1D/imGuiBuffer2D buffer
				// One option would be to fill physical display buffer is to scale a 1x1 pixel to 2x2 towards south-east direction

				uint8_t xi = 0;
				uint8_t yi = 0;
				uint8_t zy = 0;
				uint8_t zx = 0;
				uint8_t sprite_height_i = 0;
				uint8_t sprite_width = 8;
				uint16_t pixel_i = 0;
				const uint8_t scale = 2;
				const uint8_t actual_width = getScreenWidth() / scale;
				const uint8_t actual_height = getScreenHeight() / scale;
				const uint8_t x = ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8);
				const uint8_t y = ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4);
				bool schip_16x16 = false;

				xi = pChip8_registers->V[x];
				yi = pChip8_registers->V[y];

				if (pChip8_quirks->_QUIRK_CLIP)
				{
					// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
					xi &= (actual_width - 1);	// This is masking, not mod
					yi &= (actual_height - 1);	// This is masking, not mod
				}

				sprite_height_i = pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F;											// extract the sprite height 

				// Special case: S-Chip (Modern) LORES Dxy0 -> 16x16 sprites 
				if (sprite_height_i == 0)
				{
					sprite_height_i = 16;   // 16 rows
					sprite_width = 16;		// 16 columns
					schip_16x16 = true;
				}

				pChip8_registers->V[0xF] = 0;																						// VF is cleared

				for (uint32_t yy = 0; yy < sprite_height_i; yy++)																		// loop over height of sprite
				{
					// Handle vertical cliping/wrapping
					if (pChip8_quirks->_QUIRK_CLIP)
					{
						zy = (yy + yi);
						// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
						if (zy >= actual_height)
						{
							BREAK;																									// clip
						}
					}
					else
					{
						zy = (yy + yi) & (actual_height - 1);																		// wrap around the height
					}

					// Read the 2 byte pixel data for 16x16 sprite 
					if (schip_16x16 == true)
					{
						// First byte always @ even index
						// Second byte always @ odd index
						auto index0 = ((pChip8_registers->I + (yy * 2)) & 0xFFFF);
						auto index1 = ((pChip8_registers->I + ((yy * 2) + 1)) & 0xFFFF);
						pixel_i = (pChip8_memory->memory[index0] << 8) | pChip8_memory->memory[index1];
					}
					// Read the 1 byte pixel data 
					else
					{
						auto index = ((pChip8_registers->I + yy) & 0xFFFF);
						pixel_i = pChip8_memory->memory[index];
					}

					for (uint32_t xx = 0; xx < sprite_width; xx++)																		// loop over width of sprite
					{
						// Handle horizontal cliping/wrapping
						if (pChip8_quirks->_QUIRK_CLIP)
						{
							zx = (xx + xi);
							// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
							if (zx >= actual_width)
							{
								BREAK;																								// clip
							}
						}
						else
						{
							zx = (xx + xi) & (actual_width - 1);																	// wrap around the width
						}

						// Note that zx and zy are still in 32 x 64 domain
						// We need to scale it to 2x2 pixel i.e. (zx,zy) -> (sx,sy)
						// For this we need to do 2 things
						// zx and zy from here onwards should always skip odd numbers i.e 0->2->4... AND
						// for odd number, we just have to put the same color as its previous pixel

						auto sx = zx * scale; // multiply by 2
						auto sy = zy * scale; // multiply by 2

						// Check bit from MSB to LSB
						if ((pixel_i & (1 << (sprite_width - 1 - xx))) != 0)														// if bit is set in pixel_i
						{
							// Chip8 uses XOR logic while displaying pixels
							if (pChip8_display->gfx.gfx2D[0][sy][sx] == SET)														// was set
							{
								for (int dy = 0; dy < scale; dy++)
								{
									for (int dx = 0; dx < scale; dx++)
									{
										// dx=0,dy=0 is actual pixel
										// dx=0,dy=1 is scaled pixel
										// dx=1,dy=0 is scaled pixel
										// dx=1,dy=1 is scaled pixel
										pChip8_display->gfx.gfx2D[0][sy + dy][sx + dx] = CLEAR;										// to un-set
									}
								}

								// Chip8 indicates collision per sprite, not per sprite row
								pChip8_registers->V[0xF] = 1;																		// VF set to 1
							}
							else																									// was un-set
							{
								for (int dy = 0; dy < scale; dy++)
								{
									for (int dx = 0; dx < scale; dx++)
									{
										// dx=0,dy=0 is actual pixel
										// dx=0,dy=1 is scaled pixel
										// dx=1,dy=0 is scaled pixel
										// dx=1,dy=1 is scaled pixel
										pChip8_display->gfx.gfx2D[0][sy + dy][sx + dx] = SET;										// to un-set
									}
								}
							}
						}
					}
				}
			}
			else if (pChip8_display->res == C8RES::HIRES)
			{
				// S-Chip (Modern) HIRES is designed to work at 64 x 128
				// All quirks w.r.t Dxyn is supposed to assume the display boundary as 64 x 128

				uint8_t xi = 0;
				uint8_t yi = 0;
				uint8_t zy = 0;
				uint8_t zx = 0;
				uint8_t sprite_height_i = 0;
				uint8_t sprite_width = 8;
				uint16_t pixel_i = 0;
				const uint8_t actual_width = getScreenWidth();
				const uint8_t actual_height = getScreenHeight();
				const uint8_t x = ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8);
				const uint8_t y = ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4);
				bool schip_16x16 = false;

				xi = pChip8_registers->V[x];
				yi = pChip8_registers->V[y];

				if (pChip8_quirks->_QUIRK_CLIP)
				{
					// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
					xi &= (actual_width - 1);	// This is masking, not mod
					yi &= (actual_height - 1);	// This is masking, not mod
				}

				sprite_height_i = pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F;											// extract the sprite height 

				// Special case: S-Chip (Legacy) HIRES Dxy0 -> 16x16 sprites 
				if (sprite_height_i == 0)
				{
					sprite_height_i = 16;   // 16 rows
					sprite_width = 16;		// 16 columns
					schip_16x16 = true;
				}

				pChip8_registers->V[0xF] = 0;																						// VF is cleared

				for (uint32_t yy = 0; yy < sprite_height_i; yy++)																		// loop over height of sprite
				{
					// Handle vertical cliping/wrapping
					if (pChip8_quirks->_QUIRK_CLIP)
					{
						zy = (yy + yi);
						// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
						if (zy >= actual_height)
						{
							// S-Chip (Legacy) HIRES mode sets VF to the number of sprite rows with collisions plus the number of rows clipped at the bottom border
							// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1165733850182332542
							++pChip8_registers->V[0xF];																				// increment VF

							continue;																								// clip
						}
					}
					else
					{
						zy = (yy + yi) & (actual_height - 1);																		// wrap around the height
					}

					// Read the 2 byte pixel data for 16x16 sprite 
					if (schip_16x16 == true)
					{
						// First byte always @ even index
						// Second byte always @ odd index
						auto index0 = ((pChip8_registers->I + (yy * 2)) & 0xFFFF);
						auto index1 = ((pChip8_registers->I + ((yy * 2) + 1)) & 0xFFFF);
						pixel_i = (pChip8_memory->memory[index0] << 8) | pChip8_memory->memory[index1];
					}
					// Read the 1 byte pixel data 
					else
					{
						auto index = ((pChip8_registers->I + yy) & 0xFFFF);
						pixel_i = pChip8_memory->memory[index];
					}

					bool row_collision_detected = NO;																				// flag to detect sprite row collision
					for (uint32_t xx = 0; xx < sprite_width; xx++)																		// loop over width of sprite
					{
						// Handle horizontal cliping/wrapping
						if (pChip8_quirks->_QUIRK_CLIP)
						{
							zx = (xx + xi);
							// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
							if (zx >= actual_width)
							{
								BREAK;																								// clip
							}
						}
						else
						{
							zx = (xx + xi) & (actual_width - 1);																	// wrap around the width
						}

						// Check bit from MSB to LSB
						if ((pixel_i & (1 << (sprite_width - 1 - xx))) != 0)														// if bit is set in pixel_i
						{
							// Chip8 uses XOR logic while displaying pixels
							if (pChip8_display->gfx.gfx2D[0][zy][zx] == SET)														// was set
							{
								pChip8_display->gfx.gfx2D[0][zy][zx] = CLEAR;														// to un-set

								if (row_collision_detected == NO)
								{
									// S-Chip (Legacy) HIRES mode sets VF to the number of sprite rows with collisions plus the number of rows clipped at the bottom border
									// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1165733850182332542
									++pChip8_registers->V[0xF];																		// increment VF

									row_collision_detected = YES;																	// Set the row collision detected flag
								}
							}
							else																									// was un-set
							{
								pChip8_display->gfx.gfx2D[0][zy][zx] = SET;															// to un-set
							}
						}
					}
				}
			}
		}
		else if (pChip8_quirks->_quirk_xo_chip == YES)
		{
			if (pChip8_display->res == C8RES::LORES)
			{
				// XO-Chip LORES is designed to work at 32 x 64
				// All quirks w.r.t Dxyn is supposed to assume the display boundary as 32 x 64
				// Only the final display needs to be scaled and set in the imGuiBuffer1D/imGuiBuffer2D buffer
				// One option would be to fill physical display buffer is to scale a 1x1 pixel to 2x2 towards south-east direction

				uint8_t xi = 0;
				uint8_t yi = 0;
				uint8_t zy = 0;
				uint8_t zx = 0;
				uint8_t sprite_height_i = 0;
				uint8_t sprite_width = 8;
				uint16_t pixel_i = 0;
				const uint8_t scale = 2;
				const uint8_t actual_width = getScreenWidth() / scale;
				const uint8_t actual_height = getScreenHeight() / scale;
				const uint8_t x = ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8);
				const uint8_t y = ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4);
				bool schip_16x16 = false;

				xi = pChip8_registers->V[x];
				yi = pChip8_registers->V[y];

				if (pChip8_quirks->_QUIRK_CLIP)
				{
					// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
					xi &= (actual_width - 1);	// This is masking, not mod
					yi &= (actual_height - 1);	// This is masking, not mod
				}

				sprite_height_i = pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F;											// extract the sprite height 

				// Special case: S-Chip (Modern) LORES Dxy0 -> 16x16 sprites 
				if (sprite_height_i == 0)
				{
					sprite_height_i = 16;   // 16 rows
					sprite_width = 16;		// 16 columns
					schip_16x16 = true;
				}

				pChip8_registers->V[0xF] = 0;																							// VF is cleared

				uint32_t pp = 0;
				MAP8 pmask = pChip8_display->planes;
				for (uint8_t bit = 0; pmask; ++bit)
				{
					if ((pmask & (1 << bit)) == 0)
					{
						continue; // skip unused planes
					}

					for (uint32_t yy = 0; yy < sprite_height_i; yy++)															// loop over height of sprite
					{
						// Handle vertical cliping/wrapping
						if (pChip8_quirks->_QUIRK_CLIP)
						{
							zy = (yy + yi);
							// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
							if (zy >= actual_height)
							{
								BREAK;																									// clip
							}
						}
						else
						{
							zy = (yy + yi) & (actual_height - 1);																		// wrap around the height
						}

						// Read the 2 byte pixel data for 16x16 sprite 
						if (schip_16x16 == true)
						{
							// First byte always @ even index
							// Second byte always @ odd index
							auto index0 = ((pChip8_registers->I + (pp + (yy * 2))) & 0xFFFF);
							auto index1 = ((pChip8_registers->I + ((pp + (yy * 2)) + 1)) & 0xFFFF);
							pixel_i = (pChip8_memory->memory[index0] << 8) | pChip8_memory->memory[index1];
						}
						// Read the 1 byte pixel data 
						else
						{
							auto index = ((pChip8_registers->I + (yy + pp)) & 0xFFFF);
							pixel_i = pChip8_memory->memory[index];
						}

						for (uint32_t xx = 0; xx < sprite_width; xx++)																	// loop over width of sprite
						{
							// Handle horizontal cliping/wrapping
							if (pChip8_quirks->_QUIRK_CLIP)
							{
								zx = (xx + xi);
								// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
								if (zx >= actual_width)
								{
									BREAK;																								// clip
								}
							}
							else
							{
								zx = (xx + xi) & (actual_width - 1);																	// wrap around the width
							}

							// Note that zx and zy are still in 32 x 64 domain
							// We need to scale it to 2x2 pixel i.e. (zx,zy) -> (sx,sy)
							// For this we need to do 2 things
							// zx and zy from here onwards should always skip odd numbers i.e 0->2->4... AND
							// for odd number, we just have to put the same color as its previous pixel

							auto sx = zx * scale; // multiply by 2
							auto sy = zy * scale; // multiply by 2


							// Check bit from MSB to LSB
							if ((pixel_i & (1 << (sprite_width - 1 - xx))) != 0)														// if bit is set in pixel_i
							{
								// Chip8 uses XOR logic while displaying pixels
								if (pChip8_display->gfx.gfx2D[bit][sy][sx] == SET)														// was set
								{
									for (int dy = 0; dy < scale; dy++)
									{
										for (int dx = 0; dx < scale; dx++)
										{
											// dx=0,dy=0 is actual pixel
											// dx=0,dy=1 is scaled pixel
											// dx=1,dy=0 is scaled pixel
											// dx=1,dy=1 is scaled pixel
											pChip8_display->gfx.gfx2D[bit][sy + dy][sx + dx] = CLEAR;									// to un-set
										}
									}

									// Chip8 indicates collision per sprite, not per sprite row
									pChip8_registers->V[0xF] = 1;																		// VF set to 1
								}
								else																									// was un-set
								{
									for (int dy = 0; dy < scale; dy++)
									{
										for (int dx = 0; dx < scale; dx++)
										{
											// dx=0,dy=0 is actual pixel
											// dx=0,dy=1 is scaled pixel
											// dx=1,dy=0 is scaled pixel
											// dx=1,dy=1 is scaled pixel
											pChip8_display->gfx.gfx2D[bit][sy + dy][sx + dx] = SET;										// to set
										}
									}
								}
							}
						}
					}
					pp += sprite_height_i * (schip_16x16 ? 2 : 1);
					pmask &= ~(1 << bit); // clear processed plane
				}
			}
			else if (pChip8_display->res == C8RES::HIRES)
			{
				// XO-Chip HIRES is designed to work at 64 x 128
				// All quirks w.r.t Dxyn is supposed to assume the display boundary as 64 x 128

				uint8_t xi = 0;
				uint8_t yi = 0;
				uint8_t zy = 0;
				uint8_t zx = 0;
				uint8_t sprite_height_i = 0;
				uint8_t sprite_width = 8;
				uint16_t pixel_i = 0;
				const uint8_t actual_width = getScreenWidth();
				const uint8_t actual_height = getScreenHeight();
				const uint8_t x = ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8);
				const uint8_t y = ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00F0) >> 4);
				bool schip_16x16 = false;

				xi = pChip8_registers->V[x];
				yi = pChip8_registers->V[y];

				if (pChip8_quirks->_QUIRK_CLIP)
				{
					// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
					xi &= (actual_width - 1);	// This is masking, not mod
					yi &= (actual_height - 1);	// This is masking, not mod
				}

				sprite_height_i = pChip8_instance->chip8_state.cpuInstance.opcode & 0x000F;											// extract the sprite height 

				// Special case: S-Chip (Legacy) HIRES Dxy0 -> 16x16 sprites 
				if (sprite_height_i == 0)
				{
					sprite_height_i = 16;   // 16 rows
					sprite_width = 16;		// 16 columns
					schip_16x16 = true;
				}

				pChip8_registers->V[0xF] = 0;																						// VF is cleared

				uint32_t pp = 0;
				MAP8 pmask = pChip8_display->planes;
				for (uint8_t bit = 0; pmask; ++bit)
				{
					if ((pmask & (1 << bit)) == 0)
					{
						continue; // skip unused planes
					}

					for (uint32_t yy = 0; yy < sprite_height_i; yy++)																// loop over height of sprite
					{
						// Handle vertical cliping/wrapping
						if (pChip8_quirks->_QUIRK_CLIP)
						{
							zy = (yy + yi);
							// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
							if (zy >= actual_height)
							{
								BREAK;																								// clip
							}
						}
						else
						{
							zy = (yy + yi) & (actual_height - 1);																	// wrap around the height
						}

						// Read the 2 byte pixel data for 16x16 sprite 
						if (schip_16x16 == true)
						{
							// First byte always @ even index
							// Second byte always @ odd index
							auto index0 = ((pChip8_registers->I + (pp + (yy * 2))) & 0xFFFF);
							auto index1 = ((pChip8_registers->I + ((pp + (yy * 2)) + 1)) & 0xFFFF);
							pixel_i = (pChip8_memory->memory[index0] << 8) | pChip8_memory->memory[index1];
						}
						// Read the 1 byte pixel data 
						else
						{
							auto index = ((pChip8_registers->I + (yy + pp)) & 0xFFFF);
							pixel_i = pChip8_memory->memory[index];
						}

						for (uint32_t xx = 0; xx < sprite_width; xx++)																// loop over width of sprite
						{
							// Handle horizontal cliping/wrapping
							if (pChip8_quirks->_QUIRK_CLIP)
							{
								zx = (xx + xi);
								// Refer to https://discord.com/channels/465585922579103744/465586212804100106/1071662432352747600
								if (zx >= actual_width)
								{
									BREAK;																							// clip
								}
							}
							else
							{
								zx = (xx + xi) & (actual_width - 1);																// wrap around the width
							}

							// Check bit from MSB to LSB
							if ((pixel_i & (1 << (sprite_width - 1 - xx))) != 0)													// if bit is set in pixel_i
							{
								// Chip8 uses XOR logic while displaying pixels
								if (pChip8_display->gfx.gfx2D[bit][zy][zx] == SET)													// was set
								{
									pChip8_display->gfx.gfx2D[bit][zy][zx] = CLEAR;													// to un-set

									pChip8_registers->V[0xF] = 1;																	// set VF
								}
								else																								// was un-set
								{
									pChip8_display->gfx.gfx2D[bit][zy][zx] = SET;													// to set
								}
							}
						}
					}
					pp += sprite_height_i * (schip_16x16 ? 2 : 1);
					pmask &= ~(1 << bit); // clear processed plane
				}
			}
		}
		else
		{
			FATAL("Unsupported C8 Platform");
		}

		BREAK;
	}
	case 0xE000:
	{
		auto x = (pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8;
		auto key = pChip8_registers->V[x] & 0x0F;
		bool keyActive = (pChip8_io->inputKeys[key] != io_status::FREE);

		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0x9E)
		{
			if (keyActive)
			{
				handleConditionalJump();
			}
		}
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0xA1)
		{
			if (!keyActive)
			{
				handleConditionalJump();
			}
		}
		BREAK;
	}
	case 0xF000:
	{
		if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0FFF) == 0x000)
		{
			pChip8_registers->I =
				(pChip8_memory->memory[pChip8_registers->pc] << 8
					| pChip8_memory->memory[pChip8_registers->pc + 1]);

			pChip8_registers->pc += TWO;
		}
		else if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0x01)
		{
			// This is a bitmask
			pChip8_display->planes = ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8);
		}
		else if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0FFF) == 0x02)
		{
			for (INC8 ii = 0; ii < SIXTEEN; ii++)
			{
				auto index = ((pChip8_registers->I + ii) & 0xFFFF);
				pChip8_audio->audioPattern[ii] = pChip8_memory->memory[index];
			}
		}
		else if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0x07)
		{
			pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8] = pChip8_registers->delay_timer;
		}
		else if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0x0A)
		{
			for (INC8 ii = 0; ii < TO_UINT(CHIP8_KEYS::CHIP8_TOTAL); ii++)
			{
				if (pChip8_io->inputKeys[ii] == io_status::RELEASED)
				{
					pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8] = ii;
					pChip8_instance->chip8_state.io.keyRelEvtC = YES;
					BREAK;
				}
			}

			if (pChip8_instance->chip8_state.io.keyRelEvtA == NO)
			{
				pChip8_registers->pc -= TWO;
				BREAK;
			}
		}
		else if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0x15)
		{
			pChip8_registers->delay_timer = pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8];
		}
		else if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0x18)
		{
			pChip8_instance->chip8_state.audio.sound_timer = pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8];
			if (pChip8_instance->chip8_state.audio.sound_timer == RESET)
			{
				pChip8_audio->phase = 0.0;  // Reset phase when directly set to 0
			}
		}
		else if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0x1E)
		{
			pChip8_registers->I += pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8];
		}
		else if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0x29)
		{
			// Note:
			// 1) >> 8 is to get the x out of Fx29
			// 2) * 5 is because, in "chip8_fontset", each hex number is 5 bytes long, so to point to first index of the hex number, we multiply by 5
			auto x = (pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8;
			auto num = pChip8_registers->V[x] & 0x0F;
			pChip8_registers->I = CHIP8_FONT_BASE + (num * 5);
		}
		else if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0x30)
		{
			// Note:
			// 1) >> 8 is to get the x out of Fx30
			// 2) * 10 is because, in "schip_fontset", each hex number is 10 bytes long, so to point to first index of the hex number, we multiply by 10
			auto x = (pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8;
			auto num = pChip8_registers->V[x] & 0x0F;
			pChip8_registers->I = SCHIP_FONT_BASE + (num * 10);
		}
		else if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0x33)
		{
			pChip8_memory->memory[pChip8_registers->I]
				= pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8] / 100;
			pChip8_memory->memory[(pChip8_registers->I + 1) & 0xFFFF]
				= (pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8] / 10) % 10;
			pChip8_memory->memory[(pChip8_registers->I + 2) & 0xFFFF]
				= (pChip8_registers->V[(pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8] % 100) % 10;
		}
		else if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0x55)
		{
			int ii = 0;
			for (ii = 0; ii <= ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8); ii++)
			{
				auto index = ((pChip8_registers->I + ii) & 0xFFFF);
				pChip8_memory->memory[index] = pChip8_registers->V[ii];
			}
			if (pChip8_quirks->_QUIRK_MEMORY)
			{
				pChip8_registers->I += ii;
			}
		}
		else if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0x65)
		{
			int ii = 0;
			for (ii = 0; ii <= ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8); ii++)
			{
				auto index = ((pChip8_registers->I + ii) & 0xFFFF);
				pChip8_registers->V[ii] = pChip8_memory->memory[index];
			}
			if (pChip8_quirks->_QUIRK_MEMORY)
			{
				pChip8_registers->I += ii;
			}
		}
		else if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0x75)
		{
			FLAG isNotChip8 = (pChip8_quirks->_quirk_schip_modern
				|| pChip8_quirks->_quirk_schip_legacy
				|| pChip8_quirks->_quirk_xo_chip);

			if (isNotChip8)
			{
				uint8_t limit = (pChip8_quirks->_quirk_xo_chip == YES) ? SIXTEEN : EIGHT;
				for (uint8_t ii = 0; ii < limit; ii++)
				{
					pChip8_memory->rpl[ii] = pChip8_registers->V[ii];
				}
			}
		}
		else if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0x85)
		{
			FLAG isNotChip8 = (pChip8_quirks->_quirk_schip_modern
				|| pChip8_quirks->_quirk_schip_legacy
				|| pChip8_quirks->_quirk_xo_chip);

			if (isNotChip8)
			{
				uint8_t limit = (pChip8_quirks->_quirk_xo_chip == YES) ? SIXTEEN : EIGHT;
				for (uint8_t ii = 0; ii < limit; ii++)
				{
					pChip8_registers->V[ii] = pChip8_memory->rpl[ii];
				}
			}
		}
		else if ((pChip8_instance->chip8_state.cpuInstance.opcode & 0x00FF) == 0x3A)
		{
			auto Vx = pChip8_registers->V[((pChip8_instance->chip8_state.cpuInstance.opcode & 0x0F00) >> 8)];
			pChip8_audio->pitch = 4000.0f * static_cast<float>(std::exp2((static_cast<float>(Vx) - 64.0f) / 48.0f));
		}
		BREAK;
	}
	default:
	{
		FATAL("CPU Panic; unknown opcode: %04x", pChip8_instance->chip8_state.cpuInstance.opcode);
		status = false;
		BREAK;
	}

	}

	if (debugConfig._DEBUG_REGISTERS == true)
	{
		LOG_NEW_LINE;
		LOG("--------------------------------------------");
		LOG("emulated cpu cycle: \t\t%" PRId64, getEmulatedCPUCycle());
		LOG("executed opcode: \t%04x", pChip8_instance->chip8_state.cpuInstance.opcode);
		LOG("I register: \t\t%04x", pChip8_registers->I);
		LOG("program counter: \t%04x", pChip8_registers->pc);
		LOG("stack pointer: \t\t%04x", pChip8_registers->sp);
		for (INC8 ii = 0; ii < (sizeof(pChip8_registers->V) / sizeof(pChip8_registers->V[0])); ii++)
		{
			LOG("V%d: \t\t\t%02x", ii, pChip8_registers->V[ii]);
		}
		LOG("delay timer: \t\t%04x", pChip8_registers->delay_timer);
		LOG("sound timer: \t\t%04x", pChip8_instance->chip8_state.audio.sound_timer);
		LOG("--------------------------------------------");
	}
	if (debugConfig._DEBUG_KEYPAD == true)
	{
		LOG("Not Supported");
	}

	++pChip8_instance->chip8_state.cpuInstance.emulatedCPUCycle;

	RETURN status;
}