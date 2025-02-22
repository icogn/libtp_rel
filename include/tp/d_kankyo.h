/**	@file d_kankyo.h
 *	@brief Kankyo field vars
 *
 *	The d_kankyo field holds lighting information
 *
 *	@author Zephiles
 *	@author AECX
 *	@bug No known bugs.
 */
#ifndef TP_D_KANKYO_H
#define TP_D_KANKYO_H

#include <cstdint>

namespace libtp::tp::d_kankyo
{
    /**
     *	@brief Environment infos
     *
     *	@todo Fill missing vars.
     */
    struct EnvLight
    {
        /* 0x0 */ uint8_t unk_0[0x98C];
        /* 0x98C */ uint8_t currentRoom;
        /* 0x98D */ uint8_t unk_98d[0x6C3];
        /* 0x1050 */ uint8_t mEvilPacketEnabled;
        /* 0x1051 */ uint8_t unk_1051[0x2BF];
    } __attribute__( ( __packed__ ) );

    static_assert( sizeof( EnvLight ) == 0x1310 );

    extern "C"
    {
        extern EnvLight env_light;

        bool dKy_darkworld_stage_check( char const* param_0, int32_t param_1 );
    }
}     // namespace libtp::tp::d_kankyo
#endif