#pragma once

#include <cstdint>
#include <filesystem>
#include <limits>
#include <map>
#include <string_view>
#include <tuple>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
// Keep the Win32 alias from rewriting ForgeSim::Assets::LoadImage.
#undef LoadImage
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

namespace ForgeSim::Assets::Detail
{
	// Hold the lease for the image stage. A competing loader bypasses the cache
	// instead of waiting, and the OS releases the lease if the owner exits.
	class DecodedImageCacheLease
	{
	public:
		explicit DecodedImageCacheLease(const std::filesystem::path& path)
		{
#ifdef _WIN32
			m_Handle = ::CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			OVERLAPPED region{};
			m_Acquired = m_Handle != INVALID_HANDLE_VALUE &&
				::LockFileEx(m_Handle, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY,
					0, 1, 0, &region);
#else
			m_Handle = ::open(path.c_str(), O_CREAT | O_RDWR | O_CLOEXEC, 0600);
			m_Acquired = m_Handle >= 0 && ::flock(m_Handle, LOCK_EX | LOCK_NB) == 0;
#endif
		}

		~DecodedImageCacheLease()
		{
#ifdef _WIN32
			if (m_Handle != INVALID_HANDLE_VALUE)
				::CloseHandle(m_Handle);
#else
			if (m_Handle >= 0)
				::close(m_Handle);
#endif
		}

		DecodedImageCacheLease(const DecodedImageCacheLease&) = delete;
		DecodedImageCacheLease& operator=(const DecodedImageCacheLease&) = delete;

		bool Acquired() const noexcept { return m_Acquired; }

	private:
#ifdef _WIN32
		HANDLE m_Handle = INVALID_HANDLE_VALUE;
#else
		int m_Handle = -1;
#endif
		bool m_Acquired = false;
	};

	// All operations require the directory lease and caller-side thread serialization.
	class DecodedImageCacheBudget
	{
	public:
		explicit DecodedImageCacheBudget(std::uint64_t limit) : m_Limit(limit) {}

		bool Scan(const std::filesystem::path& directory)
		{
			std::error_code error;
			for (std::filesystem::directory_iterator it(directory, error), end;
				!error && it != end; it.increment(error))
			{
				const auto& file = *it;
				const auto name = file.path().filename().string();
				const bool pending = name.ends_with(".pending");
				const auto key = std::string_view{name}.substr(0,
					pending ? name.size() - sizeof(".pending") + 1 : name.size());
				if (!IsEntryName(key))
					continue;
				const auto status = file.symlink_status(error);
				if (error)
					break;
				if (!std::filesystem::is_regular_file(status))
					continue;

				if (pending)
				{
					std::error_code ignored;
					std::filesystem::remove(file.path(), ignored);
					continue;
				}

				const auto size = file.file_size(error);
				if (error)
					break;
				const auto used = file.last_write_time(error);
				if (error)
					break;
				if (size > std::numeric_limits<std::uint64_t>::max() - m_Size)
					return false;
				m_Entries.emplace(file.path(), Entry{size, used});
				m_Size += size;
			}
			return !error;
		}

		bool Contains(const std::filesystem::path& path) const { return m_Entries.contains(path); }

		void Touch(const std::filesystem::path& path)
		{
			if (const auto entry = m_Entries.find(path); entry != m_Entries.end())
			{
				const auto now = std::filesystem::file_time_type::clock::now();
				std::error_code error;
				std::filesystem::last_write_time(path, now, error);
				if (!error)
					entry->second.used = now;
			}
		}

		bool MakeRoom(const std::filesystem::path& replacing = {}, std::uint64_t bytes = 0)
		{
			if (bytes > m_Limit)
				return false;
			const auto old = m_Entries.find(replacing);
			const auto replacedBytes = old == m_Entries.end() ? 0 : old->second.bytes;
			while (m_Size - replacedBytes > m_Limit - bytes)
			{
				auto oldest = m_Entries.end();
				for (auto entry = m_Entries.begin(); entry != m_Entries.end(); ++entry)
				{
					if (entry == old)
						continue;
					if (oldest == m_Entries.end() ||
						std::tie(entry->second.used, entry->first) < std::tie(oldest->second.used, oldest->first))
						oldest = entry;
				}
				if (oldest == m_Entries.end())
					return false;

				std::error_code error;
				std::filesystem::remove(oldest->first, error);
				if (error)
					return false;
				m_Size -= oldest->second.bytes;
				m_Entries.erase(oldest);
				++m_Evictions;
			}
			return true;
		}

		bool Commit(const std::filesystem::path& temporary, const std::filesystem::path& path,
			std::uint64_t bytes)
		{
			// Reserve the map node before publishing so an allocation failure
			// cannot leave a committed entry unaccounted for.
			const auto [entry, inserted] = m_Entries.try_emplace(path);
			std::error_code error;
			std::filesystem::rename(temporary, path, error);
			if (error)
			{
				if (inserted)
					m_Entries.erase(entry);
				return false;
			}
			m_Size -= entry->second.bytes;
			entry->second = {bytes, std::filesystem::file_time_type::clock::now()};
			m_Size += bytes;
			return true;
		}

		std::uint64_t Size() const noexcept { return m_Size; }
		std::size_t Evictions() const noexcept { return m_Evictions; }

	private:
		struct Entry
		{
			std::uint64_t bytes = 0;
			std::filesystem::file_time_type used{};
		};

		static bool IsEntryName(std::string_view name)
		{
			const auto dash = name.find('-');
			const auto digits = [](std::string_view value)
			{
				return !value.empty() && value.size() <= 20 &&
					value.find_first_not_of("0123456789") == std::string_view::npos;
			};
			return dash != name.npos && digits(name.substr(0, dash)) && digits(name.substr(dash + 1));
		}

		std::map<std::filesystem::path, Entry> m_Entries;
		std::uint64_t m_Limit;
		std::uint64_t m_Size = 0;
		std::size_t m_Evictions = 0;
	};
}
