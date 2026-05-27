import * as net from 'net';

const ENGINE_HOST = '127.0.0.1';
const ENGINE_PORT = 56789;
const RETRY_INTERVAL_MS = 1000;
const MAX_RETRIES = 100;  // try for 60 seconds

const vscodeInput  = process.stdin;
const vscodeOutput = process.stdout;

let retryCount = 0;

let outputSeq    = 1000000; // high number to avoid collision with the engine sequence numbers

function sendOutputEvent( msg: string )
{
    const body = {
        seq:   outputSeq++,
        type:  'event',
        event: 'output',
        body:  {
            category: 'console',
            output:   msg + '\n'
        }
    };

    const json  = JSON.stringify( body );
    const frame = `Content-Length: ${Buffer.byteLength( json )}\r\n\r\n${json}`;
    vscodeOutput.write( frame );
}

function connect()
{
    const engineSocket = new net.Socket();

    engineSocket.connect( ENGINE_PORT, ENGINE_HOST, () =>
    {
        retryCount = 0;
        sendOutputEvent("[DoodleDebugger] Connected to engine");

        vscodeInput.on( 'data', ( data: Buffer ) =>
        {
            sendOutputEvent( `[VS Code -> Engine]\n${data.toString()}\n` );
            engineSocket.write( data );
        });

        engineSocket.on( 'data', ( data: Buffer ) =>
        {
            sendOutputEvent( `[Engine -> VS Code]\n${data.toString()}\n` );
            vscodeOutput.write( data );
        });

        // vscodeInput.on( 'data', ( data: Buffer ) => { engineSocket.write( data ); } );
        // engineSocket.on( 'data', ( data: Buffer ) => { vscodeOutput.write( data ); } );

        engineSocket.on( 'close', () =>
        {
            sendOutputEvent( '[DoodleDebugger] Engine disconnected, waiting for reconnect...' );
            vscodeInput.removeAllListeners( 'data' );
            engineSocket.destroy();
            setTimeout( () => connect(), RETRY_INTERVAL_MS );
        });
    });

    engineSocket.on( 'error', ( err: Error ) =>
    {
        vscodeInput.removeAllListeners( 'data' );
        engineSocket.destroy();
        if ( retryCount < MAX_RETRIES )
        {
            retryCount++;
            sendOutputEvent( `[DoodleDebugger] Engine not found, retrying... (${retryCount}/${MAX_RETRIES})` );
            setTimeout( () => connect(), RETRY_INTERVAL_MS );
        }
        else
        {
            sendOutputEvent( '[DoodleDebugger] Could not connect to engine after max retries' );
            process.exit( 1 );
        }
    });

    process.on( 'exit', () => { engineSocket.destroy(); } );
}

connect();