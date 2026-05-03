import type { SceneChange } from "./scene-safety.js";
export type DomainKey = "blueprint" | "leveldesign" | "vfx" | "gas" | "ui" | "networking" | "saveload" | "performance";
export interface DomainDefinitionOfDoneEntry {
    domain: string;
    main_failure_mode: string;
    required_proof: string[];
    minimum_done_bar: string[];
    not_done_if: string[];
}
export interface DomainProofGap {
    domain: DomainKey;
    label: string;
    severity: "error" | "warning";
    missing_tools: string[];
    guidance: string;
}
export declare const DOMAIN_DEFINITION_OF_DONE: Record<DomainKey, DomainDefinitionOfDoneEntry>;
export declare function normalizeDomainKey(domain: string): string;
export declare function getDomainDefinitionOfDoneContract(domain?: string): {
    source_doc: string;
    domains: DomainDefinitionOfDoneEntry[];
} | DomainDefinitionOfDoneEntry | null;
export declare function inferDomainsFromTask(task: string): DomainKey[];
export declare function inferDomainsFromHistory(sceneChanges: SceneChange[], recentTools?: string[]): DomainKey[];
export declare function findDomainProofGaps(domains: DomainKey[], recentTools?: string[]): DomainProofGap[];
export declare function buildDomainContractResourceUris(task: string): string[];