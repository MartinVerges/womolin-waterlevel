/** @type {import('./$types').RequestHandler} */
export function GET() {
	let responseBody = {
		booting: {
			rebootReason: 1,
			partitionCount: 2
		},
		bootPartition: {
			address: 65536,
			size: 1572864,
			label: 'app0',
			encrypted: false,
			type: 'app',
			subtype: 16
		},
		runningPartition: {
			address: 65536,
			size: 1572864,
			label: 'app0',
			encrypted: false,
			type: 'app',
			subtype: 16
		},
		build: {
			date: 'Dec 18 2022',
			time: '19:24:52'
		},
		ram: {
			heapSize: 224828,
			freeHeap: 107732,
			usagePercent: 47.92007828,
			minFreeHeap: 49528,
			maxAllocHeap: 65524
		},
		spi: {
			psramSize: 0,
			freePsram: 0,
			minFreePsram: 0,
			maxAllocPsram: 0
		},
		chip: {
			revision: 1,
			model: 'ESP32-D0WDQ6',
			cores: 2,
			cpuFreqMHz: 240,
			cycleCount: 862269402,
			sdkVersion: 'v4.4-dev-3569-g6a7d83af19-dirty',
			efuseMac: 242849356696008,
			temperature: 68.88888889
		},
		flash: {
			flashChipSize: 4194304,
			flashChipRealSize: 4194304,
			flashChipSpeedMHz: 80,
			flashChipMode: 2,
			sdkVersion: 4194304
		},
		sketch: {
			size: 1479392,
			maxSize: 1572864,
			usagePercent: 94.0572052,
			md5: '74d963a9e1ae6a9963d0b8252c1ac2e4'
		},
		filesystem: {
			type: 'LittleFS',
			totalBytes: 786432,
			usedBytes: 659456,
			usagePercent: 83.85417175
		}
	};
	return new Response(JSON.stringify(responseBody), { status: 200 });
}
