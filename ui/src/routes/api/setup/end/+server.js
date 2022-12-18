/** @type {import('./$types').RequestHandler} */
export function POST() {
	let responseBody = { message: 'calibration successful' };
	return new Response(JSON.stringify(responseBody), { status: 200 });
}
