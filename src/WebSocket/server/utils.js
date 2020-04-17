const nutil = require('util');

function logRequest(req) {
    const dateStr = new Date().toLocaleString();
    console.log("======= Headers =======");
	console.log(dateStr + ":" +	req.originalUrl);
    console.log("======= Headers =======");
    console.log(req.headers);
    console.log("=======================");
    console.log("======== Body =========");
    console.log(nutil.inspect(req.body, false, null, true));
    console.log("=======================");
}

module.exports = {
    logRequest: logRequest
};
