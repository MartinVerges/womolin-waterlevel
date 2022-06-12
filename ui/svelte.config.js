import preprocess from 'svelte-preprocess';
import adapter from '@sveltejs/adapter-static';

/** @type {import('@sveltejs/kit').Config} */
const config = {

	kit: {
		adapter: adapter({
			fallback: '200.html'
		}),

		vite: {
			optimizeDeps: {
				entries: []
			},
			ssr: {
				noExternal: ['@popperjs/core', "@fortawesome/pro-solid-svg-icons"]
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
