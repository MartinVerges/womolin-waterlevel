/** @type {import('./$types').RequestHandler} */
export function GET() {
	let responseBody = [
		{
			id: 0,
			level: Math.floor(Math.random() * 101),
			volume: Math.floor(Math.random() * 101) * 1000,
			sensorPressure: Math.floor(Math.random() * 101),
			getAirPressure: Math.floor(Math.random() * 101)
		},
		{
			id: 1,
			level: Math.floor(Math.random() * 101),
			volume: Math.floor(Math.random() * 101) * 1000,
			sensorPressure: Math.floor(Math.random() * 101),
			getAirPressure: Math.floor(Math.random() * 101)
		}
	];
	return new Response(JSON.stringify(responseBody), { status: 200 });
}
