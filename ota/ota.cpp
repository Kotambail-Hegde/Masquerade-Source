#include <ota.h>

namespace fs = std::filesystem;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;

FLAG isOtaPossible = NO;
const std::string github_host = "api.github.com";
const std::string github_target = "/repos/Kotambail-Hegde/Masquerade-Emulator/releases/latest";
const std::string user_agent = "Masquerade-OTA-Updater";

// ==========================================================
// OpenSSL runtime loader
// ==========================================================
struct OpenSSLWrapper {
    bool loaded = false;
    std::once_flag init_flag;

#ifdef _WIN32
    HMODULE libssl = nullptr;
    HMODULE libcrypto = nullptr;
#else
    void* libssl = nullptr;
    void* libcrypto = nullptr;
#endif

    void init() {
        std::call_once(init_flag, [this]() {
#ifdef _WIN32
            libssl = LoadLibraryA("libssl-4-x64.dll");
            libcrypto = LoadLibraryA("libcrypto-4-x64.dll");
#else
            libssl = dlopen("libssl.so", RTLD_LAZY);
            libcrypto = dlopen("libcrypto.so", RTLD_LAZY);
#endif
            if (!libssl || !libcrypto)
            {
                INFO("OpenSSL runtime not found. OTA disabled.");
                loaded = false;
#ifdef _WIN32
                if (libssl) FreeLibrary(libssl);
                if (libcrypto) FreeLibrary(libcrypto);
#else
                if (libssl) dlclose(libssl);
                if (libcrypto) dlclose(libcrypto);
#endif
            }
            else
            {
                loaded = true;
                INFO("OpenSSL runtime successfully loaded.");
            }
            });
    }

    bool is_available() {
        init();
        return loaded;
    }

    void cleanup() {
        if (!loaded) return;
#ifdef _WIN32
        if (libssl) FreeLibrary(libssl);
        if (libcrypto) FreeLibrary(libcrypto);
#else
        if (libssl) dlclose(libssl);
        if (libcrypto) dlclose(libcrypto);
#endif
        loaded = false;
    }
};

inline OpenSSLWrapper g_openssl;

// ----------------------
// Check if OpenSSL is available
// ----------------------
bool openssl_dlls_present() {
    return g_openssl.is_available();
}

// ----------------------
// HTTPS GET using Boost.Beast + OpenSSL
// ----------------------
std::string https_get(const std::string& host, const std::string& target)
{
    if (!openssl_dlls_present())
    {
        INFO("Skipping HTTPS GET: OpenSSL not available.");
        return {};
    }

    try
    {
        net::io_context ioc;
        ssl::context ctx(ssl::context::tls_client);
        beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

        net::ip::tcp::resolver resolver(ioc);
        auto const results = resolver.resolve(host, "443");

        beast::get_lowest_layer(stream).connect(results);
        stream.handshake(ssl::stream_base::client);

        http::request<http::string_body> req{ http::verb::get, target, 11 };
        req.set(http::field::host, host);
        req.set(http::field::user_agent, user_agent);

        http::write(stream, req);

        beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(stream, buffer, res);

        beast::error_code ec;
        stream.shutdown(ec);
        if (ec && ec != net::error::eof)
            FATAL("HTTPS shutdown failed: %s", ec.message().c_str());

        return res.body();
    }
    catch (const std::exception& e)
    {
        FATAL("HTTPS GET failed: %s", e.what());
        return {};
    }
}

// ==========================================================
// ota_t class
// ==========================================================
ota_t::ota_t() {
}
ota_t::~ota_t() {
    g_openssl.cleanup();
}

// ==========================================================
// Check for updates
// ==========================================================
bool ota_t::checkForUpdates(boost::property_tree::ptree& pt)
{
    try
    {
        if (!openssl_dlls_present())
        {
            isOtaPossible = NO;
            INFO("OTA check skipped because OpenSSL is missing.");
            RETURN SUCCESS;
        }

        std::string json_text = https_get(github_host, github_target);
        if (json_text.empty())
        {
            isOtaPossible = NO;
            RETURN FAILURE;
        }

        std::stringstream ss(json_text);
        boost::property_tree::ptree root;
        boost::property_tree::read_json(ss, root);

        double remote_version = root.get<double>("name", 0.0);
        double current_version = VERSION;

        isOtaPossible = (current_version < remote_version) ? YES : NO;
        INFO("OTA check: current=%.4f, remote=%.4f, possible=%s",
            current_version, remote_version, isOtaPossible == YES ? "YES" : "NO");

        RETURN SUCCESS;
    }
    catch (const std::exception& e)
    {
        FATAL("OTA check failed: %s", e.what());
        RETURN FAILURE;
    }
}

// ==========================================================
// Download GitHub ZIP
// ==========================================================
inline std::string download_github_zip(const std::string& zip_url)
{
    if (!openssl_dlls_present())
    {
        INFO("Skipping GitHub download: OpenSSL not available.");
        return {};
    }

    try
    {
        // Parse URL
        std::regex re(R"(https://([^/]+)(/.*))");
        std::smatch match;
        if (!std::regex_match(zip_url, match, re))
        {
            FATAL("Invalid ZIP URL: %s", zip_url.c_str());
            return {};
        }

        std::string host = match[1];
        std::string target = match[2];

        net::io_context ioc;
        ssl::context ctx(ssl::context::sslv23_client);
        ctx.set_default_verify_paths();
        ssl::stream<beast::tcp_stream> stream(ioc, ctx);

        // Resolve host
        net::ip::tcp::resolver resolver(ioc);
        auto const results = resolver.resolve(host, "https");
        beast::get_lowest_layer(stream).connect(results);

        // SSL handshake
        stream.handshake(ssl::stream_base::client);

        // Send GET request
        http::request<http::string_body> req{ http::verb::get, target, 11 };
        req.set(http::field::host, host);
        req.set(http::field::user_agent, user_agent);
        req.set(http::field::accept, "application/vnd.github.v3+json");

        http::write(stream, req);

        // Receive response
        beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(stream, buffer, res);

        // Handle redirect
        if (res.result_int() == 302 || res.result_int() == 301)
        {
            std::string loc(res[http::field::location]);
            INFO("Redirecting to: %s", loc.c_str());
            stream.shutdown();
            return download_github_zip(loc); // recursive
        }

        if (res.result() != http::status::ok)
        {
            FATAL("GitHub download failed: HTTP %d", res.result_int());
            return {};
        }

        // Extract body
        auto body = res.body();
        std::string data(beast::buffers_to_string(body.data()));

        stream.shutdown();
        return data;
    }
    catch (const std::exception& e)
    {
        FATAL("GitHub download exception: %s", e.what());
        return {};
    }
}

// ==========================================================
// Perform OTA upgrade
// ==========================================================
bool ota_t::upgrade(boost::property_tree::ptree& pt)
{
    try
    {
        if (!isOtaPossible)
        {
            INFO("No OTA update available or OpenSSL missing.");
            RETURN SUCCESS;
        }

        // === Fetch release JSON from GitHub ===
        std::string json_text = https_get(github_host, github_target);
        if (json_text.empty())
        {
            RETURN FAILURE;
        }

        std::stringstream ss(json_text);
        boost::property_tree::ptree root;
        boost::property_tree::read_json(ss, root);

        double remote_version = root.get<double>("name", 0.0);
        double current_version = VERSION;

        if (current_version >= remote_version)
        {
            INFO("Already up-to-date.");
            RETURN SUCCESS;
        }

        std::string zip_url = root.get<std::string>("zipball_url", "");
        if (zip_url.empty())
        {
            FATAL("No downloadable ZIP found for OTA.");
            RETURN FAILURE;
        }

        INFO("OTA update available: %s", zip_url.c_str());

        fs::path current_dir = fs::current_path();
        fs::path temp_zip = current_dir / "masquerade_update.zip";
        fs::path exe_to_backup = current_dir / "masquerade.exe";        // main exe
        fs::path ota_exe = current_dir / "masquerade-OTA.exe";          // running OTA exe

        // === Download ZIP ===
        INFO("Downloading ZIP to: %s", temp_zip.string().c_str());
        try
        {
            std::string zip_data = download_github_zip(zip_url);
            if (zip_data.empty())
            {
                FATAL("Failed to download ZIP from GitHub.");
                RETURN FAILURE;
            }

            std::ofstream ofs(temp_zip, std::ios::binary);
            ofs.write(zip_data.data(), zip_data.size());
            ofs.close();
            INFO("ZIP downloaded successfully (%zu bytes).", zip_data.size());
        }
        catch (const std::exception& e)
        {
            FATAL("Failed to download ZIP: %s", e.what());
            RETURN FAILURE;
        }

        // === Backup selected content ===
        fs::path backup_dir = current_dir / "backup_before_ota";
        INFO("Backing up selected files to: %s", backup_dir.string().c_str());
        try
        {
            if (fs::exists(backup_dir))
                fs::remove_all(backup_dir);
            fs::create_directory(backup_dir);

            // 1. Backup assets folder recursively
            fs::path assets_dir = current_dir / "assets";
            if (fs::exists(assets_dir))
            {
                fs::path dest_assets = backup_dir / "assets";
                fs::create_directory(dest_assets);

                for (auto& entry : fs::recursive_directory_iterator(assets_dir))
                {
                    fs::path relative = fs::relative(entry.path(), assets_dir);
                    fs::path target = dest_assets / relative;

                    if (entry.is_directory())
                        fs::create_directories(target);
                    else
                        fs::copy_file(entry.path(), target, fs::copy_options::overwrite_existing);
                }
            }

            // 2. Backup LICENSE.md
            fs::path license_file = current_dir / "LICENSE.md";
            if (fs::exists(license_file))
            {
                fs::copy_file(license_file, backup_dir / "LICENSE.md", fs::copy_options::overwrite_existing);
            }

            // 3. Backup main exe only (skip OTA exe)
            if (fs::exists(exe_to_backup))
            {
                fs::copy_file(exe_to_backup, backup_dir / exe_to_backup.filename(), fs::copy_options::overwrite_existing);
            }

            INFO("Backup completed successfully.");
        }
        catch (const std::exception& e)
        {
            FATAL("Backup failed: %s", e.what());
            RETURN FAILURE;
        }

        // === Extract ZIP to temporary folder ===
        fs::path temp_extract_dir = current_dir / "ota_temp";
        if (fs::exists(temp_extract_dir))
            fs::remove_all(temp_extract_dir);
        fs::create_directory(temp_extract_dir);

        INFO("Extracting ZIP to temporary folder...");
        if (!extract_zip(temp_zip, temp_extract_dir))
        {
            FATAL("Extraction failed.");
            RETURN FAILURE;
        }
        INFO("Extraction completed.");

        // === Detect top-level folder inside extracted ZIP ===
        fs::path source_dir;
        for (auto& entry : fs::directory_iterator(temp_extract_dir))
        {
            if (entry.is_directory())
            {
                source_dir = entry.path();
                break;
            }
        }
        if (source_dir.empty())
            source_dir = temp_extract_dir; // fallback if no subfolder

        // === Replace only selected files (assets/, LICENSE.md, masquerade.exe) ===
        INFO("Replacing selected files from update...");

        try
        {
            // 1. Copy assets folder recursively
            fs::path assets_src = source_dir / "assets";
            fs::path assets_dst = current_dir / "assets";

            if (fs::exists(assets_src))
            {
                INFO("Copying assets folder...");
                for (const auto& entry : fs::recursive_directory_iterator(assets_src))
                {
                    const fs::path relative = fs::relative(entry.path(), assets_src);
                    const fs::path target = assets_dst / relative;

                    if (entry.is_directory())
                    {
                        fs::create_directories(target);
                    }
                    else
                    {
                        fs::create_directories(target.parent_path());
                        fs::copy_file(entry.path(), target, fs::copy_options::overwrite_existing);
                    }
                }
            }
            else
            {
                WARN("No assets folder found in update package.");
            }

            // 2. Copy LICENSE.md
            fs::path license_src = source_dir / "LICENSE.md";
            fs::path license_dst = current_dir / "LICENSE.md";
            if (fs::exists(license_src))
            {
                fs::copy_file(license_src, license_dst, fs::copy_options::overwrite_existing);
                INFO("Copied LICENSE.md");
            }
            else
            {
                WARN("LICENSE.md not found in update package.");
            }

            // 3. Copy masquerade.exe
            fs::path exe_src = source_dir / "masquerade.exe";
            fs::path exe_dst = current_dir / "masquerade.exe";
            if (fs::exists(exe_src))
            {
                fs::copy_file(exe_src, exe_dst, fs::copy_options::overwrite_existing);
                INFO("Copied masquerade.exe");
            }
            else
            {
                WARN("masquerade.exe not found in update package.");
            }
        }
        catch (const std::exception& e)
        {
            FATAL("Failed during file replacement: %s", e.what());
            RETURN FAILURE;
        }

        // === Cleanup ===
        try
        {
            fs::remove_all(temp_extract_dir);
            fs::remove(temp_zip);
            INFO("Temporary files cleaned up.");
        }
        catch (const std::exception& e)
        {
            WARN("Cleanup warning: %s", e.what());
        }

        INFO("OTA upgrade completed successfully.");
        isOtaPossible = YES;
        RETURN SUCCESS;
    }
    catch (const std::exception& e)
    {
        FATAL("OTA upgrade failed: %s", e.what());
        RETURN FAILURE;
    }
}
