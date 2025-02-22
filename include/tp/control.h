/**	@file control.h
 *	@brief Contains functions for handling various aspects of the game
 *
 *  @author Zephiles
 *	@author Lunar Soap
 *	@bug No known bugs.
 */
#ifndef TP_CONTROL_H
#define TP_CONTROL_H

#include <cstdint>

#include "gc_wii/bmgres.h"

namespace libtp::tp::control
{
    struct TControl
    {
        void* unk_0;
        void* unk_4;
        void* unk_8;
        uint16_t unk_E;     // unk3 in setMessageCode_inSequence
        uint16_t msgId;
        void* unk_10;
        gc::bmgres::MessageEntry* msgEntry;
        const char* msg;
        uint8_t unk_1C[0x4];
        const char* wMsgRender;
        uint32_t unk_24;
    } __attribute__( ( __packed__ ) );

    extern "C"
    {
        bool setMessageCode_inSequence( TControl* control, const void* TProcessor, uint16_t unk3, uint16_t msgId );
    }
}     // namespace libtp::tp::control
#endif