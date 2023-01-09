import { error } from '@sveltejs/kit';

/** @type {import('./$types').RequestHandler} */
export async function POST({ request }) {
	let data = await request.json();

	if (!Object.prototype.hasOwnProperty.call(data, 'otaPassword')) {
		throw error(422, JSON.stringify({ message: 'Invalid data' }));
	}

	let responseBody = { message: 'Please wait while the device reboots!' };
	return new Response(JSON.stringify(responseBody), { status: 200 });
}
