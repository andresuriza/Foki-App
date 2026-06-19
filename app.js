var http = require('http');
var fs = require('fs');
const axios = require("axios");

// Set this to the IP shown by ESP32 on the Serial Monitor at boot
const ESP32_IP = "192.168.0.74";

var index = fs.readFileSync('index.html');

var SerialPort = require("serialport");
const parsers = SerialPort.parsers;
const parser = new parsers.Readline({
    delimiter: '\r\n'
});

// var port = new SerialPort('/dev/ttyACM0', {
//     baudRate: 9600,
//     dataBits: 8,
//     parity: 'none',
//     stopBits: 1,
//     flowControl: false
// });
// port.pipe(parser);

var app = http.createServer(function(req, res) {
    if (req.url === '/' || req.url === '/index.html') {
        res.writeHead(200, {'Content-Type':'text/html'});
        res.end(index);
    }
    // --- NEW PWA ROUTES ---
    else if (req.url === '/manifest.json') {
        fs.readFile('./manifest.json', function(err, data) {
            if (err) { res.writeHead(404); res.end("Not found"); }
            else { res.writeHead(200, {'Content-Type': 'application/manifest+json'}); res.end(data); }
        });
    }
    else if (req.url === '/sw.js') {
        fs.readFile('./sw.js', function(err, data) {
            if (err) { res.writeHead(404); res.end("Not found"); }
            else { res.writeHead(200, {'Content-Type': 'application/javascript'}); res.end(data); }
        });
    }
    // ----------------------
    else if (req.url.startsWith('/images/')) {
        var filePath = '.' + req.url;
        fs.readFile(filePath, function(err, data) {
            if (err) { res.writeHead(404); res.end("Image not found"); }
            else { res.writeHead(200, {'Content-Type': 'image/png'}); res.end(data); }
        });
    }
    else if (req.url.startsWith('/icons/')) {
       var filePath = '.' + req.url;
       fs.readFile(filePath, function(err, data) {
           if (err) { res.writeHead(404); res.end("Icon not found"); }
           else { res.writeHead(200, {'Content-Type': 'image/png'}); res.end(data); }
       });
    }
    else {
        res.writeHead(404);
        res.end("Not found");
    }
});

var io = require("socket.io")(app);

// ── COMMAND TRANSLATION ───────────────────────────────────────────
function translateMode(mode) {
    switch (mode) {
        case "CALM":    return 0;
        case "SENSORY": return 1;
        case "FOCUS":   return 2;
        default:        return null;
    }
}

// ── ESP32 HEALTH CHECK + ESTADO ──────────────────────────────────
async function getESP32Estado() {
    try {
        const res = await axios.get(`http://${ESP32_IP}/estado`, { timeout: 3000 });
        return { connected: true, ...res.data };
    } catch (error) {
        return { connected: false };
    }
}

// ── SOCKET EVENTS ─────────────────────────────────────────────────
io.on('connection', function(socket) {
    socket.on('lights', async function(data) {
        try {
            if (data.type === 'mode') {
                if (data.status === 'FOCUS_IDLE') {
                    await axios.get(`http://${ESP32_IP}/modo?valor=3`);
                    console.log("Focus tab → parpadeando");
                } else if (data.status === 'FOCUS_START') {
                    await axios.get(`http://${ESP32_IP}/focus?estado=0`);
                    console.log("Focus iniciado");
                } else if (data.status === 'BREAK_START') {
                    await axios.get(`http://${ESP32_IP}/focus?estado=1`);
                    console.log("Break iniciado → vibrando 3s");
                } else if (data.status === 'IDLE') {
                    await axios.get(`http://${ESP32_IP}/focus?estado=2`);
                    console.log("Focus idle");
                } else {
                    const modeValue = translateMode(data.status);
                    if (modeValue !== null) {
                        await axios.get(`http://${ESP32_IP}/modo?valor=${modeValue}`);
                        console.log("Mode updated:", data.status, "→ ESP32 mode", modeValue);
                    } else {
                        console.log("Unknown mode:", data.status);
                    }
                }
            } else if (data.type === 'color') {
                const hex = data.status.replace('#', '');
                const r = parseInt(hex.substring(0, 2), 16);
                const g = parseInt(hex.substring(2, 4), 16);
                const b = parseInt(hex.substring(4, 6), 16);
                await axios.get(`http://${ESP32_IP}/modo?valor=4`);
                await axios.get(`http://${ESP32_IP}/color?r=${r}&g=${g}&b=${b}`);
                console.log("Color custom:", data.status, "→", r, g, b);
            } else if (data.type === 'brightness') {
                const brillo = Math.round(parseInt(data.status) * 255 / 100);
                await axios.get(`http://${ESP32_IP}/modo?valor=4`);
                await axios.get(`http://${ESP32_IP}/brillo?valor=${brillo}`);
                console.log("Brillo custom:", data.status, "% →", brillo);
            } else {
                console.log("Unsupported command (skipped):", data.type, data.status);
            }
        } catch (error) {
            console.error("ESP32 communication failed:", error.message);
        }
    });
});

// ── LAMP STATUS POLLING (every 5 s) ──────────────────────────────
setInterval(async () => {
    const estado = await getESP32Estado();
    console.log("ESP32:", estado.connected ? "ONLINE" : "OFFLINE");
    io.emit("lamp-status", { connected: estado.connected });
    if (estado.connected) {
        io.emit("lamp-state", { modo: estado.modo, nombreModo: estado.nombreModo });
    }
}, 5000);

app.listen(3000, () => {
    console.log('Server is running on http://localhost:3000');
});
