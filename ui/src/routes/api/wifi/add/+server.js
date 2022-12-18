import { error } from '@sveltejs/kit';

/** @type {import('./$types').RequestHandler} */
export async function POST({ request }) {
	let data = await request.json();

	if (!('apName' in data && 'apPass' in data && String(data.apName).length > 0)) {
		throw error(422, JSON.stringify({ message: 'Invalid data' }));
	}
	return new Response(JSON.stringify({ message: 'New AP added' }), { status: 200 });
}
