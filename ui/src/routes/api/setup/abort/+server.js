/** @type {import('./$types').RequestHandler} */
export function POST() {
	let responseBody = { message: 'calibration aborted' };
	return new Response(JSON.stringify(responseBody), { status: 200 });
}
