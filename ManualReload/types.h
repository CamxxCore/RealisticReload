#pragma once

typedef struct color_s {
    uint8_t r, g, b, a;
} color_t;

struct CVector {
    float x, y, z, w;
};

class CPedInventory {
  public:
    virtual ~CPedInventory() = 0;
    virtual void SetWeaponAmmo( unsigned int weaponHash, unsigned int ammoCount ) = 0;
    virtual void SetWeaponFiredTime( unsigned int weaponHash, int time ) = 0;
    virtual void* GetOwner() = 0;
};

class CWeaponComponentClipInfo {
    char pad0[0x38];
  public:
    virtual ~CWeaponComponentClipInfo() = 0;
    int clipSize; //0x40-0x44
};

class CWeaponComponentClip {
  public:
    virtual ~CWeaponComponentClip() = 0;
    CWeaponComponentClipInfo * info;
};

struct CWheelWeapon {
    unsigned int weaponHash;
    unsigned int nameHash;
    int suffixHash; //0x8-0xC
    int ammoNotInClip; //0xC-0x10
    int ammoInClip;
    int totalAmmo;
    unsigned __int8 bEquiped;
    unsigned __int8 bUnk;
    unsigned __int16 unkShort1;
};

enum eWeaponAmmoState : char {
    WAS_READY = 0,
    WAS_FIRING = 1,
    WAS_RELOADING = 2,
    WAS_OUT_OF_AMMO = 3
};

struct CAmmoInfo {
    char _0x0000[16];
    DWORD dwAmmoNameHash; //0x0010
    DWORD dwAmmoModelHash; //0x0014
    DWORD dwAmmoAudio; //0x0018
    DWORD dwAmmoSlot; //0x001C
    __int32 iAmmoMax; //0x0020
    __int32 iAmmoMax50; //0x0024
    __int32 iAmmoMax100; //0x0028
    __int32 iAmmoMaxMP; //0x002C
    __int32 iAmmoMax50MP; //0x0030
    __int32 iAmmoMax100MP; //0x0034
    BYTE bAmmoFlags; //0x0038
    char _0x0039[7];
    void * projectileInfo; //0x0040

};//Size=0x0158

struct CAimingInfo {
    DWORD dwNameHash; //0x0000
    float fHeadingLimit; //0x0004
    float fSweepPitchMin; //0x0008
    float fSweepPitchMax; //0x000C
};

struct CBoneForce {
    DWORD dwBoneTag; //0x0000
    float fForceFront; //0x0004
    float fForceBack; //0x0008
};//Size=0x000C

struct CBoneForces {
    CBoneForce arrBones[6]; //0x0000
};//Size=0x0048

struct CWeaponInfo {
    void ** virtualTable; //0x0-0x8;
    void * extensions; //0x8-0x10
    DWORD dwNameHash;//0x10-0x14
    DWORD dwModelHash; //0x14-0x18
    DWORD dwAudioHash;
    DWORD dwWeaponSlotHash; //0x1C-0x20
    int damageType; //0x20-0x24
    DWORD dwExplosionDefault; //0x24-0x28
    DWORD dwExplosionHitCar; //0x28-0x2C
    DWORD dwExplosionHitTruck; //0x2C-0x30
    DWORD dwExplosionHitBike; //0x30-0x34
    DWORD dwExplosionHitBoat; //0x34-0x38
    DWORD dwExplosionHitPlane; //0x38-0x3C
    int iWeaponFireType; //0x3C-0x40
    int iWeaponWheelSlot; //0x40-0x44
    DWORD dwGroupHash;//0x44-0x48
    CAmmoInfo * pAmmoInfo; //0x48-0x50
    CAimingInfo * pAimingInfo; //0x50-0x58
    int clipSize; //0x58-0x5C
    float fAccuracySpread; //0x5C-0x60
    float fAccurateModeAccuracyModifier; //0x60-0x64
    float fRunAndGunAccuracyModifier; //0x64-0x68
    float fRunAndGunAccuracyMaxModifier; //0x68-0x6C
    float fRecoilAccuracyMax; //0x6C-0x70
    float fRecoilErrorTime; //0x70-0x74
    float fRecoilRecoveryRate; //0x74-0x78
    float fRecoilAccuracyToAllowHeadshotAI; //0x78-0x7C
    float fMinHeadshotDistanceAI; //0x7C-0x80
    float fMaxHeadshotDistanceAI; //0x80-0x84
    float fHeadshotDamageModifierAI; //0x84-0x88
    float fRecoilAccuracyToAllowHeadshotPlayer; //0x88-0x8C
    float fMinHeadshotDistancePlayer; //0x8C-0x90
    float fMaxHeadshotDistancePlayer; //0x90-0x94
    float fHeadShotDamageModifierPlayer; //0x0094
    float fDamage; //0x0098
    float fDamageTime; //0x009C
    float fDamageTimeInVehicle; //0x00A0
    float fDamageTimeInVehicleHeadShot; //0x00A4
    float fHitLimbsDamageModifier; //0x00A8
    float fNetworkHitLimbsDamageModifier; //0x00AC
    float fLightlyArmouredDamageModifier; //0x00B0
    float fForce; //0x00B4
    float fForceUnk; //0x00B8
    float fForceHitPed; //0x00BC
    float fForceHitVehicle; //0x00C0
    float fForceHitFlyingHeli; //0x00C4
    CBoneForces * pBoneForces; //0xC8-0xD0
    __int16 iBoneForceFrontCount; //0x00D0
    __int16 iBoneForceBackCount; //0x00D2
    char _0x00D4[4];
    float fForceMaxStrengthMult; //0x00D8
    float fForceFalloffRangeStart; //0x00DC
    float fForceFalloffRangeEnd; //0x00E0
    float fForceFalloffMin; //0x00E4
    float fProjectileForce; //0x00E8
    float fFragImpulse; //0x00EC
    float fPenetration; //0x00F0
    float fVerticalLaunchAdjustment; //0x00F4
    float fDropForwardVelocity; //0x00F8
    float fSpeed; //0x00FC
    __int32 iBulletsInBatch; //0x0100
    float fBatchSpread; //0x0104
    float fReloadTimeMP; //0x0108
    float fReloadTimeSP; //0x010C
    float fVehicleReloadTime; //0x0110
    float fAnimReloadRate; //0x0114
    __int32 iBulletsPerAnimLoop; //0x0118
    float fTimeBetweenShots; //0x011C
    float fTimeLeftBetweenShotsWhereShouldFireIsCached; //0x0120
    float fSpinUpTime; //0x0124
    float fSpinTime; //0x0128
    float fSpinDownTime; //0x012C
    float fAlternateWaitTime; //0x0130
    float fBulletBendingNearRadius; //0x0134
    float fBulletBendingFarRadius; //0x0138
    float fBulletBendingZoomedRadius; //0x013C
    float fFirstPersonBulletBendingNearRadius; //0x0140
    float fFirstPersonBulletBendingFarRadius; //0x0144
    float fFirstPersonBulletBendingZoomedRadius; //0x0148
    char _0x014C[4];
    __int32 fFxEffectGroup; //0x0150 (GUESSING - Need to map groups out)
    DWORD dwFxFlashFxHash; //0x0154
    DWORD dwFxFlashFxAltHash; //0x0158
    DWORD dwFxFlashFxFPHash; //0x015C
    DWORD dwFxFlashFxAltFPHash; //0x0160
    DWORD dwMuzzleSmokeFxHash; //0x0164
    DWORD dwMuzzleSmokeFxFPHash; //0x0168
    float fFxMuzzleSmokeFxMinLevel; //0x016C
    float fFxMuzzleSmokeFxIncPerShot; //0x0170
    float fFxMuzzleSmokeFxDecPerSec; //0x0174
    char pad2[0x80];
    BYTE boolFxGroundDisturbFxEnabled; //0x1F8-0x1F9
    char _0x01E9[3]; //0x1F9-0x1FC
    float fFxGroundDisturbFxDist; //0x1FC-0x200
    DWORD dwFxGroundDisturbFxNameDefaultHash; //0x200-0x204
    DWORD dwFxGroundDisturbFxNameSandHash; //0x204-0x208
    DWORD dwFxGroundDisturbFxNameDirtHash; //0x208-0x20C
    DWORD dwFxGroundDisturbFxNameWaterHash; //0x20C-0x210
    char _0x0200[16]; //0x210-0x220
    __int32 iInitialRumbleDuration; //0x220-0x224
    float fInitialRumbleIntensity; //0x224-0x228
    float fInitialRumbleIntensityTrigger; //0x228-0x22C
    __int32 iRumbleDuration; //0x22C-0x230
    float fRumbleIntensity; //0x230-0x234
    float fRumbleIntensityTrigger; //0x234-0x238
    float fRumbleDamageIntensity; //0x238-0x23C
    __int32 iInitialRumbleDurationFps; //0x23C-0x240
    float fInitialRumbleIntensityFps; //0x240-0x244
    __int32 iRumbleDurationFps; //0x244-0x248
    float fRumbleIntensityFps; //0x248-0x24C
    float fNetworkPlayerDamageModifier; //0x24C-0x250
    float fNetworkPedDamageModifier; //0x250-0x254
    float fNetworkHeadShotPlayerDamageModifier;  //0x254-0x258
    float fLockOnRange; //0x258-0x25C
    float fMaxRange; //0x25C-0x260
    char pad6[0x5E0];
    void *pWeaponTints; //0x840
    char pad7[0x68];
};

struct CWeapon {
    void * virtualTable; //0x0-0x8
    void * extensions; //0x8-0x10
    CVector offset; //0x10-0x20
    CVector position; //0x20-0x30
    char pad0[0x10]; //0x30-0x40
    CWeaponInfo * info; //0x40-0x48
    int activeTime; //0x48-0x4C
    int lastFiredTime; //0x4C-0x50
    int activeTime2; //0x50-0x54
    uint16_t totalAmmo;
    uint16_t ammoInClip; //0x56-0x58
    __int64 pWeaponObject;
    CPedInventory* pInventoryPed; //0x60-=0x68
    CWeapon* pUnkWeapon; //self? //0x68-0x70
    char pad2[0xB0]; //0x70-0x120
    CWeaponComponentClip * clipComponent; //0x120-0x128
    char pad3[0x30];
    void * pUnk; //0x158-0x160
    char pad4[0x60];
    eWeaponAmmoState ammoState; //0x1C0
    DWORD weaponStateFlags; // 0x80 = silenced
};
