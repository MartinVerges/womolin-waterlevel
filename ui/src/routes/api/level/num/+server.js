/** @type {import('./$types').RequestHandler} */
export function GET() {
	let responseBody = { num: Math.round(Math.random()) + 1 };
	return new Response(JSON.stringify(responseBody), { status: 200 });
}
