var http = require('http');
var fs = require('fs');

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
    // 1. Serve the main HTML page
    if (req.url === '/' || req.url === '/index.html') {
        res.writeHead(200, {'Content-Type':'text/html'});
        res.end(index);
    } 
    // 2. Intercept and serve image requests
    else if (req.url.startsWith('/images/')) {
        // Construct the file path (e.g., ./images/foki-normal.png)
        var filePath = '.' + req.url; 
        
        fs.readFile(filePath, function(err, data) {
            if (err) {
                res.writeHead(404);
                res.end("Image not found");
            } else {
                // Send the image data with the correct content type
                res.writeHead(200, {'Content-Type': 'image/png'});
                res.end(data);
            }
        });
    } 
   else if (req.url.startsWith('/icons/')) {
       var filePath = '.' + req.url;
       fs.readFile(filePath, function(err, data) {
           if (err) { res.writeHead(404); res.end("Icon not found"); }
           else { res.writeHead(200, {'Content-Type': 'image/png'}); res.end(data); }
       });
   }
    // 3. Handle anything else (404 Not Found)
    else {
        res.writeHead(404);
        res.end("Not found");
    }
});

var io = require("socket.io")(app);

io.on('connection', function(socket){
    socket.on('lights', function(data){
        // port.write(data.status); // Commented out until you plug in the ESP32
        console.log(data);
    });
});

app.listen(3000, () => {
    console.log('Server is running on http://localhost:3000');
});
