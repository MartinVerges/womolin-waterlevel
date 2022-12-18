import { error } from '@sveltejs/kit';

/** @type {import('./$types').RequestHandler} */
export async function DELETE({ request }) {
	let data = await request.json();

	if (!Object.prototype.hasOwnProperty.call(data, 'id') || isNaN(data.id)) {
		throw error(422, JSON.stringify({ message: 'Invalid data' }));
	}
	return new Response(JSON.stringify({ message: 'AP deleted' }), { status: 200 });
}
