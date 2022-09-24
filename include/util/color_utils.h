/**	@file color_utils.h
 *  @brief Utility methods related to color.
 *
 *	@author Isaac
 *	@bug No known bugs.
 */
#ifndef COLOR_UTILS_H
#define COLOR_UTILS_H

#include <cstdint>

namespace libtp::util::color
{
    uint8_t desaturateRgb565( uint16_t rgb565Val );
    uint16_t blendOverlayRgb565( uint8_t grayVal, uint8_t* rgb );
}     // namespace libtp::util::color
#endif