/**	@file d_s_logo.h
 *	@brief Contains language definitions
 *
 *	@author Zephiles
 *	@bug No known bugs.
 */
#ifndef TP_D_S_LOGO_H
#define TP_D_S_LOGO_H

#include <cstdint>

namespace libtp::tp::d_s_logo
{
    enum class Languages : int32_t
    {
        uk = 0,     // English
        de,         // German
        fr,         // French
        sp,         // Spanish
        it          // Italian
    };

    extern "C"
    {
#ifdef TP_EU
        // This function only exists in the PAL version
        Languages getPalLanguage2( void* dScnLogo_c );     // dScnLogo_c is unused
#endif
        void* dScnLogo_c_dt( void* dScnLogo_c, int16_t freeThis );
    }
}     // namespace libtp::tp::d_s_logo
#endif