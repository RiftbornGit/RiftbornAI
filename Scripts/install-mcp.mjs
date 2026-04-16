#!/usr/bin/env node
/**
 * RiftbornAI MCP Server — Global Installer
 * 
 * Registers the RiftbornAI MCP server in user-global configs for:
 *   - Claude Desktop
 *   - VS Code (user profile)
 *   - Cursor (global)
 *   - Windsurf (global)
 * 
 * Usage:
 *   node Scripts/install-mcp.mjs                   # install to all platforms
 *   node Scripts/install-mcp.mjs --uninstall        # remove from all platforms
 *   node Scripts/install-mcp.mjs --platform cursor   # only target cursor
 */

import { readFileSync, writeFileSync, mkdirSync, existsSync } from "fs";
import { spawnSync } from "child_process";
import { join, resolve, dirname } from "path";
import { homedir, platform } from "os";
import { fileURLToPath } from "url";

const __filename = fileURLToPath(import.meta.url);
const PLUGIN_ROOT = resolve(dirname(__filename), "..");
const SERVER_ENTRY = join(PLUGIN_ROOT, "mcp-server", "dist", "index.js");
const IS_WIN = platform() === "win32";

// ---- Config locations per platform ----
function getConfigPaths() {
  const home = homedir();
  const appdata = process.env.APPDATA || join(home, "AppData", "Roaming");

  return {
    "claude-desktop": IS_WIN
      ? join(appdata, "Claude", "claude_desktop_config.json")
      : join(home, "Library", "Application Support", "Claude", "claude_desktop_config.json"),
    "vscode": IS_WIN
      ? join(appdata, "Code", "User", "mcp.json")
      : join(home, ".config", "Code", "User", "mcp.json"),
    "vscode-insiders": IS_WIN
      ? join(appdata, "Code - Insiders", "User", "mcp.json")
      : join(home, ".config", "Code - Insiders", "User", "mcp.json"),
    "cursor": IS_WIN
      ? join(home, ".cursor", "mcp.json")
      : join(home, ".cursor", "mcp.json"),
    "windsurf": IS_WIN
      ? join(home, ".codeium", "windsurf", "mcp_config.json")
      : join(home, ".codeium", "windsurf", "mcp_config.json"),
  };
}

// ---- Server entry definition ----
function serverEntry(rootKey) {
  // Use forward slashes in JSON even on Windows
  const entryPath = SERVER_ENTRY.replace(/\\/g, "/");
  const pluginPath = PLUGIN_ROOT.replace(/\\/g, "/");

  const base = {
    command: "node",
    args: [entryPath],
    env: { RIFTBORN_PLUGIN_PATH: pluginPath },
  };

  // VS Code user config uses "servers" root key with "type" field
  if (rootKey === "servers") {
    return { type: "stdio", ...base };
  }
  return base;
}

function validateServerRuntime() {
  if (!existsSync(SERVER_ENTRY)) {
    console.error(`Error: MCP server not built. Expected: ${SERVER_ENTRY}`);
    process.exit(1);
  }

  const env = {
    ...process.env,
    RIFTBORN_AUTH_TOKEN: "",
    RIFTBORN_API_KEY: "",
    RIFTBORN_DEV_TOKEN: "",
  };
  const check = spawnSync(process.execPath, [SERVER_ENTRY, "--self-test", "--json"], {
    cwd: PLUGIN_ROOT,
    env,
    encoding: "utf-8",
  });

  if (check.status !== 0) {
    console.error("Error: MCP server runtime self-test failed.");
    if (check.stdout?.trim()) {
      console.error(check.stdout.trim());
    }
    if (check.stderr?.trim()) {
      console.error(check.stderr.trim());
    }
    console.error("Fix the shipped mcp-server runtime before registering it with MCP clients.");
    process.exit(check.status || 1);
  }
}

// ---- Read / write JSON safely ----
function readJson(path) {
  if (!existsSync(path)) return null;
  try {
    return JSON.parse(readFileSync(path, "utf-8"));
  } catch {
    return null;
  }
}

function writeJson(path, data) {
  mkdirSync(dirname(path), { recursive: true });
  writeFileSync(path, JSON.stringify(data, null, 2) + "\n", "utf-8");
}

// ---- Install / uninstall logic ----
function install(configPath, rootKey) {
  const existing = readJson(configPath) || {};
  if (!existing[rootKey]) existing[rootKey] = {};
  existing[rootKey]["riftborn"] = serverEntry(rootKey);
  writeJson(configPath, existing);
}

function uninstall(configPath, rootKey) {
  const existing = readJson(configPath);
  if (!existing || !existing[rootKey] || !existing[rootKey]["riftborn"]) return false;
  delete existing[rootKey]["riftborn"];
  writeJson(configPath, existing);
  return true;
}

// ---- Platform → root key mapping ----
function rootKeyFor(name) {
  // VS Code uses "servers", everything else uses "mcpServers"
  return name.startsWith("vscode") ? "servers" : "mcpServers";
}

// ---- CLI ----
const args = process.argv.slice(2);
const doUninstall = args.includes("--uninstall");
const platformFilter = args.find((a, i) => args[i - 1] === "--platform");

// Pre-flight: check the shipped server is actually runnable
if (!doUninstall) {
  validateServerRuntime();
}

const configs = getConfigPaths();
const targets = platformFilter
  ? Object.entries(configs).filter(([name]) => name === platformFilter)
  : Object.entries(configs);

if (targets.length === 0) {
  console.error(`Unknown platform: ${platformFilter}`);
  console.error(`Available: ${Object.keys(configs).join(", ")}`);
  process.exit(1);
}

console.log(doUninstall ? "Uninstalling RiftbornAI MCP server..." : "Installing RiftbornAI MCP server...");
console.log(`  Server: ${SERVER_ENTRY}\n`);

for (const [name, configPath] of targets) {
  const rootKey = rootKeyFor(name);
  try {
    if (doUninstall) {
      const removed = uninstall(configPath, rootKey);
      console.log(`  ${name}: ${removed ? "removed" : "not present"} (${configPath})`);
    } else {
      install(configPath, rootKey);
      console.log(`  ${name}: installed (${configPath})`);
    }
  } catch (err) {
    console.error(`  ${name}: FAILED — ${err.message}`);
  }
}

console.log("\nDone. Restart your editor/app to pick up the changes.");
