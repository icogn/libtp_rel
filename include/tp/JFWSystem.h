/**	@file JFWSystem.h
 *	@brief Holds symbols of the jfw_system field
 *
 *	@author Zephiles
 *	@author AECX
 *	@bug No known bugs.
 */
#ifndef TP_JFWSYSTEM_H
#define TP_JFWSYSTEM_H

// Dedicated console line used to print messages wtihout relying on the Console Class from libtp
#define JFW_DEBUG_LINE 24

#include <cstdint>

namespace libtp::tp::jfw_system
{
    struct ConsoleLine
    {
        bool showLine;
        char line[61];
    } __attribute__( ( __packed__ ) );

    /**
     *	@brief SystemConsole variables which is normally used for debugging
     *
     *	@todo Fill missing vars.
     *	@todo Get exact size.
     */
    struct SystemConsole
    {
        uint8_t unk_0[0x60];
        uint8_t consoleColor[4];     // rgba
        uint8_t unk_64[0x4];
        bool consoleEnabled;
        uint8_t unk_69[3];
        ConsoleLine consoleLine[25];     // Should figure out the total amount of lines
                                         // at some point
    } __attribute__( ( __packed__ ) );

    static_assert( sizeof( ConsoleLine ) == 0x3E );

    extern "C"
    {
        extern SystemConsole* systemConsole;
    }
}     // namespace libtp::tp::jfw_system
#endif