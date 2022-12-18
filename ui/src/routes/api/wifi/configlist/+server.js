/** @type {import('./$types').RequestHandler} */
export function GET() {
	let responseBody = [
		{
			id: 1,
			apName: 'Stored WiFi #1',
			apPass: true
		},
		{
			id: 2,
			apName: 'Stored WiFi #2',
			apPass: false
		}
	];
	return new Response(JSON.stringify(responseBody), { status: 200 });
}
