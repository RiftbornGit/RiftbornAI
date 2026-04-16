export interface RiftbornResponse {
    ok: boolean;
    result?: any;
    error?: string;
    [key: string]: any;
}
export type ToolHandler = (args: Record<string, any>) => Promise<RiftbornResponse>;
//# sourceMappingURL=riftborn-types.d.ts.map