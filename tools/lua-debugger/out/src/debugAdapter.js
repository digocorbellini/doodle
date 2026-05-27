"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    var desc = Object.getOwnPropertyDescriptor(m, k);
    if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
      desc = { enumerable: true, get: function() { return m[k]; } };
    }
    Object.defineProperty(o, k2, desc);
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || (function () {
    var ownKeys = function(o) {
        ownKeys = Object.getOwnPropertyNames || function (o) {
            var ar = [];
            for (var k in o) if (Object.prototype.hasOwnProperty.call(o, k)) ar[ar.length] = k;
            return ar;
        };
        return ownKeys(o);
    };
    return function (mod) {
        if (mod && mod.__esModule) return mod;
        var result = {};
        if (mod != null) for (var k = ownKeys(mod), i = 0; i < k.length; i++) if (k[i] !== "default") __createBinding(result, mod, k[i]);
        __setModuleDefault(result, mod);
        return result;
    };
})();
Object.defineProperty(exports, "__esModule", { value: true });
const net = __importStar(require("net"));
const ENGINE_HOST = '127.0.0.1';
const ENGINE_PORT = 56789;
const vscodeInput = process.stdin;
const vscodeOutput = process.stdout;
const engineSocket = new net.Socket();
engineSocket.connect(ENGINE_PORT, ENGINE_HOST, () => {
    process.stderr.write('[DoodleDebugger] Connected to engine\n');
});
vscodeInput.on('data', (data) => { engineSocket.write(data); });
engineSocket.on('data', (data) => { vscodeOutput.write(data); });
engineSocket.on('close', () => { process.exit(0); });
engineSocket.on('error', (err) => {
    process.stderr.write(`[DoodleDebugger] ${err.message}\n`);
    process.exit(1);
});
process.on('exit', () => { engineSocket.destroy(); });
//# sourceMappingURL=debugAdapter.js.map