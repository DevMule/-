const ws = require('ws');
const fs = require('fs');
const path = require('path');

const wss = new ws.Server({port: 8002});
console.log('lab2.2 launched');
const db_file = path.resolve(__dirname, 'db.txt');

async function readList() {
	return (await fs.readFileSync(db_file)).toString();
}

async function addToList(message) {
	let text = (await readList()).trim();
	if (text.length > 0) text += ';';
	text += message.replace(';', '');
	await fs.writeFileSync(db_file, text);
}

wss.on('connection', (user) => {
	
	console.log('connected');
	user.onclose = () => console.log('disconnected');
	user.onmessage = async (msg) => {
		try {
			
			let trimmed = msg.data.trim()
			console.log('message:', trimmed)
			
			if (trimmed.toUpperCase() === 'LIST') {
				user.send(await readList());
				
			} else if (trimmed.length > 0) {
				await addToList(trimmed);
				user.send(`Message added: \"${trimmed}\".`);
				
			} else {
				user.close();
			}
			
		} catch (e) {
			console.log(e);
			user.close();
			
		}
	};
	
});
