/** @type {import('./$types').RequestHandler} */
export function POST() {
	let responseBody = { message: 'Start calibration' };
	return new Response(JSON.stringify(responseBody), { status: 200 });
}
