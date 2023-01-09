import { error } from '@sveltejs/kit';

/** @type {import('./$types').RequestHandler} */
export function GET() {
	let responseBody = {
		enableBle: true,
		enableDac: false,
		enableMqtt: true,
		enableSoftAp: true,
		enableWifi: true,
		autoAirPump: true,
		otaPassword: 'abcd1234567890',
		otaWebUrl: 'http://s3.womolin.de/webinstaller/test/',
		otaWebEnabled: true,
		hostname: 'freshwater',
		mqttHost: '192.168.255.1',
		mqttPass: 'abcd1234',
		mqttPort: 1883,
		mqttTopic: 'freshwater',
		mqttUser: 'freshwater'
	};
	return new Response(JSON.stringify(responseBody), { status: 200 });
}

/** @type {import('./$types').RequestHandler} */
export async function POST({ request }) {
	let data = await request.json();

	if (
		!Object.prototype.hasOwnProperty.call(data, 'enableBle') ||
		!Object.prototype.hasOwnProperty.call(data, 'enableDac') ||
		!Object.prototype.hasOwnProperty.call(data, 'enableMqtt') ||
		!Object.prototype.hasOwnProperty.call(data, 'enableSoftAp') ||
		!Object.prototype.hasOwnProperty.call(data, 'enableWifi') ||
		!Object.prototype.hasOwnProperty.call(data, 'autoAirPump') ||
		!Object.prototype.hasOwnProperty.call(data, 'otaPassword') ||
		!Object.prototype.hasOwnProperty.call(data, 'otaWebUrl') ||
		!Object.prototype.hasOwnProperty.call(data, 'otaWebEnabled') ||
		!Object.prototype.hasOwnProperty.call(data, 'hostname') ||
		!Object.prototype.hasOwnProperty.call(data, 'mqttHost') ||
		!Object.prototype.hasOwnProperty.call(data, 'mqttPass') ||
		!Object.prototype.hasOwnProperty.call(data, 'mqttTopic') ||
		!Object.prototype.hasOwnProperty.call(data, 'mqttUser')
	) {
		throw error(422, JSON.stringify({ message: 'Invalid data' }));
	}

	let responseBody = { message: 'New hostname stored in NVS, reboot required!' };
	return new Response(JSON.stringify(responseBody), { status: 200 });
}
