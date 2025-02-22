#include "tools.h"

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cinttypes>

#ifndef PLATFORM_WII
#include "gc_wii/card.h"
#else
#include "gc_wii/nand.h"
#endif

#include "cxx.h"
#include "data/stages.h"
#include "display/console.h"
#include "gc_wii/dvd.h"
#include "gc_wii/OSModule.h"
#include "memory.h"
#include "tp/JFWSystem.h"
#include "tp/d_com_inf_game.h"
#include "tp/d_stage.h"
#include "tp/dzx.h"
#include "tp/f_op_actor_mng.h"
#include "tp/m_do_memcard.h"
#include "gc_wii/OSCache.h"
#include "gc_wii/OSInterrupt.h"
#include "tp/d_kankyo.h"
#include "tp/d_a_alink.h"

namespace libtp::tools
{
    uint16_t fletcher16( uint8_t* data, int32_t length )
    {
        uint16_t sum1 = 0;
        uint16_t sum2 = 0;

        for ( int32_t index = 0; index < length; ++index )
        {
            sum1 = ( sum1 + data[index] ) % 0xFF;
            sum2 = ( sum2 + sum1 ) % 0xFF;
        }

        return ( sum2 << 8 ) | sum1;
    }

    // This entire function will need to be re-looked at now that a lot of defintions are different. UPDATE: should be better.
    // updated data types in libtp
    void TriggerSaveLoad( const char* stage, uint8_t room, uint8_t spawn, uint8_t state, uint8_t event )
    {
        using namespace libtp::tp::d_com_inf_game;

        dComIfG_inf_c* gameInfo = &dComIfG_gameInfo;
        strcpy( gameInfo->play.mNextStage.stageValues.mStage, stage );
        gameInfo->play.mNextStage.stageValues.mRoomNo = room;
        gameInfo->play.mNextStage.stageValues.mPoint = spawn;
        gameInfo->play.mNextStage.stageValues.mLayer = state;

        gameInfo->play.mEvent.mOrder[0].mEventInfoIdx = event;
        gameInfo->save.restart.mLastMode = 0;
        gameInfo->play.mNextStage.stageValues.mPoint = 0;
        gameInfo->save.restart.mRoomParam = 0;
        gameInfo->play.mEvent.mOrder[0].mEventId = 0xFFFF;     // immediateControl
        gameInfo->play.mNextStage.wipe_speed = 0x13;

        gameInfo->play.mNextStage.wipe = true;
    }

    void SpawnActor( uint8_t roomID, tp::dzx::ACTR& actor )
    {
        using namespace libtp::tp::dzx;
        using namespace libtp::tp::f_op_actor_mng;

        ActorPRMClass* actorMemoryPtr = CreateAppend();

        actorMemoryPtr->params = actor.parameters;

        actorMemoryPtr->pos[0] = actor.pos[0];
        actorMemoryPtr->pos[1] = actor.pos[1];
        actorMemoryPtr->pos[2] = actor.pos[2];

        actorMemoryPtr->xRot = actor.rot[0];
        actorMemoryPtr->yRot = actor.rot[1];

        actorMemoryPtr->flag = actor.flag;
        actorMemoryPtr->enemy_id = actor.enemyID;
        actorMemoryPtr->room_id = roomID;

        tp::d_stage::ActorCreate( &actor, actorMemoryPtr );
    }

    void SpawnSCOB( uint8_t roomID, tp::dzx::SCOB& actor_data )
    {
        using namespace libtp::tp::dzx;
        using namespace libtp::tp::f_op_actor_mng;

        ActorPRMClass* actorMemoryPtr = CreateAppend();

        actorMemoryPtr->params = actor_data.parameters;

        actorMemoryPtr->pos[0] = actor_data.pos[0];
        actorMemoryPtr->pos[1] = actor_data.pos[1];
        actorMemoryPtr->pos[2] = actor_data.pos[2];

        actorMemoryPtr->xRot = actor_data.rot[0];
        actorMemoryPtr->yRot = actor_data.rot[1];

        actorMemoryPtr->flag = actor_data.flag;
        actorMemoryPtr->enemy_id = actor_data.enemyID;
        actorMemoryPtr->room_id = roomID;
        actorMemoryPtr->mScale[0] = actor_data.xScale;
        actorMemoryPtr->mScale[1] = actor_data.yScale;
        actorMemoryPtr->mScale[2] = actor_data.zScale;

        tp::d_stage::ActorCreate( &actor_data, actorMemoryPtr );
    }
#ifndef PLATFORM_WII
    int32_t mountMemoryCard( int32_t chan )
    {
        using namespace libtp::gc_wii::card;

        int32_t result;
        uint8_t* workArea;

        // Make sure the memory card is already unmounted before trying to mount it
        CARDUnmount( chan );

        // Check if memory card is valid
        for ( uint32_t i = 0; i < 1000000; i++ )
        {
            result = CARDProbeEx( chan, nullptr, nullptr );
            if ( result != CARD_RESULT_BUSY )
            {
                break;
            }
        }

        if ( result == CARD_RESULT_READY )
        {
            workArea = libtp::tp::m_Do_MemCard::MemCardWorkArea0;

            // Clear the memory for the work area
            libtp::memory::clearMemory( workArea, CARD_WORKAREA_SIZE );

            // Clear the cache for the work area
            libtp::gc_wii::os_cache::DCFlushRange( workArea, CARD_WORKAREA_SIZE );

            // Mount the memory card
            result = CARDMount( chan, workArea, nullptr );

            if ( ( result == CARD_RESULT_READY ) || ( result == CARD_RESULT_BROKEN ) )
            {
                // Check for and attempt to repair memory card errors
                result = CARDCheck( chan );
            }
        }

        return result;
    }

    int32_t ReadGCIMounted( int32_t chan,
                            const char* fileName,
                            int32_t length,
                            int32_t offset,
                            void* buffer,
                            bool startAfterComments )
    {
        using namespace libtp::gc_wii::card;

        CARDFileInfo fileInfo;
        CARDStat stat;
        int32_t result;

        int32_t adjustedOffset;
        int32_t adjustedLength;
        uint8_t* data;

        // Read data
        result = CARDOpen( chan, fileName, &fileInfo );
        if ( result == CARD_RESULT_READY )
        {
            // Increment the offset if desired
            if ( startAfterComments )
            {
                result = CARDGetStatus( chan, fileInfo.fileNo, &stat );

                if ( result != CARD_RESULT_READY )
                {
                    CARDClose( &fileInfo );
                    return result;
                }

                offset += stat.commentAddr + ( sizeof( stat.fileName ) * 2 );
            }

            // Since we can only read in and at increments of CARD_READ_SIZE do this to calculate the region we require
            adjustedOffset = ( offset / CARD_READ_SIZE ) * CARD_READ_SIZE;
            adjustedLength = ( 1 + ( ( offset - adjustedOffset + length - 1 ) / CARD_READ_SIZE ) ) * CARD_READ_SIZE;

            // Buffer might not be adjusted to the new length so create a temporary data buffer
            // Allocate the memory to the back of the heap to avoid possible fragmentation
            // Buffers that CARDRead uses must be aligned to 0x20 bytes
            data = new ( -0x20 ) uint8_t[adjustedLength];

            result = CARDRead( &fileInfo, data, adjustedLength, adjustedOffset );
            if ( result == CARD_RESULT_READY )
            {
                // Copy data to the user's buffer
                memcpy( buffer, data + ( offset - adjustedOffset ), length );
            }

            delete[] data;
            CARDClose( &fileInfo );
        }
        // CARDOpen

        return result;
    }

    int32_t ReadGCI( int32_t chan, const char* fileName, int32_t length, int32_t offset, void* buffer, bool startAfterComments )
    {
        using namespace libtp::gc_wii::card;
        int32_t result;

        // Mount the memory card
        result = mountMemoryCard( chan );
        if ( result == CARD_RESULT_READY )
        {
            result = ReadGCIMounted( chan, fileName, length, offset, buffer, startAfterComments );
            CARDUnmount( chan );
        }
        return result;
    }
#else
    int32_t ReadNAND( const char* fileName, int32_t length, int32_t offset, void* buffer )
    {
        using namespace libtp::gc_wii::nand;

        NANDFileInfo fileInfo;
        int32_t result;

        int32_t adjustedOffset;
        int32_t adjustedLength;
        uint8_t* data;

        // Read data
        result = NANDOpen( fileName, &fileInfo, NAND_OPEN_READ );
        if ( result == NAND_RESULT_READY )
        {
            result = NANDSeek( &fileInfo, adjustedOffset, NAND_SEEK_START );
            if ( result == NAND_RESULT_READY )
            {
                // Since we can only read in and at increments of NAND_READ_SIZE do this to calculate the region we require
                adjustedOffset = ( offset / NAND_READ_SIZE ) * NAND_READ_SIZE;
                adjustedLength = ( 1 + ( ( offset - adjustedOffset + length - 1 ) / NAND_READ_SIZE ) ) * NAND_READ_SIZE;

                // Buffer might not be adjusted to the new length so create a temporary data buffer
                // Allocate the memory to the back of the heap to avoid possible fragmentation
                // Buffers that NANDRead uses must be aligned to 0x20 bytes
                data = new ( -0x20 ) uint8_t[adjustedLength];

                int32_t r = NANDRead( &fileInfo, data, adjustedLength );
                result = ( r > 0 ) ? NAND_RESULT_READY : r;

                if ( result == NAND_RESULT_READY )
                {
                    // Copy data to the user's buffer
                    memcpy( buffer, data + ( offset - adjustedOffset ), length );
                }

                delete[] data;
            }
            NANDClose( &fileInfo );
        }
        // NANDOpen

        return result;
    }
#endif
    int32_t ReadFile( const char* file, int32_t length, int32_t offset, void* buffer )
    {
        using namespace libtp::gc_wii::dvd;

        DVDFileInfo fileInfo;
        int32_t result;

        int32_t adjustedOffset;
        int32_t adjustedLength;
        uint8_t* data;

        // Read data
        if ( !DVDOpen( file, &fileInfo ) )
        {
            return DVD_STATE_FATAL_ERROR;
        }

        // We can only read in multiples of DVD_READ_SIZE and at offsets in multiples of DVD_OFFSET_SIZE
        adjustedOffset = ( offset / DVD_OFFSET_SIZE ) * DVD_OFFSET_SIZE;
        adjustedLength = ( 1 + ( ( offset - adjustedOffset + length - 1 ) / DVD_READ_SIZE ) ) * DVD_READ_SIZE;

        // Buffer might not be adjusted to the new length so create a temporary data buffer
        // Allocate the memory to the back of the heap to avoid possible fragmentation
        // Buffers that DVDReadPrio uses must be aligned to 0x20 bytes
        data = new ( -0x20 ) uint8_t[adjustedLength];

        int32_t r = DVDReadPrio( &fileInfo, data, adjustedLength, adjustedOffset, 2 );
        result = ( r > 0 ) ? DVD_STATE_END : r;
        if ( result == DVD_STATE_END )
        {
            // Copy data to the user's buffer
            memcpy( buffer, data + ( offset - adjustedOffset ), length );
        }

        delete[] data;
        DVDClose( &fileInfo );

        return result;
    }
#ifdef DVD
    bool callRelProlog( const char* file )
    {
        using namespace libtp::gc_wii::dvd;
        using namespace libtp::gc_wii::os_module;

        // Try to open the file from the disc
        DVDFileInfo fileInfo;
        if ( !DVDOpen( file, &fileInfo ) )
        {
            return false;
        }

        // Get the length of the file
        uint32_t length = fileInfo.length;

        // Round the length to be in multiples of DVD_READ_SIZE
        length = ( length + DVD_READ_SIZE - 1 ) & ~( DVD_READ_SIZE - 1 );

        // Allocate bytes for the file
        // Allocate the memory to the back of the heap to avoid possible fragmentation
        // Buffers that DVDReadPrio uses must be aligned to 0x20 bytes
        uint8_t* fileData = new ( -0x20 ) uint8_t[length];
        libtp::memory::clear_DC_IC_Cache( fileData, length );

        // Read the REL from the disc
        int32_t r = DVDReadPrio( &fileInfo, fileData, length, 0, 2 );
        int32_t result = ( r > 0 ) ? DVD_STATE_END : r;

        // Close the file, as it's no longer needed
        DVDClose( &fileInfo );

        // Make sure the read was successful
        if ( result != DVD_STATE_END )
        {
            delete[] fileData;
            return false;
        }

        // Get the REL's BSS size and allocate memory for it
        OSModuleInfo* relFile = reinterpret_cast<OSModuleInfo*>( fileData );
        uint32_t bssSize = relFile->bssSize;

        // If bssSize is 0, then use an arbitrary size
        if ( bssSize == 0 )
        {
            bssSize = 0x1;
        }

        // Allocate the memory to the back of the heap to avoid fragmentation
        uint8_t* bssArea = new ( -( relFile->bssAlignment ) ) uint8_t[bssSize];

        // Disable interrupts to make sure other REL files do not try to be linked while this one is being linked
        bool enable = libtp::gc_wii::os_interrupt::OSDisableInterrupts();

        // Link the REL file
        if ( !OSLink( relFile, bssArea ) )
        {
            // Try to unlink to be safe
            OSUnlink( relFile );

            // Restore interrupts
            libtp::gc_wii::os_interrupt::OSRestoreInterrupts( enable );

            delete[] bssArea;
            delete[] relFile;
            return false;
        }

        // Restore interrupts
        libtp::gc_wii::os_interrupt::OSRestoreInterrupts( enable );

        // Call the REL's prolog functon
        reinterpret_cast<void ( * )()>( relFile->prologFuncOffset )();

        // We are done with the REL file, so call it's epilog function to perform any necessary exit code
        reinterpret_cast<void ( * )()>( relFile->epilogFuncOffset )();

        // Disable interrupts to make sure other REL files do not try to be linked while this one is being unlinked
        enable = libtp::gc_wii::os_interrupt::OSDisableInterrupts();

        // All REL functions are done, so the file can be unlinked
        OSUnlink( relFile );

        // Restore interrupts
        libtp::gc_wii::os_interrupt::OSRestoreInterrupts( enable );

        // Clear the cache of the memory used by the REL file since assembly ran from it
        libtp::memory::clear_DC_IC_Cache( relFile, length );

        // Cleanup
        delete[] bssArea;
        delete[] relFile;

        return true;
    }
#else
    bool callRelProlog( int32_t chan, uint32_t rel_id, bool isMounted, bool stayMounted )
    {
        using namespace libtp::gc_wii::card;
        using namespace libtp::gc_wii::os_module;

        int32_t result;

        // Mount the memory card if necessary
        if ( !isMounted )
        {
            if ( CARD_RESULT_READY != mountMemoryCard( chan ) )
            {
                return false;
            }
        }

        // All of the RELs should be in the main save file, which always uses an internal name of "Custom REL File"
        CARDFileInfo fileInfo;
        result = CARDOpen( chan, "Custom REL File", &fileInfo );
        if ( result != CARD_RESULT_READY )
        {
            if ( !stayMounted )
            {
                CARDUnmount( chan );
            }

            return false;
        }

        // Allocate bytes to hold the area of the file that contains the size
        // Allocate the memory to the back of the heap to avoid possible fragmentation
        // Buffers that CARDRead uses must be aligned to 0x20 bytes
        uint8_t* fileData = new ( -0x20 ) uint8_t[CARD_READ_SIZE];

        // Get the data from the area that holds the size
        result = CARDRead( &fileInfo, fileData, CARD_READ_SIZE, 0x2000 );
        if ( result != CARD_RESULT_READY )
        {
            delete[] fileData;
            CARDClose( &fileInfo );

            if ( !stayMounted )
            {
                CARDUnmount( chan );
            }

            return false;
        }

        // Loop through the REL entries until the desired one is found
        RelEntry* entry = reinterpret_cast<RelEntry*>( &fileData[0x44] );
        bool foundDesiredRel = false;

        for ( uint32_t i = 0; i < MAX_REL_ENTRIES; i++ )
        {
            uint32_t currentRelId = entry->rel_id;

            // If any of the fields are 0, then there are no more entries
            if ( ( currentRelId == 0 ) || ( entry->rel_size == 0 ) || ( entry->offset == 0 ) )
            {
                break;
            }

            if ( currentRelId == rel_id )
            {
                // Found the desired REL
                foundDesiredRel = true;
                break;
            }

            entry++;
        }

        if ( !foundDesiredRel )
        {
            delete[] fileData;
            CARDClose( &fileInfo );

            if ( !stayMounted )
            {
                CARDUnmount( chan );
            }

            return false;
        }

        // Get the variables from the entry so that fileData can be freed
        uint32_t fileSize = entry->rel_size;
        uint32_t fileOffset = entry->offset;
        delete[] fileData;

        // Since we can only read in and at increments of CARD_READ_SIZE do this to calculate the region we require
        int32_t adjustedOffset = ( fileOffset / CARD_READ_SIZE ) * CARD_READ_SIZE;
        int32_t adjustedLength = ( 1 + ( ( fileOffset - adjustedOffset + fileSize - 1 ) / CARD_READ_SIZE ) ) * CARD_READ_SIZE;

        // Buffer might not be adjusted to the new length so create a temporary data buffer
        // Allocate the memory to the back of the heap to avoid fragmentation
        // Buffers that CARDRead uses must be aligned to 0x20 bytes, and REL files must also be aligned to 0x20 bytes
        fileData = new ( -0x20 ) uint8_t[adjustedLength];
        libtp::memory::clear_DC_IC_Cache( fileData, adjustedLength );

        // Read the REL file from the memory card
        result = CARDRead( &fileInfo, fileData, adjustedLength, adjustedOffset );

        // Close the file, as it's no longer needed
        CARDClose( &fileInfo );

        // Unmount the memory card if necessary, as it's no longer needed
        if ( !stayMounted )
        {
            CARDUnmount( chan );
        }

        if ( result != CARD_RESULT_READY )
        {
            delete[] fileData;
            return false;
        }

        // Move the data so that the start of the rel file is at the start of the buffer
        memmove( fileData, fileData + ( fileOffset - adjustedOffset ), fileSize );

        // Failsafe: Be 100% sure the REL file loaded is the correct one
        OSModuleInfo* relFile = reinterpret_cast<OSModuleInfo*>( fileData );
        if ( relFile->id != rel_id )
        {
            delete[] relFile;
            return false;
        }

        // Get the REL's BSS size and allocate memory for it
        uint32_t bssSize = relFile->bssSize;

        // If bssSize is 0, then use an arbitrary size
        if ( bssSize == 0 )
        {
            bssSize = 0x1;
        }

        // Allocate the memory to the back of the heap to avoid fragmentation
        uint8_t* bssArea = new ( -( relFile->bssAlignment ) ) uint8_t[bssSize];

        // Disable interrupts to make sure other REL files do not try to be linked while this one is being linked
        bool enable = libtp::gc_wii::os_interrupt::OSDisableInterrupts();

        // Link the REL file
        if ( !OSLink( relFile, bssArea ) )
        {
            // Try to unlink to be safe
            OSUnlink( relFile );

            // Restore interrupts
            libtp::gc_wii::os_interrupt::OSRestoreInterrupts( enable );

            delete[] bssArea;
            delete[] relFile;
            return false;
        }

        // Restore interrupts
        libtp::gc_wii::os_interrupt::OSRestoreInterrupts( enable );

        // Call the REL's prolog functon
        reinterpret_cast<void ( * )()>( relFile->prologFuncOffset )();

        // We are done with the REL file, so call it's epilog function to perform any necessary exit code
        reinterpret_cast<void ( * )()>( relFile->epilogFuncOffset )();

        // Disable interrupts to make sure other REL files do not try to be linked while this one is being unlinked
        enable = libtp::gc_wii::os_interrupt::OSDisableInterrupts();

        // All REL functions are done, so the file can be unlinked
        OSUnlink( relFile );

        // Restore interrupts
        libtp::gc_wii::os_interrupt::OSRestoreInterrupts( enable );

        // Clear the cache of the memory used by the REL file since assembly ran from it
        libtp::memory::clear_DC_IC_Cache( relFile, fileSize );

        // Cleanup
        delete[] bssArea;
        delete[] relFile;

        return true;
    }
#endif
    uint32_t getRandom( uint64_t* seed, uint32_t max )
    {
        uint64_t z = ( *seed += 0x9e3779b97f4a7c15 );
        z = ( z ^ ( z >> 30 ) ) * 0xbf58476d1ce4e5b9;
        z = ( z ^ ( z >> 27 ) ) * 0x94d049bb133111eb;

        return ( z % max );
    }

    int32_t getStageIndex( const char* stage )
    {
        // Find the index of this stage
        for ( uint32_t stageIDX = 0; stageIDX < sizeof( data::stage::allStages ) / sizeof( data::stage::allStages[0] );
              stageIDX++ )
        {
            if ( strcmp( stage, libtp::data::stage::allStages[stageIDX] ) == 0 )
            {
                return static_cast<int32_t>( stageIDX );
            }
        }

        // Did not find a valid stage
        return -1;
    }

    bool playerIsInRoomStage( int32_t room, const char* stage )
    {
        // Only check room if it is valid
        if ( ( room >= 0 ) && ( room <= 0xFF ) )
        {
            if ( static_cast<uint32_t>( room ) != libtp::tp::d_kankyo::env_light.currentRoom )
            {
                return false;
            }
        }

        // Only check stage if it is valid
        if ( stage )
        {
            if ( !libtp::tp::d_a_alink::checkStageName( stage ) )
            {
                return false;
            }
        }

        return true;
    }
}     // namespace libtp::tools