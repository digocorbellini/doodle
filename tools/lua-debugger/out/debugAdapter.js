"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const net = require("net");
const vscode_debugadapter_1 = require("vscode-debugadapter");
const ENGINE_HOST = '127.0.0.1';
const ENGINE_PORT = 56789;
const RETRY_INTERVAL_MS = 1000;
const MAX_RETRIES = 100; // try for 60 seconds
const vscodeInput = process.stdin;
const vscodeOutput = process.stdout;
let retryCount = 0;
class MyDebugSession extends vscode_debugadapter_1.LoggingDebugSession {
    constructor() {
        super();
    }
    initializeRequest(response, args) {
        super.initializeRequest(response, args);
        this.sendEvent(new vscode_debugadapter_1.InitializedEvent());
        this.connect();
    }
    connect() {
        const engineSocket = new net.Socket();
        engineSocket.connect(ENGINE_PORT, ENGINE_HOST, () => {
            retryCount = 0;
            this.sendEvent(new vscode_debugadapter_1.OutputEvent("[DoodleDebugger] Connected to engine\n"));
            vscodeInput.on('data', (data) => { engineSocket.write(data); });
            engineSocket.on('data', (data) => { vscodeOutput.write(data); });
            engineSocket.on('close', () => {
                this.sendEvent(new vscode_debugadapter_1.OutputEvent('[DoodleDebugger] Engine disconnected, waiting for reconnect...\n'));
                engineSocket.destroy();
            });
        });
        engineSocket.on('error', (err) => {
            vscodeInput.removeAllListeners('data');
            engineSocket.destroy();
            if (retryCount < MAX_RETRIES) {
                retryCount++;
                this.sendEvent(new vscode_debugadapter_1.OutputEvent(`[DoodleDebugger] Engine not found, retrying... (${retryCount}/${MAX_RETRIES})\n`));
                setTimeout(() => this.connect(), RETRY_INTERVAL_MS);
            }
            else {
                this.sendEvent(new vscode_debugadapter_1.OutputEvent('[DoodleDebugger] Could not connect to engine after max retries\n'));
                this.shutdown();
            }
        });
    }
}
MyDebugSession.run(MyDebugSession);
//# sourceMappingURL=debugAdapter.js.map