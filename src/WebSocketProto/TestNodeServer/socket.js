const http = require('http');
const https = require('https');
const fs = require('fs');
const WebSocket = require('ws');
const TestMessage = require('./testmessage_pb')

const options = {
	key: fs.readFileSync('key.pem'),
	cert: fs.readFileSync('cert.pem')
};
const server = https.createServer(options);
const wss = new WebSocket.Server({ server });
wss.on('message', function incoming(data) {
	console.log(data);
  });
wss.on('connection', function connection(ws) {
    ws.on('message', function incoming(message) {
		console.log('received: %s', message);
		const bytes = Array.prototype.slice.call(message, 0);
		const messageObj = proto.TestMessage.deserializeBinary(bytes);
		console.log(messageObj.getSometext());
    	ws.send(bytes);
    });
    //ws.send('something');
});

const port = 51935;
server.listen(
	port,
	'127.0.0.1', 
	() => {
		const host = server.address().address;
		const port = server.address().port;
		console.log("Server listening at https://%s:%s", host, port);
	});

/*
const wsServer = new WebSocketServer({
	httpServer: server,
	autoAcceptConnections: true
});

function originIsAllowed(origin) {
	return true;
}

wsServer.on('request', function(request) {
	if (!originIsAllowed(request.origin)) {
		request.reject();
		console.log((new Date()) + ' Connection from origin ' + request.origin + ' rejected.');
		return;
	}
	
	const connection = request.accept('echo-protocol', request.origin);
	console.log((new Date()) + ' Connection accepted.');
	connection.on('message', function(message) {
		if (message.type === 'utf8') {
			console.log('Received Message: ' + message.utf8Data);
			connection.sendUTF(message.utf8Data);
		}
		else if (message.type === 'binary') {
			console.log('Received Binary Message of ' + message.binaryData.length + ' bytes');
			connection.sendBytes(message.binaryData);
		}
	});
	connection.on('close', function(reasonCode, description) {
		console.log((new Date()) + ' Peer ' + connection.remoteAddress + ' disconnected.');
	});
});
*/