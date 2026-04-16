// TemporalAwareness.h - Time, Weather, and Seasonal Understanding for AI Agents
// Provides agents with awareness of time of day, timezones, weather, and seasons

#pragma once

#include "CoreMinimal.h"
#include "TemporalAwareness.generated.h"

// ============================================================================
// TIME OF DAY
// ============================================================================

/**
 * Time of day periods for game environments
 */
UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn,           // ~5:00-7:00 - Early morning, sun rising
    Morning,        // ~7:00-12:00 - Full daylight
    Noon,           // ~12:00-13:00 - Sun at peak
    Afternoon,      // ~13:00-17:00 - Post-noon daylight
    Dusk,           // ~17:00-19:00 - Sun setting, golden hour
    Evening,        // ~19:00-22:00 - After sunset, twilight
    Night,          // ~22:00-5:00 - Full darkness
    Midnight        // ~0:00-2:00 - Deep night
};

/**
 * Lighting mood corresponding to time of day
 */
UENUM(BlueprintType)
enum class ETemporalLightingMood : uint8
{
    Bright,         // Full daylight
    GoldenHour,     // Warm dawn/dusk lighting
    BlueHour,       // Cool twilight
    Moonlit,        // Night with moon
    Dark,           // Minimal ambient light
    Overcast,       // Diffuse cloudy light
    Dramatic        // High contrast, stormy
};

// ============================================================================
// WEATHER SYSTEM
// ============================================================================

/**
 * Weather conditions
 */
UENUM(BlueprintType)
enum class EWeatherCondition : uint8
{
    Clear,          // No clouds, full sun/stars
    PartlyCloudy,   // Some clouds
    Cloudy,         // Overcast
    Foggy,          // Low visibility fog
    Misty,          // Light atmospheric haze
    
    // Precipitation
    LightRain,
    Rain,
    HeavyRain,
    Thunderstorm,
    
    // Winter precipitation
    LightSnow,
    Snow,
    HeavySnow,
    Blizzard,
    Sleet,
    Hail,
    
    // Atmospheric
    Windy,
    Sandstorm,
    DustStorm,
    
    // Special
    Aurora,         // Northern/Southern lights
    Eclipse,        // Solar/Lunar eclipse
    MeteorShower
};

/**
 * Wind intensity levels
 */
UENUM(BlueprintType)
enum class EWindIntensity : uint8
{
    Calm,           // 0-5 mph
    Light,          // 5-15 mph
    Moderate,       // 15-25 mph
    Strong,         // 25-40 mph
    Severe,         // 40-60 mph
    Hurricane       // 60+ mph
};

// ============================================================================
// SEASONS
// ============================================================================

/**
 * Seasonal periods
 */
UENUM(BlueprintType)
enum class ESeason : uint8
{
    Spring,         // Growth, renewal, mild weather
    Summer,         // Heat, long days, green
    Fall,           // Harvest, cooling, leaves changing
    Winter,         // Cold, short days, snow/dormancy
    
    // Fantasy/Special seasons
    Monsoon,        // Tropical rainy season
    DrySeason,      // Tropical dry season
    Eternal         // Unchanging (fantasy/sci-fi)
};

/**
 * Biome types that affect weather/seasonal behavior
 */
UENUM(BlueprintType)
enum class EBiome : uint8
{
    Temperate,      // Four seasons, moderate climate
    Tropical,       // Hot, wet/dry seasons
    Arctic,         // Extreme cold, long winter
    Desert,         // Hot days, cold nights, minimal rain
    Mediterranean,  // Mild winters, hot dry summers
    Oceanic,        // Mild, rainy
    Continental,    // Extreme temperature swings
    Mountain,       // Altitude-dependent, variable
    Swamp,          // Humid, foggy
    Volcanic,       // Ashy, unpredictable
    Fantasy,        // Magical weather patterns
    SciFi           // Artificial/controlled environment
};

// ============================================================================
// TEMPORAL STATE
// ============================================================================

/**
 * Complete temporal state of the game world
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FTemporalState
{
    GENERATED_BODY()
    
    // === TIME ===
    
    /** Current game world hour (0-23) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float GameHour = 12.0f;
    
    /** Current game world minute (0-59) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 GameMinute = 0;
    
    /** Time of day period */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    ETimeOfDay TimeOfDay = ETimeOfDay::Noon;
    
    /** Lighting mood */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    ETemporalLightingMood LightingMood = ETemporalLightingMood::Bright;
    
    /** Sun altitude angle (0 = horizon, 90 = overhead) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float SunAltitude = 60.0f;
    
    /** Sun azimuth angle (0 = North, 90 = East) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float SunAzimuth = 180.0f;
    
    /** Moon phase (0 = new, 0.5 = full, 1 = new again) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float MoonPhase = 0.5f;
    
    // === WEATHER ===
    
    /** Current weather condition */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EWeatherCondition Weather = EWeatherCondition::Clear;
    
    /** Wind intensity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EWindIntensity WindIntensity = EWindIntensity::Calm;
    
    /** Wind direction in degrees (0 = North) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float WindDirection = 0.0f;
    
    /** Temperature in Celsius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Temperature = 20.0f;
    
    /** Humidity percentage (0-100) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Humidity = 50.0f;
    
    /** Visibility in meters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Visibility = 10000.0f;
    
    /** Cloud coverage (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float CloudCoverage = 0.0f;
    
    /** Precipitation intensity (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float PrecipitationIntensity = 0.0f;
    
    // === SEASON ===
    
    /** Current season */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    ESeason Season = ESeason::Summer;
    
    /** Day of year (1-365) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    int32 DayOfYear = 180;
    
    /** Current biome type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EBiome Biome = EBiome::Temperate;
    
    /** Foliage state (0 = bare, 1 = full) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float FoliageState = 1.0f;
    
    /** Snow coverage (0 = none, 1 = full) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float SnowCoverage = 0.0f;
    
    // === REAL-WORLD TIME ===
    
    /** Real-world timezone offset from UTC */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float TimezoneOffsetHours = 0.0f;
    
    /** Real-world time string */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString RealWorldTime;
    
    /** Real-world date string */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    FString RealWorldDate;
    
    // === METHODS ===
    
    /** Get formatted game time string */
    FString GetGameTimeString() const
    {
        int32 Hour = FMath::FloorToInt(GameHour);
        return FString::Printf(TEXT("%02d:%02d"), Hour, GameMinute);
    }
    
    /** Get descriptive time of day string */
    FString GetTimeOfDayString() const;
    
    /** Get weather description */
    FString GetWeatherString() const;
    
    /** Get season description */
    FString GetSeasonString() const;
    
    /** Get full temporal description for agents */
    FString GetFullDescription() const;
};

/**
 * Weather forecast entry
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FWeatherForecast
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float HoursFromNow = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    EWeatherCondition Condition = EWeatherCondition::Clear;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float Temperature = 20.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RiftbornAI")
    float PrecipitationChance = 0.0f;
};

// ============================================================================
// TEMPORAL AWARENESS SYSTEM
// ============================================================================

/**
 * Temporal Awareness System
 * 
 * Provides agents with understanding of:
 * - Time of day (dawn, morning, noon, dusk, night, etc.)
 * - Weather conditions (clear, rain, snow, fog, etc.)
 * - Seasons (spring, summer, fall, winter)
 * - Real-world time and timezones
 * 
 * Can either:
 * 1. Track actual game world time from level actors (DirectionalLight, SkyAtmosphere)
 * 2. Simulate time based on configuration
 * 3. Use real-world time with timezone support
 * 
 * Usage:
 *   FTemporalAwareness& TA = FTemporalAwareness::Get();
 *   FTemporalState State = TA.GetCurrentState();
 *   // State.TimeOfDay == ETimeOfDay::Dusk
 *   // State.Weather == EWeatherCondition::PartlyCloudy
 */
class RIFTBORNAI_API FTemporalAwareness
{
public:
    static FTemporalAwareness& Get();
    
    // =========================================================================
    // CURRENT STATE
    // =========================================================================
    
    /** Get current temporal state */
    FTemporalState GetCurrentState() const;
    
    /** Get current time of day */
    ETimeOfDay GetTimeOfDay() const;
    
    /** Get current weather */
    EWeatherCondition GetWeather() const;
    
    /** Get current season */
    ESeason GetSeason() const;
    
    /** Get game hour (0-24) */
    float GetGameHour() const;
    
    // =========================================================================
    // TIME CONTROL
    // =========================================================================
    
    /** Set game time (0-24 hour) */
    void SetGameTime(float Hour, int32 Minute = 0);
    
    /** Advance time by hours */
    void AdvanceTime(float Hours);
    
    /** Set time of day directly */
    void SetTimeOfDay(ETimeOfDay TimeOfDay);
    
    /** Set time speed multiplier (1.0 = real-time) */
    void SetTimeScale(float Scale);
    
    /** Pause/resume time progression */
    void SetTimePaused(bool bPaused);
    
    // =========================================================================
    // WEATHER CONTROL
    // =========================================================================
    
    /** Set weather condition */
    void SetWeather(EWeatherCondition Condition);
    
    /** Set weather with transition time */
    void TransitionWeather(EWeatherCondition Target, float TransitionHours);
    
    /** Set temperature in Celsius */
    void SetTemperature(float Celsius);
    
    /** Generate weather forecast */
    TArray<FWeatherForecast> GetForecast(int32 HoursAhead) const;
    
    /** Enable/disable dynamic weather changes */
    void SetDynamicWeather(bool bEnabled);
    
    // =========================================================================
    // SEASON CONTROL
    // =========================================================================
    
    /** Set current season */
    void SetSeason(ESeason Season);
    
    /** Set day of year (1-365) */
    void SetDayOfYear(int32 Day);
    
    /** Set biome (affects weather patterns) */
    void SetBiome(EBiome Biome);
    
    /** Enable/disable seasonal progression */
    void SetDynamicSeasons(bool bEnabled);
    
    // =========================================================================
    // REAL-WORLD TIME
    // =========================================================================
    
    /** Get current real-world time */
    FDateTime GetRealWorldTime() const;
    
    /** Get real-world time in specific timezone */
    FDateTime GetTimeInTimezone(float UTCOffset) const;
    
    /** Set timezone for display */
    void SetTimezone(float UTCOffset);
    
    /** Sync game time to real-world time */
    void SyncToRealTime(float UTCOffset = 0.0f);
    
    // =========================================================================
    // LEVEL INTEGRATION
    // =========================================================================
    
    /** Scan current level for time-related actors */
    void ScanLevelForTimeActors();
    
    /** Apply current state to level actors */
    void ApplyStateToLevel();
    
    /** Get recommendations for lighting setup */
    FString GetLightingRecommendations() const;
    
    // =========================================================================
    // CONVERSION HELPERS
    // =========================================================================
    
    /** Convert hour to time of day enum */
    static ETimeOfDay HourToTimeOfDay(float Hour);
    
    /** Get typical hour range for time of day */
    static void GetTimeOfDayHours(ETimeOfDay TimeOfDay, float& OutStartHour, float& OutEndHour);
    
    /** Get typical temperature for season/time */
    static float GetTypicalTemperature(ESeason Season, ETimeOfDay TimeOfDay, EBiome Biome);
    
    /** Convert string to time of day */
    static ETimeOfDay ParseTimeOfDay(const FString& Str);
    
    /** Convert string to weather */
    static EWeatherCondition ParseWeather(const FString& Str);
    
    /** Convert string to season */
    static ESeason ParseSeason(const FString& Str);
    
    /** Convert enums to strings */
    static FString TimeOfDayToString(ETimeOfDay TimeOfDay);
    static FString WeatherToString(EWeatherCondition Weather);
    static FString SeasonToString(ESeason Season);
    static FString BiomeToString(EBiome Biome);
    static FString WindIntensityToString(EWindIntensity Wind);
    static FString LightingMoodToString(ETemporalLightingMood Mood);
    
private:
    FTemporalAwareness();
    ~FTemporalAwareness();
    
    // Current state
    FTemporalState CurrentState;
    
    // Configuration
    float TimeScale = 1.0f;
    bool bTimePaused = false;
    bool bDynamicWeather = false;
    bool bDynamicSeasons = false;
    float TimezoneOffset = 0.0f;
    
    // Weather transition
    EWeatherCondition TargetWeather;
    float WeatherTransitionProgress = 1.0f;
    float WeatherTransitionDuration = 0.0f;
    
    // Tick
    FDelegateHandle TickHandle;
    void Tick(float DeltaTime);
    
    // Internal
    void UpdateTimeOfDay();
    void UpdateLightingMood();
    void UpdateWeather(float DeltaTime);
    void UpdateSeason();
    void CalculateSunPosition();
};
