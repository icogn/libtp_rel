/**	@file f_ap_game.h
 *	@brief Holds symbols from the f_ap_game field
 *
 *	@author Zephiles
 *	@bug No known bugs.
 */
#ifndef TP_F_AP_GAME_H
#define TP_F_AP_GAME_H

namespace libtp::tp::f_ap_game
{
    extern "C"
    {
        /**
         *	@brief Runs once per frame
         */
        void fapGm_Execute();
    }
}     // namespace libtp::tp::f_ap_game
#endif