# Code Standards (customize for your project)

## C++
- Validate all external input at system boundaries (command dispatch, deserialization, file I/O)
- Bounds-check array/vector indices before access
- Never allocate on the audio thread
- Use atomic/lock-free patterns for cross-thread communication
- Forward-declare in headers; prefer forward declarations over includes

## Blueprints
- Use gameplay tags for identification, not string comparisons
- GAS abilities require proper GameplayEffect cleanup on ability end
- Keep Blueprint graphs under 50 nodes per function — extract to sub-functions

## Assets
- Follow project naming convention: SM_ for static meshes, MI_ for material instances, T_ for textures
- Save assets to the correct Content subdirectory, not the root
- Verify assets compile after creation before moving on
