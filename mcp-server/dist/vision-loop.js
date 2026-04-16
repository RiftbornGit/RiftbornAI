/**
 * RiftbornAI Vision Loop — Gives Claude eyes inside Unreal Engine
 *
 * Architecture:
 *   OBSERVE → REASON → PLAN → BUILD → VERIFY → ITERATE
 *
 * Claude captures viewport screenshots, analyzes them with vision AI,
 * reasons about what needs to change, executes tools, then verifies
 * the result visually. This creates a closed-loop visual builder.
 */
// ============================================================================
// VISION LOOP TOOLS — These extend the MCP tool set with vision-aware building
// ============================================================================
export const VISION_LOOP_TOOLS = [
    {
        name: "vision_observe",
        description: `Capture the current UE viewport and analyze what you see. Returns:
- Screenshot (saved to disk for reference)
- Scene description from vision AI (what's visible, lighting, composition)
- Actor census (what's in the scene, counts by class)
- Project state (level name, game mode, player controller)

Use this BEFORE building to understand current state.
Use this AFTER building to verify changes look correct.
This is your PRIMARY way of seeing inside Unreal Engine.`,
        inputSchema: {
            type: "object",
            properties: {
                prompt: {
                    type: "string",
                    description: "What to look for. Examples: 'Is the lighting dramatic enough?', 'Are there enough trees?', 'Does the combat arena look fun?', 'Check if materials are applied correctly'"
                },
                camera_preset: {
                    type: "string",
                    enum: ["current", "top_down", "front", "orbit_slow", "player_view"],
                    description: "Camera angle for the observation. 'current' keeps viewport as-is.",
                    default: "current"
                }
            }
        }
    },
    {
        name: "vision_compare",
        description: `Take a screenshot and compare with a previous one to see what changed.
Useful after making modifications to verify they had the intended visual effect.`,
        inputSchema: {
            type: "object",
            properties: {
                before_path: {
                    type: "string",
                    description: "Path to the 'before' screenshot"
                },
                prompt: {
                    type: "string",
                    description: "What change to look for. Example: 'Did adding fog improve atmosphere?'"
                }
            },
            required: ["prompt"]
        }
    },
    {
        name: "vision_inspect_actor",
        description: `Focus the viewport camera on a specific actor and take a close-up screenshot.
Use this to inspect individual objects, check material quality, verify mesh placement.`,
        inputSchema: {
            type: "object",
            properties: {
                actor_label: {
                    type: "string",
                    description: "Label of the actor to inspect"
                },
                distance: {
                    type: "number",
                    description: "Camera distance from actor (UU). Default 500.",
                    default: 500
                },
                angle: {
                    type: "string",
                    enum: ["front", "side", "top", "three_quarter"],
                    description: "Viewing angle",
                    default: "three_quarter"
                },
                prompt: {
                    type: "string",
                    description: "What to check. Example: 'Is the material applied correctly?'"
                }
            },
            required: ["actor_label"]
        }
    },
    {
        name: "vision_playtest",
        description: `Start PIE (Play In Editor), wait for it to stabilize, capture gameplay screenshot,
analyze the player experience, then stop PIE. This lets you SEE what the player sees.
Use after building a gameplay area to verify it feels right.`,
        inputSchema: {
            type: "object",
            properties: {
                duration_seconds: {
                    type: "number",
                    description: "How long to run PIE before capturing (seconds). Default 3.",
                    default: 3
                },
                prompt: {
                    type: "string",
                    description: "What to evaluate. Example: 'Does the arena feel like a combat space?', 'Is the HUD visible?'"
                },
                move_player: {
                    type: "boolean",
                    description: "Move the player forward during playtest to see more of the level",
                    default: false
                }
            }
        }
    },
    {
        name: "vision_sweep",
        description: `Take multiple screenshots from different angles around the scene.
Returns a panoramic understanding of the entire level.
Use this for comprehensive review before finalizing a build.`,
        inputSchema: {
            type: "object",
            properties: {
                center: {
                    type: "object",
                    properties: {
                        x: { type: "number" },
                        y: { type: "number" },
                        z: { type: "number" }
                    },
                    description: "Center point to orbit around. Default: scene center."
                },
                angles: {
                    type: "integer",
                    description: "Number of angles to capture (4=cardinal, 8=octants). Default 4.",
                    default: 4
                },
                prompt: {
                    type: "string",
                    description: "What to evaluate across all views"
                }
            }
        }
    },
    {
        name: "vision_build_and_verify",
        description: `Execute a tool call, then immediately observe the result visually.
This is the core BUILD→VERIFY loop. It:
1. Takes a 'before' screenshot
2. Executes the specified tool
3. Takes an 'after' screenshot
4. Compares before/after with vision AI
5. Returns whether the change looks correct

Use this instead of calling tools directly when you want visual confirmation.`,
        inputSchema: {
            type: "object",
            properties: {
                tool_name: {
                    type: "string",
                    description: "The RiftbornAI tool to execute"
                },
                tool_args: {
                    type: "object",
                    description: "Arguments for the tool"
                },
                verify_prompt: {
                    type: "string",
                    description: "How to verify the result visually. Example: 'The actor should now be visible at the center of the arena'"
                }
            },
            required: ["tool_name", "tool_args", "verify_prompt"]
        }
    }
];
// ============================================================================
// VISION LOOP HANDLERS — Implementation that chains existing tools
// ============================================================================
export function createVisionHandlers(executeTool, _httpRequest) {
    return {
        vision_observe: async (args) => {
            // 1. Optionally move camera
            if (args.camera_preset && args.camera_preset !== "current") {
                const cameraResult = await applyCameraPreset(executeTool, args.camera_preset);
                if (cameraResult && !cameraResult.ok) {
                    return { ok: false, error: `Camera preset '${args.camera_preset}' failed: ${cameraResult.error ?? "unknown"}` };
                }
            }
            // 2. Capture screenshot + full scene observation
            const result = await executeTool("observe_ue_project", {
                capture_screenshot: true,
                include_vision: true,
                prompt: args.prompt || "Describe everything you see in this Unreal Engine viewport. What actors are visible? How is the lighting? What's the overall composition? What needs improvement?",
                filename: `vision_observe_${Date.now()}`
            });
            if (!result?.ok) {
                return { ok: false, error: `observe_ue_project failed: ${result?.error ?? "unknown"}` };
            }
            return {
                ok: true,
                observation: result,
                tip: "Use vision_build_and_verify to make changes with automatic visual confirmation."
            };
        },
        vision_compare: async (args) => {
            // 1. Capture current state
            const afterScreenshot = await executeTool("take_screenshot", {
                filename: `vision_compare_after_${Date.now()}`
            });
            if (!afterScreenshot?.ok) {
                return { ok: false, error: `take_screenshot failed: ${afterScreenshot?.error ?? "unknown"}` };
            }
            // 2. Analyze with vision, referencing before state
            const prompt = args.before_path
                ? `Compare the current viewport with the earlier state. ${args.prompt}. Earlier screenshot was at: ${args.before_path}`
                : `Analyze the current viewport. ${args.prompt}`;
            const analysis = await executeTool("analyze_scene_screenshot", {
                auto_capture: !args.before_path,
                prompt: prompt
            });
            if (!analysis?.ok) {
                return { ok: false, error: `analyze_scene_screenshot failed: ${analysis?.error ?? "unknown"}`, after_screenshot: afterScreenshot };
            }
            return {
                ok: true,
                after_screenshot: afterScreenshot,
                analysis: analysis
            };
        },
        vision_inspect_actor: async (args) => {
            // 1. Focus camera on actor
            const focusResult = await executeTool("focus_actor", {
                actor_label: args.actor_label
            });
            if (!focusResult?.ok) {
                return { ok: false, error: `focus_actor failed for '${args.actor_label}': ${focusResult?.error ?? "unknown"}` };
            }
            // 2. Orbit to desired angle
            const angleMap = {
                front: 0, side: 90, top: 0, three_quarter: 45
            };
            const yaw = angleMap[String(args.angle || "three_quarter")] ?? 45;
            const pitch = args.angle === "top" ? -80 : -20;
            const orbitResult = await executeTool("orbit_actor", {
                target_label: args.actor_label,
                distance: args.distance || 500,
                yaw: yaw,
                pitch: pitch
            });
            // orbit_actor failure is non-fatal — we still have focus, just not the perfect angle
            if (!orbitResult?.ok) {
                console.error(`[RiftbornAI][Vision] orbit_actor failed for '${args.actor_label}', proceeding with focus view`);
            }
            // 3. Capture and analyze
            const result = await executeTool("observe_ue_project", {
                capture_screenshot: true,
                include_vision: true,
                prompt: args.prompt || `Inspect this actor (${args.actor_label}). Describe its appearance, material, scale, and placement quality.`,
                filename: `inspect_${args.actor_label.replace(/[^a-zA-Z0-9]/g, '_')}_${Date.now()}`
            });
            if (!result?.ok) {
                return { ok: false, error: `observe_ue_project failed during inspection: ${result?.error ?? "unknown"}` };
            }
            return {
                ok: true,
                actor: args.actor_label,
                inspection: result
            };
        },
        vision_playtest: async (args) => {
            // 1. Take pre-PIE screenshot (non-fatal if fails)
            const beforeScreenshot = await executeTool("take_screenshot", {
                filename: `playtest_before_${Date.now()}`
            });
            // 2. Start PIE
            const pieResult = await executeTool("start_pie", {
                num_players: 1,
                mode: "selected_viewport"
            });
            if (!pieResult?.ok) {
                return { ok: false, error: `Failed to start PIE: ${pieResult?.error ?? "unknown"}`, details: pieResult };
            }
            // 3. Wait for stabilization
            const rawDuration = Number(args.duration_seconds);
            const safeDuration = Number.isFinite(rawDuration) && rawDuration > 0 ? rawDuration : 3;
            const waitMs = Math.min(Math.max(1000, safeDuration * 1000), 30000);
            await new Promise(resolve => setTimeout(resolve, waitMs));
            // 4. Capture gameplay screenshot
            const gameplayScreenshot = await executeTool("take_screenshot", {
                filename: `playtest_gameplay_${Date.now()}`
            });
            if (!gameplayScreenshot?.ok) {
                // Still stop PIE even if screenshot failed
                await executeTool("stop_pie", {});
                return { ok: false, error: `Gameplay screenshot failed: ${gameplayScreenshot?.error ?? "unknown"}` };
            }
            // 5. Analyze what the player sees
            const analysis = await executeTool("analyze_scene_screenshot", {
                auto_capture: true,
                prompt: args.prompt || "Analyze this gameplay view. Is the level playable? Can the player see important elements? Is the HUD working? Does it feel like a game?"
            });
            // 6. Stop PIE — always stop, even if analysis failed
            await executeTool("stop_pie", {});
            return {
                ok: analysis?.ok ?? false,
                before_screenshot: beforeScreenshot,
                gameplay_screenshot: gameplayScreenshot,
                analysis: analysis,
                tip: "PIE has been stopped. Make changes and playtest again."
            };
        },
        vision_sweep: async (args) => {
            const angles = Math.min(Math.max(2, args.angles || 4), 8);
            const results = [];
            let failedAngles = 0;
            for (let i = 0; i < angles; i++) {
                const yaw = (360 / angles) * i;
                // Set camera position
                if (args.center) {
                    const moveResult = await executeTool("set_viewport_location", {
                        x: args.center.x + Math.cos(yaw * Math.PI / 180) * 2000,
                        y: args.center.y + Math.sin(yaw * Math.PI / 180) * 2000,
                        z: (args.center.z || 500) + 500,
                        pitch: -25,
                        yaw: yaw + 180 // Look toward center
                    });
                    if (!moveResult?.ok) {
                        console.error(`[RiftbornAI][Vision] sweep: set_viewport_location failed at ${yaw}°, skipping angle`);
                        failedAngles++;
                        continue;
                    }
                }
                // Capture
                const shot = await executeTool("observe_ue_project", {
                    capture_screenshot: true,
                    include_vision: true,
                    prompt: `${args.prompt || 'Describe this view.'} (Angle ${i + 1}/${angles}, ${yaw}° yaw)`,
                    filename: `sweep_${yaw}deg_${Date.now()}`
                });
                results.push({ angle_degrees: yaw, observation: shot, ok: shot?.ok ?? false });
            }
            return {
                ok: results.length > 0,
                sweep_count: results.length,
                failed_angles: failedAngles,
                views: results,
                tip: "Review each angle. Use vision_build_and_verify to fix issues found."
            };
        },
        vision_build_and_verify: async (args) => {
            const sanitizedToolName = String(args.tool_name || "").substring(0, 128);
            if (!sanitizedToolName) {
                return { ok: false, error: "tool_name is required" };
            }
            // SECURITY: Whitelist tools allowed in the vision build-and-verify loop.
            // Prevents arbitrary tool execution (e.g. execute_python) through this pipeline.
            const VISION_SAFE_TOOLS = new Set([
                // Actor operations
                "spawn_actor", "delete_actor", "set_actor_transform", "set_actor_property",
                "set_actor_mobility", "duplicate_actor", "rename_actor",
                "move_actor", "rotate_actor", "scale_actor",
                "set_actor_material", "set_actor_color",
                // Blueprints
                "create_blueprint",
                // Materials & meshes
                "set_material_parameter", "set_static_mesh",
                "create_material", "create_pbr_material", "create_material_instance",
                // Lighting & atmosphere
                "set_light_properties", "create_light", "create_post_process_volume",
                "set_sky_atmosphere", "set_fog_properties",
                // Landscape & foliage
                "create_landscape", "sculpt_landscape",
                "paint_landscape_layer", "paint_foliage",
                // VFX & UI
                "create_widget", "create_niagara_system",
                // Navigation
                "build_navmesh",
                // Vision capture (self-referential but needed for the verify step)
                "take_screenshot", "analyze_scene_screenshot",
            ]);
            if (!VISION_SAFE_TOOLS.has(sanitizedToolName)) {
                return {
                    ok: false,
                    error: `Tool '${sanitizedToolName}' is not allowed in vision_build_and_verify. Only scene-modifying tools are permitted.`
                };
            }
            // 1. Before screenshot
            const before = await executeTool("take_screenshot", {
                filename: `build_before_${Date.now()}`
            });
            // 2. Execute the tool
            const toolResult = await executeTool(sanitizedToolName, args.tool_args || {});
            // 3. After screenshot
            const after = await executeTool("take_screenshot", {
                filename: `build_after_${Date.now()}`
            });
            // 4. Visual verification
            const verification = await executeTool("analyze_scene_screenshot", {
                auto_capture: true,
                prompt: `VERIFICATION: ${args.verify_prompt}. Did the change succeed? What's different?`
            });
            return {
                ok: toolResult?.ok ?? false,
                tool_result: toolResult,
                before_screenshot: before,
                after_screenshot: after,
                visual_verification: verification,
                verified: verification?.ok ?? false
            };
        }
    };
}
// ============================================================================
// CAMERA PRESETS
// ============================================================================
async function applyCameraPreset(executeTool, preset) {
    switch (preset) {
        case "top_down":
            return executeTool("set_viewport_location", {
                x: 0, y: 0, z: 5000, pitch: -89, yaw: 0
            });
        case "front":
            return executeTool("set_viewport_location", {
                x: -3000, y: 0, z: 500, pitch: -10, yaw: 0
            });
        case "player_view":
            return executeTool("set_viewport_location", {
                x: 0, y: 0, z: 180, pitch: 0, yaw: 0
            });
        // orbit_slow handled by the orbit tool
        default:
            return { ok: true };
    }
}
//# sourceMappingURL=vision-loop.js.map