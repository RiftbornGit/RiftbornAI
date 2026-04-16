// AssetVisualIntelligence.h - Visual understanding of game assets
// Analyzes asset thumbnails to classify what they ARE (chair, rock, tree, weapon, etc.)
// Uses Vision LLMs (LLaVA, GPT-4V, Claude Vision) for semantic understanding

#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"
#include "Containers/Ticker.h"
#include "AssetVisualIntelligence.generated.h"

// Forward declarations
class UStaticMesh;
class USkeletalMesh;
class UTexture2D;
class UMaterialInterface;

// ============================================================================
// SEMANTIC ASSET CATEGORIES
// ============================================================================

/**
 * High-level semantic categories for game assets
 * These represent what an asset IS, not its technical type
 */
UENUM(BlueprintType)
enum class EAssetSemanticCategory : uint8
{
    // Environment/Nature
    Tree,
    Bush,
    Grass,
    Flower,
    Rock,
    Boulder,
    Cliff,
    Terrain,
    Water,
    
    // Architecture
    Building,
    Wall,
    Floor,
    Ceiling,
    Door,
    Window,
    Stairs,
    Pillar,
    Arch,
    
    // Furniture
    Chair,
    Table,
    Bed,
    Shelf,
    Cabinet,
    Desk,
    Couch,
    Lamp,
    
    // Props
    Barrel,
    Crate,
    Chest,
    Pot,
    Vase,
    Book,
    Candle,
    Sign,
    
    // Vehicles
    Car,
    Truck,
    Boat,
    Aircraft,
    Motorcycle,
    Cart,
    
    // Characters/Creatures
    HumanCharacter,
    AnimalCreature,
    MonsterCreature,
    RobotMech,
    
    // Weapons
    Sword,
    Axe,
    Hammer,
    Bow,
    Gun,
    Shield,
    Staff,
    
    // Food/Consumables
    Food,
    Drink,
    Potion,
    
    // Effects/Technical
    Decal,
    Particle,
    Light,
    
    // Meta
    Unknown,
    Multiple,  // Contains multiple categories
    Abstract   // Non-representational (materials, patterns)
};

/**
 * Visual style of an asset
 */
UENUM(BlueprintType)
enum class EAssetVisualStyle : uint8
{
    Realistic,
    Stylized,
    LowPoly,
    Pixel,
    Cartoon,
    Anime,
    Medieval,
    SciFi,
    Modern,
    Fantasy,
    Horror,
    Steampunk,
    PostApocalyptic,
    Unknown
};

/**
 * Quality/detail level of the asset
 */
UENUM(BlueprintType)
enum class EAssetDetailLevel : uint8
{
    AAA,           // Legacy enum value for high-detail, high-polish assets
    Indie,         // Good detail, suitable for indie games
    Prototype,     // Basic/placeholder quality
    Primitive,     // Simple shapes only
    Unknown
};

// ============================================================================
// VISUAL ANALYSIS RESULT
// ============================================================================

/**
 * Result of visual analysis of an asset
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FAssetVisualAnalysis
{
    GENERATED_BODY()
    
    /** Primary semantic category */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EAssetSemanticCategory PrimaryCategory = EAssetSemanticCategory::Unknown;
    
    /** Secondary categories if applicable */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<EAssetSemanticCategory> SecondaryCategories;
    
    /** Visual style */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EAssetVisualStyle Style = EAssetVisualStyle::Unknown;
    
    /** Detail level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EAssetDetailLevel DetailLevel = EAssetDetailLevel::Unknown;
    
    /** Confidence score 0-1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Confidence = 0.0f;
    
    /** Natural language description of what this asset is */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Description;
    
    /** Semantic tags for search (e.g., "seating", "wooden", "indoor") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FString> SemanticTags;
    
    /** Suggested use cases (e.g., "interior decoration", "forest scene") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FString> SuggestedUseCases;
    
    /** Dominant colors in the asset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    TArray<FLinearColor> DominantColors;
    
    /** Raw response from Vision LLM */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString RawVisionResponse;
    
    /** Whether analysis was successful */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bSuccess = false;
    
    /** Error message if failed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ErrorMessage;
    
    /** Time taken for analysis in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float AnalysisTimeMs = 0.0f;
    
    /** Path to rendered thumbnail used for analysis */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ThumbnailPath;
};

/**
 * Settings for visual analysis
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FVisualAnalysisSettings
{
    GENERATED_BODY()
    
    /** Vision LLM provider: ollama, openai, anthropic */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Provider = TEXT("openai");
    
    /** Model name: llava, llava:13b, gpt-5.4, claude-sonnet-4-6 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString Model = TEXT("gpt-5.4");
    
    /** API endpoint for Ollama */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString OllamaEndpoint = TEXT("http://localhost:11434");
    
    /** API key for cloud providers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString ApiKey;
    
    /** Thumbnail resolution */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 ThumbnailSize = 512;
    
    /** Number of angles to render for 3D assets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 RenderAngles = 4;
    
    /** Cache analyzed results to disk */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bCacheResults = true;
    
    /** Re-analyze if asset modified */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    bool bAutoRefresh = true;
    
    /** Max tokens for LLM response */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 MaxTokens = 500;
    
    /** Temperature for LLM */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Temperature = 0.2f;
};

// ============================================================================
// ASSET VISUAL INTELLIGENCE SYSTEM
// ============================================================================

/**
 * Asset Visual Intelligence System
 * 
 * Provides semantic understanding of game assets by:
 * 1. Rendering thumbnails of meshes/textures
 * 2. Analyzing with Vision LLMs
 * 3. Classifying into semantic categories
 * 4. Persisting metadata for future queries
 * 
 * Usage:
 *   FAssetVisualIntelligence& AVI = FAssetVisualIntelligence::Get();
 *   FAssetVisualAnalysis Result = AVI.AnalyzeAsset("/Game/Props/SM_Chair_01");
 *   // Result.PrimaryCategory == EAssetSemanticCategory::Chair
 *   // Result.Description == "A wooden chair with armrests, medieval style"
 */
class RIFTBORNAI_API FAssetVisualIntelligence
{
public:
    static FAssetVisualIntelligence& Get();
    
    // =========================================================================
    // CONFIGURATION
    // =========================================================================
    
    /** Get current settings */
    const FVisualAnalysisSettings& GetSettings() const { return Settings; }
    
    /** Update settings */
    void SetSettings(const FVisualAnalysisSettings& NewSettings) { Settings = NewSettings; }
    
    /** Load settings from config */
    void LoadSettingsFromConfig();
    
    /** Save settings to config */
    void SaveSettingsToConfig();
    
    // =========================================================================
    // SINGLE ASSET ANALYSIS
    // =========================================================================
    
    /**
     * Analyze a single asset by path
     * @param AssetPath - UE asset path (e.g., /Game/Props/SM_Chair_01)
     * @param bForceReanalyze - Force re-analysis even if cached
     * @return Visual analysis result
     */
    FAssetVisualAnalysis AnalyzeAsset(const FString& AssetPath, bool bForceReanalyze = false);

    /**
     * Analyze any image file with a custom prompt using the configured vision provider.
     * This is used for scene screenshot analysis (not just asset thumbnails).
     *
     * @param ImagePath - Absolute or project-relative path to an image file.
     * @param Prompt - Instruction prompt for the vision model.
     * @param OutResponse - Raw model response text.
     * @param OutError - Error detail on failure.
     * @return true if the vision request succeeded.
     */
    bool AnalyzeImageWithPrompt(
        const FString& ImagePath,
        const FString& Prompt,
        FString& OutResponse,
        FString& OutError);

    /**
     * Analyze multiple images in a single vision LLM call.
     * Supports up to 8 images (excess are silently capped).
     * All 3 providers (Ollama, OpenAI, Anthropic) support multi-image.
     */
    bool AnalyzeMultipleImagesWithPrompt(
        const TArray<FString>& ImagePaths,
        const FString& Prompt,
        FString& OutResponse,
        FString& OutError);

    /**
     * Analyze a single asset asynchronously (non-blocking).
     * Callback is executed on the game thread.
     */
    void AnalyzeAssetAsync(const FString& AssetPath, TFunction<void(const FString& /*AssetPath*/, const FAssetVisualAnalysis& /*Result*/)> OnComplete, bool bForceReanalyze = false);
    
    /**
     * Analyze a StaticMesh directly
     */
    FAssetVisualAnalysis AnalyzeStaticMesh(UStaticMesh* Mesh);
    
    /**
     * Analyze a SkeletalMesh directly  
     */
    FAssetVisualAnalysis AnalyzeSkeletalMesh(USkeletalMesh* Mesh);
    
    /**
     * Analyze a Texture directly
     */
    FAssetVisualAnalysis AnalyzeTexture(UTexture2D* Texture);
    
    /**
     * Analyze a Material directly
     */
    FAssetVisualAnalysis AnalyzeMaterial(UMaterialInterface* Material);
    
    // =========================================================================
    // BATCH ANALYSIS
    // =========================================================================
    
    /**
     * Analyze all assets in a folder
     * @param FolderPath - Content folder path (e.g., /Game/Props)
     * @param bRecursive - Include subfolders
     * @param ProgressCallback - Optional progress callback (current, total)
     */
    TArray<FAssetVisualAnalysis> AnalyzeFolder(
        const FString& FolderPath, 
        bool bRecursive = true,
        TFunction<void(int32, int32)> ProgressCallback = nullptr);
    
    /**
     * Analyze all unanalyzed assets in project
     * @param ProgressCallback - Optional progress callback
     */
    void AnalyzeAllUnanalyzed(TFunction<void(int32, int32)> ProgressCallback = nullptr);
    
    /**
     * Get analysis queue size
     */
    int32 GetPendingAnalysisCount() const;
    
    // =========================================================================
    // SEMANTIC QUERIES
    // =========================================================================
    
    /**
     * Find assets by semantic category
     * @param Category - What kind of asset to find
     * @return Array of asset paths matching the category
     */
    TArray<FString> FindAssetsByCategory(EAssetSemanticCategory Category) const;
    
    /**
     * Find assets by semantic tags
     * @param Tags - Tags to search for (e.g., "wooden", "medieval")
     * @param bMatchAll - Require all tags (AND) vs any tag (OR)
     */
    TArray<FString> FindAssetsByTags(const TArray<FString>& Tags, bool bMatchAll = false) const;
    
    /**
     * Find assets by natural language description
     * Uses fuzzy matching on descriptions and tags
     * @param Query - Natural language query (e.g., "a chair for a medieval tavern")
     */
    TArray<FString> FindAssetsByDescription(const FString& Query) const;
    
    /**
     * Find assets similar to a given asset
     * @param AssetPath - Reference asset
     * @param MaxResults - Maximum results to return
     */
    TArray<FString> FindSimilarAssets(const FString& AssetPath, int32 MaxResults = 10) const;
    
    // =========================================================================
    // CACHE MANAGEMENT
    // =========================================================================
    
    /**
     * Get cached analysis for an asset (thread-safe copy).
     * @return true if found, writes to OutAnalysis
     */
    bool GetCachedAnalysis(const FString& AssetPath, FAssetVisualAnalysis& OutAnalysis) const;

    /**
     * Get cached analysis pointer (NOT thread-safe — caller must hold CacheLock or be on game thread).
     * @return nullptr if not cached
     */
    const FAssetVisualAnalysis* GetCachedAnalysisUnsafe(const FString& AssetPath) const;
    
    /**
     * Clear cache for a specific asset
     */
    void ClearCacheForAsset(const FString& AssetPath);
    
    /**
     * Clear entire cache
     */
    void ClearAllCache();
    
    /**
     * Save cache to disk
     */
    void SaveCacheToDisk();
    
    /**
     * Load cache from disk
     */
    void LoadCacheFromDisk();
    
    /**
     * Get cache statistics
     */
    struct FCacheStats
    {
        int32 TotalCached;
        int32 MeshesAnalyzed;
        int32 TexturesAnalyzed;
        int32 MaterialsAnalyzed;
        int64 CacheFileSizeBytes;
    };
    FCacheStats GetCacheStats() const;
    
    // =========================================================================
    // EVENT HOOKS
    // =========================================================================
    
    /** Called when a new asset is imported - triggers auto-analysis if enabled */
    void OnAssetImported(const FString& AssetPath);
    
    /** Called when an asset is modified - triggers re-analysis if enabled */
    void OnAssetModified(const FString& AssetPath);
    
    /** Enable/disable auto-analysis on import */
    void SetAutoAnalyzeOnImport(bool bEnable);
    
    /** Check if auto-analysis is enabled */
    bool IsAutoAnalyzeEnabled() const { return bAutoAnalyzeOnImport; }
    
    // =========================================================================
    // CAPABILITY GATING (Ollama availability)
    // =========================================================================
    
    /** Check if Ollama availability has been checked */
    bool HasCheckedOllamaAvailability() const { return bOllamaAvailabilityChecked; }
    
    /** Check if Ollama is currently available */
    bool IsOllamaAvailable() const { return bOllamaAvailable; }
    
    /** Get timestamp of last Ollama availability check (empty if never checked) */
    FString GetOllamaLastCheckTimestamp() const { return OllamaLastCheckTimestamp; }
    
    /** Force a re-check of Ollama availability. Returns true if now available. */
    bool RecheckOllamaAvailability();
    
    // =========================================================================
    // INTEGRATION WITH PROJECT ASSET INDEX
    // =========================================================================
    
    /**
     * Sync visual analysis results to ProjectAssetIndex
     * Adds semantic tags and categories to the index
     */
    void SyncToProjectAssetIndex();
    
    /**
     * Update a single asset in the ProjectAssetIndex
     */
    void UpdateProjectAssetIndex(const FString& AssetPath, const FAssetVisualAnalysis& Analysis);

    /** Thread-safe capped insert. Drops half the cache on overflow to bound memory. */
    void AddToAnalysisCacheCapped(const FString& AssetPath, const FAssetVisualAnalysis& Result);

private:
    FAssetVisualIntelligence();
    ~FAssetVisualIntelligence();
    
    // Settings — guarded by CacheLock when accessed from non-game-thread
    FVisualAnalysisSettings Settings;

    // Cache — guarded by CacheLock for thread-safe reads/writes
    TMap<FString, FAssetVisualAnalysis> AnalysisCache;
    FString CacheFilePath;
    mutable FCriticalSection CacheLock;  // Guards AnalysisCache, Settings, Ollama flags

    // Async analysis queue
    struct FPendingAnalysis
    {
        FString AssetPath;
        TFunction<void(const FString&, const FAssetVisualAnalysis&)> Callback;
        bool bForceReanalyze;
    };
    TArray<FPendingAnalysis> PendingAnalysisQueue;
    mutable FCriticalSection QueueLock;
    int32 MaxConcurrentAnalyses = 2;
    int32 ActiveAnalysisCount = 0;

    // State — guarded by CacheLock
    bool bAutoAnalyzeOnImport = true;
    bool bOllamaAvailabilityChecked = false;
    bool bOllamaAvailable = false;
    FString OllamaLastCheckTimestamp;  // UTC timestamp of last availability check
    
    // Periodic health check (60s interval, no spam)
    FTSTicker::FDelegateHandle OllamaHealthTickHandle;
    double LastOllamaCheckTime = 0.0;
    static constexpr double OLLAMA_HEALTH_CHECK_INTERVAL = 60.0; // seconds
    bool PeriodicOllamaHealthCheck(float DeltaTime);
    
    // Internal methods
    FString RenderThumbnail(UObject* Asset, int32 Size, int32 AngleIndex = 0);
    TArray<FString> RenderMultiAngleThumbnails(UObject* Asset, int32 Size, int32 NumAngles);
    FAssetVisualAnalysis CallVisionLLM(const TArray<FString>& ThumbnailPaths, const FString& AssetName);
    FAssetVisualAnalysis ParseVisionResponse(const FString& Response, const FString& AssetPath);
    
    // Vision LLM providers
    FString CallOllama(const TArray<FString>& ImagePaths, const FString& Prompt);
    FString CallOpenAI(const TArray<FString>& ImagePaths, const FString& Prompt);
    FString CallAnthropic(const TArray<FString>& ImagePaths, const FString& Prompt);
    
    // Helpers
    FString GetPromptForAssetAnalysis() const;
    
    // Thumbnail rendering
    class FThumbnailRenderer* ThumbnailRenderer = nullptr;

public:
    // Category/Style conversion (public for tool access)
    EAssetSemanticCategory ParseCategory(const FString& CategoryStr) const;
    EAssetVisualStyle ParseStyle(const FString& StyleStr) const;
    FString CategoryToString(EAssetSemanticCategory Category) const;
    FString StyleToString(EAssetVisualStyle Style) const;
};

// ============================================================================
// THUMBNAIL RENDERER (Internal)
// ============================================================================

/**
 * Renders high-quality thumbnails of assets for vision analysis
 */
class RIFTBORNAI_API FThumbnailRenderer
{
public:
    FThumbnailRenderer();
    ~FThumbnailRenderer();
    
    /**
     * Render a static mesh to PNG
     * @param Mesh - The mesh to render
     * @param OutputPath - Where to save the PNG
     * @param Size - Image size (square)
     * @param YawAngle - Camera rotation angle (0-360)
     * @return true if successful
     */
    bool RenderStaticMesh(UStaticMesh* Mesh, const FString& OutputPath, int32 Size, float YawAngle = 0.0f);
    
    /**
     * Render a skeletal mesh to PNG
     */
    bool RenderSkeletalMesh(USkeletalMesh* Mesh, const FString& OutputPath, int32 Size, float YawAngle = 0.0f);
    
    /**
     * Render a texture to PNG
     */
    bool RenderTexture(UTexture2D* Texture, const FString& OutputPath, int32 Size);
    
    /**
     * Render a material preview sphere to PNG
     */
    bool RenderMaterial(UMaterialInterface* Material, const FString& OutputPath, int32 Size);
    
private:
    // Render target for thumbnail generation
    class UTextureRenderTarget2D* RenderTarget = nullptr;
    
    // Scene capture component for 3D assets
    class USceneCaptureComponent2D* SceneCapture = nullptr;
    
    void SetupRenderTarget(int32 Size);
    void CaptureToFile(const FString& OutputPath);
};
