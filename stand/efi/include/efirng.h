/* $FreeBSD$ */

#ifndef _EFIRNG_H
#define _EFIRNG_H

#define EFI_RNG_PROTOCOL_GUID \
    { 0x3152bca5, 0xeade, 0x433d, {0x86, 0x2e, 0xc0, 0x1c, 0xdc, 0x29, 0x1f, 0x44} }

INTERFACE_DECL(_EFI_RNG_PROTOCOL);

typedef EFI_GUID EFI_RNG_ALGORITHM;

typedef
EFI_STATUS
(EFIAPI *EFI_RNG_GET_INFO) (
    IN struct _EFI_RNG_PROTOCOL		*This,
    IN  OUT UINTN			*RNGAlgorithmListSize,
    OUT EFI_RNG_ALGORITHM		*RNGAlgorithmList
    );

typedef
EFI_STATUS
(EFIAPI *EFI_RNG_GET_RNG) (
    IN struct _EFI_RNG_PROTOCOL		*This,
    IN EFI_RNG_ALGORITHM		*RNGAlgorithm, OPTIONAL
    IN UINTN				RNGValueLength,
    OUT UINT8				*RNGValue
    );

typedef struct _EFI_RNG_PROTOCOL {
	EFI_RNG_GET_INFO	GetInfo;
	EFI_RNG_GET_RNG		GetRNG;
} EFI_RNG_PROTOCOL;

static EFI_GUID			rng_guid = EFI_RNG_PROTOCOL_GUID;

#endif /* _EFIRNG_H */
