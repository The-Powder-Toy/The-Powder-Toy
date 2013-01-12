/*
Format: 
	Receives:
	Action
		- CheckVersion (returns changelog) SAFE VALUES FOR BOTH: Unknown
		- Download (returns update)
	Architecture
		- Contains the build architecture
	InstructionSet 
		- Contains the instruction set. :O
	Sends: 
	sample changelog:
	4 20 3  Buildversion MajorVersion MinorVersion
	List
	of
	changes
	with \n
	in
	between
*/ 
var headers = {
	"Content-Type": "text/html",
	"X-Powered-By": "lolcats",

};
				// Build, major version, minor version
var send = 20 + " " + 84 + " " + 3;
var http = require("http");
http.createServer(function(request, response) {
	response.writeHead(200, headers);
	response.end(JSON.stringify(send));


}).listen(80, "localhost");