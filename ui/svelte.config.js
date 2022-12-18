import adapter from '@sveltejs/adapter-static';

// const build_api_routes =
//	process.env.VITE_BUILD_API === undefined || process.env.VITE_BUILD_API !== false;

/** @type {import('@sveltejs/kit').Config} */
const config = {
	kit: {
		adapter: adapter({
			// default options are shown. On some platforms
			// these options are set automatically — see below
			pages: 'build',
			assets: 'build',
			fallback: null,
			precompress: false,
			strict: false
		})
	}
};

/*
// @type {import('@sveltejs/kit').Config}
const configOLD = {

	kit: {
		adapter: adapter({
			pages: 'build',
			assets: 'build',
			//fallback: 'index.html',
			precompress: false
		}),

		trailingSlash: 'always',

		routes: filepath => {
			if (build_api_routes && filepath.startsWith('api/')) {
				console.log(`[Disabled route] ${filepath}`)
				return true
			} else {
				console.log(`[Enabled route] ${filepath}`)
				return true
			}
		},

		prerender: {
			default: true //false
		},

		vite: {
			build: {
				assetsInlineLimit: 131072,
			},
			optimizeDeps: {
				entries: []
			},
			ssr: {
				noExternal: ['@popperjs/core']
			},
			css: {
				preprocessorOptions: {
					scss: {
						additionalData: '@use "src/variables.scss" as *;'
					}
				}
			}
		}
	},

	preprocess: [
		preprocess({
			scss: {
				prependData: '@use "src/variables.scss" as *;'
			}
		})
	]
};
*/

export default config;
