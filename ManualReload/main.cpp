#include "stdafx.h"
#include "Utility/UIText.h"

using namespace Utility;

UIText g_reloadText( "Reload", 640.0f, 674.0f, 0.59f, FontCCCLocalized, AlignCenter, 255, 255, 255, 255, true );

static CConfig g_configFile( "ManualReload.ini" );

bytepatch_t setWeaponFlagsPatch;

uintptr_t g_pedWeaponMgrOffset,
          g_aimTaskWeaponFlagsOffset,
          g_weaponClipComponentOffset,
          g_weaponInfoTimeBetweenShotsOffset;

typedef CWeapon* ( *CPedWeaponMgr__GetCurrentWeapon )( LPVOID );
CPedWeaponMgr__GetCurrentWeapon g_getCurrentWeapon;

Ped g_playerPed;

struct CScriptConfig {
    bool DiscardClipAmmo;
    bool UseReloadText;
    bool UseReloadHelpText;
    bool UseEmptyChamberSound;
    char AltReloadText[256];
} g_userConfig;

static CallHook<void( * )( CWeapon*, char )> * g_set_ammo_in_clip_orig_hook;

static CallHook<bool( * )( CWeapon* )> * g_reload_check_orig_hook;

static JmpHook<int( * )( uint64_t )> * g_task_on_swap_weap_hook;

static CallHook<bool( * )( uint64_t )> * g_task_weapon_flags_orig_hook;

void readUserConfig() {

    g_userConfig.DiscardClipAmmo = g_configFile.get( "General", "DiscardClipAmmo", true );

    g_userConfig.UseReloadText = g_configFile.get( "General", "UseReloadText", true );

    g_userConfig.UseReloadHelpText = g_configFile.get( "General", "UseReloadHelpText", false );

    g_userConfig.UseEmptyChamberSound = g_configFile.get( "General", "UseEmptyChamberSound", true );

    g_configFile.getText( g_userConfig.AltReloadText, "General", "AltReloadText" );
}

CWeapon * getCurrentPedWeapon( const Ped ped ) {

    const auto address = getScriptHandleBaseAddress( ped );

    const auto weaponMgr = *reinterpret_cast<void**>(
                               reinterpret_cast<uintptr_t>( address ) + g_pedWeaponMgrOffset );

    return g_getCurrentWeapon( weaponMgr );
}

void SetAmmoInClip_Hook( CWeapon* weapon, char bAltAmmmo ) {

    if ( weapon->pInventoryPed->GetOwner() == getScriptHandleBaseAddress( g_playerPed ) &&
            g_userConfig.DiscardClipAmmo ) {
        weapon->totalAmmo -= weapon->info->clipSize;

        weapon->totalAmmo += weapon->info->clipSize - weapon->ammoInClip;

        weapon->pInventoryPed->SetWeaponAmmo( weapon->info->dwNameHash, weapon->totalAmmo );
    }

    g_set_ammo_in_clip_orig_hook->fn( weapon, bAltAmmmo );
}

bool bReloadingWeapon = false;

bool CWeapon_CanBeReloaded_Hook( CWeapon * weapon ) {

    if ( weapon->ammoInClip > 0 || weapon->pInventoryPed &&
         weapon->pInventoryPed->GetOwner() != getScriptHandleBaseAddress( g_playerPed ) )
        return g_reload_check_orig_hook->fn( weapon );

    return bReloadingWeapon;
}

int CTaskSwapWeapon_UpdateOnSwap_Hook( uint64_t ctask ) {

    const auto ped = *reinterpret_cast<void**>( ctask + 0x10 );

    if ( ped == getScriptHandleBaseAddress( g_playerPed ) ) {
        const auto weaponMgr = *reinterpret_cast<LPVOID*>(
                                   reinterpret_cast<uintptr_t>( ped ) + g_pedWeaponMgrOffset );

        const auto weapon = g_getCurrentWeapon( weaponMgr );

        if ( weapon && weapon->info->dwGroupHash == 2725924767u ) {
            *( BYTE* )( ctask + 0xEC ) |= 4;
        }
    }

    return g_task_on_swap_weap_hook->fn( ctask );
}

bool CTaskAimGunOnFoot_UpdateWeaponFlags_Hook( uint64_t ctask ) {

    if ( *reinterpret_cast<LPVOID*>( ctask + 0x10 ) != getScriptHandleBaseAddress( g_playerPed ) )
        return g_task_weapon_flags_orig_hook->fn( ctask );

    if ( bReloadingWeapon ) {
        *( BYTE* )( ctask + g_aimTaskWeaponFlagsOffset ) |= 0x20;
        return true;
    }

    return false;
}

bool bInitialized = false;

bool initialize( const eGameVersion version ) {

    g_aimTaskWeaponFlagsOffset = version > VER_1_0_877_1_NOSTEAM ? 0x12C : 0x11C;

    g_weaponInfoTimeBetweenShotsOffset = version > VER_1_0_1032_1_NoSteam ? 0x134 : 0x11C;

    #pragma region CanBeReloaded Hook (CTaskGun::UpdateBasic)

    //B8 ? ? ? ? 44 3B E8 74 1C
    auto pattern = BytePattern( ( BYTE* )"\xB8\x04\x00\x00\x00\x44\x3B\xE8\x74\x1C", "x????xxxxx" ); //BB 04 00 00 00 44 3B ? 74 17 84

    if ( !pattern.bSuccess ) {
        LOG( "Failed to find address #1. Exiting..." );
        return false;
    }

    g_reload_check_orig_hook = HookManager::SetCall( pattern.get( 44 ), CWeapon_CanBeReloaded_Hook );

    #pragma endregion

    #pragma region CanBeReloaded Hook (CTaskInCover::ShouldReloadWeapon)

    pattern = BytePattern( ( BYTE* )"\x75\x12\x80\x78\x36\x09", "xxxxxx" );

    if ( !pattern.bSuccess ) {
        LOG( "Failed to find address #2. Exiting..." );
        return false;
    }

    g_reload_check_orig_hook->add( pattern.get( 43 ) );

    #pragma endregion

    #pragma region OnSwap Hook (CTaskSwapWeapon::UpdateOnSwap)

    pattern = BytePattern( ( BYTE* )"\x48\x83\xC4\x20\x5B\xE9\x00\x00\x00\x00\x83\xFA\x06\x75\x07", "xxxxxx????xxxxx" );

    if ( !pattern.bSuccess ) {
        LOG( "Failed to find address #3. Exiting..." );
        return false;
    }

    //	g_task_on_swap_weap_hook = HookManager::SetJmp((PBYTE)pattern.get(5), CTaskSwapWeapon_UpdateOnSwap_Hook);

    #pragma endregion

    #pragma region WeaponOkCheck

    pattern = BytePattern( ( BYTE* )"\xE8\x00\x00\x00\x00\x8D\x43\xFF\x83\xF8\x03", "x????xxxxxx" );

    if ( !pattern.bSuccess ) {
        LOG( "Failed to find address #4. Exiting..." );
        return false;
    }

    auto address = pattern.get( 1 );

    // nested function CTaskAimGunOnFoot::UpdateWeaponFlags+10
    address = *( int32_t* )address + address + 4 + 10;

    // nested function CTaskAimGunOnFoot::IsWeaponReady+66
    address = *( int32_t* )address + address + 4 + 66;

    g_pedWeaponMgrOffset = *( int32_t* )address;

    address += 5;

    address = *( int32_t* )address + address + 4;

    g_getCurrentWeapon = ( CPedWeaponMgr__GetCurrentWeapon )address;

    g_task_weapon_flags_orig_hook = HookManager::SetCall( pattern.get(), CTaskAimGunOnFoot_UpdateWeaponFlags_Hook );

    #pragma endregion

    #pragma region SetWeaponFlagsPatch (CTaskAimGunOnFoot)

    pattern = BytePattern( ( BYTE* )"\x8B\x4D\x00\x8D\x41\xFE\x41\x3B", "xx?xxxxx" );

    if ( !pattern.bSuccess ) {
        LOG( "Failed to find address #5. Exiting..." );
        return false;
    }

    // don't disable aiming for an invalid total ammo count..
    setWeaponFlagsPatch = bytepatch_t( pattern.get( version > VER_1_0_463_1_NOSTEAM ? 91u : 67u ), std::vector<BYTE>( 8, NOP ) );

    setWeaponFlagsPatch.install();

    #pragma endregion

    if ( version > VER_1_0_463_1_NOSTEAM ) {
        #pragma region SetAmmoInClip (CTaskReloadWeapon)

        //48 8D 15 ? ? ? ? 48 8B CF E8 ? ? ? ? 84 C0 74 14 80
        pattern = BytePattern( ( BYTE* )"\x48\x8D\x15\x00\x00\x00\x00\x48\x8B\xCF\xE8\x00\x00\x00\x00\x84\xC0\x74\x14\x80", "xxx????xxxx????xxxxx" );
        if ( !pattern.bSuccess ) {
            LOG( "Failed to find address #6. Exiting..." );
            return false;
        }

        address = pattern.get( -11 );

        address = *( int32_t* )address + address + 168;

        auto address2 = address + 1;

        address2 = *( int32_t* )address2 + address2 + 4;

        g_set_ammo_in_clip_orig_hook = HookManager::SetCall( address, SetAmmoInClip_Hook );

        g_weaponClipComponentOffset = *( int32_t* )( address2 + 0x2B );

        #pragma endregion

        #pragma region g_persistWeaponAmmoStats toggle

        pattern = BytePattern( ( BYTE* )"\x44\x38\x2D\x00\x00\x00\x00\x74\x1B\x48\x8B\x47\x20", "xxx????xxxxxx" );

        if ( !pattern.bSuccess ) {
            LOG( "Failed to find address #7. Exiting..." );
            return false;
        }

        address = pattern.get( 3 );

        address = *( int32_t* )address + address + 4;

        *( BOOL* )address = 1; // always use ammo inventory


        #pragma endregion
    }

    return bInitialized = true;
}

int getLanguageTextId() {

    auto uiLanguage = UNK::_GET_UI_LANGUAGE_ID();

    if ( uiLanguage == 11 )
        uiLanguage = 4;

    return uiLanguage;
}

void main() {

    const auto gameVersion = getGameVersion();

    if ( gameVersion == VER_UNK ) return;

    readUserConfig();

    if ( bInitialized || initialize( gameVersion ) ) {

        auto keyStr = std::string( CONTROLS::GET_CONTROL_INSTRUCTIONAL_BUTTON( 0, ControlReload, TRUE ) );

        auto it = keyStr.find_last_of( "_" );

        if ( it != std::string::npos )
            it += 1;

        keyStr = keyStr.substr( it );

        const auto reloadText = g_userConfig.AltReloadText[0] == '\0' ?
                                getConstString( getLanguageTextId(), RELOAD_TEXT ) : g_userConfig.AltReloadText;

        g_reloadText.SetText( FormatString( "%s [%s]", reloadText, keyStr.c_str() ) );

        run();
    }
}

int weaponSoundPlayTime = 0;

bool weaponSoundTimerEnabled = false;

inline void playEmptyChamberSound( const int waitTime = 0 ) {

    weaponSoundPlayTime = GAMEPLAY::GET_GAME_TIMER() + waitTime;

    weaponSoundTimerEnabled = true;
}

inline void updateWeaponSounds( const Weapon weap ) {

    if ( weaponSoundTimerEnabled ) {

        if ( GAMEPLAY::GET_GAME_TIMER() < weaponSoundPlayTime )
            return;
        // for delayed trigger sounds, make sure we are still firing...
        if ( CONTROLS::IS_DISABLED_CONTROL_PRESSED( 0, ControlAttack ) ) {

            auto src = WEAPON::GET_WEAPONTYPE_GROUP( weap ) == 970310034 ?
                       IDR_DRYFIRE_RIFLE : IDR_DRYFIRE_SMG;

            PlaySoundResource( src, TRUE );
        }

        weaponSoundTimerEnabled = false;
    }
}

int lastAmmoInClip = 0;

void run() {
    while ( true ) {

        g_playerPed = PLAYER::PLAYER_PED_ID();

        Hash currentWeapon;

        if ( WEAPON::IS_PED_ARMED( g_playerPed, 4 ) ) {
            if ( WEAPON::GET_CURRENT_PED_WEAPON( g_playerPed, &currentWeapon, TRUE ) ) {

                updateWeaponSounds( currentWeapon );

                int ammoInClip;

                WEAPON::GET_AMMO_IN_CLIP( g_playerPed, currentWeapon, &ammoInClip );

                const int totalAmmo = WEAPON::GET_AMMO_IN_PED_WEAPON( g_playerPed, currentWeapon );

                if ( ammoInClip <= 0 && !PED::IS_PED_RELOADING( g_playerPed ) ) {

                    CONTROLS::DISABLE_CONTROL_ACTION( 0, ControlAttack, TRUE );
                    CONTROLS::DISABLE_CONTROL_ACTION( 0, ControlAttack2, TRUE );
                    // firing with empty clip
                    if ( CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED( 0, ControlAttack ) ) {

                        playEmptyChamberSound();
                    }
                    // clip just emptied
                    else if ( lastAmmoInClip == 1 ) {

                        if ( CWeapon* weapon = getCurrentPedWeapon( g_playerPed ) ) {

                            const float fTimeBetweenShots = *( float* )( ( uintptr_t )weapon->info + g_weaponInfoTimeBetweenShotsOffset );

                            playEmptyChamberSound( ( int )( fTimeBetweenShots * 1000.0f ) );
                        }
                    }

                    // Only show the prompt if we actually have ammo to load
                    if ( totalAmmo > 0 ) {
                        if ( g_userConfig.UseReloadHelpText ) {
                            UI::BEGIN_TEXT_COMMAND_DISPLAY_HELP( "STRING" );
                            UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME( "Press ~INPUT_RELOAD~ to reload" );
                            UI::END_TEXT_COMMAND_DISPLAY_HELP( 0, 0, 1, -1 );
                        }

                        else if ( g_userConfig.UseReloadText ) {
                            g_reloadText.Draw();
                        }
                    }
                }

                lastAmmoInClip = ammoInClip;

                bReloadingWeapon = CONTROLS::IS_CONTROL_PRESSED( 0, ControlReload ) && totalAmmo - ammoInClip > 0 &&
                                   ammoInClip < WEAPON::GET_MAX_AMMO_IN_CLIP( g_playerPed, currentWeapon, 1 );
            }

            else {
                // fixes problem where weapons can sometimes fire immediatley after switching.
                CONTROLS::DISABLE_CONTROL_ACTION( 0, ControlAttack, TRUE );
                CONTROLS::DISABLE_CONTROL_ACTION( 0, ControlAttack2, TRUE );
            }
        }

        WAIT( 0 );
    }
}

void unload() {
    if ( setWeaponFlagsPatch.active )
        setWeaponFlagsPatch.remove();

    HookManager::Uninititialize();
}
