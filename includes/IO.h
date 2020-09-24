
#ifndef MEMORY_ALLOCATORS_IO_H_INCLUDED
#define MEMORY_ALLOCATORS_IO_H_INCLUDED

#include <iostream>

namespace IO
{
    // This function replaces the traditional std::endl in order to prevent
    // the repeated flushing of the output buffer via std::fflush(). Rather,
    // this function simply outputs a new line, yet it keeps with the
    // syntax of traditional << std::endl usage.
    //
    // Jose Fernando Lopez Fernandez 11/07/2018 @ 2:19am (UTC)

    template <typename T, typename CharT = std::char_traits<T>>
    std::basic_ostream<T, CharT>&
        endl(std::basic_ostream<T, CharT>& outputStream)
    {
        return outputStream << outputStream.widen('\n');
    }
}

#endif // MEMORY_ALLOCATORS_IO_H_INCLUDED
