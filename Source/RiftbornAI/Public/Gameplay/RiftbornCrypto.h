// Copyright 2024-2026 RiftbornAI. All Rights Reserved.
// RiftbornCrypto.h - Centralized cryptographic utilities
// 
// CRITICAL: All proof-related hashing MUST use these functions.
// DO NOT use FSHA1 for anything labeled SHA256.
// This file is the SINGLE SOURCE OF TRUTH for proof crypto.
//
// PLATFORM SUPPORT:
//   Windows — Uses CNG (BCrypt)
//   Linux/Mac — Uses OpenSSL (via UE's bundled OpenSSL)

#pragma once

#include "CoreMinimal.h"

// Windows CNG for SHA-256
#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#include "Windows/HideWindowsPlatformTypes.h"
#else
// Linux/Mac: Use OpenSSL (bundled with UE)
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#endif

/**
 * Centralized crypto utilities for RiftbornAI.
 * 
 * IMPORTANT: All proof hashing, signature verification, and integrity
 * checks MUST use these functions to ensure:
 * 1. Consistent SHA256 (not SHA1) across the codebase
 * 2. Cross-language compatibility with Python hashlib.sha256()
 * 3. Proper hex encoding (lowercase, no prefix)
 */
namespace RiftbornCrypto
{
    /**
     * Check if a hash result is an error code.
     * All error codes start with "ERROR_".
     * 
     * @param Hash Result from Sha256, Sha256Bytes, or HmacSha256
     * @return True if the hash is an error code, false if valid hash
     */
    inline bool IsErrorHash(const FString& Hash)
    {
        return Hash.StartsWith(TEXT("ERROR_"));
    }

    /**
     * Compute SHA256 hash of a string.
     * Returns lowercase hex string (64 characters).
     * 
     * CRITICAL: This is the ONLY function that should be used for
     * proof hashes, canonical JSON hashes, and integrity verification.
     * 
     * @param Text Input string (will be converted to UTF-8)
     * @return Lowercase hex string of SHA256 hash (64 chars)
     */
    inline FString Sha256(const FString& Text)
    {
#if PLATFORM_WINDOWS
        FTCHARToUTF8 Utf8(*Text);
        const uint8* Data = reinterpret_cast<const uint8*>(Utf8.Get());
        const ULONG DataLen = static_cast<ULONG>(Utf8.Length());
        
        BCRYPT_ALG_HANDLE hAlg = nullptr;
        BCRYPT_HASH_HANDLE hHash = nullptr;
        uint8 Digest[32] = {0};  // SHA-256 produces 32 bytes
        NTSTATUS Status;
        
        // Open algorithm provider
        Status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
        if (!BCRYPT_SUCCESS(Status))
        {
            UE_LOG(LogTemp, Error, TEXT("RiftbornCrypto::Sha256 - BCryptOpenAlgorithmProvider failed: 0x%08X"), Status);
            return TEXT("ERROR_SHA256_INIT_FAILED");
        }
        
        // Create hash object
        Status = BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0);
        if (!BCRYPT_SUCCESS(Status))
        {
            BCryptCloseAlgorithmProvider(hAlg, 0);
            UE_LOG(LogTemp, Error, TEXT("RiftbornCrypto::Sha256 - BCryptCreateHash failed: 0x%08X"), Status);
            return TEXT("ERROR_SHA256_CREATE_FAILED");
        }
        
        // Hash the data
        Status = BCryptHashData(hHash, const_cast<PUCHAR>(Data), DataLen, 0);
        if (!BCRYPT_SUCCESS(Status))
        {
            BCryptDestroyHash(hHash);
            BCryptCloseAlgorithmProvider(hAlg, 0);
            UE_LOG(LogTemp, Error, TEXT("RiftbornCrypto::Sha256 - BCryptHashData failed: 0x%08X"), Status);
            return TEXT("ERROR_SHA256_HASH_FAILED");
        }
        
        // Finish and get digest
        Status = BCryptFinishHash(hHash, Digest, sizeof(Digest), 0);
        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        
        if (!BCRYPT_SUCCESS(Status))
        {
            UE_LOG(LogTemp, Error, TEXT("RiftbornCrypto::Sha256 - BCryptFinishHash failed: 0x%08X"), Status);
            return TEXT("ERROR_SHA256_FINISH_FAILED");
        }
        
        // Convert to lowercase hex string (64 chars for SHA-256)
        FString Result;
        Result.Reserve(64);
        for (int32 i = 0; i < 32; i++)
        {
            Result += FString::Printf(TEXT("%02x"), Digest[i]);
        }
        return Result;
#else
        // Linux/Mac: Use OpenSSL SHA256 (bundled with UE)
        FTCHARToUTF8 Utf8(*Text);
        const unsigned char* Data = reinterpret_cast<const unsigned char*>(Utf8.Get());
        const size_t DataLen = static_cast<size_t>(Utf8.Length());
        
        unsigned char Digest[SHA256_DIGEST_LENGTH];
        
        EVP_MD_CTX* Ctx = EVP_MD_CTX_new();
        if (!Ctx)
        {
            UE_LOG(LogTemp, Error, TEXT("RiftbornCrypto::Sha256 - EVP_MD_CTX_new failed"));
            return TEXT("ERROR_SHA256_INIT_FAILED");
        }
        
        bool bOk = (EVP_DigestInit_ex(Ctx, EVP_sha256(), nullptr) == 1)
                 && (EVP_DigestUpdate(Ctx, Data, DataLen) == 1)
                 && (EVP_DigestFinal_ex(Ctx, Digest, nullptr) == 1);
        
        EVP_MD_CTX_free(Ctx);
        
        if (!bOk)
        {
            UE_LOG(LogTemp, Error, TEXT("RiftbornCrypto::Sha256 - OpenSSL SHA256 computation failed"));
            return TEXT("ERROR_SHA256_HASH_FAILED");
        }
        
        FString Result;
        Result.Reserve(64);
        for (int32 i = 0; i < SHA256_DIGEST_LENGTH; i++)
        {
            Result += FString::Printf(TEXT("%02x"), Digest[i]);
        }
        return Result;
#endif
    }

    /**
     * Compute SHA256 hash of raw bytes.
     * Returns lowercase hex string (64 characters).
     * 
     * @param Data Pointer to input data
     * @param DataLen Length of input data in bytes
     * @return Lowercase hex string of SHA256 hash (64 chars)
     */
    inline FString Sha256Bytes(const uint8* Data, int32 DataLen)
    {
#if PLATFORM_WINDOWS
        if (!Data || DataLen <= 0)
        {
            // Hash of empty input
            return Sha256(TEXT(""));
        }
        
        BCRYPT_ALG_HANDLE hAlg = nullptr;
        BCRYPT_HASH_HANDLE hHash = nullptr;
        uint8 Digest[32] = {0};
        NTSTATUS Status;
        
        Status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
        if (!BCRYPT_SUCCESS(Status))
        {
            return TEXT("ERROR_SHA256_INIT_FAILED");
        }
        
        Status = BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0);
        if (!BCRYPT_SUCCESS(Status))
        {
            BCryptCloseAlgorithmProvider(hAlg, 0);
            return TEXT("ERROR_SHA256_CREATE_FAILED");
        }
        
        Status = BCryptHashData(hHash, const_cast<PUCHAR>(Data), static_cast<ULONG>(DataLen), 0);
        if (!BCRYPT_SUCCESS(Status))
        {
            BCryptDestroyHash(hHash);
            BCryptCloseAlgorithmProvider(hAlg, 0);
            return TEXT("ERROR_SHA256_HASH_FAILED");
        }
        
        Status = BCryptFinishHash(hHash, Digest, sizeof(Digest), 0);
        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        
        if (!BCRYPT_SUCCESS(Status))
        {
            return TEXT("ERROR_SHA256_FINISH_FAILED");
        }
        
        FString Result;
        Result.Reserve(64);
        for (int32 i = 0; i < 32; i++)
        {
            Result += FString::Printf(TEXT("%02x"), Digest[i]);
        }
        return Result;
#else
        // Linux/Mac: Use OpenSSL SHA256
        if (!Data || DataLen <= 0)
        {
            return Sha256(TEXT(""));
        }
        
        unsigned char Digest[SHA256_DIGEST_LENGTH];
        
        EVP_MD_CTX* Ctx = EVP_MD_CTX_new();
        if (!Ctx)
        {
            return TEXT("ERROR_SHA256_INIT_FAILED");
        }
        
        bool bOk = (EVP_DigestInit_ex(Ctx, EVP_sha256(), nullptr) == 1)
                 && (EVP_DigestUpdate(Ctx, Data, static_cast<size_t>(DataLen)) == 1)
                 && (EVP_DigestFinal_ex(Ctx, Digest, nullptr) == 1);
        
        EVP_MD_CTX_free(Ctx);
        
        if (!bOk)
        {
            return TEXT("ERROR_SHA256_HASH_FAILED");
        }
        
        FString Result;
        Result.Reserve(64);
        for (int32 i = 0; i < SHA256_DIGEST_LENGTH; i++)
        {
            Result += FString::Printf(TEXT("%02x"), Digest[i]);
        }
        return Result;
#endif
    }

    /**
     * Compute HMAC-SHA256 of data with a secret key.
     * Returns lowercase hex string (64 characters).
     * 
     * Used for signature generation and verification.
     * 
     * @param Data Input data string
     * @param Secret Secret key for HMAC
     * @return Lowercase hex string of HMAC-SHA256 (64 chars)
     */
    inline FString HmacSha256(const FString& Data, const FString& Secret)
    {
#if PLATFORM_WINDOWS
        FTCHARToUTF8 DataUtf8(*Data);
        FTCHARToUTF8 KeyUtf8(*Secret);
        const uint8* DataPtr = reinterpret_cast<const uint8*>(DataUtf8.Get());
        const ULONG DataLen = static_cast<ULONG>(DataUtf8.Length());
        const uint8* KeyData = reinterpret_cast<const uint8*>(KeyUtf8.Get());
        const ULONG KeyLen = static_cast<ULONG>(KeyUtf8.Length());
        
        BCRYPT_ALG_HANDLE hAlg = nullptr;
        BCRYPT_HASH_HANDLE hHash = nullptr;
        uint8 Digest[32] = {0};
        NTSTATUS Status;
        
        // Open with HMAC flag
        Status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG);
        if (!BCRYPT_SUCCESS(Status))
        {
            return TEXT("ERROR_HMAC_INIT_FAILED");
        }
        
        // Get required buffer size
        DWORD HashObjSize = 0;
        DWORD DataSize = 0;
        BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&HashObjSize, sizeof(DWORD), &DataSize, 0);
        
        TArray<uint8> HashObj;
        HashObj.SetNumZeroed(HashObjSize);
        
        // Create HMAC hash with key
        Status = BCryptCreateHash(hAlg, &hHash, HashObj.GetData(), HashObjSize,
            const_cast<PUCHAR>(KeyData), KeyLen, 0);
        if (!BCRYPT_SUCCESS(Status))
        {
            BCryptCloseAlgorithmProvider(hAlg, 0);
            return TEXT("ERROR_HMAC_CREATE_FAILED");
        }
        
        // Hash the data
        Status = BCryptHashData(hHash, const_cast<PUCHAR>(DataPtr), DataLen, 0);
        if (!BCRYPT_SUCCESS(Status))
        {
            BCryptDestroyHash(hHash);
            BCryptCloseAlgorithmProvider(hAlg, 0);
            return TEXT("ERROR_HMAC_HASH_FAILED");
        }
        
        // Get HMAC
        Status = BCryptFinishHash(hHash, Digest, sizeof(Digest), 0);
        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        
        if (!BCRYPT_SUCCESS(Status))
        {
            return TEXT("ERROR_HMAC_FINISH_FAILED");
        }
        
        FString Result;
        Result.Reserve(64);
        for (int32 i = 0; i < 32; i++)
        {
            Result += FString::Printf(TEXT("%02x"), Digest[i]);
        }
        return Result;
#else
        // Linux/Mac: Use OpenSSL HMAC-SHA256
        FTCHARToUTF8 DataUtf8(*Data);
        FTCHARToUTF8 KeyUtf8(*Secret);
        const unsigned char* DataPtr = reinterpret_cast<const unsigned char*>(DataUtf8.Get());
        const size_t DataLen = static_cast<size_t>(DataUtf8.Length());
        const unsigned char* KeyData = reinterpret_cast<const unsigned char*>(KeyUtf8.Get());
        const size_t KeyLen = static_cast<size_t>(KeyUtf8.Length());
        
        unsigned char Digest[EVP_MAX_MD_SIZE];
        unsigned int DigestLen = 0;
        
        // Use the EVP HMAC API (HMAC() is deprecated in OpenSSL 3.x)
        HMAC_CTX* Ctx = HMAC_CTX_new();
        if (!Ctx)
        {
            return TEXT("ERROR_HMAC_INIT_FAILED");
        }
        
        bool bOk = (HMAC_Init_ex(Ctx, KeyData, static_cast<int>(KeyLen), EVP_sha256(), nullptr) == 1)
                 && (HMAC_Update(Ctx, DataPtr, DataLen) == 1)
                 && (HMAC_Final(Ctx, Digest, &DigestLen) == 1);
        
        HMAC_CTX_free(Ctx);
        
        if (!bOk || DigestLen != 32)
        {
            return TEXT("ERROR_HMAC_HASH_FAILED");
        }
        
        FString Result;
        Result.Reserve(64);
        for (unsigned int i = 0; i < DigestLen; i++)
        {
            Result += FString::Printf(TEXT("%02x"), Digest[i]);
        }
        return Result;
#endif
    }

    /**
     * Verify that a hash string looks valid (64 lowercase hex chars).
     * Does NOT verify the hash matches any data - just format validation.
     */
    inline bool IsValidSha256Hex(const FString& Hash)
    {
        if (Hash.Len() != 64)
        {
            return false;
        }
        for (TCHAR c : Hash)
        {
            if (!((c >= TEXT('0') && c <= TEXT('9')) || (c >= TEXT('a') && c <= TEXT('f'))))
            {
                return false;
            }
        }
        return true;
    }

    /**
     * Check if a hash indicates an error (starts with "ERROR_").
     */
    // NOTE: IsErrorHash is defined earlier in this header.

    // Forward declarations for canonical JSON
    inline void CanonicalJsonValue(const TSharedPtr<FJsonValue>& Value, FString& OutStr);
    inline void CanonicalJsonObject(const TSharedPtr<FJsonObject>& Obj, FString& OutStr);
    
    /**
     * Serialize a JSON value to canonical string representation.
     * - Objects: keys sorted alphabetically, no extra whitespace
     * - Arrays: elements in order
     * - Strings: properly escaped
     * - Numbers: minimal representation
     * - Booleans: "true"/"false"
     * - Null: "null"
     */
    inline void CanonicalJsonValue(const TSharedPtr<FJsonValue>& Value, FString& OutStr)
    {
        if (!Value.IsValid())
        {
            OutStr += TEXT("null");
            return;
        }
        
        switch (Value->Type)
        {
            case EJson::Null:
                OutStr += TEXT("null");
                break;
                
            case EJson::Boolean:
                OutStr += Value->AsBool() ? TEXT("true") : TEXT("false");
                break;
                
            case EJson::Number:
            {
                double Num = Value->AsNumber();
                // Use integer representation if whole number
                if (FMath::IsNearlyEqual(Num, FMath::FloorToDouble(Num)))
                {
                    OutStr += FString::Printf(TEXT("%lld"), static_cast<int64>(Num));
                }
                else
                {
                    // Use minimal float representation
                    OutStr += FString::Printf(TEXT("%.17g"), Num);
                }
                break;
            }
                
            case EJson::String:
            {
                // Escape and quote string
                FString Escaped = Value->AsString();
                Escaped.ReplaceInline(TEXT("\\"), TEXT("\\\\"));
                Escaped.ReplaceInline(TEXT("\""), TEXT("\\\""));
                Escaped.ReplaceInline(TEXT("\n"), TEXT("\\n"));
                Escaped.ReplaceInline(TEXT("\r"), TEXT("\\r"));
                Escaped.ReplaceInline(TEXT("\t"), TEXT("\\t"));
                OutStr += TEXT("\"") + Escaped + TEXT("\"");
                break;
            }
                
            case EJson::Array:
            {
                const TArray<TSharedPtr<FJsonValue>>& Arr = Value->AsArray();
                OutStr += TEXT("[");
                for (int32 i = 0; i < Arr.Num(); ++i)
                {
                    if (i > 0) OutStr += TEXT(",");
                    CanonicalJsonValue(Arr[i], OutStr);
                }
                OutStr += TEXT("]");
                break;
            }
                
            case EJson::Object:
                CanonicalJsonObject(Value->AsObject(), OutStr);
                break;
                
            default:
                OutStr += TEXT("null");
                break;
        }
    }
    
    /**
     * Serialize a JSON object to canonical string representation.
     * Keys are sorted alphabetically for deterministic output.
     */
    inline void CanonicalJsonObject(const TSharedPtr<FJsonObject>& Obj, FString& OutStr)
    {
        if (!Obj.IsValid())
        {
            OutStr += TEXT("{}");
            return;
        }
        
        // Get keys and sort them
        TArray<FString> Keys;
        Obj->Values.GetKeys(Keys);
        Keys.Sort();
        
        OutStr += TEXT("{");
        for (int32 i = 0; i < Keys.Num(); ++i)
        {
            if (i > 0) OutStr += TEXT(",");
            
            // Key (escaped)
            FString EscapedKey = Keys[i];
            EscapedKey.ReplaceInline(TEXT("\\"), TEXT("\\\\"));
            EscapedKey.ReplaceInline(TEXT("\""), TEXT("\\\""));
            OutStr += TEXT("\"") + EscapedKey + TEXT("\":");
            
            // Value
            CanonicalJsonValue(Obj->Values[Keys[i]], OutStr);
        }
        OutStr += TEXT("}");
    }
    
    /**
     * PROOF-CRITICAL: Serialize JSON object to canonical string for hashing.
     * 
     * This function MUST be used for all proof hash computations.
     * It guarantees:
     * - Deterministic key ordering (alphabetically sorted)
     * - Consistent number representation
     * - No extra whitespace
     * - Proper string escaping
     * 
     * @param Obj JSON object to serialize
     * @return Canonical JSON string suitable for hashing
     */
    inline FString CanonicalizeJson(const TSharedPtr<FJsonObject>& Obj)
    {
        FString Result;
        Result.Reserve(4096); // Pre-allocate for typical proof bundle size
        CanonicalJsonObject(Obj, Result);
        return Result;
    }
    
    /**
     * Compute SHA256 hash of canonical JSON representation.
     * 
     * This is the SINGLE function that should be used for proof hashing.
     * It combines canonicalization + SHA256 to ensure:
     * - Same object always produces same hash
     * - Hash is verifiable across different JSON parsers
     * 
     * @param Obj JSON object to hash
     * @return Lowercase hex SHA256 hash (64 chars)
     */
    inline FString Sha256CanonicalJson(const TSharedPtr<FJsonObject>& Obj)
    {
        FString Canonical = CanonicalizeJson(Obj);
        return Sha256(Canonical);
    }
}
