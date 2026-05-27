import * as net from 'net';
import {
    DebugProtocol
} from 'vscode-debugprotocol';
import {
    InitializedEvent,
    LoggingDebugSession,
    OutputEvent
} from 'vscode-debugadapter';

const ENGINE_HOST = '127.0.0.1';
const ENGINE_PORT = 56789;
const RETRY_INTERVAL_MS = 1000;
const MAX_RETRIES = 100;  // try for 60 seconds

const vscodeInput  = process.stdin;
const vscodeOutput = process.stdout;

let retryCount = 0;

class MyDebugSession extends LoggingDebugSession {
    constructor() {
        super();
    }

    protected initializeRequest(response: DebugProtocol.InitializeResponse, args: DebugProtocol.InitializeRequestArguments): void {
        super.initializeRequest(response, args);
        this.sendEvent(new InitializedEvent());
        this.connect();
    }

    connect()
    {
        const engineSocket = new net.Socket();

        engineSocket.connect( ENGINE_PORT, ENGINE_HOST, () =>
        {
            retryCount = 0;
            this.sendEvent(new OutputEvent("[DoodleDebugger] Connected to engine\n"));

            vscodeInput.on( 'data', ( data: Buffer ) => { engineSocket.write( data ); } );
            engineSocket.on( 'data', ( data: Buffer ) => { vscodeOutput.write( data ); } );

            engineSocket.on( 'close', () =>
            {
                this.sendEvent(new OutputEvent( '[DoodleDebugger] Engine disconnected, waiting for reconnect...\n' ));
                engineSocket.destroy();
            });
        });

        engineSocket.on( 'error', ( err: Error ) =>
        {
            vscodeInput.removeAllListeners( 'data' );
            engineSocket.destroy();
            if ( retryCount < MAX_RETRIES )
            {
                retryCount++;
                this.sendEvent(new OutputEvent(`[DoodleDebugger] Engine not found, retrying... (${retryCount}/${MAX_RETRIES})\n`));
                setTimeout( () => this.connect(), RETRY_INTERVAL_MS );
            }
            else
            {
                this.sendEvent(new OutputEvent( '[DoodleDebugger] Could not connect to engine after max retries\n' ));
                this.shutdown();
            }
        });
    }
}

MyDebugSession.run(MyDebugSession);