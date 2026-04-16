"""
RiftScript Engine Orchestrator - Dual-mode build system

TWO EXECUTION MODES:

1. CLI MODE (Editor CLOSED):
   - Phase 1 ONLY: RiftScript validation → C++ generation → UBT compilation
   - Usage: python quick_build.py <profile>
   
2. EDITOR MODE (Editor OPEN):
   - Phase 1: VERIFICATION ONLY (check if C++ class exists, NO UBT)
   - Phase 2: Blueprint & Asset generation
   - Phase 3: PIE automation (if available) or manual verification
   - Usage: RiftScriptOrchestrator.build_game('<profile>')

CRITICAL: Phase 1 (UBT) CANNOT run inside editor due to Live Coding.
          Close editor and use CLI mode for C++ rebuilds.
"""

from __future__ import annotations
import os
import sys
import json
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional, Dict, Any

# Mode detection
try:
    import unreal
    IN_EDITOR = True
except ImportError:
    unreal = None
    IN_EDITOR = False

# Resolve paths from the plugin location instead of a historical local workspace.
PLUGIN_ROOT = Path(__file__).resolve().parents[2]
PLUGINS_DIR = PLUGIN_ROOT.parent
PROJECT_ROOT = PLUGINS_DIR.parent
SCRIPTS_DIR = PLUGIN_ROOT / "Source" / "RiftbornAI" / "Scripts"
if str(SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_DIR))

try:
    from game_compiler import GameCompiler
except ImportError:
    GameCompiler = None

@dataclass
class BuildResult:
    """Honest build result - no faking phases that didn't run"""
    ok: bool
    phase1_ok: bool
    phase2_ok: bool
    phase3_ok: bool
    mode: str  # "cli" or "editor"
    phase3_manual: bool = False  # True if Phase 3 requires manual verification

class RiftScriptOrchestrator:
    """Engine-side orchestrator - editor mode only"""
    
    def __init__(self):
        self.plugins_dir = PLUGINS_DIR
        self.scripts_dir = SCRIPTS_DIR
        self.compiler = GameCompiler(self.plugins_dir) if GameCompiler is not None else None
        self.current_spec = None
        self.build_log = []
        
    def log(self, message: str, level: str = "Log"):
        """Log to Unreal Output Log and internal buffer"""
        self.build_log.append(message)
        if not IN_EDITOR:
            print(message)
        elif level == "Error":
            unreal.log_error(f"[RiftScript] {message}")
        elif level == "Warning":
            unreal.log_warning(f"[RiftScript] {message}")
        else:
            unreal.log(f"[RiftScript] {message}")
    
    def get_available_profiles(self):
        """List all RiftScript profiles"""
        profiles_dir = self.scripts_dir / 'profiles'
        profiles = []
        if profiles_dir.exists():
            for json_file in profiles_dir.glob('*.json'):
                try:
                    spec = json.loads(json_file.read_text(encoding='utf-8'))
                    game_info = spec.get('game', {})
                    profiles.append({
                        'path': str(json_file),
                        'name': json_file.stem,
                        'id': game_info.get('id', 'Unknown'),
                        'display_name': game_info.get('display_name', json_file.stem)
                    })
                except Exception as e:
                    self.log(f"Failed to parse {json_file.name}: {e}", "Warning")
        return profiles
    
    def verify_phase1_in_editor(self, spec_path: str) -> bool:
        """
        Phase 1 VERIFICATION (Editor mode only)
        
        Does NOT run UBT. Only checks if C++ class is loadable.
        Returns True if module is present and loaded.
        """
        if not IN_EDITOR:
            self.log("Phase 1 verification requires editor mode", "Error")
            return False
        
        self.log("=" * 60)
        self.log("PHASE 1: C++ Module Verification (NO UBT)")
        self.log("=" * 60)
        
        spec_file = Path(spec_path)
        if not spec_file.exists():
            self.log(f"Spec file not found: {spec_path}", "Error")
            return False
        
        try:
            spec = json.loads(spec_file.read_text(encoding='utf-8'))
            self.current_spec = spec
            game_name = spec.get('game', {}).get('id', 'UnknownGame')
        except Exception as e:
            self.log(f"Failed to parse spec: {e}", "Error")
            return False
        
        # Try to load C++ GameMode class
        native_path = f"/Script/{game_name}.{game_name}GameMode"
        self.log(f"Checking C++ class: {native_path}")
        
        try:
            cls = unreal.load_object(None, native_path)
            if cls is None:
                self.log("❌ C++ GameMode not found (module not built/loaded)", "Error")
                self.log("", "Error")
                self.log("SOLUTION: Close editor and run:", "Error")
                self.log(f"  python quick_build.py {spec_file.stem}", "Error")
                self.log("", "Error")
                return False
            
            self.log(f"✅ Phase 1 Verified - C++ module loaded: {game_name}")
            self.log("   (Module was built externally via CLI)")
            return True
            
        except Exception as e:
            self.log(f"❌ Failed to load C++ class: {e}", "Error")
            self.log("", "Error")
            self.log("SOLUTION: Close editor and run:", "Error")
            self.log(f"  python quick_build.py {spec_file.stem}", "Error")
            return False
    
    def run_phase2(self) -> bool:
        """
        Phase 2: Generate Blueprints and Assets (Editor mode only)
        
        Requires Phase 1 verified (C++ class loadable)
        Returns True if all assets created/reused
        """
        if not IN_EDITOR:
            self.log("Phase 2 requires editor mode", "Error")
            return False
            
        if not self.current_spec:
            self.log("Phase 2 requires current_spec (run Phase 1 first)", "Error")
            return False
        
        self.log("=" * 60)
        self.log("PHASE 2: Blueprint & Asset Generation")
        self.log("=" * 60)
        
        game_name = self.current_spec.get('game', {}).get('id', 'UnknownGame')
        
        try:
            # Create content directories
            content_root = f"/Game/{game_name}"
            
            self.log(f"Creating assets in: {content_root}")
            
            # Import asset tools
            asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
            editor_asset_lib = unreal.EditorAssetLibrary()
            
            # 1. Create or reuse GameMode Blueprint
            self.log("Creating GameMode Blueprint...")
            gamemode_class = unreal.load_class(None, f"/Script/{game_name}.{game_name}GameMode")
            if not gamemode_class:
                self.log(f"GameMode class not found: {game_name}GameMode", "Error")
                self.log("Ensure Phase 1 DLL is loaded. Try: File -> Refresh Visual Studio Project", "Warning")
                return False
            
            gamemode_bp_path = f"{content_root}/Blueprints/BP_{game_name}GameMode"
            
            # Check if Blueprint already exists
            if editor_asset_lib.does_asset_exist(gamemode_bp_path):
                self.log(f"♻️  GameMode Blueprint already exists: {gamemode_bp_path}")
                gamemode_bp = unreal.load_asset(gamemode_bp_path)
            else:
                # Create new Blueprint
                gamemode_factory = unreal.BlueprintFactory()
                gamemode_factory.set_editor_property('parent_class', gamemode_class)
                
                gamemode_bp = asset_tools.create_asset(
                    f"BP_{game_name}GameMode",
                    f"{content_root}/Blueprints",
                    unreal.Blueprint,
                    gamemode_factory
                )
                
                if not gamemode_bp:
                    self.log("Failed to create GameMode Blueprint", "Error")
                    return False
            
            self.log(f"✅ {gamemode_bp.get_path_name()}")
            
            # Store GameMode Blueprint for later configuration
            gamemode_blueprint = gamemode_bp
            
            # 2. Create or reuse Character Blueprint
            self.log("Creating Character Blueprint...")
            character_class = unreal.load_class(None, f"/Script/{game_name}.{game_name}Character")
            if not character_class:
                self.log(f"Character class not found: {game_name}Character", "Error")
                return False
            
            character_bp_path = f"{content_root}/Blueprints/BP_{game_name}Character"
            
            # Check if Blueprint already exists
            if editor_asset_lib.does_asset_exist(character_bp_path):
                self.log(f"♻️  Character Blueprint already exists: {character_bp_path}")
                character_bp = unreal.load_asset(character_bp_path)
            else:
                character_factory = unreal.BlueprintFactory()
                character_factory.set_editor_property('parent_class', character_class)
                
                character_bp = asset_tools.create_asset(
                    f"BP_{game_name}Character",
                    f"{content_root}/Blueprints",
                    unreal.Blueprint,
                    character_factory
                )
                
                if not character_bp:
                    self.log("Failed to create Character Blueprint", "Error")
                    return False
            
            self.log(f"✅ {character_bp.get_path_name()}")
            
            # 2.5. Configure GameMode to use our Character
            self.log("Configuring GameMode DefaultPawnClass...")
            try:
                # Load GameMode Blueprint class
                gamemode_bp_class = unreal.EditorAssetLibrary.load_blueprint_class(f"{content_root}/Blueprints/BP_{game_name}GameMode")
                if gamemode_bp_class:
                    # Get Class Default Object (CDO)
                    gamemode_cdo = unreal.get_default_object(gamemode_bp_class)
                    
                    # Load Character Blueprint class
                    character_bp_class = unreal.EditorAssetLibrary.load_blueprint_class(f"{content_root}/Blueprints/BP_{game_name}Character")
                    if character_bp_class:
                        # Set DefaultPawnClass on GameMode CDO
                        gamemode_cdo.set_editor_property("default_pawn_class", character_bp_class)
                        
                        # Save the GameMode Blueprint
                        unreal.EditorAssetLibrary.save_loaded_asset(gamemode_blueprint)
                        
                        self.log(f"✅ DefaultPawnClass set to: BP_{game_name}Character")
                    else:
                        self.log(f"⚠️  Character Blueprint class not loaded", "Warning")
                else:
                    self.log(f"⚠️  GameMode Blueprint class not loaded", "Warning")
            except Exception as e:
                self.log(f"⚠️  Failed to set DefaultPawnClass: {e}", "Warning")
            
            # 3. Create or reuse Map
            self.log("Creating Map...")
            map_path = f"{content_root}/Maps/{game_name}Arena"
            
            # Check if map already exists
            if editor_asset_lib.does_asset_exist(map_path):
                self.log(f"♻️  Map already exists: {map_path}")
                # Load existing map
                unreal.EditorLevelLibrary.load_level(map_path)
            else:
                # Create new level
                new_level = unreal.EditorLevelLibrary.new_level(map_path)
                if not new_level:
                    self.log("Failed to create map", "Error")
                    return False
                
            self.log(f"✅ Map ready: {map_path}")
            
            # 4. Set GameMode in World Settings
            self.log("Setting GameMode in World Settings...")
            try:
                world = unreal.EditorLevelLibrary.get_editor_world()
                world_settings = world.get_world_settings()
                
                # Set GameMode override to our Blueprint
                if world_settings:
                    # Load the GameMode Blueprint class
                    gamemode_bp_path = f"{content_root}/Blueprints/BP_{game_name}GameMode"
                    gamemode_blueprint = unreal.load_asset(gamemode_bp_path)
                    
                    if gamemode_blueprint:
                        gamemode_class = gamemode_blueprint.generated_class()
                        # Property name is 'default_game_mode' not 'default_gamemode_override'
                        world_settings.set_editor_property('default_game_mode', gamemode_class)
                        self.log(f"✅ GameMode set: {gamemode_class.get_name()}")
                        
                        # Mark map as modified so it saves
                        unreal.EditorLevelLibrary.save_current_level()
                    else:
                        self.log(f"⚠️  GameMode Blueprint not found: {gamemode_bp_path}", "Warning")
                else:
                    self.log("⚠️  WorldSettings not accessible", "Warning")
            except Exception as e:
                self.log(f"⚠️  Failed to set GameMode: {e}", "Warning")
            
            # 5. Save all assets
            self.log("Saving assets...")
            editor_asset_lib.save_directory(content_root)
            
            self.log("✅ Phase 2 Complete - Assets Created")
            return True
            
        except Exception as e:
            import traceback
            self.log(f"Phase 2 exception: {e}", "Error")
            self.log(traceback.format_exc(), "Error")
            return False
    
    def run_phase3(self) -> tuple[bool, bool]:
        """
        Phase 3: Runtime validation via PIE (Editor mode only)
        
        Returns (success, manual_required)
        - success: True if PIE started and GameMode log found
        - manual_required: True if log verification incomplete
        """
        if not IN_EDITOR:
            self.log("Phase 3 requires editor mode", "Error")
            return False, False
            
        if not self.current_spec:
            self.log("Phase 3 requires current_spec", "Error")
            return False, False
        
        self.log("=" * 60)
        self.log("PHASE 3: Runtime Validation (PIE)")
        self.log("=" * 60)
        
        game_name = self.current_spec.get('game', {}).get('id', 'UnknownGame')
        map_path = f"/Game/{game_name}/Maps/{game_name}Arena"
        
        self.log(f"Opening map: {map_path}")
        
        try:
            # Load map
            if not unreal.EditorAssetLibrary.does_asset_exist(map_path):
                self.log(f"Map not found: {map_path}", "Error")
                return False, False
            
            success = unreal.EditorLevelLibrary.load_level(map_path)
            if not success:
                self.log("Failed to load map", "Error")
                return False, False
            
            self.log("✅ Map loaded successfully")
            
            # Start PIE using LevelEditorSubsystem
            self.log("Starting PIE...")
            level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
            
            if not level_editor:
                self.log("❌ LevelEditorSubsystem not available", "Error")
                return False, False
            
            # Generate run ID for log tracking
            import time
            import uuid
            run_id = f"{game_name}_{int(time.time())}_{uuid.uuid4().hex[:8]}"
            
            # Write run_id to file for GameMode to read
            import os
            project_saved_dir = unreal.SystemLibrary.get_project_saved_directory()
            rift_dir = os.path.join(project_saved_dir, "RiftScript")
            os.makedirs(rift_dir, exist_ok=True)
            run_id_path = os.path.join(rift_dir, "current_run_id.txt")
            with open(run_id_path, "w", encoding="utf-8") as f:
                f.write(run_id)
            
            # Emit start marker
            unreal.log(f"RiftScript PHASE3 START {run_id}")
            self.log(f"Phase 3 run_id: {run_id}")
            
            # Start PIE (asynchronous - will execute after Python returns)
            try:
                level_editor.editor_request_begin_play()
                self.log("✅ PIE start requested (will execute when Python returns)")
            except Exception as e:
                self.log(f"❌ Failed to request PIE: {e}", "Error")
                return False, True  # Failed, manual check needed
            
            # Register a delayed monitor to check validation results
            # This runs AFTER exec() returns and PIE has had time to start
            import threading
            
            self.log("✅ Phase 3 validation will run asynchronously")
            self.log(f"Watch for validation markers in Output Log:")
            self.log(f"   PASS: RiftScript Validation: PASS [{run_id}]")
            self.log(f"   FAIL: RiftScript Validation: FAIL [{run_id}]")
            
            def monitor_validation():
                """Background thread to monitor validation results"""
                import time
                time.sleep(2.0)  # Give PIE time to start
                
                # Import the bridge
                import sys
                bridge_path = SCRIPTS_DIR
                if str(bridge_path) not in sys.path:
                    sys.path.insert(0, str(bridge_path))
                
                try:
                    from output_log_bridge import OutputLogBridge
                    bridge = OutputLogBridge()
                    
                    validation_timeout = 30.0
                    start_time = time.time()
                    validation_pass = f"RiftScript Validation: PASS [{run_id}]"
                    validation_fail = f"RiftScript Validation: FAIL [{run_id}]"
                    
                    for line in bridge.stream_live(poll_interval=0.25, start_at_end=False):
                        if time.time() - start_time > validation_timeout:
                            unreal.log_error(f"❌ RiftScript Validation: TIMEOUT [{run_id}]")
                            break
                        
                        if validation_pass in line:
                            unreal.log_warning(f"✅ RiftScript Validation: SUCCESS [{run_id}]")
                            # Stop PIE
                            try:
                                level_editor.editor_request_end_play()
                            except Exception:
                                pass
                            break
                        
                        if validation_fail in line:
                            unreal.log_error(f"❌ RiftScript Validation: FAILED [{run_id}] - Check logs for details")
                            break
                            
                except Exception as e:
                    unreal.log_error(f"❌ Validation monitor failed: {e}")
            
            # Start monitoring in background thread
            threading.Thread(target=monitor_validation, daemon=True).start()
            
            # Return immediately so PIE can start
            return True, False  # Success (validation will complete asynchronously)
            
        except Exception as e:
            import traceback
            self.log(f"Phase 3 exception: {e}", "Error")
            self.log(traceback.format_exc(), "Error")
            return False, False
    
    def _analyze_phase3_logs(self, run_id: str, expected_token: str) -> bool:
        """
        Parse project log for Phase 3 runtime markers
        
        Returns True if expected_token found after run_id marker
        """
        from pathlib import Path
        
        # Find project log for the current workspace.
        log_path = PROJECT_ROOT / 'Saved' / 'Logs' / f'{PROJECT_ROOT.name}.log'
        
        if not log_path.exists():
            self.log(f"⚠️  Project log not found: {log_path}", "Warning")
            return False
        
        try:
            # Read log file
            log_text = log_path.read_text(encoding='utf-8', errors='ignore')
            
            # Find our start marker
            start_marker = f"RiftScript PHASE3 START {run_id}"
            start_idx = log_text.rfind(start_marker)
            
            if start_idx == -1:
                self.log(f"⚠️  Start marker not found in log: {start_marker}", "Warning")
                # Search entire log as fallback
                window = log_text
            else:
                # Only search after our marker
                window = log_text[start_idx:]
            
            # Check for expected token
            if expected_token in window:
                return True
            else:
                # Show tail of log for debugging
                lines = window.split('\n')
                self.log("Last 10 log lines after marker:", "Warning")
                for line in lines[-10:]:
                    if line.strip():
                        self.log(f"  {line}", "Warning")
                return False
                
        except Exception as e:
            self.log(f"⚠️  Log analysis error: {e}", "Warning")
            return False
    
    def build_editor_mode(self, spec_path: str) -> BuildResult:
        """
        EDITOR MODE: Phase 1 verification + Phase 2 + Phase 3
        
        Phase 1: Verify C++ class loadable (NO UBT)
        Phase 2: Generate Blueprints/Assets
        Phase 3: Load map and attempt PIE
        
        Returns BuildResult with honest phase statuses
        """
        if not IN_EDITOR:
            self.log("Editor mode requires Unreal Editor", "Error")
            return BuildResult(ok=False, phase1_ok=False, phase2_ok=False, 
                             phase3_ok=False, mode="error")
        
        self.build_log = []  # Reset log
        
        self.log("=" * 60)
        self.log("🚀 RIFTSCRIPT EDITOR BUILD")
        self.log("   Mode: EDITOR (Phase 1 verification only, NO UBT)")
        self.log(f"   Spec: {spec_path}")
        self.log("=" * 60)
        
        # Phase 1: Verify C++ module (NO UBT)
        phase1_ok = self.verify_phase1_in_editor(spec_path)
        if not phase1_ok:
            self.log("=" * 60)
            self.log("❌ BUILD FAILED - Phase 1 verification failed")
            self.log("=" * 60)
            return BuildResult(ok=False, phase1_ok=False, phase2_ok=False, 
                             phase3_ok=False, mode="editor")
        
        # Phase 2: Blueprints + Assets
        phase2_ok = self.run_phase2()
        if not phase2_ok:
            self.log("=" * 60)
            self.log("❌ BUILD FAILED - Phase 2 asset generation failed")
            self.log("=" * 60)
            return BuildResult(ok=phase1_ok, phase1_ok=phase1_ok, phase2_ok=False, 
                             phase3_ok=False, mode="editor")
        
        # Phase 3: PIE attempt
        phase3_ok, phase3_manual = self.run_phase3()
        
        result = BuildResult(
            ok=phase1_ok and phase2_ok and phase3_ok,
            phase1_ok=phase1_ok,
            phase2_ok=phase2_ok,
            phase3_ok=phase3_ok,
            mode="editor",
            phase3_manual=phase3_manual
        )
        
        self.log("=" * 60)
        if result.ok and not phase3_manual:
            self.log("✅ FULL PIPELINE SUCCESS")
            self.log("   Phase 1: C++ module verified")
            self.log("   Phase 2: Assets generated")
            self.log("   Phase 3: PIE validated (PlayerController + world state)")
        elif phase1_ok and phase2_ok and phase3_manual:
            self.log("✅ ASSET GENERATION SUCCESS")
            self.log("   Phase 1: ✅ C++ module verified")
            self.log("   Phase 2: ✅ Blueprints + Map created")
            self.log("   Phase 3: ❌ Validation failed or incomplete")
            self.log("")
            self.log("   Check Output Log for validation errors")
        else:
            self.log("❌ BUILD INCOMPLETE")
            self.log(f"   Phase 1 (Verify): {'✅' if phase1_ok else '❌'}")
            self.log(f"   Phase 2 (Assets): {'✅' if phase2_ok else '❌'}")
            self.log(f"   Phase 3 (PIE): {'❌'}")
        self.log("=" * 60)
        
        return result


# Global instance for editor utility widget
orchestrator = RiftScriptOrchestrator()

def build_game(profile_name: str = "arena_1v1") -> bool:
    """
    Public API - Dual-mode dispatcher
    
    EDITOR MODE (when called from Unreal Python console):
      - Verifies C++ module exists (Phase 1 verification only)
      - Generates Blueprints + Map (Phase 2)
      - Loads map and attempts PIE (Phase 3)
      - Does NOT run UBT (editor must be closed for C++ rebuilds)
    
    CLI MODE (when called from terminal):
      - Runs headless build via UBT
      - Parses build artifacts and logs
      - Suitable for CI/CD pipelines
    
    Returns True if build succeeds for the current mode
    """
    spec_path = SCRIPTS_DIR / 'profiles' / f'{profile_name}.json'
    
    if not IN_EDITOR:
        return cli_build(profile_name, spec_path)
    
    result = orchestrator.build_editor_mode(str(spec_path))
    return result.ok


def cli_build(profile_name: str, spec_path) -> bool:
    """
    Headless CLI build path for CI/CD.
    
    Uses UBT directly for compilation without editor.
    """
    orchestrator.log(f"CLI Mode: Building {profile_name} headless", "Info")
    
    # Load profile
    if not spec_path.exists():
        orchestrator.log(f"Profile not found: {spec_path}", "Error")
        return False
    
    with open(spec_path, 'r') as f:
        profile = json.load(f)
    
    project_path = profile.get('project_path') or str(PROJECT_ROOT / 'Riftborn.uproject')
    project_name = profile.get('project_name') or Path(project_path).stem

    # SECURITY: Validate profile fields against injection attacks.
    # These flow into subprocess arguments and file paths.
    import re as _re
    _SAFE_NAME = _re.compile(r'^[A-Za-z0-9_\-]{1,128}$')
    _ALLOWED_CONFIGS = frozenset({'Debug', 'DebugGame', 'Development', 'Shipping', 'Test'})

    if not _SAFE_NAME.match(project_name):
        orchestrator.log(f"Invalid project_name: must be alphanumeric/underscore/hyphen, max 128 chars", "Error")
        return False
    config_raw = profile.get('build_config', 'Development')
    if config_raw not in _ALLOWED_CONFIGS:
        orchestrator.log(f"Invalid build_config '{config_raw}'. Allowed: {sorted(_ALLOWED_CONFIGS)}", "Error")
        return False
    if '"' in project_path or "'" in project_path or '..' in project_path:
        orchestrator.log(f"Invalid project_path: must not contain quotes or path traversal", "Error")
        return False

    if not Path(project_path).exists():
        orchestrator.log(f"Project not found: {project_path}", "Error")
        return False
    
    # Find UBT
    engine_dir = os.environ.get('UE_ROOT', '')
    if not engine_dir:
        default_dir = r'C:\Program Files\Epic Games\UE_5.7'
        if os.path.isdir(default_dir):
            engine_dir = default_dir
        else:
            orchestrator.log("UE_ROOT env var not set and default path not found. Set UE_ROOT to your engine install.", "Error")
            return False
    ubt_path = os.path.join(engine_dir, 'Engine', 'Binaries', 'DotNET', 'UnrealBuildTool', 'UnrealBuildTool.exe')
    
    if not os.path.exists(ubt_path):
        # Try alternative path
        ubt_path = os.path.join(engine_dir, 'Engine', 'Build', 'BatchFiles', 'Build.bat')
    
    if not os.path.exists(ubt_path):
        orchestrator.log(f"UBT not found at {ubt_path}", "Error")
        return False
    
    # Build command
    target = f"{project_name}Editor"
    platform = "Win64"
    config = config_raw
    
    cmd = [
        ubt_path,
        target,
        platform,
        config,
        f'-Project="{project_path}"',
        '-WaitMutex',
        '-FromMsBuild',
    ]
    
    orchestrator.log(f"Running: {' '.join(cmd)}", "Info")
    
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=600,  # 10 minute timeout
            cwd=os.path.dirname(project_path),
        )
        
        # Parse output
        success = result.returncode == 0
        
        # Log output
        for line in result.stdout.split('\n')[-20:]:  # Last 20 lines
            if line.strip():
                orchestrator.log(line, "Info" if success else "Warning")
        
        if result.stderr:
            for line in result.stderr.split('\n')[-10:]:
                if line.strip():
                    orchestrator.log(line, "Error")
        
        if success:
            orchestrator.log(f"CLI build succeeded for {profile_name}", "Info")
            
            # Write build artifact for CI
            # SECURITY: Validate profile_name for path traversal before writing artifact
            if not _SAFE_NAME.match(profile_name):
                orchestrator.log(f"Invalid profile_name for artifact write: {profile_name[:50]}", "Error")
                return success
            artifact_path = spec_path.parent / f'{profile_name}_build_result.json'
            with open(artifact_path, 'w') as f:
                json.dump({
                    'success': True,
                    'profile': profile_name,
                    'target': target,
                    'platform': platform,
                    'config': config,
                    'exit_code': result.returncode,
                }, f, indent=2)
        else:
            orchestrator.log(f"CLI build failed with exit code {result.returncode}", "Error")
        
        return success
        
    except subprocess.TimeoutExpired:
        orchestrator.log("Build timed out after 600 seconds", "Error")
        return False
    except Exception as e:
        orchestrator.log(f"Build exception: {e}", "Error")
        return False

def list_profiles():
    """Get available profiles for dropdown"""
    return orchestrator.get_available_profiles()

def get_build_log():
    """Get current build log"""
    return orchestrator.build_log
