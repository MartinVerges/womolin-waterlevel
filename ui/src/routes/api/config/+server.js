import { error } from '@sveltejs/kit';

/** @type {import('./$types').RequestHandler} */
export function GET() {
	let responseBody = {
		enableble: true,
		enabledac: false,
		enablemqtt: true,
		enablesoftap: true,
		enablewifi: true,
		autoairpump: true,
		otapassword: 'abcd1234567890',
		hostname: 'freshwater',
		mqtthost: '192.168.255.1',
		mqttpass: 'abcd1234',
		mqttport: 1883,
		mqtttopic: 'freshwater',
		mqttuser: 'freshwater'
	};
	return new Response(JSON.stringify(responseBody), { status: 200 });
}

/** @type {import('./$types').RequestHandler} */
export async function POST({ request }) {
	let data = await request.json();

	if (
		!Object.prototype.hasOwnProperty.call(data, 'enableble') ||
		!Object.prototype.hasOwnProperty.call(data, 'enabledac') ||
		!Object.prototype.hasOwnProperty.call(data, 'enablemqtt') ||
		!Object.prototype.hasOwnProperty.call(data, 'enablesoftap') ||
		!Object.prototype.hasOwnProperty.call(data, 'enablewifi') ||
		!Object.prototype.hasOwnProperty.call(data, 'autoairpump') ||
		!Object.prototype.hasOwnProperty.call(data, 'otapassword') ||
		!Object.prototype.hasOwnProperty.call(data, 'hostname') ||
		!Object.prototype.hasOwnProperty.call(data, 'mqtthost') ||
		!Object.prototype.hasOwnProperty.call(data, 'mqttpass') ||
		!Object.prototype.hasOwnProperty.call(data, 'mqtttopic') ||
		!Object.prototype.hasOwnProperty.call(data, 'mqttuser')
	) {
		throw error(422, JSON.stringify({ message: 'Invalid data' }));
	}

	let responseBody = { message: 'New hostname stored in NVS, reboot required!' };
	return new Response(JSON.stringify(responseBody), { status: 200 });
}
