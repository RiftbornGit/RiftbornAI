// Copyright RiftbornAI. All Rights Reserved.
// Input Sanitization - Security hardening for tool parameters

#pragma once

#include "CoreMinimal.h"

/**
 * Input sanitization utilities for tool parameters.
 * 
 * SECURITY: All user-controllable inputs (labels, paths, asset names) MUST be
 * validated through these functions before use. This prevents:
 * - SSRF attacks via URL/scheme injection
 * - Path traversal attacks
 * - Header injection (CRLF)
 * - Information leakage through error messages
 */
namespace FInputSanitization
{
    /**
     * Validation result with detailed error info
     */
    struct FValidationResult
    {
        bool bValid = false;
        FString Error;
        FString SanitizedValue;
        
        static FValidationResult Ok(const FString& Value)
        {
            FValidationResult R;
            R.bValid = true;
            R.SanitizedValue = Value;
            return R;
        }
        
        static FValidationResult Fail(const FString& Reason)
        {
            FValidationResult R;
            R.bValid = false;
            R.Error = Reason;
            return R;
        }
    };

    /**
     * Check if a string contains URL scheme indicators.
     * Detects: http://, https://, file://, ftp://, dict://, gopher://, ldap://, etc.
     */
    inline bool ContainsUrlScheme(const FString& Input)
    {
        // Common URL schemes used in SSRF attacks
        static const TCHAR* DangerousSchemes[] = {
            TEXT("://"),
            TEXT("file:"),
            TEXT("http:"),
            TEXT("https:"),
            TEXT("ftp:"),
            TEXT("dict:"),
            TEXT("gopher:"),
            TEXT("ldap:"),
            TEXT("sftp:"),
            TEXT("tftp:"),
            TEXT("telnet:"),
            TEXT("data:"),
            TEXT("jar:"),
            TEXT("expect:"),
        };
        
        FString Lower = Input.ToLower();
        for (const TCHAR* Scheme : DangerousSchemes)
        {
            if (Lower.Contains(Scheme))
            {
                return true;
            }
        }
        return false;
    }

    /**
     * Check if a string contains path traversal patterns.
     */
    inline bool ContainsPathTraversal(const FString& Input)
    {
        return Input.Contains(TEXT("..")) || 
               Input.Contains(TEXT("\\..")) ||
               Input.Contains(TEXT("../")) ||
               Input.Contains(TEXT("/.."));
    }

    /**
     * Check if a string contains control characters (including CRLF).
     */
    inline bool ContainsControlChars(const FString& Input)
    {
        for (TCHAR C : Input)
        {
            if (C < 0x20 || C == 0x7F) // Control chars or DEL
            {
                return true;
            }
        }
        return false;
    }

    /**
     * Check if a string looks like a metadata/cloud endpoint (SSRF target).
     */
    inline bool ContainsMetadataEndpoint(const FString& Input)
    {
        FString Lower = Input.ToLower();
        return Lower.Contains(TEXT("169.254.169.254")) ||   // AWS metadata
               Lower.Contains(TEXT("metadata.google")) ||    // GCP metadata
               Lower.Contains(TEXT("metadata.azure")) ||     // Azure metadata
               Lower.Contains(TEXT("localhost")) ||
               Lower.Contains(TEXT("127.0.0.1")) ||
               Lower.Contains(TEXT("[::1]")) ||
               Lower.Contains(TEXT("0.0.0.0"));
    }

    /**
     * Validate an actor label.
     * Labels should be simple identifiers, not URLs or paths.
     * 
     * Allowed: alphanumeric, underscore, dash, dot, space, colon (for Unreal paths)
     * Max length: 256 characters
     */
    inline FValidationResult ValidateActorLabel(const FString& Label)
    {
        if (Label.IsEmpty())
        {
            return FValidationResult::Fail(TEXT("Label cannot be empty"));
        }
        
        if (Label.Len() > 256)
        {
            return FValidationResult::Fail(TEXT("Label too long (max 256 characters)"));
        }
        
        // Check for URL schemes
        if (ContainsUrlScheme(Label))
        {
            return FValidationResult::Fail(TEXT("Invalid label format: URL schemes not allowed"));
        }
        
        // Check for path traversal
        if (ContainsPathTraversal(Label))
        {
            return FValidationResult::Fail(TEXT("Invalid label format: path traversal not allowed"));
        }
        
        // Check for control characters
        if (ContainsControlChars(Label))
        {
            return FValidationResult::Fail(TEXT("Invalid label format: control characters not allowed"));
        }
        
        // Check for metadata endpoints
        if (ContainsMetadataEndpoint(Label))
        {
            return FValidationResult::Fail(TEXT("Invalid label format: reserved addresses not allowed"));
        }
        
        // Character whitelist check
        for (TCHAR C : Label)
        {
            bool bValid = FChar::IsAlnum(C) || 
                         C == TEXT('_') || 
                         C == TEXT('-') || 
                         C == TEXT('.') || 
                         C == TEXT(' ') || 
                         C == TEXT(':') ||
                         C == TEXT('/') ||  // For Unreal paths like /Game/...
                         C == TEXT('(') ||  // For array indices
                         C == TEXT(')');
            if (!bValid)
            {
                return FValidationResult::Fail(TEXT("Invalid label format: contains forbidden characters"));
            }
        }
        
        return FValidationResult::Ok(Label.TrimStartAndEnd());
    }

    /**
     * Validate an asset path.
     * Must start with /Game/, /Engine/, or similar valid roots.
     */
    inline FValidationResult ValidateAssetPath(const FString& Path)
    {
        if (Path.IsEmpty())
        {
            return FValidationResult::Fail(TEXT("Path cannot be empty"));
        }
        
        if (Path.Len() > 512)
        {
            return FValidationResult::Fail(TEXT("Path too long (max 512 characters)"));
        }
        
        // Check for URL schemes
        if (ContainsUrlScheme(Path))
        {
            return FValidationResult::Fail(TEXT("Invalid path format: URL schemes not allowed"));
        }
        
        // Check for path traversal
        if (ContainsPathTraversal(Path))
        {
            return FValidationResult::Fail(TEXT("Invalid path format: path traversal not allowed"));
        }
        
        // Check for control characters
        if (ContainsControlChars(Path))
        {
            return FValidationResult::Fail(TEXT("Invalid path format: control characters not allowed"));
        }
        
        // Must start with valid Unreal path root
        bool bValidRoot = Path.StartsWith(TEXT("/Game/")) ||
                         Path.StartsWith(TEXT("/Engine/")) ||
                         Path.StartsWith(TEXT("/Script/")) ||
                         Path.StartsWith(TEXT("/Temp/")) ||
                         Path.StartsWith(TEXT("/RiftbornAI/"));
        
        if (!bValidRoot)
        {
            return FValidationResult::Fail(TEXT("Invalid path format: must start with valid asset root (/Game/, /Engine/, etc.)"));
        }

        return FValidationResult::Ok(Path.TrimStartAndEnd());
    }

    /**
     * Validate a class name.
     * Should be a simple identifier or Blueprint path.
     */
    inline FValidationResult ValidateClassName(const FString& ClassName)
    {
        if (ClassName.IsEmpty())
        {
            return FValidationResult::Fail(TEXT("Class name cannot be empty"));
        }
        
        if (ClassName.Len() > 256)
        {
            return FValidationResult::Fail(TEXT("Class name too long (max 256 characters)"));
        }
        
        // Check for URL schemes
        if (ContainsUrlScheme(ClassName))
        {
            return FValidationResult::Fail(TEXT("Invalid class name format: URL schemes not allowed"));
        }
        
        // Check for control characters
        if (ContainsControlChars(ClassName))
        {
            return FValidationResult::Fail(TEXT("Invalid class name format: control characters not allowed"));
        }
        
        return FValidationResult::Ok(ClassName.TrimStartAndEnd());
    }

    /**
     * Check if a filename (without extension) matches a Windows reserved device name.
     * CON, PRN, NUL, AUX, COM1-COM9, LPT1-LPT9 are reserved on Windows
     * and can cause packaging/runtime failures.
     */
    inline bool IsWindowsReservedDeviceName(const FString& Filename)
    {
        // Extract just the filename without extension
        FString Name = FPaths::GetBaseFilename(Filename).ToUpper();

        static const TCHAR* ReservedNames[] = {
            TEXT("CON"), TEXT("PRN"), TEXT("NUL"), TEXT("AUX"),
        };

        for (const TCHAR* Reserved : ReservedNames)
        {
            if (Name == Reserved) return true;
        }

        // COM1-COM9, LPT1-LPT9
        if (Name.Len() == 4)
        {
            if ((Name.StartsWith(TEXT("COM")) || Name.StartsWith(TEXT("LPT")))
                && Name[3] >= TEXT('1') && Name[3] <= TEXT('9'))
            {
                return true;
            }
        }

        return false;
    }

    /**
     * Validate a file path (for tools that read/write files).
     * SECURITY CRITICAL: This must prevent access outside the project.
     */
    inline FValidationResult ValidateFilePath(const FString& FilePath, const FString& ProjectRoot)
    {
        if (FilePath.IsEmpty())
        {
            return FValidationResult::Fail(TEXT("File path cannot be empty"));
        }
        
        if (FilePath.Len() > 1024)
        {
            return FValidationResult::Fail(TEXT("File path too long (max 1024 characters)"));
        }
        
        // Check for URL schemes
        if (ContainsUrlScheme(FilePath))
        {
            return FValidationResult::Fail(TEXT("Invalid file path: URL schemes not allowed"));
        }
        
        // Check for control characters
        if (ContainsControlChars(FilePath))
        {
            return FValidationResult::Fail(TEXT("Invalid file path: control characters not allowed"));
        }
        
        // Normalize path
        FString NormalizedPath = FPaths::ConvertRelativePathToFull(FilePath);
        FPaths::NormalizeDirectoryName(NormalizedPath);
        
        // Must be within project root
        FString NormalizedRoot = FPaths::ConvertRelativePathToFull(ProjectRoot);
        FPaths::NormalizeDirectoryName(NormalizedRoot);

        if (!NormalizedPath.StartsWith(NormalizedRoot))
        {
            return FValidationResult::Fail(TEXT("Invalid file path: must be within project directory"));
        }

        // Reject Windows reserved device names (CON, PRN, NUL, AUX, COM1-9, LPT1-9)
        if (IsWindowsReservedDeviceName(NormalizedPath))
        {
            return FValidationResult::Fail(TEXT("Invalid file path: Windows reserved device name"));
        }

        return FValidationResult::Ok(NormalizedPath);
    }

    /**
     * Validate and sanitize a header value.
     * CRITICAL: Prevents CRLF injection attacks.
     */
    inline FValidationResult ValidateHeaderValue(const FString& Value)
    {
        if (Value.Len() > 256)
        {
            return FValidationResult::Fail(TEXT("Header value too long"));
        }
        
        // Check for control characters including CRLF
        for (TCHAR C : Value)
        {
            if (C < 0x20 || C == 0x7F)
            {
                return FValidationResult::Fail(TEXT("Header value contains control characters"));
            }
        }
        
        return FValidationResult::Ok(Value.TrimStartAndEnd());
    }

    /**
     * Sanitize a string for safe inclusion in error messages.
     * Truncates and removes potentially dangerous content.
     */
    inline FString SanitizeForErrorMessage(const FString& Input, int32 MaxLen = 50)
    {
        if (Input.IsEmpty())
        {
            return TEXT("<empty>");
        }
        
        FString Safe;
        int32 Count = 0;
        
        for (TCHAR C : Input)
        {
            if (Count >= MaxLen)
            {
                Safe += TEXT("...");
                break;
            }
            
            // Only allow safe printable ASCII
            if (C >= 0x20 && C < 0x7F && C != TEXT('<') && C != TEXT('>'))
            {
                Safe += C;
            }
            else
            {
                Safe += TEXT('?');
            }
            Count++;
        }
        
        return Safe;
    }
}
