/** @type {import('./$types').RequestHandler} */
export function POST() {
	let responseBody = { message: 'Setup completed' };
	return new Response(JSON.stringify(responseBody), { status: 200 });
}
