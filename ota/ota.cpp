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

// ----------------------------------------------------------
// Platform-specific relative paths
// ----------------------------------------------------------
#if defined(_WIN32)

const fs::path PLATFORM_DIR = "windows";
const fs::path APP_RELATIVE_PATH = PLATFORM_DIR / "masquerade.exe";
const fs::path OTA_RELATIVE_PATH = PLATFORM_DIR / "masquerade-OTA.exe";

#elif defined(__linux__)

const fs::path PLATFORM_DIR = "linux";
const fs::path APP_RELATIVE_PATH = PLATFORM_DIR / "masquerade";
const fs::path OTA_RELATIVE_PATH = PLATFORM_DIR / "masquerade-OTA";

#elif defined(__APPLE__)

const fs::path PLATFORM_DIR = "macos";
const fs::path APP_RELATIVE_PATH = PLATFORM_DIR / "Masquerade.app"; // if bundle
const fs::path OTA_RELATIVE_PATH = PLATFORM_DIR / "masquerade-OTA";

#else
#error "Unsupported platform for OTA"
#endif

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
        RETURN loaded;
    }

    void cleanup() {
        if (!loaded) RETURN;
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

// ==========================================================
// Query current main app version dynamically (Cross-platform)
// ==========================================================
double getCurrentMainAppVersion(const fs::path& app_path)
{
    try
    {
        INFO("Attempting to query version from: %s", app_path.string().c_str());

        if (!fs::exists(app_path))
        {
            WARN("Main app not found at: %s", app_path.string().c_str());
            RETURN 0.0;
        }

        namespace bp = boost::process;

        // Build command with quotes for paths with spaces
        std::string cmd = "\"" + app_path.string() + "\" --version";
        INFO("Executing command: %s", cmd.c_str());

        // Create pipe to capture stdout
        bp::ipstream pipe_stream;
        bp::child c;

        try
        {
            // Execute app with --version flag
            c = bp::child(cmd, bp::std_out > pipe_stream);
            INFO("Child process created successfully");
        }
        catch (const std::exception& e)
        {
            FATAL("Failed to create child process: %s", e.what());
            RETURN 0.0;
        }

        // Read version from stdout
        std::string line;
        if (std::getline(pipe_stream, line))
        {
            INFO("Read line from process: [%s]", line.c_str());

            // Trim whitespace
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            INFO("Trimmed line: [%s]", line.c_str());

            try
            {
                double version = std::stod(line);
                INFO("Current main app version: %.4f", version);
                c.wait();
                RETURN version;
            }
            catch (const std::exception& e)
            {
                WARN("Failed to parse version string: %s (error: %s)", line.c_str(), e.what());
                c.wait();
                RETURN 0.0;
            }
        }
        else
        {
            WARN("No output from version query - getline failed");
            WARN("Checking child process exit status...");
            try
            {
                c.wait();
                int exit_code = c.exit_code();
                WARN("Child process exited with code: %d", exit_code);
            }
            catch (const std::exception& e)
            {
                WARN("Error waiting for child process: %s", e.what());
            }
            RETURN 0.0;
        }
    }
    catch (const std::exception& e)
    {
        FATAL("Failed to query main app version: %s", e.what());
        RETURN 0.0;
    }
}

// ----------------------
// Check if OpenSSL is available
// ----------------------
bool openssl_dlls_present() {
    RETURN g_openssl.is_available();
}

// ----------------------
// HTTPS GET using Boost.Beast + OpenSSL
// ----------------------
std::string https_get(const std::string& host, const std::string& target)
{
    if (!openssl_dlls_present())
    {
        INFO("Skipping HTTPS GET: OpenSSL not available.");
        RETURN{};
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

        RETURN res.body();
    }
    catch (const std::exception& e)
    {
        FATAL("HTTPS GET failed: %s", e.what());
        RETURN{};
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

        fs::path current_dir = fs::current_path();

        double remote_version = root.get<double>("name", 0.0);

        // Query main app in current directory (same dir as OTA)
#ifdef _WIN32
        fs::path main_app = current_dir / "masquerade.exe";
#else
        fs::path main_app = current_dir / "masquerade";
#endif
        double current_version = getCurrentMainAppVersion(main_app);

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
// Download GitHub ZIP (with configurable body limit)
// ==========================================================
inline std::string download_github_zip(const std::string& zip_url)
{
    if (!openssl_dlls_present())
    {
        INFO("Skipping GitHub download: OpenSSL not available.");
        RETURN{};
    }

    try
    {
        // Parse URL
        std::regex re(R"(https://([^/]+)(/.*))");
        std::smatch match;
        if (!std::regex_match(zip_url, match, re))
        {
            FATAL("Invalid ZIP URL: %s", zip_url.c_str());
            RETURN{};
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

        // Receive response with NO BODY SIZE LIMIT
        beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;

        // Create parser with unlimited body size
        http::response_parser<http::dynamic_body> parser;
        parser.body_limit((std::numeric_limits<std::uint64_t>::max)()); // Remove size limit

        http::read(stream, buffer, parser);
        res = parser.get();

        // Handle redirect
        if (res.result_int() == 302 || res.result_int() == 301)
        {
            std::string loc(res[http::field::location]);
            INFO("Redirecting to: %s", loc.c_str());
            stream.shutdown();
            RETURN download_github_zip(loc); // recursive
        }

        if (res.result() != http::status::ok)
        {
            FATAL("GitHub download failed: HTTP %d", res.result_int());
            RETURN{};
        }

        // Extract body
        auto body = res.body();
        std::string data(beast::buffers_to_string(body.data()));
        INFO("Downloaded %zu bytes from GitHub", data.size());

        stream.shutdown();
        RETURN data;
    }
    catch (const std::exception& e)
    {
        FATAL("GitHub download exception: %s", e.what());
        RETURN{};
    }
}

// ==========================================================
// Perform OTA upgrade (with new folder structure support)
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

        fs::path current_dir = fs::current_path();

        double remote_version = root.get<double>("name", 0.0);

        // Query main app in current directory (same dir as OTA)
#ifdef _WIN32
        fs::path main_app = current_dir / "masquerade.exe";
#else
        fs::path main_app = current_dir / "masquerade";
#endif
        double current_version = getCurrentMainAppVersion(main_app);

        if (current_version >= remote_version)
        {
            INFO("Already up-to-date (%.4f >= %.4f).", current_version, remote_version);
            RETURN SUCCESS;
        }

        std::string zip_url = root.get<std::string>("zipball_url", "");
        if (zip_url.empty())
        {
            FATAL("No downloadable ZIP found for OTA.");
            RETURN FAILURE;
        }

        INFO("OTA update available: %s", zip_url.c_str());

        fs::path temp_zip = current_dir / "masquerade_update.zip";

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

            // 2. Backup LICENSE.md and README.md
            std::vector<std::string> docs = { "LICENSE.md", "README.md" };
            for (const auto& doc : docs)
            {
                fs::path doc_file = current_dir / doc;
                if (fs::exists(doc_file))
                {
                    fs::copy_file(doc_file, backup_dir / doc, fs::copy_options::overwrite_existing);
                }
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

        // === Determine platform-specific paths ===
        std::string platform_folder;
#if defined(_WIN32)
        platform_folder = "windows";
#elif defined(__linux__)
        platform_folder = "linux";
#elif defined(__APPLE__)
        platform_folder = "macos";
#else
#error "Unsupported platform for OTA"
#endif

        INFO("OTA updating for platform: %s", platform_folder.c_str());

        fs::path platform_src = source_dir / platform_folder;
        if (!fs::exists(platform_src))
        {
            FATAL("Platform folder '%s' not found in update package.", platform_folder.c_str());
            RETURN FAILURE;
        }

        // === Replace platform-specific files ===
        INFO("Replacing platform-specific files from update...");

        try
        {
            // 1. Copy all executables from platform folder (EXCEPT OTA app and crypto libs)
            for (const auto& entry : fs::directory_iterator(platform_src))
            {
                if (entry.is_regular_file())
                {
                    std::string filename = entry.path().filename().string();

                    // Skip OTA executable and crypto libraries
                    if (filename.find("OTA") != std::string::npos ||
                        filename.find("libssl") != std::string::npos ||
                        filename.find("libcrypto") != std::string::npos)
                    {
                        INFO("Skipping: %s (self-managed)", filename.c_str());
                        continue;
                    }

                    // Copy exe, dll (SDL3), and so files
                    if (filename.find(".exe") != std::string::npos ||
                        filename.find(".dll") != std::string::npos ||
                        filename.find(".so") != std::string::npos)
                    {
                        fs::path dest = current_dir / filename;
                        fs::copy_file(entry.path(), dest, fs::copy_options::overwrite_existing);
                        INFO("Copied: %s", filename.c_str());
                    }
                }
            }

            // 2. Copy assets folder recursively (located inside platform folder)
            fs::path assets_src = platform_src / "assets";
            fs::path assets_dst = current_dir / "assets";

            if (fs::exists(assets_src))
            {
                INFO("Copying assets folder from: %s", assets_src.string().c_str());
                try
                {
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
                    INFO("Assets folder updated successfully.");
                }
                catch (const std::exception& e)
                {
                    FATAL("Failed to copy assets folder: %s", e.what());
                    throw;
                }
            }
            else
            {
                WARN("No assets folder found at: %s", assets_src.string().c_str());
            }

            // 3. Copy documentation files
            std::vector<std::string> docs = { "LICENSE.md", "README.md" };
            for (const auto& doc : docs)
            {
                fs::path doc_src = source_dir / doc;
                fs::path doc_dst = current_dir / doc;
                if (fs::exists(doc_src))
                {
                    fs::copy_file(doc_src, doc_dst, fs::copy_options::overwrite_existing);
                    INFO("Copied: %s", doc.c_str());
                }
                else
                {
                    INFO("Note: %s not found in update package (not critical).", doc.c_str());
                }
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
