/**
 * Tool Readiness Gate
 *
 * Classifies every MCP tool into a maturity tier so the ListTools response
 * only advertises tools the LLM can actually call successfully.
 *
 * Tiers:
 *   PRODUCTION  – Curated handler, tested, safe to call.
 *   BETA        – Handler exists but uses generated passthrough; works when UE is running.
 *   EXPERIMENTAL– Generated tool with no curated handler; may fail or have rough edges.
 *   STUB        – Known broken or unimplemented; hidden by default.
 *   DEPRECATED  – Scheduled for removal; hidden by default.
 *
 * Usage in index.ts:
 *   import { filterToolsByReadiness, ReadinessTier, getToolReadiness } from "./tool-readiness.js";
 *   const visibleTools = filterToolsByReadiness(ALL_TOOLS, ["PRODUCTION", "BETA"]);
 */
import { getProductionToolNameSet } from "./surface-manifest.js";
import { createSanitizer } from "./sanitize-utils.js";
const EXPOSE_GENERATED_EXPERIMENTAL = process.env.RIFTBORN_EXPOSE_GENERATED_TOOLS === "true";
const PRODUCTION_TOOL_NAMES = getProductionToolNameSet();
// ─── Explicit Overrides ───────────────────────────────────────────────────────
// Tools listed here have a manually-assigned tier.  Everything else is
// classified by the heuristic in classifyTool().
const EXPLICIT_TIER = {
    // ── PRODUCTION: curated, tested, safe for paying customers ──
    // Investigation & observation (read-only, zero risk)
    get_current_level: { tier: "PRODUCTION" },
    get_project_info: { tier: "PRODUCTION" },
    list_assets: { tier: "PRODUCTION" },
    read_file: { tier: "PRODUCTION", reason: "Generated passthrough in v1 packaging — keep opt-in until the MCP path is curated end-to-end" },
    get_output_log: { tier: "PRODUCTION" },
    get_verification_status: { tier: "PRODUCTION" },
    run_diagnostic: { tier: "PRODUCTION" },
    list_diagnostics: { tier: "PRODUCTION" },
    get_viewport_info: { tier: "PRODUCTION" },
    // Crash diagnosis (bridge-independent — reads disk)
    diagnose_crash: { tier: "PRODUCTION" },
    get_known_crash_patterns: { tier: "PRODUCTION" },
    lookup_crash_pattern: { tier: "PRODUCTION" },
    record_crash_resolution: { tier: "PRODUCTION" },
    scan_project_health: { tier: "PRODUCTION" },
    audit_blueprint_health: { tier: "PRODUCTION" },
    audit_texture_memory: { tier: "PRODUCTION" },
    find_unused_assets: { tier: "PRODUCTION" },
    check_naming_conventions: { tier: "PRODUCTION" },
    // Polyhaven (free CC0 asset search + download)
    search_polyhaven: { tier: "PRODUCTION" },
    download_polyhaven_texture: { tier: "PRODUCTION" },
    generate_audio: { tier: "PRODUCTION" },
    generate_and_import_3d_model: { tier: "PRODUCTION" },
    generate_blockout: { tier: "PRODUCTION" },
    generate_texture: { tier: "PRODUCTION" },
    batch_download_and_import: { tier: "PRODUCTION" },
    // Vision (read-only screenshots + analysis)
    take_screenshot: { tier: "PRODUCTION" },
    analyze_scene_screenshot: { tier: "PRODUCTION" },
    observe_ue_project: { tier: "PRODUCTION" },
    verify_scene_360: { tier: "PRODUCTION" },
    score_scene_quality: { tier: "PRODUCTION" },
    compare_scene_states: { tier: "PRODUCTION" },
    // Safe creation (reversible, governed)
    create_blueprint: { tier: "PRODUCTION" },
    create_material: { tier: "PRODUCTION" },
    create_silpom_material: { tier: "PRODUCTION" },
    spawn_actor: { tier: "PRODUCTION" },
    edit_file: { tier: "BETA", reason: "Generated passthrough in v1 packaging — keep opt-in until the MCP path is curated end-to-end" },
    // Actor management (curated, tested)
    get_actor_info: { tier: "PRODUCTION" },
    get_actor_transform: { tier: "PRODUCTION" },
    move_actor: { tier: "PRODUCTION" },
    rotate_actor: { tier: "PRODUCTION" },
    scale_actor: { tier: "PRODUCTION" },
    delete_actor: { tier: "PRODUCTION" },
    set_actor_transform: { tier: "PRODUCTION" },
    duplicate_actor: { tier: "PRODUCTION" },
    find_actor_by_label: { tier: "PRODUCTION" },
    get_selected_actors: { tier: "PRODUCTION" },
    set_actor_material: { tier: "PRODUCTION" },
    set_actor_color: { tier: "PRODUCTION" },
    set_actor_property: { tier: "PRODUCTION" },
    get_actors_in_radius: { tier: "PRODUCTION" },
    select_actors: { tier: "PRODUCTION" },
    // Viewport & camera control
    set_viewport_location: { tier: "PRODUCTION" },
    focus_actor: { tier: "PRODUCTION" },
    look_at_and_capture: { tier: "PRODUCTION" },
    capture_viewport_sync: { tier: "PRODUCTION" },
    capture_viewport_safe: { tier: "PRODUCTION" },
    // Lighting
    create_light: { tier: "PRODUCTION" },
    // Level management
    save_level: { tier: "PRODUCTION" },
    save_scene_checkpoint: { tier: "PRODUCTION" },
    restore_scene_checkpoint: { tier: "PRODUCTION" },
    save_level_as: { tier: "PRODUCTION" },
    create_level: { tier: "PRODUCTION" },
    load_level: { tier: "PRODUCTION" },
    // Play In Editor
    is_pie_running: { tier: "PRODUCTION" },
    start_pie: { tier: "PRODUCTION" },
    stop_pie: { tier: "PRODUCTION" },
    // Blueprint editing
    open_blueprint: { tier: "PRODUCTION" },
    add_blueprint_component: { tier: "PRODUCTION" },
    add_blueprint_variable: { tier: "PRODUCTION" },
    add_blueprint_event: { tier: "PRODUCTION" },
    add_blueprint_function: { tier: "PRODUCTION" },
    add_blueprint_node: { tier: "PRODUCTION" },
    compile_blueprint: { tier: "PRODUCTION" },
    export_blueprint_text: { tier: "PRODUCTION" },
    create_blueprint_snapshot: { tier: "PRODUCTION" },
    compare_blueprint_snapshots: { tier: "PRODUCTION" },
    list_blueprint_snapshots: { tier: "PRODUCTION" },
    remove_blueprint_node: { tier: "PRODUCTION" },
    replace_blueprint_node: { tier: "PRODUCTION" },
    set_blueprint_pin_default: { tier: "PRODUCTION" },
    rename_blueprint_variable: { tier: "PRODUCTION" },
    reparent_blueprint: { tier: "PRODUCTION" },
    batch_compile_blueprints: { tier: "PRODUCTION" },
    repair_blueprint_compile_errors: { tier: "PRODUCTION" },
    validate_and_repair_blueprint_loop: { tier: "PRODUCTION" },
    // Blueprint → C++ conversion (killer feature)
    convert_blueprint_to_cpp: { tier: "PRODUCTION" },
    check_blueprint_convertibility: { tier: "PRODUCTION" },
    // Planning workflows
    create_plan: { tier: "PRODUCTION" },
    execute_plan: { tier: "PRODUCTION" },
    get_plan_status: { tier: "PRODUCTION" },
    // Asset search & NL resolution
    analyze_asset_dependencies: { tier: "PRODUCTION" },
    find_assets: { tier: "PRODUCTION" },
    search_project_assets: { tier: "PRODUCTION" },
    rebuild_asset_index: { tier: "PRODUCTION" },
    resolve_asset: { tier: "PRODUCTION" },
    // Semantic dependency graph (manual MCP composites over execute_python)
    query_dependencies: { tier: "PRODUCTION" },
    blast_radius: { tier: "PRODUCTION" },
    // Physics & traces
    line_trace: { tier: "PRODUCTION" },
    // Landscape & terrain
    create_landscape: { tier: "PRODUCTION" },
    sculpt_landscape: { tier: "PRODUCTION" },
    paint_landscape_layer: { tier: "PRODUCTION" },
    add_landscape_layer: { tier: "PRODUCTION" },
    apply_landscape_material: { tier: "PRODUCTION" },
    create_landscape_material: { tier: "PRODUCTION" },
    set_component_property: { tier: "PRODUCTION" },
    // Post-processing
    create_post_process_volume: { tier: "PRODUCTION" },
    set_post_process_settings: { tier: "PRODUCTION" },
    // Foliage & vegetation
    paint_foliage: { tier: "PRODUCTION" },
    create_landscape_grass_type: { tier: "PRODUCTION" },
    add_grass_variety: { tier: "PRODUCTION" },
    add_foliage_instance: { tier: "PRODUCTION" },
    ground_foliage_to_landscape: { tier: "PRODUCTION" },
    scatter_ecology_trees: { tier: "PRODUCTION", reason: "Ecology-aware tree placement is required for riverbank palms and biome-correct canopy placement, and now uses typed UE 5.7 water bodies for riparian scoring" },
    create_procedural_foliage_spawner: { tier: "PRODUCTION" },
    spawn_procedural_foliage_volume: { tier: "PRODUCTION" },
    resimulate_procedural_foliage: { tier: "PRODUCTION" },
    inspect_procedural_foliage: { tier: "PRODUCTION" },
    set_foliage_instance_lifecycle: { tier: "PRODUCTION" },
    promote_foliage_to_dynamic_actors: { tier: "PRODUCTION" },
    set_dynamic_foliage_actor_state: { tier: "PRODUCTION" },
    // Environment intelligence — useful but not essential for default surface
    critique_scene: { tier: "BETA", reason: "AI art director — use observe_ue_project + analyze_scene_screenshot for core vision loop" },
    apply_weathering: { tier: "BETA", reason: "Surface aging — real C++ tool, callable via BETA surface" },
    paint_biome: { tier: "BETA", reason: "Multi-layer painting — use paint_landscape_layer for individual layers" },
    // Materials
    create_pbr_material: { tier: "PRODUCTION" },
    create_material_instance: { tier: "PRODUCTION" },
    set_material_parameter: { tier: "PRODUCTION" },
    // Clouds
    set_cloud_properties: { tier: "BETA", reason: "Generated passthrough in v1 packaging — keep opt-in until the MCP path is curated end-to-end" },
    // Lighting workflows (setup_outdoor_lighting promoted to PRODUCTION in Lighting domain section below)
    build_lighting: { tier: "PRODUCTION", reason: "Real generated backend, but demoted from the default surface until explicit route-registry proof exists" },
    // Console commands (filtered by allowlist)
    execute_console_command: { tier: "PRODUCTION" },
    // Replication configuration (non-Iris actor replication + multi-client PIE)
    set_actor_replication_config: { tier: "BETA", reason: "New tool — sets core AActor replication properties without Iris" },
    start_multi_client_pie: { tier: "BETA", reason: "New tool — launches PIE with multiple clients + dedicated server" },
    // Scene coherence (cross-domain audit)
    audit_scene_coherence: { tier: "BETA", reason: "New tool — cross-domain color/lighting/audio consistency audit" },
    // Flow analysis (navmesh-based spatial analysis)
    analyze_player_flow: { tier: "BETA", reason: "New tool — NavMesh path analysis, chokepoint detection" },
    audit_cover_layout: { tier: "BETA", reason: "New tool — cover density and sightline analysis" },
    // Art direction enforcement
    enforce_color_palette: { tier: "BETA", reason: "New tool — validates scene colors against reference hex palette" },
    // Performance profiling (draw call breakdown + trace bookmarks)
    analyze_draw_calls: { tier: "BETA", reason: "New tool — per-type draw call and triangle breakdown" },
    capture_trace_bookmark: { tier: "BETA", reason: "New tool — insert named bookmarks into Unreal Insights trace" },
    // C++ / asset pipeline
    create_cpp_class: { tier: "PRODUCTION" },
    create_source_file: { tier: "PRODUCTION" },
    edit_source_file: { tier: "PRODUCTION" },
    validate_cpp_class_spec: { tier: "PRODUCTION" },
    generate_actor_class_from_description: { tier: "PRODUCTION" },
    generate_character_class_from_description: { tier: "PRODUCTION" },
    generate_component_class_from_description: { tier: "PRODUCTION" },
    generate_controller_class_from_description: { tier: "PRODUCTION" },
    generate_gamemode_class_from_description: { tier: "PRODUCTION" },
    generate_subsystem_class_from_description: { tier: "PRODUCTION" },
    import_asset_from_url: { tier: "PRODUCTION" },
    // ── BETA: works but needs more testing or has higher risk ──
    // Geometry modeling (real UE 5.7 Geometry Script surface, but not yet production-hardened)
    create_dynamic_mesh_actor: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    generate_box_mesh: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    generate_sphere_mesh: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    generate_cylinder_mesh: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    generate_capsule_mesh: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    generate_torus_mesh: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    generate_plane_mesh: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    generate_linear_stairs_mesh: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    generate_curved_stairs_mesh: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    generate_revolve_polygon_mesh: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    generate_spiral_revolve_polygon_mesh: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    generate_swept_polygon_mesh: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    generate_spline_swept_polygon_mesh: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    generate_extruded_polygon_mesh: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    generate_tapered_extruded_polygon_mesh: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — purpose-built for pyramids, obelisks, pylons, and other monument-grade tapered forms" },
    create_grass_card_mesh: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    apply_mesh_boolean: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    apply_mesh_self_union: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    apply_mesh_plane_cut: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    apply_mesh_plane_slice: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    apply_mesh_mirror: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    apply_mesh_noise: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    apply_mesh_shell: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    apply_mesh_smooth: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    apply_mesh_uniform_remesh: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    apply_mesh_uniform_tessellation: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    apply_mesh_pn_tessellation: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    apply_mesh_selective_tessellation: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    weld_mesh_edges: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    fill_mesh_holes: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    repair_mesh_degenerate_geometry: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    split_mesh_bowties: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    auto_generate_mesh_uvs: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    auto_repair_mesh_normals: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    get_dynamic_mesh_stats: { tier: "PRODUCTION" },
    select_mesh_faces_by_normal: { tier: "PRODUCTION" },
    select_mesh_faces_by_material: { tier: "PRODUCTION" },
    select_mesh_connected_region: { tier: "PRODUCTION" },
    extrude_mesh_selection: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    inset_outset_mesh_selection: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    bevel_mesh_selection: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    duplicate_mesh_selection: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    disconnect_mesh_selection: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    delete_mesh_selection: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    generate_mesh_collision: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script collision surface — callable and governed, but still needs broader end-to-end hardening" },
    recompute_mesh_normals: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    set_static_mesh_nanite: { tier: "PRODUCTION", reason: "Editor asset-hardening surface — callable and governed, but still needs broader end-to-end hardening" },
    create_roman_column: { tier: "PRODUCTION", reason: "Roman modular kit generator built on the governed UE 5.7 Geometry Script surface — real, but still needs broader end-to-end hardening" },
    create_roman_arch: { tier: "PRODUCTION", reason: "Roman modular kit generator built on the governed UE 5.7 Geometry Script surface — real, but still needs broader end-to-end hardening" },
    create_roman_wall_bay: { tier: "PRODUCTION", reason: "Roman modular kit generator built on the governed UE 5.7 Geometry Script surface — real, but still needs broader end-to-end hardening" },
    create_roman_street_grid: { tier: "PRODUCTION", reason: "Roman district layout surface — real and governed, but still needs broader end-to-end hardening" },
    compose_roman_forum: { tier: "PRODUCTION", reason: "Roman forum composition surface — real and governed, but still needs broader end-to-end hardening" },
    compose_roman_insula_block: { tier: "PRODUCTION", reason: "Roman modular district-composition surface — real and governed, but still needs broader end-to-end hardening" },
    compose_roman_district: { tier: "PRODUCTION", reason: "Roman district build surface — real and governed, but still needs broader end-to-end hardening" },
    audit_roman_district: { tier: "PRODUCTION", reason: "Roman-specific audit surface — real and governed, but still needs broader end-to-end hardening" },
    dress_roman_district: { tier: "PRODUCTION", reason: "Roman environment-dressing surface — real and governed, but still needs broader end-to-end hardening" },
    review_roman_district: { tier: "PRODUCTION", reason: "Roman observe/build/verify review surface — real and governed, but still needs broader end-to-end hardening" },
    create_building_from_floor_plan: { tier: "PRODUCTION", reason: "Architecture compiler surface — real and governed on the UE editor side, but still needs broader end-to-end hardening" },
    create_staircase: { tier: "PRODUCTION", reason: "Architecture compiler surface — real and governed on the UE editor side, but still needs broader end-to-end hardening" },
    create_spline_architecture: { tier: "PRODUCTION", reason: "Architecture compiler surface — real and governed on the UE editor side, but still needs broader end-to-end hardening" },
    set_mesh_material: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    convert_static_to_dynamic: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    export_dynamic_to_static: { tier: "PRODUCTION", reason: "UE 5.7 Geometry Script modeling surface — callable and governed, but still needs broader end-to-end hardening" },
    capture_level_snapshot: { tier: "PRODUCTION", reason: "UE 5.7 Level Snapshots surface — real and governed, but still needs broader end-to-end hardening" },
    restore_level_snapshot: { tier: "PRODUCTION", reason: "UE 5.7 Level Snapshots recovery surface — real and governed, but still needs broader end-to-end hardening" },
    list_level_snapshots: { tier: "PRODUCTION", reason: "UE 5.7 Level Snapshots inspection surface — real and governed, but still needs broader end-to-end hardening" },
    spawn_mass_crowd_spawner: { tier: "PRODUCTION", reason: "UE 5.7 MassCrowd spawner surface — real and governed, but still needs broader end-to-end hardening" },
    run_mass_crowd_spawner: { tier: "PRODUCTION", reason: "UE 5.7 MassCrowd execution surface — real and governed, but still needs broader end-to-end hardening" },
    inspect_mass_crowd_state: { tier: "PRODUCTION", reason: "UE 5.7 MassCrowd inspection surface — real and governed, but still needs broader end-to-end hardening" },
    set_mass_crowd_lane_state: { tier: "PRODUCTION", reason: "UE 5.7 MassCrowd lane-control surface — real and governed, but still needs broader end-to-end hardening" },
    write_niagara_data_channel_batch: { tier: "PRODUCTION", reason: "UE 5.7 Niagara Data Channels write surface — real and governed, but still needs broader end-to-end hardening" },
    read_niagara_data_channel_batch: { tier: "PRODUCTION" },
    create_soundscape_metasound_source: { tier: "PRODUCTION", reason: "UE 5.7 MetaSound Builder surface — real and governed, but still needs broader end-to-end hardening" },
    create_movie_graph_config: { tier: "PRODUCTION", reason: "UE 5.7 Movie Graph editor surface — real and governed, but still needs broader end-to-end hardening" },
    queue_trailer_render_job: { tier: "PRODUCTION", reason: "UE 5.7 Movie Render Pipeline editor surface — real and governed, but still needs broader end-to-end hardening" },
    create_city_state_data_layers: { tier: "PRODUCTION", reason: "UE 5.7 Data Layer orchestration surface — real and governed, but still needs broader end-to-end hardening" },
    set_city_state_runtime: { tier: "PRODUCTION", reason: "UE 5.7 Data Layer runtime-state surface — real and governed, but still needs broader end-to-end hardening" },
    create_geometry_foundry_graph: { tier: "PRODUCTION", reason: "UE 5.7 PCG Geometry Script interop surface — real and governed, but still needs broader end-to-end hardening" },
    spawn_geometry_foundry_volume: { tier: "PRODUCTION", reason: "UE 5.7 PCG foundry execution surface — real and governed, but still needs broader end-to-end hardening" },
    add_chaos_character_mover: { tier: "PRODUCTION", reason: "UE 5.7 ChaosMover integration surface — real and governed, but still needs broader end-to-end hardening" },
    queue_chaos_mover_mode: { tier: "PRODUCTION", reason: "UE 5.7 ChaosMover mode queue surface — real and governed, but still needs broader end-to-end hardening" },
    launch_chaos_mover: { tier: "PRODUCTION", reason: "UE 5.7 ChaosMover launch surface — real and governed, but still needs broader end-to-end hardening" },
    override_chaos_mover_settings: { tier: "PRODUCTION", reason: "UE 5.7 ChaosMover settings override surface — real and governed, but still needs broader end-to-end hardening" },
    // Variant universe switchboard
    create_level_variant_sets_asset: { tier: "PRODUCTION", reason: "UE 5.7 Variant Manager asset-authoring surface — real and governed, but still needs broader end-to-end hardening" },
    create_level_variant_sets_actor: { tier: "PRODUCTION", reason: "UE 5.7 Variant Manager actor-binding surface — real and governed, but still needs broader end-to-end hardening" },
    add_variant_set: { tier: "PRODUCTION", reason: "UE 5.7 Variant Manager authoring surface — real and governed, but still needs broader end-to-end hardening" },
    add_variant: { tier: "PRODUCTION", reason: "UE 5.7 Variant Manager authoring surface — real and governed, but still needs broader end-to-end hardening" },
    bind_actor_to_variant: { tier: "PRODUCTION", reason: "UE 5.7 Variant Manager actor-binding surface — real and governed, but still needs broader end-to-end hardening" },
    capture_variant_property: { tier: "PRODUCTION", reason: "UE 5.7 Variant Manager property-capture surface — real and governed, now using the standard CaptureProperty path first with a fallback for nested component/index captures, but still needs broader end-to-end hardening" },
    switch_variant_by_name: { tier: "PRODUCTION", reason: "UE 5.7 Variant Manager runtime switching surface — real and governed, but still needs broader end-to-end hardening" },
    // Remote-control world cockpit
    create_remote_control_preset: { tier: "PRODUCTION", reason: "UE 5.7 Remote Control preset-authoring surface — real and governed, but still needs broader end-to-end hardening" },
    expose_actor_to_remote_control: { tier: "PRODUCTION", reason: "UE 5.7 Remote Control actor-exposure surface — real and governed, but still needs broader end-to-end hardening" },
    expose_property_to_remote_control: { tier: "PRODUCTION", reason: "UE 5.7 Remote Control property-exposure surface — real and governed, but still needs broader end-to-end hardening" },
    expose_function_to_remote_control: { tier: "PRODUCTION", reason: "UE 5.7 Remote Control function-exposure surface — real and governed, but still needs broader end-to-end hardening" },
    apply_remote_control_color_wheel_delta: { tier: "PRODUCTION", reason: "UE 5.7 Remote Control color-wheel surface — real and governed, but still needs broader end-to-end hardening" },
    apply_remote_control_color_grading_delta: { tier: "PRODUCTION", reason: "UE 5.7 Remote Control grading surface — real and governed, but still needs broader end-to-end hardening" },
    // Smart-object society compiler
    register_smart_object_actor: { tier: "PRODUCTION", reason: "UE 5.7 Smart Objects registration surface — real and governed, but still needs broader end-to-end hardening" },
    set_smart_object_actor_enabled: { tier: "PRODUCTION", reason: "UE 5.7 Smart Objects actor-state surface — real and governed, but still needs broader end-to-end hardening" },
    inspect_smart_object_actor: { tier: "PRODUCTION" },
    set_smart_object_slot_enabled: { tier: "PRODUCTION", reason: "UE 5.7 Smart Objects slot-state surface — real and governed, but still needs broader end-to-end hardening" },
    claim_smart_object_slot: { tier: "PRODUCTION", reason: "UE 5.7 Smart Objects claim surface — real and governed, but claim handles are session-scoped and need broader end-to-end hardening" },
    occupy_smart_object_slot: { tier: "PRODUCTION", reason: "UE 5.7 Smart Objects occupancy surface — real and governed, but uses session-scoped claim handles and still needs broader end-to-end hardening" },
    release_smart_object_slot: { tier: "PRODUCTION", reason: "UE 5.7 Smart Objects release surface — real and governed, but uses session-scoped claim handles and still needs broader end-to-end hardening" },
    send_smart_object_slot_event: { tier: "PRODUCTION", reason: "UE 5.7 Smart Objects event surface — real and governed, but still needs broader end-to-end hardening" },
    // Motion-matching combat director
    create_pose_search_database: { tier: "PRODUCTION", reason: "UE 5.7 Pose Search authoring surface — real and governed, but still needs broader end-to-end hardening" },
    add_animation_to_pose_search_database: { tier: "PRODUCTION", reason: "UE 5.7 Pose Search database authoring surface — real and governed, but still needs broader end-to-end hardening" },
    build_pose_search_database_index: { tier: "PRODUCTION", reason: "UE 5.7 Pose Search index build surface — real and governed, but still needs broader end-to-end hardening" },
    // Control Rig fabricator / auto-rig factory
    create_control_rig_asset: { tier: "PRODUCTION", reason: "UE 5.7 Control Rig authoring surface — real and governed, but still needs broader end-to-end hardening" },
    create_control_rig_from_skeletal_mesh: { tier: "PRODUCTION", reason: "UE 5.7 Control Rig import surface — real and governed, but still needs broader end-to-end hardening" },
    add_control_rig_bone: { tier: "PRODUCTION", reason: "UE 5.7 Rig hierarchy authoring surface — real and governed, but still needs broader end-to-end hardening" },
    add_control_rig_null: { tier: "PRODUCTION", reason: "UE 5.7 Rig hierarchy authoring surface — real and governed, but still needs broader end-to-end hardening" },
    add_control_rig_control: { tier: "PRODUCTION", reason: "UE 5.7 Rig hierarchy authoring surface — real and governed, but still needs broader end-to-end hardening" },
    add_control_rig_variable_node: { tier: "PRODUCTION", reason: "UE 5.7 RigVM graph-authoring surface — real and governed, but still needs broader end-to-end hardening" },
    add_control_rig_branch_node: { tier: "PRODUCTION", reason: "UE 5.7 RigVM graph-authoring surface — real and governed, but still needs broader end-to-end hardening" },
    add_control_rig_comment_node: { tier: "PRODUCTION", reason: "UE 5.7 RigVM graph-authoring surface — real and governed, but still needs broader end-to-end hardening" },
    link_control_rig_pins: { tier: "PRODUCTION", reason: "UE 5.7 RigVM pin-link surface — real and governed, but still needs broader end-to-end hardening" },
    set_control_rig_pin_default: { tier: "PRODUCTION", reason: "UE 5.7 RigVM pin-default surface — real and governed, but still needs broader end-to-end hardening" },
    set_ik_retarget_root: { tier: "PRODUCTION", reason: "UE 5.7 IK Rig authoring surface — real and governed, but still needs broader end-to-end hardening" },
    add_ik_retarget_chain: { tier: "PRODUCTION", reason: "UE 5.7 IK Rig authoring surface — real and governed, but still needs broader end-to-end hardening" },
    auto_map_retarget_chains: { tier: "PRODUCTION", reason: "UE 5.7 IK Retargeter authoring surface — real and governed, but still needs broader end-to-end hardening" },
    create_retarget_pose: { tier: "PRODUCTION", reason: "UE 5.7 IK Retargeter pose-authoring surface — real and governed, but still needs broader end-to-end hardening" },
    set_current_retarget_pose: { tier: "PRODUCTION", reason: "UE 5.7 IK Retargeter pose-selection surface — real and governed, but still needs broader end-to-end hardening" },
    set_retarget_pose_rotation_offset: { tier: "PRODUCTION", reason: "UE 5.7 IK Retargeter pose-offset surface — real and governed, but still needs broader end-to-end hardening" },
    // Neural-morph performance layer
    create_neural_morph_asset: { tier: "PRODUCTION", reason: "UE 5.7 ML Deformer / Neural Morph authoring surface — real and governed, but still needs broader end-to-end hardening" },
    configure_neural_morph_asset: { tier: "PRODUCTION", reason: "UE 5.7 ML Deformer / Neural Morph config surface — real and governed, but still needs broader end-to-end hardening" },
    attach_neural_morph_component: { tier: "PRODUCTION", reason: "UE 5.7 ML Deformer runtime-component surface — real and governed, but still needs broader end-to-end hardening" },
    // USD live twin
    spawn_usd_stage_actor: { tier: "PRODUCTION", reason: "UE 5.7 USD Stage spawn surface — real and governed, but still needs broader end-to-end hardening" },
    set_usd_stage_root_layer: { tier: "PRODUCTION", reason: "UE 5.7 USD Stage root-layer surface — real and governed, but still needs broader end-to-end hardening" },
    configure_usd_stage_import: { tier: "PRODUCTION", reason: "UE 5.7 USD Stage import-config surface — real and governed, but still needs broader end-to-end hardening" },
    set_usd_stage_time: { tier: "PRODUCTION", reason: "UE 5.7 USD Stage time-scrub surface — real and governed, but still needs broader end-to-end hardening" },
    // Chaos field destruction painter
    add_field_system_component: { tier: "PRODUCTION", reason: "UE 5.7 Field System setup surface — real and governed, but still needs broader end-to-end hardening" },
    apply_linear_force_field: { tier: "PRODUCTION", reason: "UE 5.7 Field System linear-force surface — real and governed, but still needs broader end-to-end hardening" },
    apply_radial_force_field: { tier: "PRODUCTION", reason: "UE 5.7 Field System radial-force surface — real and governed, but still needs broader end-to-end hardening" },
    apply_uniform_force_field: { tier: "PRODUCTION", reason: "UE 5.7 Field System uniform-force surface — real and governed, but still needs broader end-to-end hardening" },
    add_persistent_radial_falloff_field: { tier: "PRODUCTION", reason: "UE 5.7 Field System persistent-field surface — real and governed, but still needs broader end-to-end hardening" },
    reset_field_system: { tier: "PRODUCTION", reason: "UE 5.7 Field System reset surface — real and governed, but still needs broader end-to-end hardening" },
    // Gameplay camera brain
    create_gameplay_camera_asset: { tier: "PRODUCTION", reason: "UE 5.7 Gameplay Cameras asset-authoring surface — real and governed, but still needs broader end-to-end hardening" },
    create_camera_rig_asset: { tier: "PRODUCTION", reason: "UE 5.7 Gameplay Cameras rig-authoring surface — real and governed, but still needs broader end-to-end hardening" },
    add_gameplay_camera_component: { tier: "PRODUCTION", reason: "UE 5.7 Gameplay Cameras component surface — real and governed, but still needs broader end-to-end hardening" },
    set_gameplay_camera_asset: { tier: "PRODUCTION", reason: "UE 5.7 Gameplay Cameras camera-binding surface — real and governed, but still needs broader end-to-end hardening" },
    activate_persistent_camera_rig: { tier: "PRODUCTION", reason: "UE 5.7 Gameplay Cameras rig-activation surface — real and governed via checked UFUNCTION dispatch because the declared component layer APIs are not link-safe across modules, but still needs broader end-to-end hardening" },
    // Chooser brain
    // Live balance fabric
    register_data_registry_asset: { tier: "PRODUCTION", reason: "UE 5.7 Data Registry asset-registration surface — real and governed, but still needs broader end-to-end hardening" },
    // Contextual interaction composer
    create_contextual_anim_bindings_for_two_actors: { tier: "PRODUCTION", reason: "UE 5.7 Contextual Animation bindings surface — real and governed, with reconstructable handles over an experimental engine plugin, but still needs broader end-to-end hardening" },
    calculate_contextual_anim_warp_points: { tier: "PRODUCTION", reason: "UE 5.7 Contextual Animation warp-point surface — real and governed, but still needs broader end-to-end hardening" },
    activate_contextual_anim_warp_targets: { tier: "PRODUCTION", reason: "UE 5.7 Contextual Animation warp-target activation surface — real and governed, but still needs broader end-to-end hardening" },
    // Learning Agents auto-tuner
    make_learning_agents_imitation_trainer: { tier: "PRODUCTION", reason: "UE 5.7 Learning Agents trainer-construction surface — real and governed, but the underlying trainer remains session-scoped on an experimental engine plugin" },
    begin_learning_agents_training: { tier: "PRODUCTION", reason: "UE 5.7 Learning Agents training-start surface — real and governed, and can recreate missing transient trainers from the handle recipe before starting" },
    iterate_learning_agents_training: { tier: "PRODUCTION", reason: "UE 5.7 Learning Agents training-iteration surface — real and governed, but only valid for an actively training trainer in the current editor session" },
    end_learning_agents_training: { tier: "PRODUCTION", reason: "UE 5.7 Learning Agents training-stop surface — real and governed, but only valid for an actively training trainer in the current editor session" },
    // Take recorder factory
    set_take_recorder_target_sequence: { tier: "PRODUCTION", reason: "UE 5.7 Take Recorder sequence-target surface — real and governed, but still needs broader end-to-end hardening" },
    add_take_recorder_actor_source: { tier: "PRODUCTION", reason: "UE 5.7 Take Recorder actor-source surface — real and governed, but still needs broader end-to-end hardening" },
    start_take_recording: { tier: "PRODUCTION", reason: "UE 5.7 Take Recorder start surface — real and governed, but still needs broader end-to-end hardening" },
    stop_take_recording: { tier: "PRODUCTION", reason: "UE 5.7 Take Recorder stop surface — real and governed, but still needs broader end-to-end hardening" },
    // Live Link performance router
    set_live_link_subject_enabled: { tier: "PRODUCTION", reason: "UE 5.7 Live Link subject-toggle surface — real and governed, but still needs broader end-to-end hardening" },
    pause_live_link_subject: { tier: "PRODUCTION", reason: "UE 5.7 Live Link subject-pause surface — real and governed, but still needs broader end-to-end hardening" },
    unpause_live_link_subject: { tier: "PRODUCTION", reason: "UE 5.7 Live Link subject-unpause surface — real and governed, but still needs broader end-to-end hardening" },
    // Replication contract layer
    create_replication_group: { tier: "PRODUCTION", reason: "UE 5.7 Iris replication-group surface — real and governed, but still needs broader end-to-end hardening" },
    add_actor_to_replication_group: { tier: "PRODUCTION", reason: "UE 5.7 Iris replication-group membership surface — real and governed, but still needs broader end-to-end hardening" },
    set_replication_group_filter_status: { tier: "PRODUCTION", reason: "UE 5.7 Iris replication filter surface — real and governed, but still needs broader end-to-end hardening" },
    set_actor_cull_distance_override: { tier: "PRODUCTION", reason: "UE 5.7 Iris cull-distance override surface — real and governed, but still needs broader end-to-end hardening" },
    force_actor_net_update: { tier: "PRODUCTION", reason: "UE 5.7 Iris force-update surface — real and governed, but still needs broader end-to-end hardening" },
    // World rule fabric
    // Python execution (promoted 2026-04-06: needed for material graph wiring that has no dedicated tool)
    execute_python: { tier: "PRODUCTION" },
    // ── EditorSurface tools (32 real C++ implementations in ProjectToolsModule_EditorSurfaces.cpp, 2744 lines) ──
    // The Blueprint, Material, Sequencer, Control Rig, Niagara, Widget, and PCG context lanes are
    // default-visible after live asset-editor proof was restored.
    get_editor_focus_state: { tier: "PRODUCTION" },
    get_editor_control_state: { tier: "PRODUCTION" },
    get_blueprint_editor_context: { tier: "PRODUCTION" },
    get_widget_editor_context: { tier: "PRODUCTION" },
    focus_asset_editor: { tier: "PRODUCTION" },
    focus_editor_tab: { tier: "PRODUCTION" },
    list_blueprint_graphs: { tier: "PRODUCTION" },
    list_blueprint_nodes: { tier: "PRODUCTION" },
    find_blueprint_nodes: { tier: "PRODUCTION" },
    get_blueprint_compile_diagnostics: { tier: "PRODUCTION" },
    assert_blueprint_compiles: { tier: "PRODUCTION" },
    focus_blueprint_node: { tier: "PRODUCTION" },
    get_material_editor_context: { tier: "PRODUCTION" },
    list_material_expressions: { tier: "PRODUCTION" },
    inspect_material_expression: { tier: "PRODUCTION" },
    assert_material_compiles: { tier: "PRODUCTION" },
    set_material_expression_property: { tier: "PRODUCTION" },
    delete_material_expression: { tier: "PRODUCTION" },
    replace_material_expression: { tier: "PRODUCTION" },
    add_material_function_call: { tier: "PRODUCTION" },
    connect_material_attributes: { tier: "PRODUCTION" },
    reparent_material_instance: { tier: "PRODUCTION" },
    batch_compile_materials: { tier: "PRODUCTION" },
    recompile_material_asset: { tier: "BETA", reason: "EditorSurface: forces material recompile" },
    layout_material_asset_expressions: { tier: "BETA", reason: "EditorSurface: auto-layouts material graph nodes" },
    get_sequencer_editor_context: { tier: "PRODUCTION" },
    list_sequence_bindings: { tier: "PRODUCTION" },
    assert_sequence_binding_exists: { tier: "PRODUCTION" },
    assert_sequencer_selection: { tier: "BETA", reason: "EditorSurface: asserts selected keys/sections" },
    add_selected_actors_to_active_sequence: { tier: "BETA", reason: "EditorSurface: binds selected actors into active sequence" },
    get_control_rig_editor_context: { tier: "PRODUCTION" },
    list_control_rig_controls: { tier: "PRODUCTION" },
    assert_control_rig_selection: { tier: "PRODUCTION" },
    select_control_rig_control: { tier: "PRODUCTION" },
    get_niagara_editor_context: { tier: "PRODUCTION" },
    get_niagara_stack_context: { tier: "PRODUCTION" },
    list_niagara_modules: { tier: "PRODUCTION" },
    assert_niagara_compiles: { tier: "PRODUCTION" },
    get_pcg_editor_context: { tier: "PRODUCTION" },
    list_pcg_nodes: { tier: "PRODUCTION" },
    inspect_pcg_node: { tier: "PRODUCTION" },
    assert_pcg_graph_valid: { tier: "PRODUCTION" },
    // ── ControlCore tools (21 real C++ implementations in ProjectToolsModule_ControlCore.cpp) ──
    // Default-lane control spine: blocker detection, diagnostics, and reflected control loops.
    get_output_log_context: { tier: "PRODUCTION" },
    get_message_log_context: { tier: "PRODUCTION" },
    drain_log_alerts: { tier: "PRODUCTION" },
    get_notification_center_state: { tier: "PRODUCTION" },
    get_modal_blockers: { tier: "PRODUCTION" },
    assert_no_modal_blockers: { tier: "PRODUCTION" },
    list_editor_windows: { tier: "PRODUCTION" },
    focus_editor_window: { tier: "PRODUCTION" },
    minimize_editor_window: { tier: "PRODUCTION" },
    restore_editor_window: { tier: "PRODUCTION" },
    resize_editor_window: { tier: "PRODUCTION" },
    close_editor_window: { tier: "PRODUCTION" },
    assert_output_log_clean: { tier: "BETA", reason: "ControlCore: asserts no errors/warnings in Output Log" },
    assert_asset_dirty_state: { tier: "PRODUCTION" },
    assert_actor_selection: { tier: "PRODUCTION" },
    assert_editor_focus: { tier: "PRODUCTION" },
    get_world_outliner_context: { tier: "PRODUCTION" },
    assert_object_property_equals: { tier: "BETA", reason: "ControlCore: asserts UObject property value" },
    get_compile_diagnostics: { tier: "PRODUCTION" },
    list_object_properties: { tier: "PRODUCTION" },
    get_object_property_typed: { tier: "PRODUCTION" },
    set_object_property_typed: { tier: "PRODUCTION" },
    call_reflected_function: { tier: "PRODUCTION" },
    save_dirty_assets: { tier: "PRODUCTION" },
    checkout_asset: { tier: "BETA", reason: "ControlCore: checks out asset from source control" },
    revert_asset: { tier: "BETA", reason: "ControlCore: reverts asset to source control version" },
    // ── BETA: curated schema in manual TOOLS array, auto-routed via generated fallback ──
    generate_project_insights: { tier: "PRODUCTION" },
    create_anim_blueprint: { tier: "PRODUCTION" },
    create_anim_montage: { tier: "PRODUCTION" },
    create_blend_space: { tier: "PRODUCTION" },
    get_animation_assets: { tier: "PRODUCTION", reason: "Generated helper remains callable, but is not on the default surface until route proof exists" },
    get_skeletons: { tier: "PRODUCTION", reason: "Generated helper remains callable, but is not on the default surface until route proof exists" },
    play_animation_montage: { tier: "PRODUCTION" },
    // Animation extensions (AnimationToolsModule — 4 additional real tools)
    add_anim_state: { tier: "PRODUCTION" },
    add_anim_transition: { tier: "PRODUCTION" },
    set_anim_slot: { tier: "PRODUCTION" },
    get_skeleton_bones: { tier: "PRODUCTION" },
    get_audio_assets: { tier: "PRODUCTION" },
    create_sound_cue: { tier: "PRODUCTION" },
    play_sound_at_location: { tier: "PRODUCTION" },
    play_sound_2d: { tier: "PRODUCTION" },
    stop_all_sounds: { tier: "PRODUCTION" },
    list_sounds: { tier: "PRODUCTION" },
    spawn_audio_component: { tier: "PRODUCTION" },
    // MetaSound (MetaSoundToolsModule — 11 real tools using MetasoundFrontend API)
    create_metasound: { tier: "PRODUCTION" },
    create_metasound_source: { tier: "PRODUCTION" },
    create_metasound_preset: { tier: "PRODUCTION" },
    list_metasounds: { tier: "PRODUCTION" },
    get_metasound_info: { tier: "PRODUCTION" },
    add_metasound_node: { tier: "PRODUCTION" },
    connect_metasound_nodes: { tier: "PRODUCTION" },
    set_metasound_input: { tier: "PRODUCTION" },
    play_metasound_preview: { tier: "PRODUCTION" },
    list_metasound_node_types: { tier: "PRODUCTION" },
    set_physics_enabled: { tier: "PRODUCTION" },
    set_collision_profile: { tier: "PRODUCTION" },
    apply_impulse: { tier: "PRODUCTION" },
    apply_force: { tier: "PRODUCTION" },
    set_mass: { tier: "PRODUCTION" },
    get_physics_info: { tier: "PRODUCTION", reason: "Generated helper remains callable, but is not on the default surface until route proof exists" },
    add_physics_constraint: { tier: "PRODUCTION" },
    get_collision_channels: { tier: "PRODUCTION" },
    create_widget: { tier: "PRODUCTION" },
    get_widget_blueprints: { tier: "PRODUCTION" },
    add_widget_to_viewport: { tier: "PRODUCTION" },
    create_level_sequence: { tier: "PRODUCTION" },
    open_sequence: { tier: "PRODUCTION" },
    add_sequence_track: { tier: "PRODUCTION" },
    add_keyframe: { tier: "PRODUCTION" },
    play_sequence: { tier: "PRODUCTION" },
    stop_sequence: { tier: "PRODUCTION" },
    list_sequences: { tier: "PRODUCTION" },
    get_sequence_info: { tier: "PRODUCTION" },
    create_gameplay_ability: { tier: "PRODUCTION" },
    create_gameplay_effect: { tier: "PRODUCTION" },
    create_attribute_set: { tier: "PRODUCTION" },
    get_gas_assets: { tier: "PRODUCTION" },
    add_ability_to_actor: { tier: "PRODUCTION" },
    configure_gameplay_effect: { tier: "BETA", reason: "Generated helper remains callable, but is not on the default surface until route proof exists" },
    configure_ge_stacking: { tier: "BETA", reason: "Generated helper remains callable, but is not on the default surface until route proof exists" },
    set_ability_cooldown: { tier: "BETA", reason: "Generated helper remains callable, but is not on the default surface until route proof exists" },
    set_ability_cost: { tier: "BETA", reason: "Generated helper remains callable, but is not on the default surface until route proof exists" },
    set_ability_policies: { tier: "BETA", reason: "Generated helper remains callable, but is not on the default surface until route proof exists" },
    create_niagara_system: { tier: "PRODUCTION" },
    create_niagara_from_template: { tier: "PRODUCTION" },
    spawn_niagara_at_location: { tier: "PRODUCTION" },
    spawn_niagara_attached: { tier: "PRODUCTION" },
    list_niagara_systems: { tier: "PRODUCTION" },
    get_niagara_assets: { tier: "PRODUCTION" },
    // Niagara extensions (NiagaraToolsModule — 12 additional real tools)
    spawn_forest_air_particles: { tier: "PRODUCTION" },
    edit_forest_air_particles: { tier: "PRODUCTION" },
    set_niagara_parameter: { tier: "PRODUCTION" },
    preview_niagara: { tier: "PRODUCTION" },
    get_niagara_emitters: { tier: "PRODUCTION" },
    get_niagara_parameters: { tier: "PRODUCTION" },
    activate_niagara: { tier: "PRODUCTION" },
    reset_niagara: { tier: "PRODUCTION" },
    inspect_niagara_system: { tier: "PRODUCTION" },
    diff_niagara_systems: { tier: "PRODUCTION" },
    save_niagara_snapshot: { tier: "PRODUCTION" },
    create_pcg_graph: { tier: "PRODUCTION" },
    spawn_pcg_volume: { tier: "PRODUCTION" },
    execute_pcg: { tier: "BETA", reason: "Generated helper remains callable, but is not on the default surface until route proof exists" },
    list_pcg_graphs: { tier: "BETA", reason: "Generated helper remains callable, but is not on the default surface until route proof exists" },
    get_pcg_info: { tier: "BETA", reason: "Generated helper remains callable, but is not on the default surface until route proof exists" },
    create_input_action: { tier: "PRODUCTION" },
    create_input_mapping_context: { tier: "PRODUCTION" },
    add_input_mapping: { tier: "PRODUCTION" },
    add_input_trigger: { tier: "BETA" },
    add_input_modifier: { tier: "BETA" },
    set_input_action_player_mappable: { tier: "BETA" },
    list_input_mappings: { tier: "BETA" },
    // 2026-04-05: character tool family empirically verified via HTTP bridge against real project assets.
    // The scaffolded core remains default-visible; the convenience wrappers stay beta-gated until
    // explicit route-registry proof catches up with the generated backend.
    // get_engine_mannequins reports project-local discovery (resolver fix); use_manny_mesh and
    // set_character_mesh mutate skeletal mesh + AnimBP on existing characters; create_character_from_third_person
    // now scaffolds a SpringArm+Camera rig and tuned movement defaults; set_playable_character_input_assets wires
    // ARiftbornPlayableCharacter Blueprint defaults to curated Enhanced Input assets.
    // make_character_playable parses gamemode_path robustly, recovers zombie packages, and saves the GameMode BP + level.
    use_manny_mesh: { tier: "BETA", reason: "Convenience wrapper remains callable, but is not on the default surface until route proof exists" },
    get_engine_mannequins: { tier: "BETA", reason: "Convenience wrapper remains callable, but is not on the default surface until route proof exists" },
    set_character_mesh: { tier: "BETA", reason: "Convenience wrapper remains callable, but is not on the default surface until route proof exists" },
    create_character_from_third_person: { tier: "PRODUCTION" },
    set_playable_character_input_assets: { tier: "PRODUCTION" },
    spawn_third_person_character: { tier: "PRODUCTION", reason: "Convenience wrapper remains callable, but is not on the default surface until route proof exists" },
    make_character_playable: { tier: "PRODUCTION", reason: "Convenience wrapper remains callable, but is not on the default surface until route proof exists" },
    get_game_time: { tier: "BETA" },
    set_game_time: { tier: "BETA" },
    set_time_of_day: { tier: "BETA" },
    get_weather: { tier: "BETA" },
    set_weather: { tier: "BETA" },
    set_season: { tier: "BETA" },
    get_temporal_state: { tier: "BETA" },
    get_lighting_recommendations: { tier: "BETA" },
    // ── MMO/Game Building Essentials (promoted from EXPERIMENTAL) ──
    create_session: { tier: "PRODUCTION", reason: "Generated helper remains callable, but is not on the default surface until route proof exists" },
    destroy_session: { tier: "PRODUCTION" },
    find_sessions: { tier: "PRODUCTION", reason: "Generated helper remains callable, but is not on the default surface until route proof exists" },
    join_session: { tier: "PRODUCTION", reason: "Generated helper remains callable, but is not on the default surface until route proof exists" },
    get_online_subsystem_info: { tier: "PRODUCTION", reason: "Generated helper remains callable, but is not on the default surface until route proof exists" },
    create_string_table: { tier: "PRODUCTION", reason: "Generated helper remains callable, but is not on the default surface until route proof exists" },
    add_string_table_entry: { tier: "PRODUCTION", reason: "Generated helper remains callable, but is not on the default surface until route proof exists" },
    // ── World Partition (WorldPartitionToolsModule — 11 real tools using UWorldPartition/UDataLayerManager API) ──
    list_data_layers: { tier: "PRODUCTION" },
    create_data_layer: { tier: "PRODUCTION" },
    set_actor_data_layer: { tier: "PRODUCTION" },
    configure_streaming_source: { tier: "PRODUCTION" },
    get_world_partition_info: { tier: "PRODUCTION" },
    create_hlod_layer: { tier: "PRODUCTION" },
    configure_level_instance: { tier: "PRODUCTION" },
    inspect_world_partition_live_state: { tier: "PRODUCTION" },
    // ── Optimization (OptimizationToolsModule — real HISM merge + perf audit) ──
    merge_static_meshes: { tier: "PRODUCTION" },
    audit_performance: { tier: "PRODUCTION" },
    // ── Material Graph (MaterialGraphToolsModule — 12 real tools, UMaterialExpression API) ──
    add_fresnel_effect: { tier: "PRODUCTION" },
    add_lerp_blend: { tier: "PRODUCTION" },
    add_material_expression: { tier: "PRODUCTION" },
    add_texture_sample: { tier: "PRODUCTION" },
    add_uv_transform: { tier: "PRODUCTION" },
    add_world_position_offset: { tier: "PRODUCTION" },
    connect_material_nodes: { tier: "PRODUCTION" },
    disconnect_material_node: { tier: "PRODUCTION" },
    enable_landscape_auto_grass: { tier: "PRODUCTION" },
    list_material_nodes: { tier: "PRODUCTION" },
    remove_material_expression: { tier: "PRODUCTION" },
    set_expression_value: { tier: "PRODUCTION" },
    // ── Navigation (NavigationToolsModule — real UNavigationSystemV1 API) ──
    build_navmesh: { tier: "PRODUCTION" },
    get_navmesh_status: { tier: "PRODUCTION" },
    add_nav_link: { tier: "PRODUCTION" },
    add_nav_modifier_volume: { tier: "PRODUCTION" },
    annotate_navmesh_tactical: { tier: "PRODUCTION" },
    configure_ai_perception: { tier: "PRODUCTION" },
    get_ai_perception_info: { tier: "PRODUCTION" },
    // ── AI / Behavior Trees (AIToolsModule — real UBehaviorTree/UBlackboardData API) ──
    create_behavior_tree: { tier: "PRODUCTION" },
    create_blackboard: { tier: "PRODUCTION" },
    add_bt_task: { tier: "PRODUCTION" },
    add_bt_decorator: { tier: "PRODUCTION" },
    add_bt_service: { tier: "PRODUCTION" },
    run_behavior_tree: { tier: "PRODUCTION" },
    set_blackboard_key: { tier: "PRODUCTION" },
    create_state_tree: { tier: "PRODUCTION" },
    // ── EQS (EQSToolsModule — real UEnvQuery/UEnvQueryGenerator API) ──
    create_env_query: { tier: "PRODUCTION" },
    add_eqs_generator: { tier: "PRODUCTION" },
    add_eqs_test: { tier: "PRODUCTION" },
    add_ai_sense: { tier: "PRODUCTION" },
    run_eqs_query: { tier: "PRODUCTION" },
    list_eqs_queries: { tier: "PRODUCTION" },
    get_env_queries: { tier: "PRODUCTION" },
    // ── State Tree (StateTreeToolsModule — real UStateTree API) ──
    add_state_tree_state: { tier: "PRODUCTION" },
    animation_state: { tier: "PRODUCTION" },
    get_state_tree_info: { tier: "PRODUCTION" },
    list_state_trees: { tier: "PRODUCTION" },
    // ── Data Tables (DataTableToolsModule — real UDataTable API) ──
    create_datatable: { tier: "PRODUCTION" },
    add_datatable_row: { tier: "PRODUCTION" },
    get_datatable_rows: { tier: "PRODUCTION" },
    create_data_asset: { tier: "PRODUCTION" },
    create_curve_table: { tier: "PRODUCTION" },
    add_curve_key: { tier: "PRODUCTION" },
    // ── Water (WaterToolsModule — real AWaterBody/UWaterSpline API) ──
    create_water_body: { tier: "PRODUCTION" },
    get_water_info: { tier: "PRODUCTION" },
    set_water_level: { tier: "PRODUCTION" },
    set_water_flow: { tier: "PRODUCTION" },
    set_water_material: { tier: "PRODUCTION" },
    create_water_spline: { tier: "PRODUCTION" },
    create_procedural_river: { tier: "PRODUCTION" },
    // ── Splines (SplineToolsModule — real USplineComponent API) ──
    create_spline_actor: { tier: "PRODUCTION" },
    add_spline_point: { tier: "PRODUCTION" },
    remove_spline_point: { tier: "PRODUCTION" },
    create_spline_mesh: { tier: "PRODUCTION" },
    get_spline_info: { tier: "PRODUCTION" },
    // ── Chaos Destruction (ChaosToolsModule — real UGeometryCollection API) ──
    create_geometry_collection: { tier: "PRODUCTION" },
    fracture_mesh: { tier: "PRODUCTION" },
    enable_destruction: { tier: "PRODUCTION" },
    // ── Skeletal Mesh (SkeletalMeshToolsModule — real USkeletalMesh/USkeleton API) ──
    get_skeleton_info: { tier: "PRODUCTION" },
    copy_skeleton_sockets: { tier: "PRODUCTION" },
    add_socket: { tier: "PRODUCTION" },
    set_skeletal_mesh: { tier: "PRODUCTION" },
    retarget_animation: { tier: "PRODUCTION" },
    create_ik_rig: { tier: "PRODUCTION" },
    create_ik_retargeter: { tier: "PRODUCTION" },
    assign_mesh_material: { tier: "PRODUCTION" },
    // ── Camera (CameraToolsModule — real ACineCameraActor/USpringArmComponent API) ──
    spawn_camera: { tier: "PRODUCTION" },
    create_spring_arm: { tier: "PRODUCTION" },
    add_camera_shake: { tier: "PRODUCTION" },
    get_camera_info: { tier: "PRODUCTION" },
    // ── Lighting (LightingToolsModule — real APointLight/ASpotLight API) ──
    spawn_light: { tier: "PRODUCTION" },
    list_lights: { tier: "PRODUCTION" },
    set_light_mobility: { tier: "PRODUCTION" },
    set_light_channel: { tier: "PRODUCTION" },
    batch_set_light_property: { tier: "PRODUCTION" },
    set_skylight_cubemap: { tier: "PRODUCTION" },
    set_lumen_settings: { tier: "PRODUCTION" },
    set_sky_atmosphere: { tier: "PRODUCTION" },
    setup_outdoor_lighting: { tier: "PRODUCTION" },
    create_hdri_backdrop: { tier: "PRODUCTION" },
    set_exposure_settings: { tier: "PRODUCTION" },
    // ── LOD / Nanite (LODToolsModule — real FStaticMeshLODResources/IMeshReduction API) ──
    auto_generate_lods: { tier: "PRODUCTION" },
    auto_lod_chain: { tier: "PRODUCTION" },
    auto_lod_material: { tier: "PRODUCTION" },
    batch_set_lod_group: { tier: "PRODUCTION" },
    auto_instance_optimizer: { tier: "PRODUCTION" },
    // ── Import (ImportToolsModule — real UFbxFactory/IAssetTools API) ──
    import_asset: { tier: "PRODUCTION" },
    import_audio: { tier: "PRODUCTION" },
    import_fbx: { tier: "PRODUCTION" },
    import_texture: { tier: "PRODUCTION" },
    // ── Performance (PerformanceToolsModule — real FPlatformMemory/RHI API) ──
    get_fps_stats: { tier: "PRODUCTION" },
    get_gpu_stats: { tier: "PRODUCTION" },
    get_memory_stats: { tier: "PRODUCTION" },
    get_performance_report: { tier: "PRODUCTION", reason: "Combined runtime performance snapshot plus tool-execution telemetry" },
    get_render_stats: { tier: "PRODUCTION" },
    get_performance_snapshot: { tier: "PRODUCTION" },
    get_performance_bottlenecks: { tier: "PRODUCTION" },
    get_texture_streaming_stats: { tier: "PRODUCTION" },
    profile_scene_complexity: { tier: "PRODUCTION" },
    analyze_scene_cost: { tier: "PRODUCTION" },
    get_actor_render_cost: { tier: "PRODUCTION" },
    get_stat_group: { tier: "PRODUCTION" },
    optimize_world_partition: { tier: "PRODUCTION" },
    // ── Batch Operations (BatchToolsModule — real actor iteration/mutation) ──
    batch_clone_actors: { tier: "PRODUCTION" },
    batch_delete_actors: { tier: "PRODUCTION" },
    batch_move_actors: { tier: "PRODUCTION" },
    batch_move_assets: { tier: "PRODUCTION" },
    batch_set_actor_folder: { tier: "PRODUCTION" },
    batch_set_material: { tier: "PRODUCTION" },
    // ── Validation / Testing (real validation, playtest, and audit surfaces) ──
    run_quick_playtest: { tier: "PRODUCTION" },
    validate_assets: { tier: "PRODUCTION" },
    validate_asset_quick: { tier: "PRODUCTION" },
    validate_world_state: { tier: "PRODUCTION" },
    validate_blueprint_health: { tier: "PRODUCTION" },
    validate_for_packaging: { tier: "PRODUCTION" },
    audit_net_replication: { tier: "PRODUCTION" },
    // ── Config / World Settings (ConfigToolsModule — real AWorldSettings/UEditorPerProjectUserSettings API) ──
    get_project_settings: { tier: "PRODUCTION" },
    set_project_setting: { tier: "PRODUCTION" },
    get_world_settings: { tier: "PRODUCTION" },
    set_world_setting: { tier: "PRODUCTION" },
    set_kill_z: { tier: "PRODUCTION" },
    set_world_gravity: { tier: "PRODUCTION" },
    set_game_instance_class: { tier: "PRODUCTION" },
    get_console_variables: { tier: "PRODUCTION" },
    set_console_variable: { tier: "PRODUCTION" },
    set_editor_preference: { tier: "PRODUCTION" },
    // ── Movie Render / Cinematic (MovieRenderQueueToolsModule — real UMoviePipeline API) ──
    render_sequence: { tier: "PRODUCTION" },
    capture_frame_sequence: { tier: "PRODUCTION" },
    add_render_pass: { tier: "PRODUCTION" },
    capture_at_resolution: { tier: "PRODUCTION" },
    capture_depth_buffer: { tier: "PRODUCTION" },
    // ── Widget / UMG (WidgetToolsModule — real UWidgetBlueprint/UWidgetTree API) ──
    add_widget_child: { tier: "PRODUCTION" },
    set_widget_property: { tier: "PRODUCTION" },
    set_widget_position: { tier: "PRODUCTION" },
    set_widget_size: { tier: "PRODUCTION" },
    set_widget_visibility: { tier: "PRODUCTION" },
    set_widget_anchors: { tier: "PRODUCTION" },
    set_text_block_text: { tier: "PRODUCTION" },
    set_text_block_font_size: { tier: "PRODUCTION" },
    set_image_brush: { tier: "PRODUCTION" },
    set_progress_bar_percent: { tier: "PRODUCTION" },
    get_widget_info: { tier: "PRODUCTION" },
    list_widgets: { tier: "PRODUCTION" },
    // ── Blueprint (extended — BlueprintToolsModule) ──
    connect_blueprint_nodes: { tier: "PRODUCTION" },
    set_blueprint_pin_value: { tier: "PRODUCTION" },
    setup_blueprint_events: { tier: "PRODUCTION" },
    // ── GAS (extended) ──
    add_gameplay_tag: { tier: "PRODUCTION" },
    set_actor_gameplay_tags: { tier: "PRODUCTION" },
    // ── Selection / Undo (SelectionToolsModule) ──
    add_to_selection: { tier: "PRODUCTION" },
    remove_from_selection: { tier: "PRODUCTION" },
    select_by_class: { tier: "PRODUCTION" },
    select_by_tag: { tier: "PRODUCTION" },
    end_transaction: { tier: "PRODUCTION" },
    get_undo_history: { tier: "PRODUCTION" },
    // ── Level (extended — LevelToolsModule) ──
    assert_actor_exists: { tier: "PRODUCTION" },
    get_level_actors: { tier: "PRODUCTION" },
    set_actor_folder: { tier: "PRODUCTION" },
    create_basic_geometry: { tier: "PRODUCTION" },
    delete_all_actors: { tier: "PRODUCTION" },
    // ── Physics (extended) ──
    box_trace: { tier: "PRODUCTION" },
    capsule_trace: { tier: "PRODUCTION" },
    sphere_trace: { tier: "PRODUCTION" },
    // ── Wave 3: Registry-verified tools promoted 2026-04-09 ─────────────────
    // Post-Process extras (real FPostProcessSettings API — rest superseded by set_post_process_settings)
    set_motion_blur: { tier: "PRODUCTION" },
    set_chromatic_aberration: { tier: "PRODUCTION" },
    set_lens_flare: { tier: "PRODUCTION" },
    set_local_exposure: { tier: "PRODUCTION" },
    // Editor GUI (real FSlateApplication / FEditorGuiState / FEditorModeTools API)
    set_editor_pref: { tier: "PRODUCTION" },
    click_editor_button: { tier: "PRODUCTION" },
    get_editor_gui_state: { tier: "PRODUCTION" },
    get_editor_workspace_state: { tier: "PRODUCTION" },
    get_editor_mode_state: { tier: "PRODUCTION" },
    activate_editor_mode: { tier: "PRODUCTION" },
    deactivate_editor_mode: { tier: "PRODUCTION" },
    dismiss_modal_dialog: { tier: "PRODUCTION" },
    // Asset Management (real FAssetToolsModule / UEditorAssetSubsystem / UAssetEditorSubsystem API)
    rename_asset: { tier: "PRODUCTION" },
    duplicate_asset: { tier: "PRODUCTION" },
    move_asset: { tier: "PRODUCTION" },
    save_asset: { tier: "PRODUCTION" },
    open_asset_editor: { tier: "PRODUCTION" },
    close_asset_editor: { tier: "PRODUCTION" },
    get_open_asset_editors: { tier: "PRODUCTION" },
    // Content Browser (real FContentBrowserModule / IContentBrowserSingleton API)
    get_content_browser_state: { tier: "PRODUCTION" },
    focus_content_browser: { tier: "PRODUCTION" },
    sync_content_browser_to_asset: { tier: "PRODUCTION" },
    sync_content_browser_to_folder: { tier: "PRODUCTION" },
    navigate_content_browser: { tier: "PRODUCTION" },
    // Scene Graph / World State (real FWorldStateDigest / TActorIterator API)
    get_world_state_digest: { tier: "PRODUCTION" },
    diff_world_state_digest: { tier: "PRODUCTION" },
    get_systemic_world_state: { tier: "PRODUCTION" },
    explain_systemic_world_state: { tier: "PRODUCTION" },
    get_scene_graph: { tier: "PRODUCTION" },
    resolve_scene_graph_node: { tier: "PRODUCTION" },
    select_scene_graph_node: { tier: "PRODUCTION" },
    // Collision (real UPrimitiveComponent API)
    set_collision: { tier: "PRODUCTION" },
    set_collision_preset: { tier: "PRODUCTION" },
    set_collision_response: { tier: "PRODUCTION" },
    add_collision_shape: { tier: "PRODUCTION" },
    create_collision_preset: { tier: "PRODUCTION" },
    // LOD extras (real UStaticMesh / FStaticMeshLODResources / FMeshReductionSettings)
    get_mesh_lod_info: { tier: "PRODUCTION" },
    set_lod_screen_size: { tier: "PRODUCTION" },
    remove_lod: { tier: "PRODUCTION" },
    set_lod_count: { tier: "PRODUCTION" },
    get_lod_group_info: { tier: "PRODUCTION" },
    list_lod_groups: { tier: "PRODUCTION" },
    // Foliage extras (real AInstancedFoliageActor / FFoliageInfo API)
    get_foliage_density: { tier: "PRODUCTION" },
    scatter_foliage: { tier: "PRODUCTION" },
    scatter_canopy_trees: { tier: "PRODUCTION" },
    // Blueprint extras (real FKismetEditorUtilities / FBPVariableDescription)
    set_blueprint_variable_default: { tier: "PRODUCTION" },
    set_blueprint_default: { tier: "PRODUCTION" },
    recompile_blueprint: { tier: "PRODUCTION" },
    // EQS (real UEnvQuery / UEnvQueryGenerator / UEnvQueryTest API)
    create_eqs_query: { tier: "PRODUCTION" },
    get_eqs_query_info: { tier: "PRODUCTION" },
    list_eqs_generators: { tier: "PRODUCTION" },
    list_eqs_tests: { tier: "PRODUCTION" },
    // Game Mode (real AWorldSettings::DefaultGameMode)
    // Build System (real URiftbornLogWatcher / FHotReloadIntegration API)
    get_build_status: { tier: "PRODUCTION" },
    get_build_errors: { tier: "PRODUCTION" },
    get_build_events: { tier: "PRODUCTION" },
    get_cook_stats: { tier: "PRODUCTION" },
    hot_reload_cpp: { tier: "PRODUCTION", reason: "Public hot-reload lane with explicit timeout budget and governance route" },
    trigger_live_coding: { tier: "PRODUCTION", reason: "UE 5.7 live coding compile surface with explicit timeout budget and governance route" },
    run_ubt: { tier: "PRODUCTION", reason: "UE 5.7 full UBT rebuild surface with explicit timeout budget and governance route" },
    cook_project: { tier: "PRODUCTION", reason: "RunUAT cook launcher with governed bridge route and long-running timeout budget" },
    package_project: { tier: "PRODUCTION", reason: "RunUAT package launcher with governed bridge route and long-running timeout budget" },
    get_error_summary: { tier: "PRODUCTION" },
    get_errors_since: { tier: "PRODUCTION" },
    wait_for_build: { tier: "PRODUCTION" },
    // Tool Introspection (real FClaudeToolRegistry / TActorIterator)
    inspect_actor: { tier: "PRODUCTION" },
    describe_tool: { tier: "PRODUCTION" },
    get_viewport_exposure_state: { tier: "PRODUCTION" },
    set_viewport_exposure_state: { tier: "PRODUCTION" },
    // Performance / Adaptive Budget (real URiftAdaptiveSceneBudgetingSubsystem / USignificanceManager)
    configure_visibility_budget: { tier: "STUB", reason: "Compile-disabled when the adaptive scene budgeting runtime module is absent in this build" },
    get_visibility_budget_status: { tier: "STUB", reason: "Compile-disabled when the adaptive scene budgeting runtime module is absent in this build" },
    get_actor_significance_policy: { tier: "STUB", reason: "Compile-disabled when the adaptive scene budgeting runtime module is absent in this build" },
    set_actor_significance_policy: { tier: "STUB", reason: "Compile-disabled when the adaptive scene budgeting runtime module is absent in this build" },
    apply_performance_budget: { tier: "STUB", reason: "Compile-disabled when the adaptive scene budgeting runtime module is absent in this build" },
    define_protected_quality_zone: { tier: "STUB", reason: "Compile-disabled when the adaptive scene budgeting runtime module is absent in this build" },
    verify_quality_preservation: { tier: "STUB", reason: "Compile-disabled when the adaptive scene budgeting runtime module is absent in this build" },
    optimize_streaming_residency: { tier: "STUB", reason: "Compile-disabled when the adaptive scene budgeting runtime module is absent in this build" },
    apply_distant_material_distillation: { tier: "STUB", reason: "Compile-disabled when the adaptive scene budgeting runtime module is absent in this build" },
    schedule_shadow_updates: { tier: "STUB", reason: "Compile-disabled when the adaptive scene budgeting runtime module is absent in this build" },
    optimize_open_world_scene: { tier: "STUB", reason: "Compile-disabled when the adaptive scene budgeting runtime module is absent in this build" },
    analyze_perceptual_cost: { tier: "STUB", reason: "Compile-disabled when the adaptive scene budgeting runtime module is absent in this build" },
    // Widget / UI extended (real UWidgetBlueprint / FSlateApplication / PIE widget introspection)
    add_widget_animation: { tier: "PRODUCTION" },
    set_widget_is_enabled: { tier: "PRODUCTION" },
    set_widget_focusable: { tier: "PRODUCTION" },
    set_widget_navigation_rule: { tier: "PRODUCTION" },
    set_widget_navigation_routing_policy: { tier: "PRODUCTION" },
    set_widget_navigation_method: { tier: "PRODUCTION" },
    get_widget_navigation_state: { tier: "PRODUCTION" },
    apply_widget_interaction_state: { tier: "PRODUCTION" },
    create_widget_from_json: { tier: "PRODUCTION" },
    list_widget_tree: { tier: "PRODUCTION" },
    compile_widget_blueprint: { tier: "PRODUCTION" },
    rename_widget: { tier: "PRODUCTION" },
    verify_widget_blueprint_layout: { tier: "PRODUCTION" },
    verify_widget_present_in_pie: { tier: "PRODUCTION" },
    assert_widget_visible_in_pie: { tier: "PRODUCTION" },
    verify_hud_flow_in_pie: { tier: "PRODUCTION" },
    run_ui_flow_test: { tier: "PRODUCTION" },
    capture_ui_state: { tier: "PRODUCTION" },
    capture_widget_at_resolution: { tier: "PRODUCTION" },
    verify_widget_focus_traversal_in_pie: { tier: "PRODUCTION" },
    simulate_widget_navigation_in_pie: { tier: "PRODUCTION" },
    set_widget_focus_in_pie: { tier: "PRODUCTION" },
    get_live_viewport_widgets: { tier: "PRODUCTION" },
    run_widget_interaction_script_in_pie: { tier: "PRODUCTION" },
    run_hud_resolution_sweep: { tier: "PRODUCTION" },
    get_focused_widget_in_pie: { tier: "PRODUCTION" },
    add_widget_property_binding: { tier: "PRODUCTION" },
    remove_widget_property_binding: { tier: "PRODUCTION" },
    create_editor_utility_widget: { tier: "PRODUCTION" },
    // PCG extended
    add_pcg_node: { tier: "PRODUCTION" },
    connect_pcg_nodes: { tier: "PRODUCTION" },
    // Chooser / Data Registry / World Rules (UE 5.4+ systems)
    evaluate_chooser_table: { tier: "PRODUCTION" },
    evaluate_chooser_table_multi: { tier: "PRODUCTION" },
    evaluate_proxy_table: { tier: "PRODUCTION" },
    evaluate_proxy_asset: { tier: "PRODUCTION" },
    evaluate_data_registry_curve: { tier: "PRODUCTION" },
    get_data_registry_id_display_text: { tier: "PRODUCTION" },
    list_data_registry_ids: { tier: "PRODUCTION" },
    evaluate_actor_gameplay_tag_world_rule: { tier: "PRODUCTION" },
    evaluate_actor_distance_world_rule: { tier: "PRODUCTION" },
    evaluate_world_rule_bundle: { tier: "PRODUCTION" },
    // LiveLink (real ILiveLinkClient API)
    evaluate_live_link_frame: { tier: "PRODUCTION" },
    list_live_link_subjects: { tier: "PRODUCTION" },
    // GAS extras
    get_gameplay_tags: { tier: "PRODUCTION" },
    create_gameplay_tag_table: { tier: "PRODUCTION" },
    // Control Rig
    get_rig_info: { tier: "PRODUCTION" },
    // Config / Settings extras
    get_settings_profile: { tier: "PRODUCTION" },
    list_settings_profiles: { tier: "PRODUCTION" },
    set_settings_profile_value: { tier: "PRODUCTION" },
    // Camera extras (real UCameraComponent / USpringArmComponent)
    set_camera_properties: { tier: "PRODUCTION" },
    set_spring_arm_properties: { tier: "PRODUCTION" },
    // Actor extras
    spawn_actor_with_tags: { tier: "PRODUCTION" },
    get_actor_components: { tier: "PRODUCTION" },
    set_actor_lod_distance_factor: { tier: "PRODUCTION" },
    // Undo / Transaction
    begin_transaction: { tier: "PRODUCTION" },
    commit_transaction: { tier: "PRODUCTION" },
    rollback_transaction: { tier: "PRODUCTION" },
    undo_last_transaction: { tier: "PRODUCTION" },
    list_transactions: { tier: "PRODUCTION" },
    // Level Snapshots
    capture_destruction_snapshot: { tier: "PRODUCTION" },
    restore_destruction_snapshot: { tier: "PRODUCTION" },
    // Voxel / Terrain (real GeometryScripting API)
    dig_terrain_voxel: { tier: "PRODUCTION" },
    spawn_voxel_stamp: { tier: "PRODUCTION" },
    // Animation / Viewport
    set_animation_blueprint: { tier: "PRODUCTION" },
    set_viewport_mode: { tier: "PRODUCTION" },
    // Visual Intelligence / Ollama
    configure_visual_intelligence: { tier: "PRODUCTION" },
    recheck_ollama_availability: { tier: "PRODUCTION" },
    // Raycast
    // Landscape extras
    set_landscape_material: { tier: "PRODUCTION" },
    // Component property (typed variant)
    set_component_property_typed: { tier: "PRODUCTION" },
    // UE 5.4+ Subsystem Introspection
    inspect_pose_search_database: { tier: "PRODUCTION" },
    inspect_neural_morph_asset: { tier: "PRODUCTION" },
    inspect_usd_stage_actor: { tier: "PRODUCTION" },
    inspect_take_recorder_state: { tier: "PRODUCTION" },
    inspect_actor_replication: { tier: "PRODUCTION" },
    inspect_contextual_anim_bindings: { tier: "PRODUCTION" },
    inspect_learning_agents_manager: { tier: "PRODUCTION" },
    // Soil / Foliage Material
    compute_soil_map: { tier: "PRODUCTION" },
    create_foliage_material_preset: { tier: "PRODUCTION" },
    // Material extras
    set_material_blend_mode: { tier: "PRODUCTION" },
    create_voxel_terrain_material: { tier: "PRODUCTION" },
    create_vme_material: { tier: "PRODUCTION" },
    // Forest atmosphere
    apply_forest_atmosphere_preset: { tier: "PRODUCTION" },
    // Session / workflow orchestration
    batch_execute: { tier: "PRODUCTION" },
    explain_tool_execution: { tier: "PRODUCTION" },
    find_tools: { tier: "PRODUCTION" },
    generate_scene_report: { tier: "PRODUCTION" },
    start_agent_task: { tier: "PRODUCTION" },
    get_agentic_session: { tier: "PRODUCTION" },
    get_agent_task: { tier: "PRODUCTION" },
    cancel_agent_task: { tier: "PRODUCTION" },
    list_agent_tasks: { tier: "PRODUCTION" },
    find_agent_skills: { tier: "PRODUCTION" },
    get_project_graph: { tier: "PRODUCTION" },
    plan_workflow: { tier: "PRODUCTION" },
    get_workflow: { tier: "PRODUCTION" },
    pipeline_stats: { tier: "PRODUCTION" },
    pipeline_trace: { tier: "PRODUCTION" },
    verify_session: { tier: "PRODUCTION" },
    // Session persistence / bookmarks
    bookmark_session: { tier: "PRODUCTION" },
    list_bookmarks: { tier: "PRODUCTION" },
    // Undo / redo helpers
    redo: { tier: "PRODUCTION" },
    rollback: { tier: "PRODUCTION" },
    undo_last: { tier: "PRODUCTION" },
    // PIE verification helper
    smoke_test_pie: { tier: "PRODUCTION" },
    // Navigation extras
    create_navmesh_bounds: { tier: "PRODUCTION" },
    // Static mesh
    create_static_mesh_actor: { tier: "PRODUCTION" },
    // ── DEPRECATED ──
    // (none currently — add here when retiring tools)
};
// ─── Heuristic Classifier ─────────────────────────────────────────────────────
/**
 * Classify a tool that has no explicit override.
 * Rules:
 *   1. If the tool name appears in TOOL_HANDLERS (passed set), it gets BETA
 *      (has curated handler with non-trivial logic).
 *   2. If the tool is in the manual TOOLS array AND has a generated backend,
 *      it gets BETA (curated schema over a real executable surface).
 *   3. If the tool is generated only, it is EXPERIMENTAL/STUB depending on
 *      the expose-generated flag.
 *   4. Manual-only tools with no handler and no generated backend are STUB.
 */
export function classifyTool(toolName, handlerNames, manualToolNames, generatedToolNames) {
    const hasHandler = handlerNames.has(toolName);
    const isManual = manualToolNames?.has(toolName) === true;
    const isGenerated = generatedToolNames?.has(toolName) === true;
    if (hasHandler)
        return "BETA";
    if (isManual && isGenerated)
        return "BETA";
    if (isGenerated) {
        return EXPOSE_GENERATED_EXPERIMENTAL ? "EXPERIMENTAL" : "STUB";
    }
    if (isManual)
        return "STUB";
    return "STUB";
}
// ─── Public API ───────────────────────────────────────────────────────────────
/**
 * Get the readiness tier for a single tool.
 */
export function getToolReadiness(toolName, handlerNames, manualToolNames, generatedToolNames) {
    const inferredTier = classifyTool(toolName, handlerNames, manualToolNames, generatedToolNames);
    if (PRODUCTION_TOOL_NAMES.has(toolName)) {
        if (inferredTier === "STUB") {
            // Generated tools have a real C++ backend — the index.ts fallback routes them
            if (generatedToolNames?.has(toolName)) {
                return { tier: "PRODUCTION" };
            }
            return {
                tier: "STUB",
                reason: "Canonical production surface tool is missing a handler or generated backend",
            };
        }
        return { tier: "PRODUCTION" };
    }
    const explicit = EXPLICIT_TIER[toolName];
    if (explicit) {
        if (explicit.tier === "PRODUCTION" && !PRODUCTION_TOOL_NAMES.has(toolName)) {
            if (inferredTier === "STUB") {
                if (generatedToolNames?.has(toolName)) {
                    return {
                        tier: "BETA",
                        reason: "Curated and callable, but not on the canonical default production surface",
                    };
                }
                return {
                    tier: "STUB",
                    reason: "Manual schema exists, but no handler or generated backend is available",
                };
            }
            return {
                tier: "BETA",
                reason: "Curated and callable, but not on the canonical default production surface",
            };
        }
        if (inferredTier === "STUB" && explicit.tier !== "STUB" && explicit.tier !== "DEPRECATED") {
            // Generated tools have a real C++ backend — the index.ts fallback
            // routes them even without a curated handler.  Honor the explicit tier.
            if (generatedToolNames?.has(toolName)) {
                return explicit;
            }
            return {
                tier: "STUB",
                reason: "Manual schema exists, but no handler or generated backend is available",
            };
        }
        return explicit;
    }
    return { tier: inferredTier };
}
/**
 * Filter a list of tools to those matching the requested tiers.
 * Also injects a [TIER] badge into the tool description so the LLM
 * knows the maturity level.
 */
export function filterToolsByReadiness(tools, allowedTiers, handlerNames, manualToolNames, generatedToolNames) {
    const allowed = new Set(allowedTiers);
    const sanitizeToolMetadata = createSanitizer({ trackCircular: true });
    const cloneTool = (tool, description) => {
        const cloned = {
            name: tool.name,
            inputSchema: sanitizeToolMetadata(tool.inputSchema),
            description,
        };
        if (tool.title) {
            cloned.title = tool.title;
        }
        if (tool.outputSchema) {
            cloned.outputSchema = sanitizeToolMetadata(tool.outputSchema);
        }
        if (tool.annotations) {
            cloned.annotations = sanitizeToolMetadata(tool.annotations);
        }
        if (tool._meta) {
            cloned._meta = sanitizeToolMetadata(tool._meta);
        }
        return cloned;
    };
    return tools
        .filter((t) => {
        const entry = getToolReadiness(t.name, handlerNames, manualToolNames, generatedToolNames);
        return allowed.has(entry.tier);
    })
        .map((t) => {
        const entry = getToolReadiness(t.name, handlerNames, manualToolNames, generatedToolNames);
        // Inject tier badge into description for LLM awareness
        const badge = `[${entry.tier}]`;
        const desc = t.description || "";
        if (desc.startsWith("[")) {
            // Already has a badge — replace it
            return cloneTool(t, desc.replace(/^\[[A-Z_]+\]\s*/, `${badge} `));
        }
        return cloneTool(t, `${badge} ${desc}`);
    });
}
/**
 * Return a summary of how many tools fall into each tier.
 * Useful for diagnostics and the bootstrap report.
 */
export function getReadinessSummary(tools, handlerNames, manualToolNames, generatedToolNames) {
    const summary = {
        PRODUCTION: 0,
        BETA: 0,
        EXPERIMENTAL: 0,
        STUB: 0,
        DEPRECATED: 0,
    };
    for (const t of tools) {
        const entry = getToolReadiness(t.name, handlerNames, manualToolNames, generatedToolNames);
        summary[entry.tier]++;
    }
    return summary;
}
//# sourceMappingURL=tool-readiness.js.map