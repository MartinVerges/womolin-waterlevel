import preprocess from 'svelte-preprocess';
import adapter from '@sveltejs/adapter-static';


/** @type {import('@sveltejs/kit').Config} */
const config = {

	kit: {
		adapter: adapter({
			pages: 'build',
			assets: 'build',
			fallback: 'index.html',
			precompress: false
		}),

		routes: filepath => {
			if (!filepath.startsWith('api/')) {
				console.log(`[Enable route] ${filepath}`)
				return true
			} else {
				console.log(`[Disabled route] ${filepath}`)
				return true
			}
		},

		prerender: {
			default: false
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

export default config;
