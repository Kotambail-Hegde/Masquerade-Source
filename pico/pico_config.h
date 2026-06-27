#pragma once
#include <cstdint>
#include <cstddef>

// ======================================================
// AUTO-GENERATED FROM CONFIG.INI (DO NOT EDIT)
// ======================================================

enum PicoType : uint8_t
{
    BOOL = 0,
    INT,
    FLOAT,
    STRING
};

struct PicoConfigEntry
{
    uint32_t hash;
    PicoType type;

    union {
        bool b;
        int i;
        float f;
        const char* s;
    };
};

inline constexpr uint32_t fnv1a(const char* str)
{
    uint32_t hash = 2166136261u;
    while (*str)
    {
        hash ^= (uint8_t)(*str++);
        hash *= 16777619u;
    }
    return hash;
}

inline constexpr PicoConfigEntry PICO_CONFIG_TABLE[] =
{

    // ===== GAMEENGINE =====
    { 2571424002u, PicoType::STRING, { .s = "dd89bb14" } },
    { 609445532u, PicoType::STRING, { .s = "IMGUI_OPENGL_SDL3" } },
    { 465970757u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== UPDATER =====
    { 1541925208u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== INTERNAL =====
    { 883674042u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\internal" } },
    { 2180995797u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\ui\\config" } },
    { 3376399820u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\ui\\sprites" } },
    { 1977340662u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== GAMEOFLIFE =====
    { 2471044178u, PicoType::INT, { .i = 15 } },
    { 2997321090u, PicoType::BOOL, { .b = false } },
    { 1971919351u, PicoType::INT, { .i = 256 } },
    { 3792335424u, PicoType::INT, { .i = 1024 } },
    { 4048119653u, PicoType::INT, { .i = 1 } },
    { 4046269898u, PicoType::INT, { .i = 1 } },
    { 536674390u, PicoType::BOOL, { .b = true } },
    { 3136309672u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\saves" } },
    { 780264358u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== CHIP8 =====
    { 505269527u, PicoType::INT, { .i = 1 } },
    { 787202488u, PicoType::BOOL, { .b = true } },
    { 4013944373u, PicoType::BOOL, { .b = false } },
    { 1412203681u, PicoType::BOOL, { .b = false } },
    { 4221716118u, PicoType::BOOL, { .b = false } },
    { 2576176132u, PicoType::BOOL, { .b = false } },
    { 684780229u, PicoType::BOOL, { .b = false } },
    { 4213097738u, PicoType::BOOL, { .b = false } },
    { 3143962743u, PicoType::BOOL, { .b = false } },
    { 1905883663u, PicoType::BOOL, { .b = false } },
    { 2543983291u, PicoType::BOOL, { .b = false } },
    { 3859509097u, PicoType::BOOL, { .b = false } },
    { 2697654936u, PicoType::BOOL, { .b = true } },
    { 2788471583u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\saves" } },
    { 389629324u, PicoType::FLOAT, { .f = 0.1f } },
    { 4080896337u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== SPACEINVADERS =====
    { 2006895073u, PicoType::INT, { .i = 2 } },
    { 119505918u, PicoType::INT, { .i = -1 } },
    { 610316622u, PicoType::BOOL, { .b = true } },
    { 643871860u, PicoType::BOOL, { .b = true } },
    { 694204717u, PicoType::BOOL, { .b = true } },
    { 677427098u, PicoType::BOOL, { .b = true } },
    { 1594705464u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\spaceInvaders\\audio\\0.wav" } },
    { 4179957964u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\spaceInvaders\\audio\\1.wav" } },
    { 1487336533u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\spaceInvaders\\audio\\2.wav" } },
    { 4198784001u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\spaceInvaders\\audio\\3.wav" } },
    { 1177720772u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\spaceInvaders\\audio\\4.wav" } },
    { 1228053629u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\spaceInvaders\\audio\\5.wav" } },
    { 1211276010u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\spaceInvaders\\audio\\6.wav" } },
    { 1127387915u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\spaceInvaders\\audio\\7.wav" } },
    { 274007434u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\spaceInvaders\\audio\\8.wav" } },
    { 1992142753u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\saves" } },
    { 1845111086u, PicoType::FLOAT, { .f = 0.5f } },
    { 3254798235u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== PACMAN =====
    { 3011786311u, PicoType::INT, { .i = 3 } },
    { 692716936u, PicoType::INT, { .i = -1 } },
    { 908256047u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\saves" } },
    { 3577397308u, PicoType::FLOAT, { .f = 0.1f } },
    { 4084603841u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== NES =====
    { 2080626799u, PicoType::INT, { .i = 4 } },
    { 1232550199u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\saves" } },
    { 1528123908u, PicoType::FLOAT, { .f = 0.1f } },
    { 1859931081u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== GB_GBC =====
    { 2918234363u, PicoType::INT, { .i = 5 } },
    { 158945496u, PicoType::BOOL, { .b = true } },
    { 2154444210u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\gb\\bios\\dmg_rom.bin" } },
    { 169076572u, PicoType::BOOL, { .b = true } },
    { 580009134u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\gbc\\bios\\cgb_boot.bin" } },
    { 1417668979u, PicoType::STRING, { .s = "GearBoy" } },
    { 4174147026u, PicoType::BOOL, { .b = false } },
    { 2579888350u, PicoType::BOOL, { .b = false } },
    { 1201476700u, PicoType::BOOL, { .b = false } },
    { 2391969135u, PicoType::BOOL, { .b = true } },
    { 4097870297u, PicoType::BOOL, { .b = true } },
    { 1171977589u, PicoType::FLOAT, { .f = 0.6f } },
    { 3564649664u, PicoType::FLOAT, { .f = 0.4f } },
    { 3895302883u, PicoType::FLOAT, { .f = 0.3f } },
    { 2006548163u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\saves" } },
    { 2577955896u, PicoType::FLOAT, { .f = 0.025f } },
    { 2152154621u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== SNES =====
    { 1946299618u, PicoType::INT, { .i = 6 } },
    { 1630296886u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== N64 =====
    { 2524517u, PicoType::INT, { .i = 7 } },
    { 835706919u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== GBA =====
    { 245069313u, PicoType::INT, { .i = 8 } },
    { 2504037759u, PicoType::BOOL, { .b = true } },
    { 896622522u, PicoType::BOOL, { .b = true } },
    { 3610687484u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\gba\\bios\\gba_bios.bin" } },
    { 1495018113u, PicoType::STRING, { .s = "C:\\Data\\Workspace\\Emulators\\Masquerade-(Multi-Console)\\Prototype-52\\masquerade\\assets\\saves" } },
    { 1094512462u, PicoType::FLOAT, { .f = 0.1f } },
    { 1371724091u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== GAMECUBE =====
    { 223121820u, PicoType::INT, { .i = 9 } },
    { 2779179376u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== DS =====
    { 3043417250u, PicoType::INT, { .i = 10 } },
    { 2460864502u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== WII =====
    { 1625055990u, PicoType::INT, { .i = 11 } },
    { 777880514u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== DS3 =====
    { 1032058451u, PicoType::INT, { .i = 12 } },
    { 766453301u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== WIIU =====
    { 3207032461u, PicoType::INT, { .i = 13 } },
    { 3080756079u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== NINTENDO_SWITCH =====
    { 3473650355u, PicoType::INT, { .i = 14 } },
    { 3864860181u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== MASQUERADE =====
    { 833022472u, PicoType::FLOAT, { .f = 0.7003f } },
    { 3098342553u, PicoType::INT, { .i = 0 } },
    { 1903284659u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== MODS =====
    { 2412877577u, PicoType::BOOL, { .b = true } },
    { 1718377735u, PicoType::BOOL, { .b = false } },
    { 599835406u, PicoType::STRING, { .s = "DARK" } },
    { 2502762915u, PicoType::STRING, { .s = "LCD_FILTER" } },
    { 4180372123u, PicoType::BOOL, { .b = true } },
    { 1669080081u, PicoType::INT, { .i = 1 } },
    { 553020929u, PicoType::BOOL, { .b = false } },
    { 4133255382u, PicoType::BOOL, { .b = false } },
    { 371499784u, PicoType::INT, { .i = 1 } },
    { 1492995829u, PicoType::BOOL, { .b = false } },
    { 2564284543u, PicoType::INT, { .i = 100 } },
    { 3860017724u, PicoType::BOOL, { .b = true } },
    { 2362117756u, PicoType::BOOL, { .b = false } },
    { 2725677374u, PicoType::BOOL, { .b = false } },
    { 342802545u, PicoType::INT, { .i = 5000 } },
    { 4251470012u, PicoType::BOOL, { .b = false } },
    { 4021271045u, PicoType::INT, { .i = 345 } },
    { 4004493426u, PicoType::INT, { .i = 200 } },
    { 586842342u, PicoType::STRING, { .s = "XXXXXXXXXX" } },
    // ===== DEBUG =====
    { 3382444269u, PicoType::BOOL, { .b = false } },
    { 1093175641u, PicoType::BOOL, { .b = false } },
    { 3294273298u, PicoType::INT, { .i = 71 } },
    { 3898844726u, PicoType::BOOL, { .b = false } },
    { 2439370175u, PicoType::INT, { .i = 0 } },
    { 3145036322u, PicoType::BOOL, { .b = false } },
    { 2990981220u, PicoType::BOOL, { .b = false } },
    { 693973266u, PicoType::BOOL, { .b = false } },
    { 740861479u, PicoType::BOOL, { .b = false } },
    { 4031540574u, PicoType::STRING, { .s = "XXXXXXXXXX" } },

};

// ======================================================
// TABLE SIZE
// ======================================================

inline constexpr size_t PICO_CONFIG_TABLE_SIZE =
    sizeof(PICO_CONFIG_TABLE) / sizeof(PICO_CONFIG_TABLE[0]);

// ======================================================
// END AUTO-GENERATED CONFIG
// ======================================================
