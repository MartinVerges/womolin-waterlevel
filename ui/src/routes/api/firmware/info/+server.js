/** @type {import('./$types').RequestHandler} */
export function GET() {
	let responseBody = {
		address: 65536,
		encrypted: false,
		firmware_date: '2023-01-04',
		firmware_version: 'v2.1-1-g76566b3',
		label: 'app0',
		partition_subtype: 16,
		partition_type: 0,
		size: 1572864
	};
	return new Response(JSON.stringify(responseBody), { status: 200 });
}
