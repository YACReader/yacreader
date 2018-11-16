// memory_file.hpp
// Copyright (c) 2015-2018 Ben Hanson (http://www.benhanson.net/)
// Inspired by http://en.wikibooks.org/wiki/Optimizing_C%2B%2B/
// General_optimization_techniques/Input/Output#Memory-mapped_file
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file licence_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef LEXERTL_MEMORY_FILE_HPP
#define LEXERTL_MEMORY_FILE_HPP

#include <cstddef>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif

// Only files small enough to fit into memory are supported.
namespace lexertl
{
template<typename char_type>
class basic_memory_file
{
public:
    basic_memory_file()
    {
    }

    basic_memory_file(const char *pathname_)
    {
        open(pathname_);
    }

    ~basic_memory_file()
    {
        close();
    }

    void open(const char *pathname_)
    {
        if (_data) close();

#ifdef _WIN32
        _fh = ::CreateFileA(pathname_, GENERIC_READ, FILE_SHARE_READ, 0,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        _fmh = 0;

        if (_fh != INVALID_HANDLE_VALUE)
        {
            _fmh = ::CreateFileMapping(_fh, 0, PAGE_READONLY, 0, 0, 0);

            if (_fmh != 0)
            {
                _data = static_cast<char_type *>(::MapViewOfFile
                    (_fmh, FILE_MAP_READ, 0, 0, 0));

                if (_data) _size = ::GetFileSize(_fh, 0) / sizeof(char_type);
            }
        }
#else
        _fh = ::open(pathname_, O_RDONLY);

        if (_fh > -1)
        {
            struct stat sbuf_;

            if (::fstat(_fh, &sbuf_) > -1)
            {
                _data = static_cast<const char_type *>
                    (::mmap(0, sbuf_.st_size, PROT_READ, MAP_SHARED, _fh, 0));

                if (_data == MAP_FAILED)
                {
                    _data = nullptr;
                }
                else
                {
                    _size = sbuf_.st_size / sizeof(char_type);
                }
            }
        }
#endif
    }

    const char_type *data() const
    {
        return _data;
    }

    std::size_t size() const
    {
        return _size;
    }

    void close()
    {
#ifdef _WIN32
        ::UnmapViewOfFile(_data);
        ::CloseHandle(_fmh);
        ::CloseHandle(_fh);
#else
        ::munmap(const_cast<char_type *>(_data), _size);
        ::close(_fh);
#endif
        _data = nullptr;
        _size = 0;
        _fh = 0;
#ifdef _WIN32
        _fmh = 0;
#endif
    }

private:
    const char_type *_data = nullptr;
    std::size_t _size = 0;
#ifdef _WIN32
    HANDLE _fh = 0;
    HANDLE _fmh = 0;
#else
    int _fh = 0;
#endif

    // No copy construction.
    basic_memory_file(const basic_memory_file &) = delete;
    // No assignment.
    basic_memory_file &operator =(const basic_memory_file &) = delete;
};

using memory_file = basic_memory_file<char>;
using wmemory_file = basic_memory_file<wchar_t>;
}

#endif
