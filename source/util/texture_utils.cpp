#include <cstdint>
#include <cstring>
// #include <functional>

#include "util/color_utils.h"

namespace libtp::util::texture
{
    uint8_t* findTexHeaderInTex1Section( uint8_t* tex1Ptr, const char* textureName )
    {
        if ( tex1Ptr == nullptr )
        {
            return nullptr;
        }

        uint16_t numTextures = *( reinterpret_cast<uint16_t*>( tex1Ptr + 8 ) );
        uint8_t* strTable = tex1Ptr + *( reinterpret_cast<uint32_t*>( tex1Ptr + 0x10 ) );

        uint16_t numStrings = *( reinterpret_cast<uint16_t*>( strTable ) );

        for ( uint16_t i = 0; i < numStrings && i < numTextures; i++ )
        {
            uint16_t offsetInStrTable = *( reinterpret_cast<uint16_t*>( strTable + 4 + 2 + 4 * i ) );
            char* strPtr = reinterpret_cast<char*>( strTable + offsetInStrTable );
            if ( strcmp( strPtr, textureName ) == 0 )
            {
                return tex1Ptr + 0x20 + i * 0x20;
            }
        }

        return nullptr;
    }

    uint32_t swapIndexBits( bool leftIsGreater, uint32_t bits )
    {
        if ( leftIsGreater )
        {
            return bits ^ 0x55555555;
        }
        else
        {
            uint32_t r = ( ( bits >> 1 ) & 0x55555555 ) ^ 0x55555555;
            return bits ^ r;
        }
    }

    // template<typename Func>
    // void recolorCmprTexture( uint8_t* tex1Ptr,
    //                          const char* textureName,
    //                          uint8_t* rgb,
    //                          std::function<bool( uint8_t, uint8_t, uint8_t )> fn )
    // {
    //     uint8_t* texHeaderPtr = findTexHeaderInTex1Section( tex1Ptr, textureName );
    //     if ( texHeaderPtr == nullptr )
    //     {
    //         return;
    //     }

    //     if ( texHeaderPtr[0] != 0xE )
    //     {
    //         // Texture is not CMPR
    //         return;
    //     }

    //     uint16_t recolors[0x100];
    //     for ( int i = 0; i < 0x100; i++ )
    //     {
    //         recolors[i] = libtp::util::color::blendOverlayRgb565( i, rgb );
    //     }

    //     // These two come from tex1 data.
    //     // int width = 0x100;
    //     // int height = 0x80;

    //     int width = *( reinterpret_cast<uint16_t*>( texHeaderPtr + 2 ) );
    //     int height = *( reinterpret_cast<uint16_t*>( texHeaderPtr + 4 ) );

    //     int blockWidth = 8;
    //     int blockHeight = 8;
    //     // int blockStride = 0x20;

    //     int roundedWidth = width + ( ( blockWidth - ( width % blockWidth ) ) % blockWidth );
    //     int roundedHeight = height + ( ( blockHeight - ( height % blockHeight ) ) % blockHeight );

    //     int numBlocks = roundedWidth / blockWidth * roundedHeight / blockHeight;

    //     int iterations = numBlocks * 4;

    //     uint8_t* currentAddr = texHeaderPtr + *( reinterpret_cast<int32_t*>( texHeaderPtr + 0x1C ) );
    //     for ( int i = 0; i < iterations; i++ )
    //     {
    //         uint16_t* rgb565Ptr = reinterpret_cast<uint16_t*>( currentAddr );

    //         uint16_t leftRgb565 = rgb565Ptr[0];
    //         uint16_t rightRgb565 = rgb565Ptr[1];
    //         bool leftIsGreater = leftRgb565 > rightRgb565;

    //         uint8_t leftGrayVal = libtp::util::color::desaturateRgb565( leftRgb565 );
    //         uint8_t rightGrayVal = libtp::util::color::desaturateRgb565( rightRgb565 );

    //         uint16_t leftNewRgb565 = recolors[leftGrayVal];
    //         uint16_t rightNewRgb565 = recolors[rightGrayVal];
    //         // bool leftChanged = true;
    //         // bool rightChanged = true;

    //         // uint32_t leftR = ( leftRgb565 & 0xf800 ) >> 10;
    //         // if ( leftR < 0x3f && leftR & 0x20 )
    //         // {
    //         //     leftR++;
    //         // }
    //         // uint32_t leftG = ( leftRgb565 & 0x7e0 ) >> 5;
    //         // uint32_t leftB = ( leftRgb565 & 0x1f ) << 1;
    //         // if ( leftB < 0x3f && leftB & 0x20 )
    //         // {
    //         //     leftB++;
    //         // }

    //         // // // if ( leftR <= leftG + 1 || leftR <= leftB + 1 ) // bird + tf
    //         // // // if ( leftR <= leftG + 2 || leftR <= leftB + 1 )
    //         // // if ( leftR <= leftG + 3 || leftR <= leftB + 1 )     // seems like just bird
    //         // // {
    //         // //     leftNewRgb565 = leftRgb565;
    //         // //     // leftChanged = false;
    //         // // }

    //         // if ( fn != nullptr && !fn( leftR, leftG, leftB ) )
    //         // {
    //         //     // Don't update the left color. Note that 'left' and 'right'
    //         //     // might swap later in this function.
    //         //     leftNewRgb565 = leftRgb565;
    //         // }

    //         // uint32_t rightR = ( rightRgb565 & 0xf800 ) >> 10;
    //         // if ( rightR < 0x3f && rightR & 0x20 )
    //         // {
    //         //     rightR++;
    //         // }
    //         // uint32_t rightG = ( rightRgb565 & 0x7e0 ) >> 5;
    //         // uint32_t rightB = ( rightRgb565 & 0x1f ) << 1;
    //         // if ( rightB < 0x3f && rightB & 0x20 )
    //         // {
    //         //     rightB++;
    //         //     // rightChanged = false;
    //         // }

    //         // // // if ( rightR <= rightG + 1 || rightR <= rightB + 1 )     // bird + tf
    //         // // // if ( rightR <= rightG + 2 || rightR <= rightB + 1 )
    //         // // if ( rightR <= rightG + 3 || rightR <= rightB + 1 )     // seems like just bird
    //         // // {
    //         // //     rightNewRgb565 = rightRgb565;
    //         // // }

    //         // if ( fn != nullptr && !fn( rightR, rightG, rightB ) )
    //         // {
    //         //     // Don't update the right color. Note that 'left' and 'right'
    //         //     // might swap later in this function.
    //         //     rightNewRgb565 = rightRgb565;
    //         // }

    //         // When left is greater than right
    //         // 0b00 points to the left color
    //         // 0b01 points to the right color
    //         // 0b10 is closer to left color
    //         // 0b11 is closer to right color

    //         // When left is not greater than right
    //         // 0b00 points to the left color
    //         // 0b01 points to the right color
    //         // 0b10 is midway between the colors
    //         // 0b11 is transparent

    //         // That means when maintaining the relative order, if we have to swap the colors:

    //         // in the case of left being greater than right:

    //         // 0b00 will swap to 0b01
    //         // 0b01 will swap to 0b00
    //         // 0b10 will swap to 0b11
    //         // 0b11 will swap to 0b10
    //         // So the left bit stays the same, and the right bit changes
    //         // Can do xor (^) like 0b01010101 or 0x55 for each u16

    //         // in the case of left not being greater than right:
    //         // 0b00 will swap to 0b01
    //         // 0b01 will swap to 0b00
    //         // 0b10 will stay the same
    //         // 0b11 will stay the same
    //         // so if the left bit is a 0, the right bit will change
    //         // Can generate the xor value using a calc (instead of using hardcoded 0x55)

    //         // Want to know all of them which are 0 on the left bit
    //         //  0xAA (0b10101010)
    //         // (Word & 0xAA) >> 1

    //         // new method, changing indexes
    //         if ( leftIsGreater )
    //         {
    //             if ( leftNewRgb565 == rightNewRgb565 )
    //             {
    //                 if ( leftNewRgb565 == 0 )
    //                 {
    //                     leftNewRgb565 = 1;
    //                     rightNewRgb565 = 0;
    //                 }
    //                 else
    //                 {
    //                     // Need to make sure that subtracting 1 does not mess everything up.
    //                     // For example, 0x1000 - 1 => 0x0fff which is a completely different color.
    //                     if ( ( leftNewRgb565 & 0x1f ) == 0 )
    //                     {
    //                         // If left value has 0 blue, we change its blue to 1.
    //                         leftNewRgb565 += 1;
    //                     }
    //                     rightNewRgb565 = leftNewRgb565 - 1;
    //                 }
    //             }
    //             else if ( leftNewRgb565 < rightNewRgb565 )
    //             {
    //                 uint16_t temp = leftNewRgb565;
    //                 leftNewRgb565 = rightNewRgb565;
    //                 rightNewRgb565 = temp;

    //                 uint32_t* wordPtr = reinterpret_cast<uint32_t*>( currentAddr );
    //                 uint32_t bits = wordPtr[1];

    //                 uint32_t newBits = swapIndexBits( true, bits );
    //                 wordPtr[1] = newBits;
    //             }
    //         }
    //         else if ( leftNewRgb565 > rightNewRgb565 )
    //         {
    //             uint16_t temp = leftNewRgb565;
    //             leftNewRgb565 = rightNewRgb565;
    //             rightNewRgb565 = temp;

    //             uint32_t* wordPtr = reinterpret_cast<uint32_t*>( currentAddr );
    //             uint32_t bits = wordPtr[1];

    //             uint32_t newBits = swapIndexBits( false, bits );
    //             wordPtr[1] = newBits;
    //         }

    //         // // old method, changing colors
    //         // if ( leftIsGreater )
    //         // {
    //         //     if ( leftNewRgb565 <= rightNewRgb565 )
    //         //     {
    //         //         // if ( leftChanged && !rightChanged )
    //         //         // if ( leftChanged == true && rightChanged == false )
    //         //         if ( leftChanged == true )
    //         //         {
    //         //             // prefers left
    //         //             if ( leftNewRgb565 == 0 )
    //         //             {
    //         //                 leftNewRgb565 = 1;
    //         //                 rightNewRgb565 = 0;
    //         //             }
    //         //             else
    //         //             {
    //         //                 // need to make sure that subtracting 1 does not mess everything up.
    //         //                 // For example, 0x1000 - 1 => 0x0fff which is a completely different color.
    //         //                 rightNewRgb565 = leftNewRgb565 - 1;
    //         //             }
    //         //         }
    //         //         else
    //         //         {
    //         //             // prefers right
    //         //             if ( rightNewRgb565 == 0xffff )
    //         //             {
    //         //                 leftNewRgb565 = 0xffff;
    //         //                 rightNewRgb565 = 0xfffe;
    //         //             }
    //         //             else
    //         //             {
    //         //                 leftNewRgb565 = rightNewRgb565 + 1;
    //         //             }
    //         //         }
    //         //     }
    //         // }
    //         // else if ( leftNewRgb565 > rightNewRgb565 )
    //         // {
    //         //     // if ( leftChanged && !rightChanged )
    //         //     // if ( leftChanged == true && rightChanged == false )
    //         //     if ( leftChanged == true )
    //         //     {
    //         //         // prefers left
    //         //         rightNewRgb565 = leftNewRgb565;
    //         //     }
    //         //     else
    //         //     {
    //         //         // prefers right
    //         //         leftNewRgb565 = rightNewRgb565;
    //         //     }

    //         //     // old
    //         //     // rightNewRgb565 = leftNewRgb565;
    //         // }

    //         rgb565Ptr[0] = leftNewRgb565;
    //         rgb565Ptr[1] = rightNewRgb565;

    //         currentAddr += 8;
    //     }
    // }

    // void recolorCmprTexture( uint8_t* tex1Ptr, const char* textureName, uint8_t* rgb )
    // {
    //     recolorCmprTexture( tex1Ptr, textureName, rgb, nullptr );
    // }
    void recolorCmprTexture( uint8_t* tex1Ptr, const char* textureName, uint8_t* rgb )
    {
        uint8_t* texHeaderPtr = findTexHeaderInTex1Section( tex1Ptr, textureName );
        if ( texHeaderPtr == nullptr )
        {
            return;
        }

        if ( texHeaderPtr[0] != 0xE )
        {
            // Texture is not CMPR
            return;
        }

        uint16_t recolors[0x100];
        for ( int i = 0; i < 0x100; i++ )
        {
            recolors[i] = libtp::util::color::blendOverlayRgb565( i, rgb );
        }

        // These two come from tex1 data.
        // int width = 0x100;
        // int height = 0x80;

        int width = *( reinterpret_cast<uint16_t*>( texHeaderPtr + 2 ) );
        int height = *( reinterpret_cast<uint16_t*>( texHeaderPtr + 4 ) );

        int blockWidth = 8;
        int blockHeight = 8;
        // int blockStride = 0x20;

        int roundedWidth = width + ( ( blockWidth - ( width % blockWidth ) ) % blockWidth );
        int roundedHeight = height + ( ( blockHeight - ( height % blockHeight ) ) % blockHeight );

        int numBlocks = roundedWidth / blockWidth * roundedHeight / blockHeight;

        int iterations = numBlocks * 4;

        uint8_t* currentAddr = texHeaderPtr + *( reinterpret_cast<int32_t*>( texHeaderPtr + 0x1C ) );
        for ( int i = 0; i < iterations; i++ )
        {
            uint16_t* rgb565Ptr = reinterpret_cast<uint16_t*>( currentAddr );

            uint16_t leftRgb565 = rgb565Ptr[0];
            uint16_t rightRgb565 = rgb565Ptr[1];
            bool leftIsGreater = leftRgb565 > rightRgb565;

            uint8_t leftGrayVal = libtp::util::color::desaturateRgb565( leftRgb565 );
            uint8_t rightGrayVal = libtp::util::color::desaturateRgb565( rightRgb565 );

            uint16_t leftNewRgb565 = recolors[leftGrayVal];
            uint16_t rightNewRgb565 = recolors[rightGrayVal];
            // bool leftChanged = true;
            // bool rightChanged = true;

            // uint32_t leftR = ( leftRgb565 & 0xf800 ) >> 10;
            // if ( leftR < 0x3f && leftR & 0x20 )
            // {
            //     leftR++;
            // }
            // uint32_t leftG = ( leftRgb565 & 0x7e0 ) >> 5;
            // uint32_t leftB = ( leftRgb565 & 0x1f ) << 1;
            // if ( leftB < 0x3f && leftB & 0x20 )
            // {
            //     leftB++;
            // }

            // // // if ( leftR <= leftG + 1 || leftR <= leftB + 1 ) // bird + tf
            // // // if ( leftR <= leftG + 2 || leftR <= leftB + 1 )
            // // if ( leftR <= leftG + 3 || leftR <= leftB + 1 )     // seems like just bird
            // // {
            // //     leftNewRgb565 = leftRgb565;
            // //     // leftChanged = false;
            // // }

            // if ( fn != nullptr && !fn( leftR, leftG, leftB ) )
            // {
            //     // Don't update the left color. Note that 'left' and 'right'
            //     // might swap later in this function.
            //     leftNewRgb565 = leftRgb565;
            // }

            // uint32_t rightR = ( rightRgb565 & 0xf800 ) >> 10;
            // if ( rightR < 0x3f && rightR & 0x20 )
            // {
            //     rightR++;
            // }
            // uint32_t rightG = ( rightRgb565 & 0x7e0 ) >> 5;
            // uint32_t rightB = ( rightRgb565 & 0x1f ) << 1;
            // if ( rightB < 0x3f && rightB & 0x20 )
            // {
            //     rightB++;
            //     // rightChanged = false;
            // }

            // // // if ( rightR <= rightG + 1 || rightR <= rightB + 1 )     // bird + tf
            // // // if ( rightR <= rightG + 2 || rightR <= rightB + 1 )
            // // if ( rightR <= rightG + 3 || rightR <= rightB + 1 )     // seems like just bird
            // // {
            // //     rightNewRgb565 = rightRgb565;
            // // }

            // if ( fn != nullptr && !fn( rightR, rightG, rightB ) )
            // {
            //     // Don't update the right color. Note that 'left' and 'right'
            //     // might swap later in this function.
            //     rightNewRgb565 = rightRgb565;
            // }

            // When left is greater than right
            // 0b00 points to the left color
            // 0b01 points to the right color
            // 0b10 is closer to left color
            // 0b11 is closer to right color

            // When left is not greater than right
            // 0b00 points to the left color
            // 0b01 points to the right color
            // 0b10 is midway between the colors
            // 0b11 is transparent

            // That means when maintaining the relative order, if we have to swap the colors:

            // in the case of left being greater than right:

            // 0b00 will swap to 0b01
            // 0b01 will swap to 0b00
            // 0b10 will swap to 0b11
            // 0b11 will swap to 0b10
            // So the left bit stays the same, and the right bit changes
            // Can do xor (^) like 0b01010101 or 0x55 for each u16

            // in the case of left not being greater than right:
            // 0b00 will swap to 0b01
            // 0b01 will swap to 0b00
            // 0b10 will stay the same
            // 0b11 will stay the same
            // so if the left bit is a 0, the right bit will change
            // Can generate the xor value using a calc (instead of using hardcoded 0x55)

            // Want to know all of them which are 0 on the left bit
            //  0xAA (0b10101010)
            // (Word & 0xAA) >> 1

            // new method, changing indexes
            if ( leftIsGreater )
            {
                if ( leftNewRgb565 == rightNewRgb565 )
                {
                    if ( leftNewRgb565 == 0 )
                    {
                        leftNewRgb565 = 1;
                        rightNewRgb565 = 0;
                    }
                    else
                    {
                        // Need to make sure that subtracting 1 does not mess everything up.
                        // For example, 0x1000 - 1 => 0x0fff which is a completely different color.
                        if ( ( leftNewRgb565 & 0x1f ) == 0 )
                        {
                            // If left value has 0 blue, we change its blue to 1.
                            leftNewRgb565 += 1;
                        }
                        rightNewRgb565 = leftNewRgb565 - 1;
                    }
                }
                else if ( leftNewRgb565 < rightNewRgb565 )
                {
                    uint16_t temp = leftNewRgb565;
                    leftNewRgb565 = rightNewRgb565;
                    rightNewRgb565 = temp;

                    uint32_t* wordPtr = reinterpret_cast<uint32_t*>( currentAddr );
                    uint32_t bits = wordPtr[1];

                    uint32_t newBits = swapIndexBits( true, bits );
                    wordPtr[1] = newBits;
                }
            }
            else if ( leftNewRgb565 > rightNewRgb565 )
            {
                uint16_t temp = leftNewRgb565;
                leftNewRgb565 = rightNewRgb565;
                rightNewRgb565 = temp;

                uint32_t* wordPtr = reinterpret_cast<uint32_t*>( currentAddr );
                uint32_t bits = wordPtr[1];

                uint32_t newBits = swapIndexBits( false, bits );
                wordPtr[1] = newBits;
            }

            // // old method, changing colors
            // if ( leftIsGreater )
            // {
            //     if ( leftNewRgb565 <= rightNewRgb565 )
            //     {
            //         // if ( leftChanged && !rightChanged )
            //         // if ( leftChanged == true && rightChanged == false )
            //         if ( leftChanged == true )
            //         {
            //             // prefers left
            //             if ( leftNewRgb565 == 0 )
            //             {
            //                 leftNewRgb565 = 1;
            //                 rightNewRgb565 = 0;
            //             }
            //             else
            //             {
            //                 // need to make sure that subtracting 1 does not mess everything up.
            //                 // For example, 0x1000 - 1 => 0x0fff which is a completely different color.
            //                 rightNewRgb565 = leftNewRgb565 - 1;
            //             }
            //         }
            //         else
            //         {
            //             // prefers right
            //             if ( rightNewRgb565 == 0xffff )
            //             {
            //                 leftNewRgb565 = 0xffff;
            //                 rightNewRgb565 = 0xfffe;
            //             }
            //             else
            //             {
            //                 leftNewRgb565 = rightNewRgb565 + 1;
            //             }
            //         }
            //     }
            // }
            // else if ( leftNewRgb565 > rightNewRgb565 )
            // {
            //     // if ( leftChanged && !rightChanged )
            //     // if ( leftChanged == true && rightChanged == false )
            //     if ( leftChanged == true )
            //     {
            //         // prefers left
            //         rightNewRgb565 = leftNewRgb565;
            //     }
            //     else
            //     {
            //         // prefers right
            //         leftNewRgb565 = rightNewRgb565;
            //     }

            //     // old
            //     // rightNewRgb565 = leftNewRgb565;
            // }

            rgb565Ptr[0] = leftNewRgb565;
            rgb565Ptr[1] = rightNewRgb565;

            currentAddr += 8;
        }
    }

    uint8_t* findTex1InBmd( uint8_t* bmdPtr )
    {
        if ( bmdPtr == nullptr )
        {
            return nullptr;
        }

        uint32_t j3d2Magic = *( reinterpret_cast<uint32_t*>( bmdPtr ) );
        if ( j3d2Magic != 0x4A334432 )
        {
            // Model was not a BMD or BDL! (J3D2 magic not found)
            return nullptr;
        }
        uint32_t modelMagic = *( reinterpret_cast<uint32_t*>( bmdPtr + 4 ) );
        if ( ( modelMagic != 0x62646C34 ) && ( modelMagic != 0x626D6433 ) )
        {
            // Model was not a BMD or BDL! (Model type was not bmd3 or bdl4)
            return nullptr;
        }

        int32_t modelSize = *( reinterpret_cast<int32_t*>( bmdPtr + 8 ) );
        int32_t sectionCount = *( reinterpret_cast<int32_t*>( bmdPtr + 0xC ) );
        uint8_t* endOfModelAddr = bmdPtr + modelSize;

        uint8_t* currentSectionAddr = bmdPtr + 0x20;
        for ( int i = 0; i < sectionCount; i++ )
        {
            uint32_t sectionMagic = *( reinterpret_cast<uint32_t*>( currentSectionAddr ) );
            if ( sectionMagic == 0x54455831 )
            {
                // Section is TEX1
                return currentSectionAddr;
            }

            currentSectionAddr += *( reinterpret_cast<int32_t*>( currentSectionAddr + 4 ) );

            if ( currentSectionAddr >= endOfModelAddr )
            {
                break;
            }
        }

        return nullptr;
    }

}     // namespace libtp::util::texture