/** @type {import('./$types').RequestHandler} */
export function GET() {
	let responseBody = [
		{
			ssid: 'Test WiFi #1',
			encryptionType: Math.round(Math.random()),
			rssi: -31,
			channel: Math.floor(Math.random() * 10)
		},
		{
			ssid: 'Test WiFi #2',
			encryptionType: Math.round(Math.random()),
			rssi: -42,
			channel: Math.floor(Math.random() * 10)
		},
		{
			ssid: 'Test WiFi #3',
			encryptionType: Math.round(Math.random()),
			rssi: -53,
			channel: Math.floor(Math.random() * 10)
		},
		{
			ssid: 'Test WiFi #4',
			encryptionType: Math.round(Math.random()),
			rssi: -64,
			channel: Math.floor(Math.random() * 10)
		},
		{
			ssid: 'Test WiFi #5',
			encryptionType: Math.round(Math.random()),
			rssi: -69,
			channel: Math.floor(Math.random() * 10)
		},
		{
			ssid: 'Test WiFi #6',
			encryptionType: Math.round(Math.random()),
			rssi: -75,
			channel: Math.floor(Math.random() * 10)
		},
		{
			ssid: 'Test WiFi #7',
			encryptionType: Math.round(Math.random()),
			rssi: -97,
			channel: Math.floor(Math.random() * 10)
		}
	];
	return new Response(JSON.stringify(responseBody), { status: 200 });
}
