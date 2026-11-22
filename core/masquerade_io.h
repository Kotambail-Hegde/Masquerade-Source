#pragma once

// masquerade_io.h
//
// SDL based file I/O for the emulation cores.

#ifndef __RPI_PICO__

#include "macros.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <ios>
#include <string>
#include <vector>

namespace masquerade_io
{
	// ---- paths -------------------------------------------------------------------------------

	MASQ_INLINE std::string normalize(std::string path)
	{
		std::replace(path.begin(), path.end(), '\\', '/');
		RETURN path;
	}

	MASQ_INLINE std::string join(const std::string& a, const std::string& b)
	{
		if (a.empty()) RETURN b;
		std::string result = a;
		if (result.back() != '/' && result.back() != '\\') result += '/';
		RETURN result + b;
	}

	MASQ_INLINE std::string androidAssetPath(const std::string& configValue)
	{
		if (configValue.compare(ZERO, 10, "content://") == ZERO) RETURN configValue;
		const std::string p = normalize(configValue);
		const size_t pos = p.rfind("assets/");
		if (pos == std::string::npos) RETURN p;
		if (pos != ZERO && p[pos - 1] != '/') RETURN p;
		RETURN p.substr(pos + 7);
	}

	MASQ_INLINE std::string androidAssetRelative(const std::string& path)
	{
		RETURN androidAssetPath(path);
	}

	MASQ_INLINE std::string androidSaveDir()
	{
		char* pref = SDL_GetPrefPath("Kotambail-Hegde", "Masquerade");
		std::string dir = pref ? (std::string(pref) + "saves") : std::string("saves");
		if (pref) SDL_free(pref);
		RETURN dir;
	}

	MASQ_INLINE std::string androidAppDir(const std::string& subFolder)
	{
		char* pref = SDL_GetPrefPath("Kotambail-Hegde", "Masquerade");
		std::string dir = pref ? (std::string(pref) + subFolder) : subFolder;
		if (pref) SDL_free(pref);
		RETURN dir;
	}

	MASQ_INLINE std::string androidBiosFile(const std::string& fileName)
	{
		RETURN join(androidAppDir("bios"), fileName);
	}

	MASQ_INLINE FLAG ensureDir(const std::string& path)
	{
		if (path.empty()) RETURN NO;
		RETURN SDL_CreateDirectory(normalize(path).c_str());
	}

	MASQ_INLINE FLAG exists(const std::string& path)
	{
		SDL_IOStream* io = SDL_IOFromFile(path.c_str(), "rb");
#ifdef __ANDROID__
		if (!io) io = SDL_IOFromFile(androidAssetRelative(path).c_str(), "rb");
#endif
		if (!io) RETURN NO;
		SDL_CloseIO(io);
		RETURN YES;
	}

	MASQ_INLINE FLAG loadFile(const std::string& path, std::vector<uint8_t>& out)
	{
		size_t size = ZERO;
		void* data = SDL_LoadFile(path.c_str(), &size);
#ifdef __ANDROID__
		if (!data) data = SDL_LoadFile(androidAssetRelative(path).c_str(), &size);
#endif
		if (!data) RETURN NO;
		out.assign(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + size);
		SDL_free(data);
		RETURN YES;
	}

	MASQ_INLINE FLAG readTextFile(const std::string& path, std::string& out)
	{
		std::vector<uint8_t> bytes;
		if (!loadFile(path, bytes)) RETURN NO;
		out.assign(reinterpret_cast<const char*>(bytes.data()), bytes.size());
		RETURN YES;
	}

	MASQ_INLINE FLAG saveFile(const std::string& path, const void* data, size_t size)
	{
		if (size == ZERO)
		{
			SDL_IOStream* io = SDL_IOFromFile(path.c_str(), "wb");
			if (!io) RETURN NO;
			RETURN SDL_CloseIO(io);
		}
		RETURN SDL_SaveFile(path.c_str(), data, size);
	}

	// std::ifstream replacement

	class ifstream
	{
	public:
		ifstream() = default;
		explicit ifstream(const std::string& path, std::ios_base::openmode = std::ios_base::in) { open(path); }
		explicit ifstream(const char* path, std::ios_base::openmode = std::ios_base::in) { open(path ? path : ""); }
		explicit ifstream(const std::filesystem::path& path, std::ios_base::openmode = std::ios_base::in) { open(path.string()); }

		void open(const std::string& path, std::ios_base::openmode = std::ios_base::in)
		{
			close();
			data_.clear();
			pos_ = ZERO;
			gcount_ = ZERO;
			state_ = ZERO;
			open_ = loadFile(path, data_);
			if (!open_) state_ = FAIL;
		}

		void close() { open_ = NO; }

		bool is_open() const { RETURN open_; }
		bool fail() const { RETURN (state_ & FAIL) != ZERO; }
		bool eof() const { RETURN (state_ & EOFBIT) != ZERO; }
		bool good() const { RETURN state_ == ZERO; }
		bool operator!() const { RETURN fail(); }
		explicit operator bool() const { RETURN !fail(); }
		void clear() { state_ = ZERO; }

		ifstream& read(char* dst, std::streamsize count)
		{
			gcount_ = ZERO;
			if (state_ != ZERO || count < ZERO)
			{
				state_ |= FAIL;
				RETURN *this;
			}

			const std::streamoff size = static_cast<std::streamoff>(data_.size());
			const std::streamoff available = (pos_ < size) ? (size - pos_) : ZERO;
			const std::streamoff n = (static_cast<std::streamoff>(count) <= available) ? static_cast<std::streamoff>(count) : available;

			if (n > ZERO && dst) std::memcpy(dst, data_.data() + pos_, static_cast<size_t>(n));
			pos_ += n;
			gcount_ = n;

			if (n < static_cast<std::streamoff>(count)) state_ |= (EOFBIT | FAIL);
			RETURN *this;
		}

		std::streamsize gcount() const { RETURN gcount_; }

		ifstream& seekg(std::streamoff off, std::ios_base::seekdir dir = std::ios_base::beg)
		{
			state_ &= ~EOFBIT;
			if (fail()) RETURN *this;

			std::streamoff base = ZERO;
			if (dir == std::ios_base::cur) base = pos_;
			else if (dir == std::ios_base::end) base = static_cast<std::streamoff>(data_.size());

			const std::streamoff target = base + off;
			if (target < ZERO) state_ |= FAIL;
			else pos_ = target;
			RETURN *this;
		}

		std::streamoff tellg() const { RETURN fail() ? static_cast<std::streamoff>(-1) : pos_; }

	private:
		static constexpr int EOFBIT = 1;
		static constexpr int FAIL = 2;

		std::vector<uint8_t> data_;
		std::streamoff pos_ = 0;
		std::streamsize gcount_ = 0;
		int state_ = 0;
		bool open_ = false;
	};

	// std::ofstream replacement

	class ofstream
	{
	public:
		ofstream() = default;
		explicit ofstream(const std::string& path, std::ios_base::openmode = std::ios_base::out) { open(path); }
		explicit ofstream(const char* path, std::ios_base::openmode = std::ios_base::out) { open(path ? path : ""); }
		explicit ofstream(const std::filesystem::path& path, std::ios_base::openmode = std::ios_base::out) { open(path.string()); }
		~ofstream() { close(); }

		ofstream(const ofstream&) = delete;
		ofstream& operator=(const ofstream&) = delete;

		void open(const std::string& path, std::ios_base::openmode = std::ios_base::out)
		{
			close();
			buffer_.clear();
			path_ = path;
			failed_ = !saveFile(path_, nullptr, ZERO); // truncate / test that the location is writable
			open_ = !failed_;
		}

		void close()
		{
			if (!open_) RETURN;
			open_ = NO;
			if (!buffer_.empty() && !saveFile(path_, buffer_.data(), buffer_.size())) failed_ = YES;
			buffer_.clear();
		}

		bool is_open() const { RETURN open_; }
		bool fail() const { RETURN failed_; }
		bool good() const { RETURN !failed_; }
		bool operator!() const { RETURN failed_; }
		explicit operator bool() const { RETURN !failed_; }
		void flush() {}

		ofstream& write(const char* src, std::streamsize count)
		{
			if (open_ && !failed_ && src && count > ZERO)
				buffer_.insert(buffer_.end(), src, src + count);
			RETURN *this;
		}

		ofstream& operator<<(const std::string& text) { RETURN write(text.data(), static_cast<std::streamsize>(text.size())); }

	private:
		std::string path_;
		std::vector<char> buffer_;
		bool open_ = false;
		bool failed_ = false;
	};
}

#endif // !__RPI_PICO__
