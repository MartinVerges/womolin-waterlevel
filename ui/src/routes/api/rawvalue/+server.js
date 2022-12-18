/** @type {import('./$types').RequestHandler} */
export function GET() {
	let responseBody = { raw: Math.floor(Math.random() * 10001) };
	return new Response(JSON.stringify(responseBody), { status: 200 });
}
